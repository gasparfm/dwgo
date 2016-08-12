 /********************************************************************************
 *  File: XDraw.cpp   								*
 *  Date: 20080805            							*
 *  Author: Gaspar Fernández (helyo@totaki.com) 				*
 *										*
 *  Copyright (C) 2008   Gaspar Fernández					*
 *										*
 *  This program is free software: you can redistribute it and/or modify	*
 *  it under the terms of the GNU General Public License as published by	*
 *  the Free Software Foundation, either version 3 of the License, or 		*
 *  (at your option) any later version.  	      	  	   		*
 *										*
 *  This program is distributed in the hope that it will be useful,		*
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of		*
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the		*
 *  GNU General Public License for more details.				*
 *										*
 *  You should have received a copy of the GNU General Public License		*
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.	*
 *									        *
 ********************************************************************************
 *   Description:
 *      It containt functions to display XPM images using Xlib. Also we can 
 *    render text and make some drawing in colors.
 *      Based in Xpm.cc by Per Linden (included in Temperature.app 1.4) released
 *    under GNU GPL v2 (or later) license.
 *
 *   Change History:
 *    Date       Author     Modification
 *    
 ********************************************************************************/ 

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include "XDraw.h"
#include "errors.h"
#include <cstring>

using namespace std;

 
/*************************************************************
 *     Constructor XDraw                                     *
 *************************************************************
 *  Description:                                             *
 *      Load a background image into a Window shown in a     *
 *  Display                                                  *
 *                                                           *
 * Input:                                                    *
 *     Display* disp - Current display                       *
 *     Window   root - Root Window where to draw             *
 *     const char data* - XPM Image data                     *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
XDraw::XDraw(Display* disp, Window root, const char* data) 

{
   xDisplay = disp;
   defaultWin = root;
   
   load_bkgrnd(data);
}

/*************************************************************
 *     Method: load_bkgrnd()                                 *
 *************************************************************
 *  Description:                                             *
 *     Load Xpm data as a background image.                  *
 *                                                           *
 * Input:                                                    *
 *     const char* data - XPM image data                     *
 *                                                           *
 * Output:                                                   *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
void XDraw::load_bkgrnd(const char* data)
{
   int error;

   Attributes.valuemask = 0;
   error = XpmCreatePixmapFromBuffer(xDisplay, defaultWin, (char *) data, &Image, &Mask, &Attributes);
   if (error!=XpmSuccess)
     error_handler(ERR_XPMERROR,NULL);
}

/*************************************************************
 *     Destructor ~XDraw                                     *
 *************************************************************
 *  Description:                                             *
 *     Frees memory                                          *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
XDraw::~XDraw() 
{
  xpmfree();
}

/*************************************************************
 *     Method xpmfree()                                      *
 *************************************************************
 *  Description:                                             *
 *     Frees Memory                                          *
 *                                                           *
 * Input:                                                    *
 *     Nothing                                               *
 *                                                           *
 * Output:                                                   *
 *     Nothing                                               *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
void XDraw::xpmfree()
{
   if (Image) {
      XFreePixmap(xDisplay, Image);
   }

   if (Mask) {
      XFreePixmap(xDisplay, Mask);
   }
}

/*************************************************************
 *     Method: replace_background()                          *
 *************************************************************
 *  Description:                                             *
 *    Replaces background image data in the current window   *
 *                                                           *
 * Input:                                                    *
 *    const char* bkg_data - New XPM image to replace actual *
 *                          background data                  *
 *                                                           *
 * Output:                                                   *
 *    Nothing                                                *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
void XDraw::replace_background(const char* bkg_data)
{
  xpmfree();			// Free image and mask
  load_bkgrnd(bkg_data);
}

/*************************************************************
 *     Function: setWindowPixmap(), setWindowPixmapShaped(), *
 *               setwpxmap()                                 * 
 *************************************************************
 *  Description:                                             *
 *    Renders the image into the Window                      *
 *    We can do this with the default window, or with        *
 *    another one.                                           *
 *                                                           *
 * Input:                                                    *
 *    Window win - Window to render the image                *
 *    bool shaped - Wether we have to apply mask or not.     *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
void XDraw::setWindowPixmap(Window win) 
{
  setwpxmap(win, false);
}

void XDraw::setWindowPixmap() 
{
  setwpxmap(defaultWin, false);
}

void XDraw::setwpxmap(Window win, bool shaped)	// SetWindowPixmap
{
   XResizeWindow(xDisplay, win, Attributes.width, Attributes.height);
   XSetWindowBackgroundPixmap(xDisplay, win, Image);
   if (shaped)
     XShapeCombineMask(xDisplay, win, ShapeBounding, 0, 0, Mask, ShapeSet);
   XClearWindow(xDisplay, win);
}

void XDraw::setWindowPixmapShaped(Window win) 
{
  setwpxmap(win, true);
}

void XDraw::setWindowPixmapShaped() 
{
  setwpxmap(defaultWin, true);
}

/*************************************************************
 *     Function: setDefaultWindow                            *
 *************************************************************
 *  Description:                                             *
 *    Replaces default window with another one.              *
 *                                                           *
 * Input:                                                    *
 *    Window win - The new default window to work with.      *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
void XDraw::setDefaultWindow(Window win)
{
  defaultWin = win;
}

/*************************************************************
 *     Function: Sync, doxsync                               *
 *************************************************************
 *  Description:                                             *
 *     XSync the Window                                      *
 *                                                           *
 * Input:                                                    *
 *   Window win - The Window                                *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
void XDraw::Sync(Window win)
{
  doxsync(win);
}

void XDraw::Sync()
{
  doxsync(defaultWin);
}

void XDraw::doxsync(Window win)
{
  XClearWindow(xDisplay, win);
  XSync(xDisplay, win);
}

/*************************************************************
 *     Method: DrawRect                                      *
 *************************************************************
 *  Description:                                             *
 *    Draws a filled rectangle                               *
 *                                                           *
 * Input:                                                    *
 *   int x, int y - X, Y positions                           *
 *   unsigned int w, unsigned int h - Width and Height       *
 *   XDrawColor color - Color of the filled rectangle        *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
void XDraw::DrawRect(int x, int y, unsigned int w, unsigned int h, XDrawColor color)
{
   GC           gc;
   XGCValues    gcv;

   gcv.foreground=setColor(color);
   gc = XCreateGC(xDisplay, Image, GCForeground, &gcv);

   XFillRectangle(xDisplay, Image, gc, x,y,w,h);
}

/*************************************************************
 *     Function: setColor                                    *
 *************************************************************
 *  Description:                                             *
 *     Gets unsigned long value, from rgb data. It can be    *
 *   extracted from individual ints or from a XDrawColor     *
 *   structure                                               *
 *                                                           *
 * Input:                                                    *
 *   int red, int gree, int blue (rgb data)                  *
 *   or XDrawColor color (rgb data within a XDrawColor       *
 *                        structure)                         *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
unsigned long XDraw::setColor(XDrawColor color)
{
	unsigned long ret_color;
	int r, g, b;
	r = (int)((256-color.r) * 255) & 0xff;
	g = (int)((256-color.g) * 255) & 0xff;
	b = (int)((256-color.b) * 255) & 0xff;
	
	ret_color = (r<<16)+(g<<8)+b;
	return ret_color;
}

unsigned long XDraw::setColor(int red, int green, int blue)
{
	unsigned long ret_color;
	int r, g, b;
	r = (int)((256-red) * 255) & 0xff;
	g = (int)((256-green) * 255) & 0xff;
	b = (int)((256-blue) * 255) & 0xff;
	
	ret_color = (r<<16)+(g<<8)+b;
	return ret_color;
}

/*************************************************************
 *     Function: drawString                                  *
 *************************************************************
 *  Description:                                             *
 *     Renders text into the Image                           *
 *                                                           *
 * Input:                                                    *
 *    int x, int y - X, Y position of the text               *
 *     (if x==CENTER_TEXT) it will be centered.              *
 *    int maxX - max. width in pixels of the text            *
 *    XDrawColor color - Color to render the text            *
 *    char* font - Font to use                               *
 *    char* str - Text to render                             *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
void XDraw::drawString(int x, int y, int maxX, XDrawColor color, char* font, char* str)
{
   XFontStruct* fontStruct;
   GC           gc;
   XGCValues    gcv;
   if ((fontStruct = XLoadQueryFont(xDisplay, font)) == 0)
     error_handler(ERR_BADFONT, font);
   

   gcv.foreground=setColor(color);
   gc = XCreateGC(xDisplay, Image, GCForeground, &gcv);

   int strLength = strlen(str);
   int strWidth = XTextWidth(fontStruct, str, strLength);
   if (strWidth>maxX)
     {
       maxX=maxX-XTextWidth(fontStruct, (char*)"...", 3);
       strLength--;
       while (strWidth>maxX)
	 {
	   strLength--;
	   strWidth = XTextWidth(fontStruct, str, strLength);
	 }
       str[strLength]='\0';	// Cut string
       strcat(str,"...");	// Add ... at the end
       strLength=strLength+3;
     }
   if (x==CENTER_TEXT)
     x = (Attributes.width / 2) - (strWidth / 2);

   XSetFont(xDisplay, gc, fontStruct->fid);
   XDrawString(xDisplay, Image, gc, x, y, str, strLength);

   XFreeGC(xDisplay, gc);
   XFreeFont(xDisplay, fontStruct);
}

/*************************************************************
 *     Function: setDrawColor()                              *
 *************************************************************
 *  Description:                                             *
 *     Inserts individual rgb data into a XDrawColor         *
 *  structure.                                               *
 *                                                           *
 * Input:                                                    *
 *   int r, int g, int b - RGB data                          *
 *                                                           *
 * Output:                                                   *
 *   XDrawColor structure containing rgb data                *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
XDraw::XDrawColor setDrawColor(int r, int g, int b)
{
  XDraw::XDrawColor color;
  color.r=r;
  color.b=b;
  color.g=g;
  return color;
}

/*************************************************************
 *     Function: testfont                                    *
 *************************************************************
 *  Description:                                             *
 *     Can we use this font?                                 *
 *     Tests if it is installed on the system and it is      *
 *  usable.                                                  *
 *                                                           *
 * Input:                                                    *
 *    char* font - Font to test                              *
 *    Display* disp - Display to use                         *
 *                                                           *
 * Output:                                                   *
 *   True if we can use it, false if we can't                *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
bool testfont(const char *font, Display *disp)
{
  XFontStruct* fontstruct;
  return ((fontstruct = XLoadQueryFont(disp, font)) != 0);
}
