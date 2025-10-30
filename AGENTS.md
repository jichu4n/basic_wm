# Agent instruction: Before you start development, setup build env:

1. apt update

2. add-apt-repository universe

3. apt update && apt upgrade

4. apt-get install \
    build-essential pkg-config libx11-dev libgoogle-glog-dev \
    xserver-xephyr xinit x11-apps xterm apt-utils dialog


#### Install Zig

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
