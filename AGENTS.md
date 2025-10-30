### Project goal: 

Rewrite the basic_wm in in Zig for a Zig based WM w/ TDD w/ high coverage testing with a focus on porting, optimization, testing and confirming handling in real use, collecting data and efficiency.

### Important Setup:

Before you start any coding, install:

1. apt update

2. add-apt-repository universe

3. apt update && apt upgrade

4. apt-get install \
    build-essential pkg-config libx11-dev libgoogle-glog-dev \
    xserver-xephyr xinit x11-apps xterm apt-utils dialog xvfb python3-xlib


#### Code Practices / Guidance

* Always end and remove/clean up Xvfb process after test.
* use dialog package to add animated progress/run indicators to tests


#### Install Zig

0. Check if Zig is installed and if not install it:
   
```bash
zig version
```

1. Get the latest Zig version from GitHub:

```bash
ZIG_VERSION=$(curl -s "https://api.github.com/repos/ziglang/zig/releases/latest" | grep -Po '"tag_name": "\K[0-9.]+')
```

2.  Download the Zig archive:

```bash
wget -qO zig.tar.xz https://ziglang.org/download/${ZIG_VERSION}/zig-x86_64-linux-${ZIG_VERSION}.tar.xz
```

3. Create installation directory and extract files to it:

```bash
sudo mkdir /opt/zig
```

```bash
sudo tar xf zig.tar.xz --strip-components=1 -C /opt/zig
```

4. Create a symbolic link to make zig command available system-wide:

```bash
sudo ln -s /opt/zig/zig /usr/local/bin/zig
```

5. Verify the installation by checking Zig version:

```bash
zig version
```

6. The tar.xz file is no longer needed, remove it:

```bash
rm -rf zig.tar.xz
```

#### Testing Zig

1. Create a main.zig file:

```bash
nano main.zig
```

2. Paste the following code into main.zig:

```zig
const std = @import("std");

pub fn main() !void {
    std.debug.print("Hello world\n", .{});
}
```

3. Compile the Zig file into an executable named test:

```bash
zig build-exe main.zig -femit-bin=test
```

4. Run the compiled program:

```bash
./test
```

---

## ZIG PORT ROADMAP AND DIRECTIVES

### 1. CODEBASE ANALYSIS - CURRENT STATE

#### 1.1 Architecture Overview
The basic_wm is a pedagogical X11 window manager written in C++11 with the following components:

**Core Files:**
- `main.cpp` (19 lines): Entry point, initializes glog and WindowManager
- `window_manager.hpp` (95 lines): WindowManager class interface
- `window_manager.cpp` (593 lines): Core WM implementation with X11 event loop
- `util.hpp` (225 lines): Template-based geometry types (Size, Position, Vector2D) and utility functions
- `util.cpp` (348 lines): X11 event and error string conversion utilities

**Key Features:**
- Reparenting window manager (wraps client windows in frames)
- Window operations: move (Alt+Left-click), resize (Alt+Right-click), close (Alt+F4), switch (Alt+Tab)
- X11 event handling: CreateNotify, DestroyNotify, MapRequest, ConfigureRequest, ButtonPress, MotionNotify, KeyPress
- Error handling with custom X error handlers
- Frame windows with colored borders (red border, blue background)

**External Dependencies:**
- X11 (Xlib) for window system interaction
- google-glog for logging
- C++ STL (unique_ptr, unordered_map, mutex, string)

**Testing Infrastructure:**
- Python-based tests using python-xlib
- Headless testing with Xvfb
- Test scenarios: window move, resize, close (Alt+F4), focus switch (Alt+Tab)
- Interactive testing with Xephyr

#### 1.2 Critical Implementation Details

**Window Management State:**
- `Display* display_`: Connection to X server
- `Window root_`: Root window handle
- `unordered_map<Window, Window> clients_`: Maps client windows to frame windows
- Drag state for move/resize: `drag_start_pos_`, `drag_start_frame_pos_`, `drag_start_frame_size_`

**X11 Atoms:**
- `WM_PROTOCOLS`: For graceful window deletion
- `WM_DELETE_WINDOW`: Client message for closing windows
- `TEST_COMMAND`: Custom atom for automated testing
- `TEST_COMMAND_PROPERTY`: Property atom for test communication

**Key Algorithms:**
1. **Initialization:** Check for existing WM, grab server, reparent existing windows, ungrab server
2. **Framing:** Create frame window, reparent client, grab button/key events, map frame
3. **Unframing:** Unmap frame, reparent to root, destroy frame
4. **Event Loop:** Infinite loop with XNextEvent, switch on event type
5. **Error Handling:** Static error handlers for X errors and WM detection

