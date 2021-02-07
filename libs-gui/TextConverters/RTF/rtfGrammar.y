/* rtfGrammer.y

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

/*  
  if processed using -p GSRTFP (as recommended) it will introduce the following global symbols:
  'GSRTFPparse', `GSRTFPlex', `GSRTFPerror', `GSRTFPnerrs', `GSRTFPlval',
  `GSRTFPchar', `GSRTFPdebug
*/

/*	we request for a reentrant parser	*/
%define api.pure

%{

/*
  The overall plan is to make this grammer universal in usage.
  Intrested buddies can implement plain C functions to consume what
  the grammer is producing. this way the rtf-grammer-tree can be
  converted to what is needed: GNUstep attributed strings, tex files,
  ...
  
  The plan is laid out by defining a set of C functions which cover
  all what is needed to mangle rtf information (it is NeXT centric
  however and may even lack some features).  Be aware that some
  functions are called at specific times when some information may or
  may not be available. The first argument of all functions is a
  context, which is asked to be maintained by the consumer at
  whichever purpose seems appropriate.  This context must be passed to
  the parser by issuing 'value = GSRTFparse(ctxt, lctxt);' in the
  first place.
*/

#import <AppKit/AppKit.h>
#include <stdlib.h>
#include <string.h>
#include "rtfScanner.h"

/*	this context is passed to the interface functions	*/
typedef void	*GSRTFctxt;
/*#undef YYLSP_NEEDED*/
#define CTXT            ctxt

#define	YYERROR_VERBOSE
#define YYDEBUG 1

#include "RTFConsumerFunctions.h"
/*int GSRTFlex (YYSTYPE *lvalp, RTFscannerCtxt *lctxt); */
int GSRTFlex(void *lvalp, void *lctxt);

/* */
int fieldStart = 0;

%}

%parse-param {void *ctxt}
%parse-param {void *lctxt}
%lex-param {void *lctxt}

%union {
	int		number;
	const char	*text;
	RTFcmd		cmd;
}

/*	<!><p> RTFtext values have to be freed	*/
%token <text> RTFtext
%token RTFstart
%token RTFansi
%token RTFmac
%token RTFpc
%token RTFpca
%token RTFignore
%token RTFinfo
%token RTFstylesheet
%token RTFfootnote
%token RTFheader
%token RTFfooter
%token RTFpict
%token RTFplain
%token RTFparagraph
%token RTFdefaultParagraph
%token RTFrow
%token RTFcell
%token RTFtabulator
%token RTFemdash
%token RTFendash
%token RTFemspace
%token RTFenspace
%token RTFbullet
%token RTFfield
%token RTFfldinst
%token RTFfldalt
%token RTFfldrslt
%token RTFflddirty
%token RTFfldedit
%token RTFfldlock
%token RTFfldpriv
%token RTFfttruetype
%token RTFlquote
%token RTFrquote
%token RTFldblquote
%token RTFrdblquote
%token <cmd> RTFred
%token <cmd> RTFgreen
%token <cmd> RTFblue
%token <cmd> RTFcolorbg
%token <cmd> RTFcolorfg
%token <cmd> RTFunderlinecolor
%token <cmd> RTFcolortable
%token <cmd> RTFfont
%token <cmd> RTFfontSize
%token <cmd> RTFNeXTGraphic
%token <cmd> RTFNeXTGraphicWidth
%token <cmd> RTFNeXTGraphicHeight
%token <cmd> RTFNeXTHelpLink
%token <cmd> RTFNeXTHelpMarker
%token <cmd> RTFNeXTfilename
%token <cmd> RTFNeXTmarkername
%token <cmd> RTFNeXTlinkFilename
%token <cmd> RTFNeXTlinkMarkername
%token <cmd> RTFpaperWidth
%token <cmd> RTFpaperHeight
%token <cmd> RTFmarginLeft
%token <cmd> RTFmarginRight
%token <cmd> RTFmarginTop
%token <cmd> RTFmarginButtom
%token <cmd> RTFfirstLineIndent
%token <cmd> RTFleftIndent
%token <cmd> RTFrightIndent
%token <cmd> RTFalignCenter
%token <cmd> RTFalignJustified
%token <cmd> RTFalignLeft
%token <cmd> RTFalignRight
%token <cmd> RTFlineSpace
%token <cmd> RTFspaceAbove
%token <cmd> RTFstyle
%token <cmd> RTFbold
%token <cmd> RTFitalic
%token <cmd> RTFunderline
%token <cmd> RTFunderlineDot
%token <cmd> RTFunderlineDash
%token <cmd> RTFunderlineDashDot
%token <cmd> RTFunderlineDashDotDot
%token <cmd> RTFunderlineDouble
%token <cmd> RTFunderlineStop
%token <cmd> RTFunderlineThick
%token <cmd> RTFunderlineThickDot
%token <cmd> RTFunderlineThickDash
%token <cmd> RTFunderlineThickDashDot
%token <cmd> RTFunderlineThickDashDotDot
%token <cmd> RTFunderlineWord
%token <cmd> RTFstrikethrough
%token <cmd> RTFstrikethroughDouble
%token <cmd> RTFunichar
%token <cmd> RTFsubscript
%token <cmd> RTFsuperscript
%token <cmd> RTFtabstop
%token <cmd> RTFfcharset
%token <cmd> RTFfprq
%token <cmd> RTFcpg
%token <cmd> RTFansicpg
%token <cmd> RTFOtherStatement
%token RTFfontListStart

