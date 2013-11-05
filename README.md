x:choyce
===

#### A light-weight, Exposé-like window switcher for X (feat. OpenGL) ####


### About the name ####
The `x` stands (obviously) for the X Window System. I thought it might be a good
idea written for the X environment an identifier or namespace.

The `choyce` part is a portmanteau of choice and joy. Go figure.

### Features ###
* Light-weight: No additional libraries except core XCB and OpenGL libraries
* Exposé-like: Shows thumbnails of all your windows for selection
* OpenGL is used for fast and appealing graphical effects
* OpenGL shader is an external file, can be customized by users
* Unfocused windows are black and white, slightly blurred & transparent
* Written in modern C++11 with a modular OO oriented approach

### Compilation
Clone the repository and type `make`.
If this fails, check if the [required
libraries](https://github.com/jrk-/x-choyce/blob/master/Makefile#L1-2) are
installed. Check also if your compiler is recent enough.

For reference, this is the software I'm using:

Compiler:

```
g++ (GCC) 4.8.2
```

Libraries:

```
xcb:            1.9.1
xcb-atom:       0.3.9
xcb-keysyms:    0.3.9
xcb-composite:  1.9.1
xcb-damage:     1.9.1
xcb-xinerama:   1.9.1
xcb-xfixes:     1.9.1
x11:            1.6.2
x11-xcb:        1.6.2
gl:             9.2.2
```

### Running
Currently no configuration is available, so just type `./x:choyce` and the
program should start up. Your window manager needs to support at least the
`_NET_CLIENT_LIST_STACKING`, `_NET_CURRENT_DESKTOP` and `_NET_ACTIVE_WINDOW`
hints from the EWMH specification. If not, maybe a black hole will summon and
suck in your computer.

### Usage
After starting the program, hit `Alt-Tab` and your window should appear in
a grid like layout. Hit `Alt-Tab` once more to select another window. After
releasing the `Alt` key the selected will be brought to focus.

Alternatively try using the mouse pointer. Once the mouse pointer has been
moved, the `Alt` key can be released and a window can be chosen by clicking on
it. Selecting with `Alt-Tab` is still possible, releasing the `Alt` key will
then again have the same effect as before.

Last, but not least, using the `h`, `j`, `k` and `l` for moving around is also
supported. Again, the selection is committed by releasing the `Alt` key.