### 2. ZIG PORT STRATEGY - PHASED APPROACH

#### Phase 1: Foundation and X11 Bindings (PRIORITY: CRITICAL)

**Objective:** Establish Zig project structure and X11 interop layer

**Tasks:**
1. Create Zig build system (`build.zig`)
   - Link with X11 library (`-lX11`)
   - Set up compilation flags
   - Configure debug and release modes
   - Add test runner integration

2. Create X11 bindings module (`src/x11.zig`)
   - Import X11 C headers using `@cImport`
   - Create Zig-friendly type wrappers for X11 types
   - Wrap key functions: XOpenDisplay, XDefaultRootWindow, XNextEvent, etc.
   - Create error handling wrappers that return Zig errors

3. Create geometry utilities module (`src/util.zig`)
   - Port Size, Position, Vector2D structs
   - Implement operator overloading using Zig methods
   - Add ToString formatting using std.fmt
   - Write unit tests for all geometry operations

4. Create logging abstraction (`src/log.zig`)
   - Implement logging levels (DEBUG, INFO, WARN, ERROR)
   - Use std.log as backing implementation
   - Create macros similar to glog's LOG(INFO), LOG(ERROR)
   - Add structured logging support

**Success Criteria:**
- [ ] `zig build` compiles without errors
- [ ] Can open X11 display connection from Zig
- [ ] Can query root window properties
- [ ] Geometry types have 100% test coverage
- [ ] Logging outputs to stderr correctly

**Testing Directives:**
- Write unit tests for every geometry operation
- Create integration test that opens/closes X11 display
- Test with DISPLAY=:0 and verify no crashes
- Use Xvfb for headless CI testing