//	<!> we assume token numbers to be sequential
//	\fnil | \froman | \fswiss | \fmodern | \fscript | \fdecor | \ftech
//	look at rtfScanner.h for enum definition
%token	RTFfamilyNil
%token	RTFfamilyRoman
%token	RTFfamilySwiss
%token	RTFfamilyModern
%token	RTFfamilyScript
%token	RTFfamilyDecor
%token	RTFfamilyTech
%token	RTFfamilyBiDi

%type	<number> rtfFontFamily rtfCharset rtfFontStatement
%type	<text> rtfFieldinst

/*	let's go	*/

%%

rtfFile:	'{' { GSRTFstart(CTXT); } RTFstart rtfIngredients { GSRTFstop(CTXT); } '}'
		;

rtfCharset: RTFansi { GSRTFencoding(CTXT, 1); }
		|	RTFmac { GSRTFencoding(CTXT, 2); }
		|	RTFpc  { GSRTFencoding(CTXT, 3); }
		|	RTFpca { GSRTFencoding(CTXT, 4); }
		|	rtfCharset RTFansicpg { GSRTFencoding(CTXT, $2.parameter); }
		;

rtfIngredients:	/*	empty	*/
		|	rtfIngredients rtfCharset
		|	rtfIngredients rtfFontList
		|	rtfIngredients rtfColorDef
		|	rtfIngredients rtfStatement
		|	rtfIngredients RTFtext		{ GSRTFmangleText(CTXT, $2); free((void *)$2); }
		|	rtfIngredients rtfBlock
		|	rtfIngredients error
		;

rtfBlock:	'{' { GSRTFopenBlock(CTXT, NO); } rtfIngredients rtfNeXTstuff '}' { GSRTFcloseBlock(CTXT, NO); } /* may be empty */
		|	'{' { GSRTFopenBlock(CTXT, YES); } RTFignore rtfIngredients '}' { GSRTFcloseBlock(CTXT, YES); }
		|	'{' { GSRTFopenBlock(CTXT, YES); } RTFinfo rtfIngredients '}' { GSRTFcloseBlock(CTXT, YES); }
		|	'{' { GSRTFopenBlock(CTXT, YES); } RTFstylesheet rtfIngredients '}' { GSRTFcloseBlock(CTXT, YES); }
		|	'{' { GSRTFopenBlock(CTXT, YES); } RTFfootnote rtfIngredients '}' { GSRTFcloseBlock(CTXT, YES); }
		|	'{' { GSRTFopenBlock(CTXT, YES); } RTFheader rtfIngredients '}' { GSRTFcloseBlock(CTXT, YES); }
		|	'{' { GSRTFopenBlock(CTXT, YES); } RTFfooter rtfIngredients '}' { GSRTFcloseBlock(CTXT, YES); }
		|	'{' { GSRTFopenBlock(CTXT, YES); } RTFpict rtfIngredients '}' { GSRTFcloseBlock(CTXT, YES); }
		|	'{' { GSRTFopenBlock(CTXT, NO); } RTFfield rtfField '}' { GSRTFcloseBlock(CTXT, NO); }
		|	'{' error '}'
		;


