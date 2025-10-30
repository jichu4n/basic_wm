import subprocess
import sys
import types
from types import SimpleNamespace

import pytest


try:
    from Xlib import X, Xatom, display, error
    from Xlib.protocol import event as xevent
except ModuleNotFoundError:
    xlib_module = types.ModuleType("Xlib")
    x_module = types.ModuleType("Xlib.X")
    xatom_module = types.ModuleType("Xlib.Xatom")
    display_module = types.ModuleType("Xlib.display")
    protocol_module = types.ModuleType("Xlib.protocol")
    protocol_event_module = types.ModuleType("Xlib.protocol.event")
    error_module = types.ModuleType("Xlib.error")

    class _DummyError(Exception):
        pass

    error_module.BadWindow = _DummyError
    error_module.BadDrawable = _DummyError

    display_module.Display = object

    xlib_module.X = x_module
    xlib_module.Xatom = xatom_module
    xlib_module.display = display_module
    xlib_module.protocol = protocol_module
    protocol_module.event = protocol_event_module
    xlib_module.error = error_module

    sys.modules["Xlib"] = xlib_module
    sys.modules["Xlib.X"] = x_module
    sys.modules["Xlib.Xatom"] = xatom_module
    sys.modules["Xlib.display"] = display_module
    sys.modules["Xlib.protocol"] = protocol_module
    sys.modules["Xlib.protocol.event"] = protocol_event_module
    sys.modules["Xlib.error"] = error_module

    from Xlib import X, Xatom, display, error
    from Xlib.protocol import event as xevent


from tests import headless_session


class DummyDisplay:
    def __init__(self, *args, **kwargs):
        self.closed = False

    def intern_atom(self, name, only_if_exists=False):
        return f"atom-{name}"

    def close(self):
        self.closed = True

    def sync(self):
        return None

    def screen(self):
        return SimpleNamespace(root=SimpleNamespace(id=1))


class DummyProcess:
    def __init__(self):
        self._poll = None
        self.terminated = False
        self.killed = False

    def poll(self):
        return self._poll

    def terminate(self):
        self.terminated = True
        self._poll = 0

    def wait(self, timeout=None):
        self._poll = 0
        return 0

    def kill(self):
        self.killed = True
        self._poll = 0


@pytest.fixture
def session(monkeypatch):
    sess = headless_session.BasicWMTestSession()
    monkeypatch.setattr(sess, "_build_binary", lambda: None)
    monkeypatch.setattr(sess, "_allocate_display", lambda: 99)
    monkeypatch.setattr(sess, "_wait_for_display", lambda timeout=5.0: None)
    monkeypatch.setattr(sess, "_wait_for_wm", lambda timeout=5.0: None)
    monkeypatch.setattr(sess, "_wait_for_window", lambda name, timeout=5.0: SimpleNamespace(id=1))
    monkeypatch.setattr(sess, "_wait_for_frame", lambda window, timeout=5.0: SimpleNamespace(id=2))
    monkeypatch.setattr(headless_session.display, "Display", DummyDisplay)
    created = []

    def fake_popen(args, **kwargs):
        proc = DummyProcess()
        created.append({"args": args, "kwargs": kwargs, "proc": proc})
        return proc

    monkeypatch.setattr(headless_session.subprocess, "Popen", fake_popen)
    sess._clients_created = created
    return sess


def test_processes_use_devnull(session):
    session.start()
    client = session.launch_xterm("test-client")
    assert client.window.id == 1
    assert client.frame.id == 2

    popen_calls = session._clients_created
    assert len(popen_calls) == 3

    for call in popen_calls:
        assert call["kwargs"].get("stdout") is subprocess.DEVNULL
        assert call["kwargs"].get("stderr") is subprocess.DEVNULL

    session.shutdown()
    for call in popen_calls:
        assert call["proc"].terminated
