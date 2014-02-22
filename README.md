Basic Window Manager
====================

This is a very simple X window manager that demonstrates how to implement the
fundamental functionality of a window manager:

  * Managing the life cycle of windows
  * Reparenting windows
  * Basic window operations (moving, resizing, closing)


Building and Running
--------------------

To build this example window manager, you will need:

* A C++-11 enabled C++ compiler
* [SCons](http://www.scons.org/)
* Xlib headers and libraries
* [google-glog](https://code.google.com/p/google-glog/) library

To run and test it, you will need:

* [Xephyr](https://code.google.com/p/google-glog/)
* `xinit`
* Random X utilities such as `xclock`, `xeyes`, and `xterm` to play with

On [Arch Linux](http://www.archlinux.org), you can install these with:

    yaourt -S base-devel scons xproto google-glog \
        xorg-server-xephyr xorg-xinit xorg-xclock xorg-xeyes xterm

Consult your own distro's documentation for how to install these.

Once you have all the dependencies, building and running it is as simple as:

    ./build_and_run.sh

This will launch a simple Xephyr session like in the following screenshot:
![Screenshot](http://seasonofcode.com/assets/files/basic_wm_screenshot.png =400)