rtfField: { fieldStart = GSRTFgetPosition(CTXT);} rtfFieldMod rtfFieldinst rtfFieldrslt { GSRTFaddField(CTXT, fieldStart, $3); free((void *)$3); }
		|	error
		;

rtfFieldMod:	/*	empty	*/
		|	rtfFieldMod RTFflddirty
		|	rtfFieldMod RTFfldedit
		|	rtfFieldMod RTFfldlock
		|	rtfFieldMod RTFfldpriv
		;

rtfIgnore:  	/*	empty	*/
		| RTFignore
		;

rtfFieldinst: '{' rtfIgnore RTFfldinst RTFtext rtfFieldalt '}' { $$ = $4;}
		| '{' rtfIgnore RTFfldinst '{' { GSRTFopenBlock(CTXT, YES); } rtfStatementList RTFtext rtfFieldalt '}' { GSRTFcloseBlock(CTXT, YES); } '}' { $$ = $7;}
		| '{' error '}' { $$ = NULL;}
		;

rtfFieldalt:  	/*	empty	*/
		| RTFfldalt
		;

rtfFieldrslt: '{' rtfIgnore RTFfldrslt rtfIngredients '}' 
		| '{' error '}'
		;

rtfStatementList: 	/*	empty	*/
		| rtfStatementList rtfStatement
		| rtfStatementList rtfBlock
		;

/*
	RTF statements start with a '\', have a alpha name and a number argument
*/