**Agent Directives:**
- Use `@cImport` with `@cInclude("X11/Xlib.h")` for X11 bindings
- Prefer Zig idioms over direct C translations
- Use `std.testing.expect` for all unit tests
- Document all public functions with doc comments (///)
- Handle all errors explicitly - no `try` without context

---

#### Phase 2: Window Manager Core Structure (PRIORITY: CRITICAL)

**Objective:** Port WindowManager class to Zig struct with basic initialization

**Tasks:**
1. Create WindowManager struct (`src/window_manager.zig`)
   - Port all member variables from C++ class
   - Use `std.AutoHashMap(Window, Window)` for clients map
   - Store Atoms as struct fields (WM_PROTOCOLS, WM_DELETE_WINDOW, etc.)
   - Implement Create() factory function (returns !WindowManager)

2. Implement initialization sequence
   - Port WindowManager.Create() static factory
   - Port constructor logic (XInternAtom calls)
   - Implement destructor equivalent (deinit() method with XCloseDisplay)
   - Add WM detection logic (OnWMDetected handler)

3. Implement error handling infrastructure
   - Create custom error set: `WMError` with variants (DisplayOpenFailed, AlreadyRunning, XError, etc.)
   - Port OnXError handler as error callback
   - Port OnWMDetected handler for existing WM check
   - Create error string conversion utilities

4. Setup basic event loop skeleton
   - Port Run() method with infinite for loop
   - Implement XNextEvent wrapper
   - Add event type switch statement (placeholder handlers)
   - Add graceful shutdown mechanism

**Success Criteria:**
- [ ] WindowManager.create() successfully opens display
- [ ] WindowManager.create() detects existing WM and returns error
- [ ] WindowManager.init() interns all required atoms
- [ ] Event loop receives and prints events without crashing
- [ ] deinit() closes display without leaks

**Testing Directives:**
- Test Create() with valid DISPLAY
- Test Create() with invalid DISPLAY (should return error)
- Test Create() with running WM (should detect and error)
- Test event loop receives CreateNotify, MapRequest events
- Use Valgrind equivalent in Zig to check for leaks

**Agent Directives:**
- Use `const self: *WindowManager` for methods that don't mutate
- Use `self: *WindowManager` for methods that mutate state
- Initialize all fields explicitly in create() function
- Call deinit() in defer block in tests
- Use `std.AutoHashMap` with appropriate allocator (use std.heap.page_allocator for now, optimize later)

---

#### Phase 3: Window Framing and Client Management (PRIORITY: HIGH)

**Objective:** Implement window framing/unframing logic

**Tasks:**
1. Implement Frame() function
   - Port logic from window_manager.cpp:207-303
   - Create frame window with XCreateSimpleWindow
   - Set border (red 0xff0000) and background (blue 0x0000ff)
   - Reparent client window into frame
   - Add client to save set
   - Grab button and key events (Alt+Button1, Alt+Button3, Alt+F4, Alt+Tab)
   - Store client->frame mapping in clients map

2. Implement Unframe() function
   - Port logic from window_manager.cpp:305-326
   - Reverse framing steps: unmap, reparent to root, remove from save set
   - Destroy frame window
   - Remove from clients map
   - Handle errors gracefully

3. Implement client lifecycle event handlers
   - OnCreateNotify (currently no-op, document why)
   - OnDestroyNotify (currently no-op, document why)
   - OnMapRequest: Frame window if needed, then XMapWindow
   - OnUnmapNotify: Unframe client window
   - OnReparentNotify (currently no-op, document why)

4. Implement initial window reparenting in Run()
   - Query existing top-level windows with XQueryTree
   - Frame each existing window that is viewable and not override_redirect
   - Implement grab/ungrab server around reparenting

**Success Criteria:**
- [ ] New windows get framed with red border
- [ ] Frames are correct size matching client window
- [ ] Client windows reparented correctly into frames
- [ ] Closing client triggers unframe and cleanup
- [ ] Existing windows (before WM start) get framed
- [ ] Override-redirect windows are NOT framed

**Testing Directives:**
- Launch xterm, verify it gets framed
- Check xwininfo shows frame as parent of xterm
- Kill xterm, verify frame is destroyed
- Start WM with existing windows, verify they get framed
- Test with override-redirect window (xmessage), verify not framed
- Use python test: test_headless_session_launches()

**Agent Directives:**
- Check for null/error after every X11 call
- Use defer to ensure cleanup in error paths
- Log frame creation with window IDs for debugging
- Match C++ border width, colors exactly for visual parity
- Document magic numbers (BORDER_WIDTH=3, colors) with constants

---

#### Phase 4: Interactive Window Operations (PRIORITY: HIGH)

**Objective:** Implement mouse and keyboard driven window operations

**Tasks:**
1. Implement move window operation (Alt+Left-click)
   - OnButtonPress: Save cursor position and frame geometry
   - OnMotionNotify: Calculate delta, move frame with XMoveWindow
   - OnButtonRelease: Finalize move
   - Test: test_move_window_with_alt_drag()

2. Implement resize window operation (Alt+Right-click)
   - OnButtonPress: Save initial state (same as move)
   - OnMotionNotify: Calculate size delta, resize frame AND client
   - Handle negative sizes (clamp to minimum 1x1)
   - OnButtonRelease: Finalize resize
   - Test: test_resize_window_with_alt_drag()

3. Implement close window (Alt+F4)
   - OnKeyPress: Detect Alt+F4 key combination
   - Check if client supports WM_DELETE_WINDOW protocol
   - If yes: Send ClientMessage with WM_DELETE_WINDOW
   - If no: XKillClient (force kill)
   - Test: test_alt_f4_closes_window()

4. Implement focus switch (Alt+Tab)
   - OnKeyPress: Detect Alt+Tab combination
   - Iterate to next client in clients map
   - XRaiseWindow on next frame
   - XSetInputFocus on next client
   - Handle wrap-around when reaching end
   - Test: test_alt_tab_switches_focus()

5. Implement ConfigureRequest handler
   - Honor client resize requests
   - Apply changes to both frame and client
   - Use XConfigureWindow with appropriate masks

**Success Criteria:**
- [ ] Alt+Left-drag moves windows smoothly
- [ ] Alt+Right-drag resizes windows (both frame and client)
- [ ] Alt+F4 gracefully closes supporting clients
- [ ] Alt+F4 force-kills non-supporting clients
- [ ] Alt+Tab cycles focus between windows
- [ ] Windows can resize themselves (ConfigureRequest)
- [ ] All interactive tests pass: test_move, test_resize, test_close, test_focus

**Testing Directives:**
- Use Python test suite with Xvfb
- Test with real xterm (supports WM_DELETE_WINDOW)
- Test with xmessage (may not support WM_DELETE_WINDOW)
- Verify smooth motion (no lag, no tearing)
- Test edge cases: resize to 0x0, move off-screen, single window Alt+Tab

**Agent Directives:**
- Use XSync after move/resize for immediate feedback
- Calculate deltas carefully to avoid integer overflow
- Check clients map before accessing in event handlers
- Use std.math.max to clamp minimum sizes
- Log all key window operations (move, resize, close, focus) for debugging

---

#### Phase 5: Testing Infrastructure and Test Harness (PRIORITY: HIGH)

**Objective:** Create comprehensive test infrastructure matching Python tests

**Tasks:**
1. Port Python test infrastructure to Zig
   - Create Zig test harness that launches Xvfb
   - Implement BasicWMTestSession equivalent in Zig
   - Add functions: launch_xterm, move_client, resize_client, close_client
   - Implement test command protocol (TEST_COMMAND atom)

2. Implement test command protocol
   - Port HandleTestCommand and DispatchTestCommand
   - Support commands: 1=Move, 2=Resize, 3=Close, 4=Focus
   - Use ClientMessage events for command dispatch
   - Use PropertyNotify for property-based commands

3. Create Zig unit tests
   - Test all geometry utilities (Size, Position, Vector2D)
   - Test error handling paths
   - Test clients map operations
   - Test atom interning

4. Create Zig integration tests
   - Port test_move_window_with_alt_drag
   - Port test_resize_window_with_alt_drag
   - Port test_alt_f4_closes_window
   - Port test_alt_tab_switches_focus
   - Port test_headless_session_launches

5. Setup CI/CD pipeline
   - Create GitHub Actions workflow
   - Run tests with Xvfb in headless mode
   - Generate test coverage reports
   - Enforce minimum 80% code coverage

**Success Criteria:**
- [ ] All unit tests pass (zig build test)
- [ ] All integration tests pass with Xvfb
- [ ] Test coverage >= 80%
- [ ] Tests run in CI without flakiness
- [ ] Memory leaks detected and fail tests
- [ ] Can run tests with: zig build test

**Testing Directives:**
- Use std.testing for all unit tests
- Use expectEqual, expectError for assertions
- Always clean up Xvfb processes after tests (defer)
- Add timeouts to prevent hung tests
- Use dialog package for animated test progress

**Agent Directives:**
- Write tests BEFORE implementing features (TDD)
- One test function per feature/behavior
- Use descriptive test names: test_given_when_then style
- Clean up ALL resources in defer blocks
- Use std.testing.allocator for leak detection

---

#### Phase 6: Logging and Debugging (PRIORITY: MEDIUM)

**Objective:** Create robust logging and debugging infrastructure

**Tasks:**
1. Port utility string functions from util.cpp
   - Port ToString(XEvent) with all event type names
   - Port XConfigureWindowValueMaskToString
   - Port XRequestCodeToString
   - Create XErrorEventToString for error logging

2. Enhance logging throughout codebase
   - Log all X11 events received (like C++ version)
   - Log window frame/unframe operations with IDs
   - Log error conditions with context
   - Add performance timing logs for key operations

3. Create debugging utilities
   - Add --verbose flag for debug logging
   - Add --log-file option to log to file
   - Create window tree dump utility
   - Create client map dump utility for troubleshooting

4. Add runtime statistics
   - Count events processed by type
   - Track number of clients managed
   - Measure frame/unframe timing
   - Report statistics on shutdown

**Success Criteria:**
- [ ] All X11 events logged with readable format
- [ ] Errors logged with full context (request code, error text, resource ID)
- [ ] Can enable verbose logging with flag
- [ ] Statistics reported at end of session
- [ ] Logs parseable for automated analysis

**Testing Directives:**
- Verify log output format matches expectations
- Test with --verbose flag
- Capture stderr and validate log messages
- Test log file creation and writing

**Agent Directives:**
- Use std.log.info, std.log.err consistently
- Include window IDs in all window-related logs
- Log errors BEFORE returning error values
- Use std.fmt for string formatting in logs
- Keep log messages concise but informative

---

#### Phase 7: Performance Optimization (PRIORITY: MEDIUM)

**Objective:** Optimize Zig implementation for performance and efficiency

**Tasks:**
1. Memory management optimization
   - Replace page_allocator with GeneralPurposeAllocator
   - Add arena allocator for short-lived allocations
   - Profile memory usage under load
   - Eliminate unnecessary allocations

2. Event loop optimization
   - Batch similar events (e.g., multiple MotionNotify)
   - Optimize event dispatch with comptime switch
   - Reduce XSync calls to minimum necessary
   - Profile event handler performance

3. X11 interaction optimization
   - Batch X11 requests where possible
   - Reduce roundtrips to X server
   - Use XCheckTypedWindowEvent for event coalescing (already in C++ for MotionNotify)
   - Cache frequently accessed properties

4. Benchmarking and profiling
   - Create benchmark suite for key operations
   - Measure startup time
   - Measure event processing latency
   - Compare with C++ version performance

5. Compile-time optimization
   - Use comptime where appropriate
   - Optimize debug vs release builds
   - Enable LTO for release builds
   - Profile binary size

**Success Criteria:**
- [ ] Startup time <= C++ version
- [ ] Event latency <= C++ version + 10%
- [ ] Memory usage <= C++ version
- [ ] Binary size <= C++ version + 20%
- [ ] No memory leaks under load
- [ ] Can manage 100+ windows smoothly

**Testing Directives:**
- Create stress test with 100+ windows
- Measure performance with perf/flamegraph
- Run long-duration tests (8+ hours)
- Test under Valgrind/AddressSanitizer equivalent
- Compare benchmarks against C++ version

**Agent Directives:**
- Profile before optimizing
- Document all performance optimizations
- Keep optimizations readable
- Use std.time for performance measurements
- Test that optimizations don't break correctness

---

#### Phase 8: Real-World Testing and Validation (PRIORITY: CRITICAL)

**Objective:** Validate WM works correctly in real usage scenarios

**Tasks:**
1. Interactive testing with real applications
   - Test with xterm, rxvt-unicode, alacritty
   - Test with GUI apps: xclock, xeyes, firefox
   - Test with complex apps: gimp, inkscape
   - Test with Qt/GTK applications

2. Edge case testing
   - Test with misbehaving clients
   - Test with windows that set override_redirect
   - Test with windows that change size frequently
   - Test rapid window creation/destruction
   - Test with zero-size or huge windows

3. Multi-monitor testing
   - Test with multiple screens (if available)
   - Test with different DPI settings
   - Test with screen rotation

4. Stability testing
   - Run WM for extended periods (24+ hours)
   - Monitor for memory leaks
   - Monitor for resource leaks (X resources)
   - Test recovery from X server disconnection

5. Compatibility testing
   - Test on different X servers (Xorg, Xephyr, Xnest)
   - Test on different distributions
   - Test with different X11 versions
   - Test with different hardware

**Success Criteria:**
- [ ] Works with all tested applications without crashes
- [ ] No visual glitches or artifacts
- [ ] No memory leaks after 24h runtime
- [ ] Gracefully handles misbehaving clients
- [ ] Works on at least 3 different Linux distributions
- [ ] No regressions compared to C++ version

**Testing Directives:**
- Create test matrix of applications × operations
- Document all bugs found with reproduction steps
- Test each bug fix with regression test
- Use Xephyr for safe interactive testing
- Keep log of all testing sessions

**Agent Directives:**
- Fix bugs from real testing before new features
- Add regression test for every bug found
- Document any limitations discovered
- Update README with compatibility notes
- Create issues for known limitations

---

#### Phase 9: Documentation and Polish (PRIORITY: LOW)

**Objective:** Create comprehensive documentation for Zig port

**Tasks:**
1. API documentation
   - Document all public functions with doc comments
   - Create module-level documentation
   - Document design decisions and tradeoffs
   - Create architecture diagrams

2. User documentation
   - Update README for Zig version
   - Create INSTALL guide for Zig WM
   - Document configuration options
   - Create troubleshooting guide

3. Developer documentation
   - Create CONTRIBUTING guide
   - Document build system
   - Document testing procedures
   - Create porting guide (C++ → Zig lessons learned)

4. Code cleanup
   - Remove debug code and commented code
   - Ensure consistent code style
   - Run Zig fmt on all files
   - Fix all compiler warnings

**Success Criteria:**
- [ ] 100% of public APIs documented
- [ ] README updated and accurate
- [ ] zig fmt passes on all files
- [ ] No compiler warnings
- [ ] Build instructions tested by fresh user

**Agent Directives:**
- Use /// for doc comments on public items
- Use // for implementation notes
- Keep docs concise but complete
- Include code examples in docs
- Run zig fmt before every commit

---

#### Phase 10: Packaging and Distribution (PRIORITY: LOW)

**Objective:** Package Zig WM for distribution

**Tasks:**
1. Build system finalization
   - Finalize build.zig with all options
   - Support install target
   - Create uninstall target
   - Support PREFIX for custom install locations

2. Packaging
   - Create .desktop file for display managers
   - Create man page
   - Create example xinitrc
   - Package for common distributions (deb, rpm, AUR)

3. Release preparation
   - Create release notes
   - Tag version in git
   - Create GitHub release
   - Announce on relevant forums

**Success Criteria:**
- [ ] Can install with: zig build install
- [ ] Shows up in display manager session list
- [ ] Man page installed and accessible
- [ ] Works after fresh install on clean system

---

### 3. C++ TO ZIG TRANSLATION GUIDE

#### 3.1 Type Mappings

| C++ Type | Zig Equivalent | Notes |
|----------|----------------|-------|
| `std::unique_ptr<T>` | `*T` with manual deinit | Use defer for cleanup |
| `std::unordered_map<K,V>` | `std.AutoHashMap(K, V)` | Requires allocator |
| `std::string` | `[]const u8` or `[]u8` | Use ArrayList(u8) for growable |
| `std::mutex` | `std.Thread.Mutex` | Similar API |
| `std::lock_guard<mutex>` | `mutex.lock()` + `defer mutex.unlock()` | Use defer pattern |
| `class` | `struct` | No inheritance, use composition |
| `static` member | top-level const/var | Or put in separate namespace struct |
| `nullptr` | `null` | For optional pointers use `?*T` |
| `bool` | `bool` | Same, but lowercase |
| `const char*` | `[*:0]const u8` | Sentinel-terminated pointer |

#### 3.2 Pattern Translations

**C++ RAII → Zig defer:**
```cpp
std::lock_guard<mutex> lock(wm_detected_mutex_);
// ... protected code ...
```
```zig
wm_detected_mutex_.lock();
defer wm_detected_mutex_.unlock();
// ... protected code ...
```

**C++ Static Factory → Zig Factory Function:**
```cpp
static std::unique_ptr<WindowManager> Create(const std::string& display_str);
```
```zig
pub fn create(allocator: Allocator, display_str: ?[]const u8) !*WindowManager
```

**C++ Destructor → Zig deinit:**
```cpp
~WindowManager() { XCloseDisplay(display_); }
```
```zig
pub fn deinit(self: *WindowManager) void {
    _ = x11.XCloseDisplay(self.display);
    self.allocator.destroy(self);
}
```

**C++ Template → Zig Generic:**
```cpp
template <typename T>
struct Size { T width, height; };
```
```zig
fn Size(comptime T: type) type {
    return struct {
        width: T,
        height: T,
    };
}
```

**C++ Static Error Handler → Zig Callback:**
```cpp
static int OnXError(Display* display, XErrorEvent* e);
```
```zig
fn onXError(display: ?*x11.Display, e: [*c]x11.XErrorEvent) callconv(.C) c_int
```

#### 3.3 Error Handling

**C++ (implicit):**
```cpp
Display* display = XOpenDisplay(display_c_str);
if (display == nullptr) {
    LOG(ERROR) << "Failed to open X display";
    return nullptr;
}
```

**Zig (explicit):**
```zig
const display = x11.XOpenDisplay(display_cstr) orelse {
    std.log.err("Failed to open X display", .{});
    return error.DisplayOpenFailed;
};
```

#### 3.4 Memory Management

**C++ (RAII):**
```cpp
auto window_manager = WindowManager::Create();
// automatic cleanup via unique_ptr
```

**Zig (explicit defer):**
```zig
var wm = try WindowManager.create(allocator, null);
defer wm.deinit();
```

### 4. TESTING STRATEGY

#### 4.1 Test Pyramid

1. **Unit Tests (60% of tests)**
   - Test all geometry operations (Size, Position, Vector2D)
   - Test utility functions (string conversion, etc.)
   - Test data structure operations (clients map)
   - Test error handling paths
   - Run with: `zig build test`

2. **Integration Tests (30% of tests)**
   - Test WM initialization and shutdown
   - Test frame/unframe operations
   - Test event handling flow
   - Test with mock X server or Xvfb
   - Run with: `zig build test-integration`

3. **End-to-End Tests (10% of tests)**
   - Test full interactive scenarios
   - Test with real X clients (xterm, xclock)
   - Match existing Python test suite
   - Run with: `pytest tests/` (keep Python tests)

#### 4.2 Test Coverage Goals

- **Critical paths: 100% coverage** (init, event loop, frame/unframe)
- **Event handlers: 95% coverage** (all event types)
- **Utility code: 90% coverage**
- **Error paths: 85% coverage**
- **Overall: >= 80% coverage**

#### 4.3 Test Environment

- Use Xvfb for headless testing
- Use Xephyr for interactive testing
- Use Valgrind/AddressSanitizer for leak detection
- Use perf for performance profiling
- Use GitHub Actions for CI/CD

#### 4.4 Regression Testing

- Keep Python test suite functional during transition
- Add regression test for every bug fixed
- Compare behavior against C++ version
- Maintain test matrix document

### 5. SUCCESS CRITERIA AND VALIDATION

#### 5.1 Functional Success Criteria

- [ ] **Feature Parity:** All features from C++ version work identically
- [ ] **Window Framing:** New windows framed with correct border/color
- [ ] **Window Move:** Alt+Left-drag moves windows smoothly
- [ ] **Window Resize:** Alt+Right-drag resizes windows correctly
- [ ] **Window Close:** Alt+F4 closes windows (graceful or forced)
- [ ] **Window Focus:** Alt+Tab cycles focus between windows
- [ ] **Existing WM Detection:** Fails gracefully if another WM running
- [ ] **Error Handling:** No crashes on malformed X events or bad clients
- [ ] **Memory Safety:** No leaks, no use-after-free, no buffer overflows

#### 5.2 Performance Success Criteria

- [ ] **Startup Time:** <= C++ version
- [ ] **Event Latency:** <= C++ version + 10%
- [ ] **Memory Usage:** <= C++ version
- [ ] **CPU Usage:** <= C++ version + 5%
- [ ] **Smoothness:** 60 FPS during window operations
- [ ] **Scalability:** Handles 100+ windows without degradation

#### 5.3 Code Quality Success Criteria

- [ ] **Test Coverage:** >= 80% overall
- [ ] **Documentation:** 100% of public APIs documented
- [ ] **Code Style:** All code passes `zig fmt`
- [ ] **Warnings:** Zero compiler warnings
- [ ] **Type Safety:** All errors handled explicitly
- [ ] **Build Time:** < 5 seconds for full rebuild

#### 5.4 Compatibility Success Criteria

- [ ] **X11 Versions:** Works with X11R7.7+
- [ ] **Linux Distributions:** Tested on Ubuntu, Fedora, Arch
- [ ] **Applications:** Works with xterm, firefox, gimp, emacs, etc.
- [ ] **Display Servers:** Works with Xorg, Xephyr, Xvfb
- [ ] **Backends:** No Wayland dependencies (pure X11)

#### 5.5 Validation Process

1. **Phase Gate Reviews:** Each phase must meet success criteria before next phase
2. **Code Review:** All code reviewed before merge
3. **CI/CD Pipeline:** All tests must pass in CI
4. **Performance Benchmarks:** Must meet performance criteria
5. **Real-World Testing:** Must work with real applications for 24h+
6. **User Acceptance:** Original author review and approval

### 6. RISK MITIGATION

#### 6.1 Technical Risks

| Risk | Impact | Mitigation |
|------|--------|------------|
| X11 API complexity | HIGH | Incremental porting, extensive testing |
| Memory safety bugs | HIGH | Use Zig's safety features, testing |
| Performance regression | MEDIUM | Continuous benchmarking |
| Behavioral differences | HIGH | Detailed comparison testing |
| Build system complexity | LOW | Keep build.zig simple |

#### 6.2 Project Risks

| Risk | Impact | Mitigation |
|------|--------|------------|
| Scope creep | MEDIUM | Strict phase boundaries, no new features |
| Testing gaps | HIGH | 80% coverage requirement, test-first |
| Documentation lag | LOW | Document as you code |
| Maintainability | MEDIUM | Clear code, good naming, comments |

### 7. AGENT DIRECTIVES SUMMARY

#### 7.1 Coding Standards

1. **Use Zig idioms, not C++ translations**
   - Prefer Zig error handling over null checks
   - Use defer for cleanup, not destructors
   - Use comptime where appropriate
   - Embrace Zig's explicitness

2. **Memory management**
   - Pass allocator explicitly
   - Use defer for cleanup
   - Test with std.testing.allocator
   - Document ownership clearly

3. **Error handling**
   - All errors must be handled explicitly
   - Create descriptive error types
   - Log errors with context before returning
   - Test error paths

4. **Testing**
   - Write tests first (TDD)
   - One test per behavior
   - Clean up resources in defer
   - Use descriptive test names

5. **Documentation**
   - /// for public APIs
   - // for implementation notes
   - Document all assumptions
   - Explain non-obvious code

#### 7.2 Development Workflow

1. **Before coding:**
   - Read and understand C++ implementation
   - Identify edge cases
   - Write test cases
   - Design Zig API

2. **During coding:**
   - Implement test first
   - Implement feature incrementally
   - Run tests frequently
   - Keep commits small and focused

3. **After coding:**
   - Run zig fmt
   - Check test coverage
   - Update documentation
   - Review against success criteria

4. **Code review:**
   - Self-review before submitting
   - Check for error handling
   - Verify tests pass
   - Confirm no memory leaks

#### 7.3 Priority and Sequencing

**MUST DO FIRST (Phases 1-3):**
- Phase 1: X11 bindings and foundation
- Phase 2: WindowManager structure
- Phase 3: Framing logic

**CORE FUNCTIONALITY (Phases 4-5):**
- Phase 4: Interactive operations
- Phase 5: Testing infrastructure

**QUALITY AND POLISH (Phases 6-8):**
- Phase 6: Logging
- Phase 7: Performance
- Phase 8: Real-world validation

**OPTIONAL (Phases 9-10):**
- Phase 9: Documentation polish
- Phase 10: Packaging

#### 7.4 Key Implementation Notes

1. **X11 Event Loop:** Infinite for loop with XNextEvent, not while(true)
2. **Static Error Handlers:** Must use callconv(.C) for X error handlers
3. **Window IDs:** Use Window directly, it's already an opaque type (unsigned long)
4. **Atom Initialization:** Intern all atoms in constructor, not lazily
5. **Grab Server:** Use XGrab/UngrabServer around initial window query
6. **Motion Events:** Coalesce with XCheckTypedWindowEvent (already in C++)
7. **Frame Colors:** Match exactly - border=0xff0000, bg=0x0000ff, width=3
8. **Alt Key:** Mod1Mask (not Mod4Mask/Super)
9. **Test Protocol:** Use both ClientMessage and PropertyNotify approaches
10. **Cleanup Order:** Unmap frame, reparent client, remove save set, destroy frame

---

### 8. REFERENCE IMPLEMENTATION CHECKLIST

Use this checklist to track porting progress:

#### Foundation
- [ ] build.zig created with X11 linking
- [ ] src/x11.zig with X11 bindings
- [ ] src/util.zig with geometry types
- [ ] src/log.zig with logging abstraction
- [ ] Unit tests for util.zig (100% coverage)

#### Core Structure
- [ ] src/window_manager.zig created
- [ ] WindowManager struct with all fields
- [ ] create() factory function
- [ ] deinit() cleanup function
- [ ] Error handling infrastructure
- [ ] Event loop skeleton

#### Window Management
- [ ] frame() function implemented
- [ ] unframe() function implemented
- [ ] OnMapRequest handler
- [ ] OnUnmapNotify handler
- [ ] OnConfigureRequest handler
- [ ] Initial window reparenting

#### Interactive Operations
- [ ] OnButtonPress handler (save state)
- [ ] OnMotionNotify handler (move/resize)
- [ ] OnButtonRelease handler
- [ ] OnKeyPress handler (Alt+F4, Alt+Tab)
- [ ] CloseClient function (WM_DELETE_WINDOW)
- [ ] FocusNextClient function

#### Testing
- [ ] Test harness with Xvfb
- [ ] Test command protocol
- [ ] Unit tests passing
- [ ] Integration tests passing
- [ ] Python tests still passing
- [ ] 80%+ coverage achieved

#### Quality
- [ ] Logging implemented
- [ ] Performance benchmarks passing
- [ ] 24h stability test passing
- [ ] Real application testing done
- [ ] Documentation complete
- [ ] All warnings fixed

---

### 9. GETTING STARTED

#### For AI Coding Agents:

1. **Read this entire document** before writing any code
2. **Start with Phase 1, Task 1:** Create build.zig
3. **Follow TDD:** Write tests before implementation
4. **Check success criteria** after each phase
5. **Ask questions** if requirements are unclear

#### First Steps:

```bash
# 1. Ensure environment is set up
./setup_environment.sh

# 2. Create Zig project structure
mkdir -p src tests
touch build.zig src/main.zig

# 3. Write first test
# Create tests/test_util.zig with Size/Position tests

# 4. Implement to pass tests
# Implement src/util.zig

# 5. Build and test
zig build test

# 6. Continue to next task
```

#### Example build.zig Template:

```zig
const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "basic_wm_zig",
        .root_source_file = .{ .path = "src/main.zig" },
        .target = target,
        .optimize = optimize,
    });

    exe.linkSystemLibrary("X11");
    exe.linkLibC();

    b.installArtifact(exe);

    const tests = b.addTest(.{
        .root_source_file = .{ .path = "src/main.zig" },
        .target = target,
        .optimize = optimize,
    });

    tests.linkSystemLibrary("X11");
    tests.linkLibC();

    const test_step = b.step("test", "Run tests");
    test_step.dependOn(&b.addRunArtifact(tests).step);
}
```

---

## CONCLUSION

This roadmap provides a comprehensive, phase-by-phase plan for porting basic_wm from C++ to Zig. Each phase has:
- Clear objectives
- Detailed tasks
- Success criteria
- Testing directives
- Specific agent directives

The port should maintain 100% functional parity with the C++ version while leveraging Zig's safety and performance advantages. Follow the phases in order, validate at each gate, and maintain high test coverage throughout.

**Expected Timeline:** 4-6 weeks for complete port with testing
**Primary Goal:** Fully functional, well-tested Zig window manager
**Success Metric:** All tests passing, 80%+ coverage, performance parity

---
