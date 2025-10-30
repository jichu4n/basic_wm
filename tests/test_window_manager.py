import pytest

from tests.headless_session import BasicWMTestSession


@pytest.fixture
def wm_session():
    session = BasicWMTestSession()
    session.start()
    try:
        yield session
    finally:
        session.shutdown()


def test_headless_session_launches():
    session = BasicWMTestSession()
    result = session.run_headless_check()
    assert result["display"].startswith(":")
    assert result["client"]["width"] > 0
    assert result["client"]["height"] > 0


def test_move_window_with_alt_drag(wm_session):
    client = wm_session.launch_xterm("move-test", geometry="80x24+40+40")
    wm_session.resize_client(client, delta=(-500, -360))
    start = wm_session.window_geometry(client.frame)
    moved = wm_session.move_client(client, delta=(120, 80))
    assert moved.x == pytest.approx(start.x + 120, abs=2)
    assert moved.y == pytest.approx(start.y + 80, abs=2)
    wm_session.close_client(client)


def test_resize_window_with_alt_drag(wm_session):
    client = wm_session.launch_xterm("resize-test", geometry="80x24+40+40")
    start = wm_session.window_geometry(client.frame)
    resized = wm_session.resize_client(client, delta=(-500, -360))
    assert resized.width == pytest.approx(max(1, start.width - 500), abs=4)
    assert resized.height == pytest.approx(max(1, start.height - 360), abs=4)
    wm_session.close_client(client)


def test_alt_f4_closes_window(wm_session):
    client = wm_session.launch_xterm("close-test")
    wm_session.focus_client(client)
    wm_session.send_alt_f4(client)
    wm_session.wait_for_client_exit(client)


def test_alt_tab_switches_focus(wm_session):
    first = wm_session.launch_xterm("first-test")
    second = wm_session.launch_xterm("second-test")
    wm_session.focus_client(first)
    focused_before = wm_session.focus_window_id()
    assert focused_before == first.window.id
    wm_session.send_alt_tab(first)
    focused_after = wm_session.focus_window_id()
    assert focused_after in {first.window.id, second.window.id}
    assert focused_after != focused_before
    wm_session.close_client(first)
    wm_session.close_client(second)