rtfStatement: RTFfont				{ int font;
		    
						  if ($1.isEmpty)
						      font = 0;
						  else
						      font = $1.parameter;
						  GSRTFfontNumber(CTXT, font); }
		|	RTFfontSize		{ int size;

						  if ($1.isEmpty)
						      size = 24;
						  else
						      size = $1.parameter;
						  GSRTFfontSize(CTXT, size); }
		|	RTFpaperWidth		{ int width; 
		
		                                  if ($1.isEmpty)
						      width = 12240;
						  else
						      width = $1.parameter;
						  GSRTFpaperWidth(CTXT, width);}
		|	RTFpaperHeight		{ int height; 
		
		                                  if ($1.isEmpty)
						      height = 15840;
						  else
						      height = $1.parameter;
						  GSRTFpaperHeight(CTXT, height);}
		|	RTFmarginLeft		{ int margin; 
		
		                                  if ($1.isEmpty)
						      margin = 1800;
						  else
						      margin = $1.parameter;
						  GSRTFmarginLeft(CTXT, margin);}
		|	RTFmarginRight		{ int margin; 
		
		                                  if ($1.isEmpty)
						      margin = 1800;
						  else
						      margin = $1.parameter;
						  GSRTFmarginRight(CTXT, margin); }
		|	RTFmarginTop		{ int margin; 
		
		                                  if ($1.isEmpty)
						      margin = 1440;
						  else
						      margin = $1.parameter;
						  GSRTFmarginTop(CTXT, margin); }
		|	RTFmarginButtom		{ int margin; 
		
		                                  if ($1.isEmpty)
						      margin = 1440;
						  else
						      margin = $1.parameter;
						  GSRTFmarginButtom(CTXT, margin); }
		|	RTFfirstLineIndent	{ int indent; 
		
		                                  if ($1.isEmpty)
						      indent = 0;
						  else
						      indent = $1.parameter;
						  GSRTFfirstLineIndent(CTXT, indent); }
		|	RTFleftIndent		{ int indent; 
		
		                                  if ($1.isEmpty)
						      indent = 0;
						  else
						      indent = $1.parameter;
						  GSRTFleftIndent(CTXT, indent);}
		|	RTFrightIndent		{ int indent; 
		
		                                  if ($1.isEmpty)
						      indent = 0;
						  else
						      indent = $1.parameter;
						  GSRTFrightIndent(CTXT, indent);}
		|	RTFtabstop		{ int location; 
		
		                                  if ($1.isEmpty)
						      location = 0;
						  else
						      location = $1.parameter;
						  GSRTFtabstop(CTXT, location);}
		|	RTFalignCenter		{ GSRTFalignCenter(CTXT); }
		|	RTFalignJustified	{ GSRTFalignJustified(CTXT); }
		|	RTFalignLeft		{ GSRTFalignLeft(CTXT); }
		|	RTFalignRight		{ GSRTFalignRight(CTXT); }
		|	RTFspaceAbove		{ int space; 
		
		                                  if ($1.isEmpty)
						      space = 0;
						  else
						      space = $1.parameter;
						  GSRTFspaceAbove(CTXT, space); }
		|	RTFlineSpace		{ GSRTFlineSpace(CTXT, $1.parameter); }
		|	RTFdefaultParagraph	{ GSRTFdefaultParagraph(CTXT); }
		|	RTFstyle		{ GSRTFstyle(CTXT, $1.parameter); }
		|	RTFcolorbg		{ int color; 
		
		                                  if ($1.isEmpty)
						      color = 0;
						  else
						      color = $1.parameter;
						  GSRTFcolorbg(CTXT, color); }
		|	RTFcolorfg		{ int color; 
		
		                                  if ($1.isEmpty)
						      color = 0;
						  else
						      color = $1.parameter;
						  GSRTFcolorfg(CTXT, color); }
		|	RTFunderlinecolor	{ int color; 
		
		                                  if ($1.isEmpty)
						      color = 0;
						  else
						      color = $1.parameter;
						  GSRTFunderlinecolor(CTXT, color); }
		|	RTFsubscript		{ int script;
		
		                                  if ($1.isEmpty)
						      script = 6;
						  else
						      script = $1.parameter;
						  GSRTFsubscript(CTXT, script); }
		|	RTFsuperscript		{ int script;
		
		                                  if ($1.isEmpty)
						      script = 6;
						  else
						      script = $1.parameter;
						  GSRTFsuperscript(CTXT, script); }
		|	RTFbold			{ BOOL on;

		                                  if ($1.isEmpty || $1.parameter)
						      on = YES;
						  else
						      on = NO;
						  GSRTFbold(CTXT, on); }
		|	RTFitalic		{ BOOL on;

		                                  if ($1.isEmpty || $1.parameter)
						      on = YES;
						  else
						      on = NO;
						  GSRTFitalic(CTXT, on); }
		|	RTFunderline		{ BOOL on;

		                                  if ($1.isEmpty || $1.parameter)
						      on = YES;
						  else
						      on = NO;
						  GSRTFunderline(CTXT, on, NSUnderlineStyleSingle | NSUnderlinePatternSolid); }
		|	RTFunderlineDot		{ BOOL on;

		                                  if ($1.isEmpty || $1.parameter)
						      on = YES;
						  else
						      on = NO;
						  GSRTFunderline(CTXT, on, NSUnderlineStyleSingle | NSUnderlinePatternDot); }
		|	RTFunderlineDash	{ BOOL on;

		                                  if ($1.isEmpty || $1.parameter)
						      on = YES;
						  else
						      on = NO;
						  GSRTFunderline(CTXT, on, NSUnderlineStyleSingle | NSUnderlinePatternDash); }
		|	RTFunderlineDashDot	{ BOOL on;

		                                  if ($1.isEmpty || $1.parameter)
						      on = YES;
						  else
						      on = NO;
						  GSRTFunderline(CTXT, on, NSUnderlineStyleSingle | NSUnderlinePatternDashDot); }
		|	RTFunderlineDashDotDot	{ BOOL on;

		                                  if ($1.isEmpty || $1.parameter)
						      on = YES;
						  else
						      on = NO;
						  GSRTFunderline(CTXT, on, NSUnderlineStyleSingle | NSUnderlinePatternDashDotDot); }
		|	RTFunderlineDouble	{ BOOL on;

		                                  if ($1.isEmpty || $1.parameter)
						      on = YES;
						  else
						      on = NO;
						  GSRTFunderline(CTXT, on, NSUnderlineStyleDouble | NSUnderlinePatternSolid); }
