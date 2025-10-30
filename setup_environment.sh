#!/bin/bash
#
# Setup script for basic_wm development environment with Zig
# This script installs all dependencies needed for C++ development and Zig porting

set -e

echo "========================================="
echo "basic_wm Environment Setup"
echo "========================================="
echo ""

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if running as root or with sudo
if [ "$EUID" -ne 0 ]; then
    log_error "This script must be run as root or with sudo"
    exit 1
fi

# Step 1: Update package repositories
log_info "Step 1: Updating package repositories..."
apt-get update

# Step 2: Add universe repository
log_info "Step 2: Adding universe repository..."
add-apt-repository -y universe

# Step 3: Update and upgrade packages
log_info "Step 3: Updating and upgrading packages..."
apt-get update && apt-get upgrade -y

# Step 4: Install C++ build dependencies and X11 libraries
log_info "Step 4: Installing build dependencies and X11 libraries..."
apt-get install -y \
    build-essential \
    pkg-config \
    libx11-dev \
    libgoogle-glog-dev \
    xserver-xephyr \
    xinit \
    x11-apps \
    xterm \
    apt-utils \
    dialog \
    xvfb \
    python3-xlib \
    python3-pip \
    wget \
    curl \
    git

# Step 5: Install Python testing dependencies
log_info "Step 5: Installing Python testing dependencies..."
pip3 install pytest python-xlib

# Step 6: Check if Zig is already installed
log_info "Step 6: Checking for existing Zig installation..."
if command -v zig &> /dev/null; then
    INSTALLED_VERSION=$(zig version)
    log_warn "Zig is already installed (version: $INSTALLED_VERSION)"
    read -p "Do you want to reinstall Zig? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        log_info "Skipping Zig installation"
        SKIP_ZIG=true
    fi
fi

# Step 7: Install Zig
if [ "$SKIP_ZIG" != true ]; then
    log_info "Step 7: Installing Zig..."

    # Get the latest Zig version from GitHub
    log_info "Fetching latest Zig version from GitHub..."
    ZIG_VERSION=$(curl -s "https://api.github.com/repos/ziglang/zig/releases/latest" | grep -Po '"tag_name": "\K[0-9.]+')

    if [ -z "$ZIG_VERSION" ]; then
        log_error "Failed to fetch Zig version from GitHub"
        exit 1
    fi

    log_info "Latest Zig version: $ZIG_VERSION"

    # Download Zig archive
    log_info "Downloading Zig $ZIG_VERSION..."
    wget -q --show-progress -O /tmp/zig.tar.xz \
        "https://ziglang.org/download/${ZIG_VERSION}/zig-linux-x86_64-${ZIG_VERSION}.tar.xz"

    # Create installation directory
    log_info "Creating Zig installation directory..."
    mkdir -p /opt/zig

    # Extract Zig to /opt/zig
    log_info "Extracting Zig archive..."
    tar xf /tmp/zig.tar.xz --strip-components=1 -C /opt/zig

    # Create symbolic link
    log_info "Creating symbolic link to /usr/local/bin..."
    ln -sf /opt/zig/zig /usr/local/bin/zig

    # Clean up downloaded archive
    log_info "Cleaning up temporary files..."
    rm -f /tmp/zig.tar.xz

    # Verify Zig installation
    if command -v zig &> /dev/null; then
        log_info "Zig successfully installed: $(zig version)"
    else
        log_error "Zig installation failed"
        exit 1
    fi
else
    log_info "Step 7: Skipped Zig installation"
fi

# Step 8: Test Zig installation
log_info "Step 8: Testing Zig installation..."
cat > /tmp/test_zig.zig << 'EOF'
const std = @import("std");

pub fn main() !void {
    std.debug.print("Hello from Zig!\n", .{});
}
EOF

zig build-exe /tmp/test_zig.zig -femit-bin=/tmp/test_zig
if /tmp/test_zig | grep -q "Hello from Zig!"; then
    log_info "Zig test compilation and execution successful"
else
    log_error "Zig test execution failed"
    exit 1
fi

# Clean up test files
rm -f /tmp/test_zig.zig /tmp/test_zig /tmp/test_zig.o

# Step 9: Build existing C++ window manager
log_info "Step 9: Building existing C++ window manager..."
cd "$(dirname "$0")"
if [ -f Makefile ]; then
    make clean
    make basic_wm
    log_info "C++ window manager built successfully"
else
    log_warn "Makefile not found, skipping C++ build"
fi

echo ""
echo "========================================="
echo "Setup Complete!"
echo "========================================="
echo ""
log_info "Environment setup completed successfully"
echo ""
echo "Installed components:"
echo "  - C++ build tools (gcc, g++, make)"
echo "  - X11 development libraries"
echo "  - Google glog library"
echo "  - Xephyr, xinit, and X11 utilities"
echo "  - Xvfb for headless testing"
echo "  - Python3 with pytest and python-xlib"
echo "  - Zig $(zig version)"
echo ""
echo "You can now:"
echo "  1. Build the C++ window manager: make basic_wm"
echo "  2. Run the window manager: ./build_and_run.sh"
echo "  3. Run tests: pytest tests/"
echo "  4. Start porting to Zig following the roadmap in AGENTS.md"
echo ""
