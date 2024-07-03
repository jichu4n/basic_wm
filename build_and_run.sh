#!/bin/bash
#
# Build and run basic_wm in an Xephyr instance.

set -e

# 1. Build binary.
make basic_wm

# 2. Run.
#
# We need to specify the full path to Xephyr, as otherwise xinit will not
# interpret it as an argument specifying the X server to launch and will launch
# the default X server instead.
XEPHYR=$(whereis -b Xephyr | sed -E 's/^.*: ?//')
if [ -z "$XEPHYR" ]; then
    echo "Xephyr not found, exiting"
    exit 1
fi
xinit ./xinitrc -- \
    "$XEPHYR" \
        :100 \
        -ac \
        -screen 800x600 \
        -host-cursor