|	RTFunderlineStop	{ GSRTFunderline(CTXT, NO, NSUnderlineStyleNone); }
		|	RTFunderlineThick	{ BOOL on;

		                                  if ($1.isEmpty || $1.parameter)
						      on = YES;
						  else
						      on = NO;
						  GSRTFunderline(CTXT, on, NSUnderlineStyleThick | NSUnderlinePatternSolid); }
		|	RTFunderlineThickDot	{ BOOL on;

		                                  if ($1.isEmpty || $1.parameter)
						      on = YES;
						  else
						      on = NO;
						  GSRTFunderline(CTXT, on, NSUnderlineStyleThick | NSUnderlinePatternDot); }
		|	RTFunderlineThickDash	{ BOOL on;

		                                  if ($1.isEmpty || $1.parameter)
						      on = YES;
						  else
						      on = NO;
						  GSRTFunderline(CTXT, on, NSUnderlineStyleThick | NSUnderlinePatternDash); }
		|	RTFunderlineThickDashDot	{ BOOL on;

		                                  if ($1.isEmpty || $1.parameter)
						      on = YES;
						  else
						      on = NO;
						  GSRTFunderline(CTXT, on, NSUnderlineStyleThick | NSUnderlinePatternDashDot); }
		|	RTFunderlineThickDashDotDot { BOOL on;

		                                  if ($1.isEmpty || $1.parameter)
						      on = YES;
						  else
						      on = NO;
						  GSRTFunderline(CTXT, on, NSUnderlineStyleThick | NSUnderlinePatternDashDotDot); }
		|	RTFunderlineWord	{ BOOL on;

		                                  if ($1.isEmpty || $1.parameter)
						      on = YES;
						  else
						      on = NO;
						  GSRTFunderline(CTXT, on, NSUnderlineStyleSingle | NSUnderlinePatternSolid | NSUnderlineByWordMask); }
|	RTFstrikethrough	{   NSInteger style;
   if ($1.isEmpty || $1.parameter)
     style = NSUnderlineStyleSingle | NSUnderlinePatternSolid;
   else
     style = NSUnderlineStyleNone;
   GSRTFstrikethrough(CTXT, style); }
|	RTFstrikethroughDouble	{ GSRTFstrikethrough(CTXT, NSUnderlineStyleDouble | NSUnderlinePatternSolid); }
		|	RTFunichar	        { GSRTFunicode(CTXT, $1.parameter); }
                |	RTFplain	        { GSRTFdefaultCharacterStyle(CTXT); }
                |	RTFparagraph	        { GSRTFparagraph(CTXT); }
                |	RTFrow   	        { GSRTFparagraph(CTXT); }
		|	RTFOtherStatement	{ GSRTFgenericRTFcommand(CTXT, $1); 
		                                  free((void*)$1.name); }
		;

rtfNeXTstuff: /* empty */
		|	rtfNeXTGraphic
		|	rtfNeXTHelpLink
		|	rtfNeXTHelpMarker
		;

/*
	NeXTGraphic (images)
	This is a Apple/NeXT extension. The format of the command is
	  {{\NeXTGraphic attachment \widthN \heightN} string}
	and the string is ignored (on OS X it is always 0xAC).
	See the Section "RTF Files and Attributed Strings" in Apple's
	Attributed Strings Programming Guide.
*/

rtfNeXTGraphic: '{' RTFNeXTGraphic RTFtext RTFNeXTGraphicWidth RTFNeXTGraphicHeight '}' { GSRTFopenBlock(CTXT, YES); } rtfIngredients { GSRTFcloseBlock(CTXT, YES); }
		{
			GSRTFNeXTGraphic (CTXT, $3, $4.parameter, $5.parameter);
		};

/*
	NeXTHelpLink
	This is a NeXT extension. The format of the command is
	  {{\NeXTHelpLinkN \markername markername; \linkFilename filename;
	    \linkMarkername markername;} string}
	and the string is ignored (it is always 0xAC on NeXT).
	Note that the {\NeXTHelpLink } group may itself be preceded by
	other commands.
*/

