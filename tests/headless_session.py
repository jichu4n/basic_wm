import os
import subprocess
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Optional, Tuple

from Xlib import X, Xatom, display, error
from Xlib.protocol import event as xevent


@dataclass
class WindowGeometry:
    x: int
    y: int
    width: int
    height: int

    def as_dict(self) -> Dict[str, int]:
        return {
            "x": int(self.x),
            "y": int(self.y),
            "width": int(self.width),
            "height": int(self.height),
        }


class BasicWMTestSession:
    """Helper for launching basic_wm inside a headless Xvfb session."""

    def __init__(self, screen_size: Tuple[int, int] = (800, 600)) -> None:
        self._screen_size = screen_size
        self.display_name: Optional[str] = None
        self._xvfb: Optional[subprocess.Popen] = None
        self._wm: Optional[subprocess.Popen] = None
        self._display: Optional[display.Display] = None
        self._clients = []
        self._started = False
        self._env: Optional[Dict[str, str]] = None
        self._test_atom = None

    # -- public API ---------------------------------------------------------
    def __enter__(self) -> "BasicWMTestSession":
        return self.start()

    def __exit__(self, exc_type, exc, tb) -> None:
        self.shutdown()

    def run_headless_check(self) -> Dict[str, Dict[str, int]]:
        with self:
            client = self.launch_xterm("wm-headless-check")
            client_geometry = self.window_geometry(client.window)
            frame_geometry = self.window_geometry(client.frame)
            self.close_client(client)
            return {
                "display": self.display_name or "",
                "client": client_geometry.as_dict(),
                "frame": frame_geometry.as_dict(),
            }

    def start(self) -> "BasicWMTestSession":
        if self._started:
            return self

        self._build_binary()
        display_number = self._allocate_display()
        self.display_name = f":{display_number}"
        screen = f"{self._screen_size[0]}x{self._screen_size[1]}x24"
        self._xvfb = subprocess.Popen(
            [
                "Xvfb",
                self.display_name,
                "-screen",
                "0",
                screen,
                "-nolisten",
                "tcp",
            ],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        self._wait_for_display()
        env = os.environ.copy()
        env["DISPLAY"] = self.display_name
        env.setdefault("GLOG_logtostderr", "1")
        self._env = env
        self._display = display.Display(self.display_name)
        self._test_atom = self._display.intern_atom("_BASIC_WM_TEST_DATA", False)
        self._wm = subprocess.Popen(
            ["./basic_wm"],
            env=self._env,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        self._wait_for_wm()
        self._started = True
        return self

    def shutdown(self) -> None:
        for client in list(self._clients):
            self.close_client(client)
        if self._wm is not None:
            self._terminate_process(self._wm)
            self._wm = None
        if self._display is not None:
            try:
                self._display.close()
            finally:
                self._display = None
        if self._xvfb is not None:
            self._terminate_process(self._xvfb)
            self._xvfb = None
        self._env = None
        self._test_atom = None
        self._started = False

    @dataclass
    class Client:
        window: object
        frame: object
        process: subprocess.Popen

    def launch_xterm(self, name: str, geometry: str = "80x24+20+20") -> "BasicWMTestSession.Client":
        if not self._started or self._env is None:
            raise RuntimeError("Session not started")
        env = self._env.copy()
        proc = subprocess.Popen(
            [
                "xterm",
                "-name",
                name,
                "-title",
                name,
                "+ls",
                "-geometry",
                geometry,
            ],
            env=env,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        window = self._wait_for_window(name)
        frame = self._wait_for_frame(window)
        client = BasicWMTestSession.Client(window=window, frame=frame, process=proc)
        self._clients.append(client)
        return client

    def close_client(self, client: "BasicWMTestSession.Client") -> None:
        if client in self._clients:
            self._clients.remove(client)
        self._terminate_process(client.process)

    def window_geometry(self, window) -> WindowGeometry:
        if self._display is None:
            raise RuntimeError("Display not ready")
        self._display.sync()
        geom = window.get_geometry()
        try:
            coords = window.translate_coords(self._display.screen().root, 0, 0).reply()
            x = getattr(coords, "dst_x", None)
            if x is None:
                x = getattr(coords, "x", None)
            y = getattr(coords, "dst_y", None)
            if y is None:
                y = getattr(coords, "y", None)
            if x is None:
                x = geom.x
            if y is None:
                y = geom.y
        except (error.BadWindow, error.BadDrawable):
            x = geom.x
            y = geom.y
        return WindowGeometry(x=x, y=y, width=geom.width, height=geom.height)

    def move_client(self, client: "BasicWMTestSession.Client", delta: Tuple[int, int]) -> WindowGeometry:
        frame_geom = self.window_geometry(client.frame)
        dest_x = frame_geom.x + delta[0]
        dest_y = frame_geom.y + delta[1]
        for _ in range(5):
            self._send_test_command(1, client.window.id, dest_x, dest_y)
            time.sleep(0.05)
            current = self.window_geometry(client.frame)
            if abs(current.x - dest_x) <= 2 and abs(current.y - dest_y) <= 2:
                return current
        return self.window_geometry(client.frame)

    def resize_client(self, client: "BasicWMTestSession.Client", delta: Tuple[int, int]) -> WindowGeometry:
        frame_geom = self.window_geometry(client.frame)
        dest_width = max(1, frame_geom.width + delta[0])
        dest_height = max(1, frame_geom.height + delta[1])
        for _ in range(5):
            self._send_test_command(2, client.window.id, dest_width, dest_height)
            time.sleep(0.05)
            current = self.window_geometry(client.frame)
            if (abs(current.width - dest_width) <= 4 and
                    abs(current.height - dest_height) <= 4):
                return current
        return self.window_geometry(client.frame)

    def focus_client(self, client: "BasicWMTestSession.Client") -> None:
        if self._display is None:
            raise RuntimeError("Display not ready")
        self._raise_window(client.frame)
        center = self.window_geometry(client.window)
        self._warp_pointer(center.x + center.width // 2, center.y + center.height // 2)
        try:
            self._display.set_input_focus(client.window, X.RevertToPointerRoot, X.CurrentTime)
        except error.BadMatch:
            pass
        self._display.sync()

    def send_alt_f4(self, client: "BasicWMTestSession.Client") -> None:
        if self._display is None:
            raise RuntimeError("Display not ready")
        self.focus_client(client)
        self._send_test_command(3, client.window.id)

    def wait_for_client_exit(self, client: "BasicWMTestSession.Client", timeout: float = 5.0) -> None:
        deadline = time.time() + timeout
        while time.time() < deadline:
            if client.process.poll() is not None:
                break
            time.sleep(0.1)
        if client.process.poll() is None:
            raise RuntimeError("Client did not exit in time")
        if self._display:
            try:
                self._display.sync()
                client.window.get_geometry()
            except (error.BadWindow, error.BadDrawable):
                pass
        self.close_client(client)

    def focus_window_id(self) -> Optional[int]:
        if self._display is None:
            return None
        focus = self._display.get_input_focus().focus
        return getattr(focus, "id", None)

    def send_alt_tab(self, client: Optional["BasicWMTestSession.Client"] = None) -> None:
        if self._display is None:
            raise RuntimeError("Display not ready")
        if client is not None:
            window_id = client.window.id
        else:
            window_id = self.focus_window_id() or 0
        previous_focus = self.focus_window_id()
        self._send_test_command(4, window_id)
        deadline = time.time() + 1.0
        while time.time() < deadline:
            current = self.focus_window_id()
            if current is not None and current != previous_focus:
                return
            time.sleep(0.05)

    def _build_binary(self) -> None:
        subprocess.run(["make", "basic_wm"], check=True)

    def _allocate_display(self) -> int:
        base = 100
        for offset in range(10):
            display_number = base + offset
            socket_path = Path(f"/tmp/.X11-unix/X{display_number}")
            if socket_path.exists():
                continue
            return display_number
        raise RuntimeError("Unable to allocate display number")

    def _wait_for_display(self, timeout: float = 5.0) -> None:
        deadline = time.time() + timeout
        while time.time() < deadline:
            if self._xvfb and self._xvfb.poll() is not None:
                raise RuntimeError("Xvfb exited prematurely")
            try:
                disp = display.Display(self.display_name)
                disp.close()
                return
            except Exception:
                time.sleep(0.1)
        raise RuntimeError("Timed out waiting for Xvfb")

    def _wait_for_wm(self, timeout: float = 5.0) -> None:
        deadline = time.time() + timeout
        while time.time() < deadline:
            if self._wm and self._wm.poll() is not None:
                raise RuntimeError("Window manager exited prematurely")
            if self._display is not None:
                self._display.sync()
                return
            time.sleep(0.1)
        raise RuntimeError("Timed out waiting for window manager")

    def _wait_for_window(self, name: str, timeout: float = 5.0):
        if self._display is None:
            raise RuntimeError("Display not ready")
        root = self._display.screen().root
        deadline = time.time() + timeout
        while time.time() < deadline:
            self._display.sync()
            window = self._find_window(root, name)
            if window is not None:
                return window
            time.sleep(0.1)
        raise RuntimeError(f"Timed out waiting for window '{name}'")

    def _wait_for_frame(self, window, timeout: float = 5.0):
        if self._display is None:
            raise RuntimeError("Display not ready")
        root = self._display.screen().root
        deadline = time.time() + timeout
        while time.time() < deadline:
            self._display.sync()
            try:
                tree = window.query_tree()
            except Exception:
                time.sleep(0.05)
                continue
            parent = getattr(tree, "parent", None)
            if parent is not None and getattr(parent, "id", None) not in {None, root.id}:
                return parent
            time.sleep(0.05)
        raise RuntimeError("Timed out waiting for window frame")

    def _find_window(self, window, name: str):
        try:
            wm_name = window.get_wm_name()
        except Exception:
            wm_name = None
        if isinstance(wm_name, bytes):
            wm_name = wm_name.decode(errors="ignore")
        if wm_name == name:
            return window
        try:
            children = window.query_tree().children
        except Exception:
            return None
        for child in children:
            result = self._find_window(child, name)
            if result is not None:
                return result
        return None

    def _send_test_command(self, command: int, window_id: int, arg2: int = 0, arg3: int = 0) -> None:
        if self._display is None or self._test_atom is None:
            raise RuntimeError("Display not ready")
        data = [command, window_id, arg2, arg3]
        root = self._display.screen().root
        root.change_property(
            self._test_atom,
            Xatom.CARDINAL,
            32,
            data,
        )
        self._display.sync()

    def _raise_window(self, window) -> None:
        if self._display is None:
            raise RuntimeError("Display not ready")
        window.configure(stack_mode=X.Above)
        self._display.sync()

    def _warp_pointer(self, x: int, y: int) -> None:
        if self._display is None:
            raise RuntimeError("Display not ready")
        root = self._display.screen().root
        root.warp_pointer(x, y)
        self._display.sync()

    @staticmethod
    def _terminate_process(proc: subprocess.Popen, timeout: float = 2.0) -> None:
        if proc.poll() is not None:
            return
        proc.terminate()
        try:
            proc.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            proc.kill()
            proc.wait()
