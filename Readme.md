


# Night And Day : Automatic desktop color changer
#### Copyright (C) 1998-1999 Jean-Baptiste M. Queru

![screenshot.png](screenshot.png)

### Welcome to the Night And Day "ReadMe" file!

Night And Day is a small program I wrote for the BeOS to be
sure I still knew how to use some of the BeOS classes. This program
was inspired by the "TaveDesktop" program by the Tave Software
Group. I think that "TaveDesktop" was a good idea, but I wanted
to be able to choose my own colors, since the "TaveDesktop"
colors looks bad on an 8bpp display.

This program is free software; you can redistribute it and/or 
modify it under the terms of the GNU General Public License 
as published by the Free Software Foundation; either version 2 
of the License, or (at your option) any later version.

This program should be self-explanatory. You'll find a small
Popup menu when clicking the small screen at the top-right
corner of the window.

### Internal problems:

Most routines are slow, especially drawing the preview bitmap.
The source code would require much cleanup to be made understandable.
There are probably some possible deadlocks, especially when quitting
  the program before it is completely running.

### Revision history:

0.1.2 : dutch language support
0.1.1 : re-release for R4.5. minor changes.
0.1 : first beta release