rtfNeXTHelpLink: '{' RTFNeXTHelpLink RTFNeXTmarkername RTFtext RTFNeXTlinkFilename RTFtext RTFNeXTlinkMarkername RTFtext '}' { GSRTFopenBlock(CTXT, YES); } rtfIngredients { GSRTFcloseBlock(CTXT, YES); }
		{
			GSRTFNeXTHelpLink (CTXT, $2.parameter, $4, $6, $8);
		};

/*
	NeXTHelpMarker
	This is a NeXT extension. The format of the command is
	  {{\NeXTHelpMarkerN \markername markername;} string}
	and the string is ignored (it is always 0xAC on NeXT).
	Note that the {\NeXTHelpLink } group may itself be preceded by
	other commands.
*/

rtfNeXTHelpMarker: '{' RTFNeXTHelpMarker RTFNeXTmarkername RTFtext '}' { GSRTFopenBlock(CTXT, YES); } rtfIngredients { GSRTFcloseBlock(CTXT, YES); }
		{
			GSRTFNeXTHelpMarker (CTXT, $2.parameter, $4);
		};

/*
	Font description
*/

rtfFontList: '{' RTFfontListStart rtfFonts '}'
		;

rtfFonts:
		|	rtfFonts rtfFontStatement
		|	rtfFonts '{' rtfFontStatement '}'
		|	rtfFonts '{' rtfFontStatement rtfBlock RTFtext '}'
                    { free((void *)$5);}
		;

/* the first RTFfont tags the font with a number */
/* RTFtext introduces the fontName */
rtfFontStatement:	RTFfont rtfFontFamily rtfFontAttrs RTFtext	{ GSRTFregisterFont(CTXT, $4, $2, $1.parameter);
                                                          free((void *)$4); }
/* fbidi should be a font family, but it seems to be used differently */
		|	RTFfont RTFfamilyBiDi rtfFontFamily rtfFontAttrs RTFtext	{ GSRTFregisterFont(CTXT, $5, $3, $1.parameter);
                                                          free((void *)$5); }
/* Theme fonts */
		|	RTFOtherStatement RTFfont RTFfamilyBiDi rtfFontFamily rtfFontAttrs RTFtext	{ GSRTFregisterFont(CTXT, $6, $4, $2.parameter);
                                                          free((void *)$6); }
		;

rtfFontAttrs: /* empty */
                | rtfFontAttrs RTFfcharset 
                | rtfFontAttrs RTFfprq
                | rtfFontAttrs RTFcpg
                | rtfFontAttrs RTFfttruetype
                | rtfFontAttrs rtfBlock
                ;


rtfFontFamily:
			RTFfamilyNil	{ $$ = RTFfamilyNil - RTFfamilyNil; }
		|	RTFfamilyRoman	{ $$ = RTFfamilyRoman - RTFfamilyNil; }
		|	RTFfamilySwiss	{ $$ = RTFfamilySwiss - RTFfamilyNil; }
		|	RTFfamilyModern	{ $$ = RTFfamilyModern - RTFfamilyNil; }
		|	RTFfamilyScript	{ $$ = RTFfamilyScript - RTFfamilyNil; }
		|	RTFfamilyDecor	{ $$ = RTFfamilyDecor - RTFfamilyNil; }
		|	RTFfamilyTech	{ $$ = RTFfamilyTech - RTFfamilyNil; }
		;


/*
	Colour definition
*/

rtfColorDef: '{' RTFcolortable rtfColors '}'
		;

rtfColors: /* empty */
 		|	rtfColors rtfColorStatement
		;

/* We get the ';' as RTFText */
rtfColorStatement: RTFred RTFgreen RTFblue RTFtext 
                     { 
		       GSRTFaddColor(CTXT, $1.parameter, $2.parameter, $3.parameter);
		       free((void *)$4);
		     }
 		|	RTFtext 
                     { 
		       GSRTFaddDefaultColor(CTXT);
		       free((void *)$1);
		     }
		;

/*
	some cludgy trailer
dummyNonTerminal: '\\' { @1.first_line; }	/ * we introduce a @n to fix the lex attributes * /
		;
*/

%%

/*	some C code here	*/

