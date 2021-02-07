/* rtfConsumerFunctions.h created by pingu on Wed 17-Nov-1999

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author:  Stefan Böhringer (stefan.boehringer@uni-bochum.de)
   Date: Dec 1999

   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

/*	here we define the interface functions to grammer consumers */

#ifndef rtfConsumerFunctions_h_INCLUDE
#define rtfConsumerFunctions_h_INCLUDE

#include	"rtfScanner.h"

/* general statements:
 * measurement is usually in twips: one twentieth of a point (this is
 * about 0.01764 mm) a tabstop of 540 twips (as it occurs on NeXT) is
 * therefore about 0.95 cm
 */
#define halfpoints2points(a) ((a)/2.0)
#define twips2points(a) ((a)/20.0)
#define twips2mm(a) ((a)*0.01764)

/* prepare the ctxt, or whatever you want */
void GSRTFstart(void *ctxt);

/* seal the parsing process, the context or whatever you want */
void GSRTFstop(void *ctxt);

/* */
int GSRTFgetPosition(void *ctxt);

/*
 * those pairing functions enclose RTFBlocks. Use it to capture the
 * hierarchical attribute changes of blocks.  i.e. attributes of a
 * block are forgotten once a block is closed
 */
void GSRTFopenBlock(void *ctxt, BOOL ignore);
void GSRTFcloseBlock(void *ctxt, BOOL ignore);

/* handle errors */
void GSRTFerror(void *ctxt, void *lctxt, const char *msg);

/* handle rtf commands not expicated in the grammer */
void GSRTFgenericRTFcommand(void *ctxt, RTFcmd cmd);

/* go, handle text */
void GSRTFmangleText(void *ctxt, const char *text);
void GSRTFunicode (void *ctxt, int uchar);

/*
 * font functions
 */

/* get noticed that a particular font is introduced */
void GSRTFregisterFont(void *ctxt, const char *fontName, 
		       RTFfontFamily family, int fontNumber);

/* change font number */
void GSRTFfontNumber(void *ctxt, int fontNumber);
/* change font size in half points*/
void GSRTFfontSize(void *ctxt, int fontSize);

/* set paper width in twips */
void GSRTFpaperWidth(void *ctxt, int width);
/* set paper height in twips */
void GSRTFpaperHeight(void *ctxt, int height);
/* set left margin in twips */
void GSRTFmarginLeft(void *ctxt, int margin);
/* set right margin in twips */
void GSRTFmarginRight(void *ctxt, int margin);
/* set top margin in twips */
void GSRTFmarginTop(void *ctxt, int margin);
/* set buttom margin in twips */
void GSRTFmarginButtom(void *ctxt, int margin);
/* set first line indent */
void GSRTFfirstLineIndent(void *ctxt, int indent);
/* set left indent */
void GSRTFleftIndent(void *ctxt, int indent);
/* set right indent */
void GSRTFrightIndent(void *ctxt, int indent);
/* set tabstop */
void GSRTFtabstop(void *ctxt, int location);
/* set center alignment */
void GSRTFalignCenter(void *ctxt);
/* set justified alignment */
void GSRTFalignJustified(void *ctxt);
/* set left alignment */
void GSRTFalignLeft(void *ctxt);
/* set right alignment */
void GSRTFalignRight(void *ctxt);
/* set space above */
void GSRTFspaceAbove(void *ctxt, int location);
/* set line space */
void GSRTFlineSpace(void *ctxt, int location);
/* set default paragraph style */
void GSRTFdefaultParagraph(void *ctxt);
/* set paragraph style */
void GSRTFstyle(void *ctxt, int style);
/* Add a colour to the colour table*/
void GSRTFaddColor(void *ctxt, int red, int green, int blue);
/* Add the default colour to the colour table*/
void GSRTFaddDefaultColor(void *ctxt);
/* set background colour */
void GSRTFcolorbg(void *ctxt, int color);
/* set foreground colour */
void GSRTFcolorfg(void *ctxt, int color);
/* set underline colour */
void GSRTFunderlinecolor(void *ctxt, int color);
/* set default character style */
void GSRTFdefaultCharacterStyle(void *ctxt);
/* set subscript in half points */
void GSRTFsubscript(void *ctxt, int script);
/* set superscript in half points */
void GSRTFsuperscript(void *ctxt, int script);
/* Switch bold mode on or off */
void GSRTFbold(void *ctxt, BOOL on);
/* Switch italic mode on or off */
void GSRTFitalic(void *ctxt, BOOL on);
/* Set the underline style */
void GSRTFunderline(void *ctxt, BOOL on, NSInteger style);
/* Set the strikethrough style */
void GSRTFstrikethrough(void *ctxt, NSInteger style);
/* new paragraph */
void GSRTFparagraph(void *ctxt);
/* NeXTGraphic */
void GSRTFNeXTGraphic(void *ctxt, const char *fileName, int width, int height);
/* NeXTHelpLink */
void GSRTFNeXTHelpLink(void *ctxt, int num, const char *markername,
		       const char *linkFilename, const char *linkMarkername);
/* NeXTHelpMarker */
void GSRTFNeXTHelpMarker(void *ctxt, int num, const char *markername);

void GSRTFaddField (void *ctxt, int start, const char *inst);

/* set encoding */
void GSRTFencoding(void *ctxt, int encoding);

#endif

