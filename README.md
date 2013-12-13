x:choyce
===

#### A light-weight, Exposé-like window switcher for X (feat. OpenGL) ####

### Version ###
2013.12.12

### About the name ####
The `x` stands (obviously) for X Window System. I thought it might be a good
idea to prefix applications written for X with an identifier or namespace.

The `choyce` part is a portmanteau of choice and joy. Go figure.

### Features ###
* Light-weight: No additional libraries except XCB, Xft and OpenGL
* Exposé-like: Shows thumbnails of all your windows for selection
* Draws window icons from `_NET_WM_ICON` or `WM_HINTS`
* Draws window titles from `_NET_WM_NAME` and/or `WM_NAME` & `WM_CLASS`
* Unfocused windows are black and white & slightly blurred
* Entirely configurable through Xresources (`man 1 xrdb`)
* Automatic configuration reload (e.g. after `xrdb -load`) without restart
* Makes use of OpenGL for fast and appealing graphical effects
* GLSL shaders in external file, stringified at compile time
* Written in modern C++11 with a modular object oriented approach

### OpenGL Version
<font color="red">
This is important: The minimum required **OpenGL** version is **3.0**
</font>

You can check your driver's version with `glxinfo`. Currently, there are no
runtime checks, so `x:choyce` might behave oddly or just crash with versions
below 3.0.

