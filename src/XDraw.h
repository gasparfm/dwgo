//
//  Temperature.app
// 
//  Copyright (c) 2008 Gaspar Fernández
// 
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, 
//  USA.
//

// Based in Xpm.h by Per Linden (included in Temperature.app 1.4)

#ifndef _XPM_H_
#define _XPM_H_

#define CENTER_TEXT  -5

#include <X11/Xlib.h>
#include <X11/xpm.h>

class XDraw
{
 public:
  typedef struct
  {
    int r,g,b;
  } XDrawColor;

  XDraw(Display* disp, Window root, const char* data);
  virtual ~XDraw();
  void setDefaultWindow(Window win);
  void setWindowPixmap(Window win);
  void setWindowPixmap();	/* Default window */
  void setWindowPixmapShaped(Window win);
  void setWindowPixmapShaped();	/* Default window */
  void Sync(Window win);
  void Sync();		        /* Default window */

  void DrawRect(int x, int y, unsigned int w, unsigned int h, XDrawColor color);

  void drawString(int x, int y, int maxX, XDrawColor color, char* font, char* str);
  void replace_background(const char* bkg_data);

 private:
  Display*      xDisplay;
  Window        defaultWin;
  XpmAttributes Attributes;
  Pixmap        Image;
  Pixmap        Mask;

  void xpmfree();
  void setwpxmap(Window win, bool shaped);
  void load_bkgrnd(const char* data);
  void doxsync(Window win);
  unsigned long setColor(XDrawColor color);
  unsigned long setColor(int red, int green, int blue);
};

bool testfont(const char *font, Display *disp);	/* Can this font be used? */
XDraw::XDrawColor setDrawColor(int r, int g, int b);

/* Defined outside the class, it may be used before the image is defined */

#endif