### Compilation
Clone the repository and type `make`.
If this fails, check if the [required
libraries](https://github.com/jrk-/x-choyce/blob/master/Makefile#L1-2) are
installed. Check also if your compiler is recent enough.

For reference, this is the software I'm using (compare with `make version`):

```
xcb:                1.9.1
xcb-atom:           0.3.9
xcb-keysyms:        0.3.9
xcb-composite:      1.9.1
xcb-damage:         1.9.1
xcb-xinerama:       1.9.1
xcb-xfixes:         1.9.1
x11:                1.6.2
x11-xcb:            1.6.2
gl:                 9.2.3
xcb-ewmh:           0.3.9
xcb-icccm:          0.3.9
xcb-image:          0.3.9
xft:                2.3.1

GCC version:
Using built-in specs.
COLLECT_GCC=gcc
COLLECT_LTO_WRAPPER=/usr/lib/gcc/x86_64-unknown-linux-gnu/4.8.2/lto-wrapper
Target: x86_64-unknown-linux-gnu
Configured with: /build/gcc-multilib/src/gcc-4.8.2/configure --prefix=/usr --libdir=/usr/lib --libexecdir=/usr/lib --mandir=/usr/share/man --infodir=/usr/share/info --with-bugurl=https://bugs.archlinux.org/ --enable-languages=c,c++,ada,fortran,go,lto,objc,obj-c++ --enable-shared --enable-threads=posix --with-system-zlib --enable-__cxa_atexit --disable-libunwind-exceptions --enable-clocale=gnu --disable-libstdcxx-pch --enable-gnu-unique-object --enable-linker-build-id --enable-cloog-backend=isl --disable-cloog-version-check --enable-lto --enable-gold --enable-ld=default --enable-plugin --with-plugin-ld=ld.gold --with-linker-hash-style=gnu --disable-install-libiberty --enable-multilib --disable-libssp --disable-werror --enable-checking=release
Thread model: posix
gcc version 4.8.2 (GCC)
```

### Installation
Run `sudo make install`. The default prefix is `/usr`, hence the binary will be
installed to `/usr/bin` and the shader programs to
`/usr/share/x:choyce/shaders`. The prefix is configurable, e.g. `make install
PREFIX=${HOME}/.local/x:choyce`.

### Running
Type `x:choyce` and the program should start up.

Your window manager needs to support at least the `_NET_CLIENT_LIST_STACKING`,
`_NET_CURRENT_DESKTOP` and `_NET_ACTIVE_WINDOW` hints from the EWMH
specification. If not, you could summon a black which will swallow your
computer.

### Usage
After starting the program, hit `Super-Tab` and your window should appear in
a grid like layout. Hit `Super-Tab` once more to select another window. Pressing
`Super-Shift-Tab` reverts the direction. After releasing the `Super` key the
selected window will be brought to focus. Alternatively the `raise` key can be
pressed. This will raise the currently selected window above all others, but not
focus it. For example, this could be useful with a multi-monitor setup, where
one wishes to view a different window on another monitor without focusing it.

Alternatively try using the mouse pointer. Once the mouse pointer has been
moved, the `Super` key can be released and a window can be chosen by clicking on
it. Selecting with `Super-Tab` is still possible, releasing the `Super` key will
then again have the same effect as before.

Last, but not least, using the `h`, `j`, `k` and `l` for moving around is also
supported. Again, the selection is committed by releasing the `Super` key.

### Configuration
The program can be configured (even at run time) using Xresources. You can read
more about this [here](http://tronche.com/gui/x/xlib/resource-manager/) or in
the `xrdb` manpage.

All options listed below can also be given on the command line. For example, run
`x:choyce --titlebgalpha 1.0` for a fully opaque title strip. If an option is
set on the commandline it cannot be changed during runtime via Xresources.

#### Quick start

Put this in your `~/.Xresources` file:

```
XChoyce.mod:                    mod4
XChoyce.action:                 Tab
XChoyce.focusedalpha:           0.75
XChoyce.focusedcolor:           #daa520
XChoyce.unfocusedalpha:         0.5
XChoyce.unfocusedcolor:         #404040
XChoyce.titlebgcolor:           #292929
XChoyce.titlebgalpha:           0.25
XChoyce.titlefgcolor:           #616161
XChoyce.titlefont:              Ubuntu:bold:pixelsize=28:antialias=true
XChoyce.subtitlefont:           Ubuntu Condensed:bold:pixelsize=18:antialias=true
```

Now run `xrdb -merge ~/.Xresources`, et voilà, configuration is accomplished.

#### Complete list of configuration options:

```
focusedalpha:   double.  Alpha value for border when focused.
focusedcolor:   string.  Color name for border when focused.
unfocusedalpha: double.  Alpha value for border when unfocused.
unfocusedcolor: string.  Color name for border when unfocused.
iconsize:       integer. Size of window icon.
borderwidth:    integer. Width of border for additional highlighting.
titlefont:      string.  Font to use for window name (e.g. "Firefox").
subtitlefont:   string.  Font to use for window subtitle (e.g. "Google").
titlefgcolor:   string.  Color used for window title fonts.
titlebgalpha:   double.  Alpha value for window title background.
titlebgcolor:   string.  Color used for window title background.
north:          string.  Key for moving north.
south:          string.  Key for moving south.
east:           string.  Key for moving east.
west:           string.  Key for moving west.
raise:          string.  Key for raising the selected window. The selected
                         window will be raised above all others, but not focused
                         (mainly for multi-monitor environments)
action:         string.  Key for iteratively stepping through windows. Pressing
                         shift will reverse direction.
escape:         string.  Key for stopping choosing without selecting a window.
mod:            string.  Modifier key. Release chooses the currently selected
                         window. Valid options are 'mod1' through 'mod5',
                         'control' and combinations like 'mod1+control'.
```

#### Default values:

```
focusedalpha:   0.75
focusedcolor:   #daa520
unfocusedalpha: 0.5
unfocusedcolor: #404040
iconsize:       64
borderwidth:    4
titlefont:      Sans:bold:pixelsize=26:antialias=true
subtitlefont:   Sans:bold:pixelsize=16:antialias=true
titlefgcolor:   #303030
titlebgalpha:   0.375
titlebgcolor:   #484848
north:          k
south:          j
east:           l
west:           h
raise:          space
action:         Tab
escape:         Escape
mod:            mod4
```

#### Contributors

* [SanskritFritz](https://github.com/SanskritFritz) helped a lot to
[indentify and resolve multiple issues](https://github.com/jrk-/x-choyce/issues/1)
