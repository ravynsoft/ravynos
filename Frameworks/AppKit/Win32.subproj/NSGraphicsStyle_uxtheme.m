#import "NSGraphicsStyle_uxtheme.h"
#import <AppKit/NSGraphicsContext.h>
#import <Onyx2D/O2Context.h>
#import <Onyx2D/O2Surface.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSColor.h>
#import "Win32DeviceContextWindow.h"
#import <AppKit/NSInterfacePartAttributedString.h>
#import <AppKit/NSInterfacePartDisabledAttributedString.h>
#undef WINVER
#define WINVER 0x0501
#import <uxtheme.h>
#import <tmschema.h>

static void *functionWithName(const char *name){
   void              *result;
   static BOOL        lookForUxTheme=YES;
   static HANDLE      uxtheme=NULL;
   static NSMapTable *table=NULL;
   
   if(lookForUxTheme){
    if((uxtheme=LoadLibrary("UXTHEME"))!=NULL)
     table=NSCreateMapTable(NSObjectMapKeyCallBacks,NSNonOwnedPointerMapValueCallBacks,0);
     
    lookForUxTheme=NO;
   }
   
   if(table==NULL)
    result=NULL;
   else {
    NSString *string=[[NSString alloc] initWithCString:name];
    
    if((result=NSMapGet(table,string))==NULL){
     if((result=GetProcAddress(uxtheme,name))==NULL)
      NSLog(@"GetProcAddress(\"UXTHEME\",%s) FAILED",name);
     else
      NSMapInsert(table,string,result);
    }
    
    [string release];
   }
   
   return result;
}

static BOOL isThemeActive(){
   WINAPI BOOL (*function)(void)=functionWithName("IsThemeActive");
   
   if(function==NULL)
    return NO;
    
   return function();
}

HANDLE openThemeData(HWND window,LPCWSTR classList){
   WINAPI HANDLE (*function)(HWND,LPCWSTR)=functionWithName("OpenThemeData");
   
   if(function==NULL)
    return NULL;
   
   return function(window,classList);
}

void closeThemeData(HTHEME theme){
   WINAPI HRESULT (*function)(HTHEME)=functionWithName("CloseThemeData");
   
   if(function==NULL)
    return;
   
   if(function(theme)!=S_OK)
    NSLog(@"CloseThemeData failed");
}

static BOOL getThemePartSize(HTHEME theme,HDC dc,int partId,int stateId,LPCRECT prc,THEME_SIZE eSize,SIZE *size){
   WINAPI HRESULT (*function)(HTHEME,HDC,int,int,LPCRECT,THEME_SIZE,SIZE *)=functionWithName("GetThemePartSize");
   
   if(function==NULL)
    return NO;
   
   if(function(theme,dc,partId,stateId,prc,eSize,size)!=S_OK){
    NSLog(@"GetThemePartSize failed");
    return NO;
   }
   
   return YES;
}

static BOOL getThemeInt(HTHEME theme,int partId,int stateId,int propId,int *val){
	WINAPI HRESULT (*function)(HTHEME ,int,int,int,int*)=functionWithName("GetThemeInt");
	
	NSLog(@"getThemeInt = %p",function);
	if(function==NULL)
		return NO;
	
	if(function(theme,partId,stateId,propId,val)!=S_OK){
		NSLog(@"GetThemeInt failed");
		return NO;
	}
	
	return YES;
}

static BOOL getThemeMargins(HTHEME theme,HDC dc,int partId,int stateId,int propId,LPCRECT prc,MARGINS *margins){
	WINAPI HRESULT (*function)(HTHEME,HDC,int,int,int,LPCRECT,MARGINS *)=functionWithName("GetThemeMargins");
	
	if(function==NULL)
		return NO;
	
	if(function(theme,dc,partId,stateId,propId,prc,margins)!=S_OK){
		NSLog(@"GetThemeMargins failed");
		return NO;
	}
	
	return YES;
}

static BOOL drawThemeBackground(HTHEME theme,HDC dc,int partId,int stateId,const RECT *rect,const RECT *clip){
   WINAPI HRESULT (*function)(HTHEME,HDC,int,int,const RECT *,const RECT *)=functionWithName("DrawThemeBackground");
   
   if(function==NULL)
    return NO;
   
   if(function(theme,dc,partId,stateId,rect,clip)!=S_OK){
    NSLog(@"DrawThemeBackground(%x,%x,%d,%d,{%d %d %d %d}) failed",theme,dc,partId,stateId,rect->top,rect->left,rect->bottom,rect->right);
    return NO;
   }
   return YES;
}

@interface NSGraphicsStyle_uxtheme(Private)

-(HANDLE)themeForClassList:(LPCWSTR)classList deviceContext:(O2DeviceContext_gdi *)deviceContext;
-(O2Context *)context;
-(O2DeviceContext_gdi *)deviceContext;
-(BOOL)marginsOfPartId:(int)partId stateId:(int)stateId uxthClassId:(int)uxthClassId margins:(Margins *)result;
-(BOOL)sizeOfPartId:(int)partId stateId:(int)stateId uxthClassId:(int)uxthClassId size:(NSSize *)result;
-(BOOL)drawPartId:(int)partId stateId:(int)stateId uxthClassId:(int)uxthClassId inRect:(NSRect)rect;
-(BOOL)drawButtonPartId:(int)partId stateId:(int)stateId inRect:(NSRect)rect;
@end

@implementation NSGraphicsStyle_uxtheme

static NSDictionary *sNormalMenuTextAttributes = nil;
static NSDictionary *sDimmedMenuTextAttributes = nil;

+ (void)initialize
{
	if (sNormalMenuTextAttributes == nil)
	{
		NSFont *menuFont = [NSFont menuFontOfSize:0];
		sNormalMenuTextAttributes = [[NSDictionary dictionaryWithObjectsAndKeys:
									 menuFont,NSFontAttributeName,
									 [NSColor menuItemTextColor],NSForegroundColorAttributeName,
									 nil] retain];
		sDimmedMenuTextAttributes = [[NSDictionary dictionaryWithObjectsAndKeys:
									 menuFont,NSFontAttributeName,
									 [NSColor grayColor],NSForegroundColorAttributeName,
									 nil] retain];
	}
}

@end

@implementation NSGraphicsStyle(uxtheme)

+allocWithZone:(NSZone *)zone {
	if(isThemeActive())
		return NSAllocateObject([NSGraphicsStyle_uxtheme class],0,zone);
	
	return [super allocWithZone:zone];
}

@end


@implementation NSGraphicsStyle_uxtheme(Private)

-(HANDLE)themeForClassList:(LPCWSTR)classList deviceContext:(O2DeviceContext_gdi *)deviceContext  {
   HWND windowHandle=[[deviceContext windowDeviceContext] windowHandle];
   
   if(windowHandle==NULL)
    return NULL;
    
   return openThemeData(windowHandle,classList);
}

static inline RECT transformToRECT(O2AffineTransform matrix,NSRect rect) {
   RECT    result;
   NSPoint point1=O2PointApplyAffineTransform(rect.origin,matrix);
   NSPoint point2=O2PointApplyAffineTransform(NSMakePoint(NSMaxX(rect),NSMaxY(rect)),matrix);

   if(point2.y<point1.y){
    float temp=point2.y;
    point2.y=point1.y;
    point1.y=temp;
   }

   result.top=point1.y;
   result.left=point1.x;
   result.bottom=point2.y;
   result.right=point2.x;
   
   return result;
}

-(O2Context *)context {
   O2Context *context=[[NSGraphicsContext currentContext] graphicsPort];
   
   return context;
}

-(O2Surface *)surface  {
   return [[self context] surface];
}

-(O2DeviceContext_gdi *)deviceContext {
   O2Context *context=[[NSGraphicsContext currentContext] graphicsPort];
   
   if([context respondsToSelector:@selector(deviceContext)]){
    O2DeviceContext_gdi *result=[context performSelector:@selector(deviceContext)];
    
    if([result isKindOfClass:[O2DeviceContext_gdi class]])
     return result;
   }
   
   return nil;
}

-(BOOL)marginsOfPartId:(int)partId stateId:(int)stateId uxthClassId:(int)uxthClassId margins:(Margins *)result {
	O2DeviceContext_gdi *deviceContext=[self deviceContext];
	HANDLE               theme;
	
	if(deviceContext==nil)
		return NO;
    
	if((theme=[[deviceContext windowDeviceContext] theme:uxthClassId])!=NULL){
		MARGINS margins;
		
		if (getThemeMargins(theme,[deviceContext dc],partId,stateId,/*TMT_CONTENTMARGINS*/3602,NULL,&margins)) {
			result->left = margins.cxLeftWidth;
			result->right = margins.cxRightWidth;
			result->top = margins.cyTopHeight;
			result->bottom = margins.cyBottomHeight;
		}
		return YES;
	}
	return NO;
}


-(BOOL)valueForPartId:(int)partId stateId:(int)stateId propId:(int)propId uxthClassId:(int)uxthClassId value:(int *)result {
	O2DeviceContext_gdi *deviceContext=[self deviceContext];
	HANDLE               theme;
	
	if(deviceContext==nil)
		return NO;
    
	if((theme=[[deviceContext windowDeviceContext] theme:uxthClassId])!=NULL){
		
		getThemeInt(theme,partId,stateId,propId,result);
		return YES;
	}
	return NO;
}

-(BOOL)sizeOfPartId:(int)partId stateId:(int)stateId uxthClassId:(int)uxthClassId size:(NSSize *)result {
	O2DeviceContext_gdi *deviceContext=[self deviceContext];
	HANDLE               theme;
	
	if(deviceContext==nil)
		return NO;
    
	if((theme=[[deviceContext windowDeviceContext] theme:uxthClassId])!=NULL){
		SIZE size;
		
		if(getThemePartSize(theme,[deviceContext dc],partId,stateId,NULL,TS_DRAW,&size)){
			result->width=size.cx;
			result->height=size.cy;
			// should invert translate here
		}    
		return YES;
	}
	return NO;
}

-(BOOL)drawPartId:(int)partId stateId:(int)stateId uxthClassId:(int)uxthClassId inRect:(NSRect)rect {
   O2DeviceContext_gdi *deviceContext=[self deviceContext];
   HANDLE               theme;
   
   if(deviceContext==nil)
    return NO;
       
   if((theme=[[deviceContext windowDeviceContext] theme:uxthClassId])!=NULL){
    O2AffineTransform matrix;
    RECT tlbr;

    matrix=O2ContextGetUserSpaceToDeviceSpaceTransform([self context]);
    tlbr=transformToRECT(matrix,rect);

    O2Surface *surface=[self surface];
    
    O2SurfaceLock(surface);
    drawThemeBackground(theme,[deviceContext dc],partId,stateId,&tlbr,NULL);
    O2SurfaceUnlock(surface);
    
    return YES;
   }
   return NO;
}

-(BOOL)drawButtonPartId:(int)partId stateId:(int)stateId inRect:(NSRect)rect {
   return [self drawPartId:partId stateId:stateId uxthClassId:uxthBUTTON inRect:rect];
}

@end

@implementation NSGraphicsStyle_uxtheme (NSMenu)

/*
 //
 //  MENUSTYLE class parts and states 
 //
 #define VSCLASS_MENUSTYLE	L"MENUSTYLE"
 #define VSCLASS_MENU	L"MENU"
 
 enum MENUPARTS {
 MENU_MENUITEM_TMSCHEMA = 1,
 MENU_MENUDROPDOWN_TMSCHEMA = 2,
 MENU_MENUBARITEM_TMSCHEMA = 3,
 MENU_MENUBARDROPDOWN_TMSCHEMA = 4,
 MENU_CHEVRON_TMSCHEMA = 5,
 MENU_SEPARATOR_TMSCHEMA = 6,
 MENU_BARBACKGROUND = 7,
 MENU_BARITEM = 8,
 MENU_POPUPBACKGROUND = 9,
 MENU_POPUPBORDERS = 10,
 MENU_POPUPCHECK = 11,
 MENU_POPUPCHECKBACKGROUND = 12,
 MENU_POPUPGUTTER = 13,
 MENU_POPUPITEM = 14,
 MENU_POPUPSEPARATOR = 15,
 MENU_POPUPSUBMENU = 16,
 MENU_SYSTEMCLOSE = 17,
 MENU_SYSTEMMAXIMIZE = 18,
 MENU_SYSTEMMINIMIZE = 19,
 MENU_SYSTEMRESTORE = 20,
 };
 
 #define MENUSTYLEPARTS MENUPARTS;
 
 enum BARBACKGROUNDSTATES {
 MB_ACTIVE = 1,
 MB_INACTIVE = 2,
 };
 
 enum BARITEMSTATES {
 MBI_NORMAL = 1,
 MBI_HOT = 2,
 MBI_PUSHED = 3,
 MBI_DISABLED = 4,
 MBI_DISABLEDHOT = 5,
 MBI_DISABLEDPUSHED = 6,
 };
 
 enum POPUPCHECKSTATES {
 MC_CHECKMARKNORMAL = 1,
 MC_CHECKMARKDISABLED = 2,
 MC_BULLETNORMAL = 3,
 MC_BULLETDISABLED = 4,
 };
 
 enum POPUPCHECKBACKGROUNDSTATES {
 MCB_DISABLED = 1,
 MCB_NORMAL = 2,
 MCB_BITMAP = 3,
 };
 
 enum POPUPITEMSTATES {
 MPI_NORMAL = 1,
 MPI_HOT = 2,
 MPI_DISABLED = 3,
 MPI_DISABLEDHOT = 4,
 };
 
 enum POPUPSUBMENUSTATES {
 MSM_NORMAL = 1,
 MSM_DISABLED = 2,
 };
 
 enum SYSTEMCLOSESTATES {
 MSYSC_NORMAL = 1,
 MSYSC_DISABLED = 2,
 };
 
 enum SYSTEMMAXIMIZESTATES {
 MSYSMX_NORMAL = 1,
 MSYSMX_DISABLED = 2,
 };
 
 enum SYSTEMMINIMIZESTATES {
 MSYSMN_NORMAL = 1,
 MSYSMN_DISABLED = 2,
 };
 
 enum SYSTEMRESTORESTATES {
 MSYSR_NORMAL = 1,
 MSYSR_DISABLED = 2,
 };
 
 
 */
#pragma mark -
#pragma mark Menu Metrics

#define BRANCH_ARROW_LEFT_MARGIN 2
#define BRANCH_ARROW_RIGHT_MARGIN 2

-(NSSize)menuItemSeparatorSize {
	NSSize result = NSZeroSize;

    if(![self sizeOfPartId:/*MENU_POPUPSEPARATOR*/15 stateId:0 uxthClassId:uxthMENU size:&result])
		return [super menuItemSeparatorSize];
	else if (result.height == 0.0f)
		result.height = 6.0f;	// Why I don't know, but this doesn't return a non-zero value on Win7.

	return result;
}

-(Margins)menuItemTextMargins {
	Margins result;
    if(![self marginsOfPartId:/*MENU_POPUPITEM*/14 stateId:0 uxthClassId:uxthMENU margins:&result])
		return [super menuItemTextMargins];
	else
		return result;
}

-(Margins)menuItemBranchArrowMargins {
	Margins result = [self menuItemTextMargins];
	
	result.left = BRANCH_ARROW_LEFT_MARGIN;
	result.right = BRANCH_ARROW_RIGHT_MARGIN;
	
	return result;
}

-(NSSize)menuItemBranchArrowSize {
	NSSize result = NSZeroSize;
	Margins margins = [self menuItemBranchArrowMargins];

	if(![self sizeOfPartId:/*MENU_POPUPSUBMENU*/16 stateId:0 uxthClassId:uxthMENU size:&result])
		result = [super menuItemBranchArrowSize];

	result.height += (margins.top + margins.bottom);
	result.width += (margins.left + margins.right);

	return result;
}

-(NSSize)menuItemCheckMarkSize {
	NSSize result = NSZeroSize;
	
	if(![self sizeOfPartId:/*MENU_POPUPCHECK*/11 stateId:0 uxthClassId:uxthMENU size:&result])
		return [super menuItemCheckMarkSize];
	else
		return result;
}

-(Margins)menuItemGutterMargins {
	Margins result;
	
    if(![self marginsOfPartId:/*MENU_POPUPCHECK*/11 stateId:0 uxthClassId:uxthMENU margins:&result])
		return [super menuItemGutterMargins];
	else
		return result;
}

-(NSSize)menuItemGutterSize {
	NSSize result = NSZeroSize;
	Margins margins = [self menuItemGutterMargins];
	
	result = [self menuItemCheckMarkSize];

	result.height += (margins.top + margins.bottom);
	result.width += (margins.left + margins.right);
	
	return result;
}

-(NSSize)menuItemTextSize:(NSString *)title {
	NSSize result = NSZeroSize;
	Margins margins = [self menuItemTextMargins];
	result = [title sizeWithAttributes:sNormalMenuTextAttributes];
    
	result.height += (margins.top + margins.bottom);
	result.width += (margins.left + margins.right);
	
	return result;
}

-(NSSize)menuItemAttributedTextSize:(NSAttributedString *)title
{
	NSSize result = NSZeroSize;
	Margins margins = [self menuItemTextMargins];
    
	result = [title size];
	
	result.height += (margins.top + margins.bottom);
	result.width += (margins.left + margins.right);
	
	return result;
}

-(float)menuBarHeight
{
	int result = 0;
    if(![self valueForPartId:/*MENU_BARBACKGROUND*/7 stateId:0 propId:/*TMT_MENUBARHEIGHT*/1209 uxthClassId:uxthMENU value:&result])
		return [super menuBarHeight];
	else
		return (float)result;
}

-(float)menuItemGutterGap
{
	return 11;
}

#pragma mark -

-(void)drawMenuSeparatorInRect:(NSRect)rect {
	if(![self drawPartId:/*MENU_POPUPSEPARATOR*/15 stateId:MS_NORMAL uxthClassId:uxthMENU inRect:rect])
		[super drawMenuSeparatorInRect:rect];
}

-(void)drawMenuGutterInRect:(NSRect)rect {
	if(![self drawPartId:/*MENU_POPUPGUTTER*/13 stateId:1 uxthClassId:uxthMENU inRect:rect])
		[super drawMenuGutterInRect:rect];
}

-(void)drawMenuCheckmarkInRect:(NSRect)rect enabled:(BOOL)enabled selected:(BOOL)selected {
	Margins margins=[self menuItemGutterMargins];
	int     state;
	
	if (enabled)
		state = selected ? /*MBI_HOT*/2 : /*MBI_NORMAL*/1;
	else
		state = selected ? /*MBI_DISABLEDHOT*/5 : /*MBI_DISABLED*/4;


	if(![self drawPartId:/*MENU_POPUPCHECKBACKGROUND*/12 stateId:state uxthClassId:uxthMENU inRect:rect])
		[super drawMenuCheckmarkInRect:rect enabled:enabled selected:selected];
	
	NSRect themeRect = rect;
	themeRect.origin.x += margins.left;
	themeRect.origin.y += margins.top;
	themeRect.size.width -= (margins.left + margins.right);
	themeRect.size.height -= (margins.top + margins.bottom);
	if(![self drawPartId:/*MENU_POPUPCHECK*/11 stateId:state uxthClassId:uxthMENU inRect:themeRect])
		[super drawMenuCheckmarkInRect:rect enabled:enabled selected:selected];
}

-(void)drawMenuItemText:(NSString *)string inRect:(NSRect)rect enabled:(BOOL)enabled selected:(BOOL)selected {
	O2DeviceContext_gdi *deviceContext=[self deviceContext];	
	if(deviceContext==nil)
		return;

    // Ensure we have enough width - fractional widths give float comparison trouble
    rect.size.width = ceilf(rect.size.width);

	if ([[deviceContext windowDeviceContext] theme:uxthMENU])
	{

		Margins margins=[self menuItemTextMargins];
		
		rect.origin.x += margins.left;
		rect.origin.y += margins.top;
		rect.size.width -= (margins.left + margins.right);
		rect.size.height -= (margins.top + margins.bottom);
		
		rect.origin.y += (rect.size.height - [string sizeWithAttributes:sNormalMenuTextAttributes].height) / 2;
		
		if (enabled)
		{
			[string drawInRect:rect withAttributes:sNormalMenuTextAttributes];
		}
		else
		{
			[string drawInRect:rect withAttributes:sDimmedMenuTextAttributes];
		}
	}
	else {
		[super drawMenuItemText:string inRect:rect enabled:enabled selected:selected];
    }
}

-(void)drawMenuBranchArrowInRect:(NSRect)rect enabled:(BOOL)enabled selected:(BOOL)selected {
	Margins margins=[self menuItemBranchArrowMargins];
	NSRect  themeRect = rect;
	int     state = enabled ? /*MBI_NORMAL*/1 : /*MBI_DISABLED*/4;
	
	themeRect.origin.x += margins.left;
	themeRect.origin.y += margins.top;
	themeRect.size.width -= (margins.left + margins.right);
	themeRect.size.height -= (margins.top + margins.bottom);

	if(![self drawPartId:/*MENU_POPUPSUBMENU*/16 stateId:state uxthClassId:uxthMENU inRect:themeRect])
		[super drawMenuBranchArrowInRect:rect enabled:enabled selected:selected];
}

-(void)drawMenuSelectionInRect:(NSRect)rect enabled:(BOOL)enabled {
	O2DeviceContext_gdi *deviceContext=[self deviceContext];
	SetBkColor([deviceContext dc], RGB(251,245,55));
	
	if(![self drawPartId:/*MENU_POPUPITEM*/14 stateId:enabled ? /*MPI_HOT*/2 : /*MPI_DISABLEDHOT*/4 uxthClassId:uxthMENU inRect:rect])
		[super drawMenuSelectionInRect:rect enabled:enabled];
}

-(void)drawMenuWindowBackgroundInRect:(NSRect)rect {
	if(![self drawPartId:/*MENU_POPUPBACKGROUND*/9 stateId:0 uxthClassId:uxthMENU inRect:rect])
		[super drawMenuWindowBackgroundInRect:rect];
	else
		[self drawPartId:/*MENU_POPUPBORDERS*/10 stateId:0 uxthClassId:uxthMENU inRect:rect];
}

-(void)drawMenuBarItemBorderInRect:(NSRect)rect hover:(BOOL)hovering selected:(BOOL)selected {
	int state = /*MBI_NORMAL*/1;
	if (hovering)
		state = selected ?/*MBI_PUSHED*/3 : /*MBI_HOT*/2;
	
	if(![self drawPartId:/*MENU_BARITEM*/8 stateId:state uxthClassId:uxthMENU inRect:rect])
		[super drawMenuBarItemBorderInRect:rect hover:hovering selected:selected];
}

-(void)drawMenuBarBackgroundInRect:(NSRect)rect {
	if (![self drawPartId:/*MENU_BARBACKGROUND*/7 stateId:1 uxthClassId:uxthMENU inRect:rect])
		[super drawMenuBarBackgroundInRect:rect];
}

@end

@implementation NSGraphicsStyle_uxtheme (NSButton)

-(void)drawPushButtonNormalInRect:(NSRect)rect defaulted:(BOOL)defaulted {
   if(![self drawButtonPartId:BP_PUSHBUTTON stateId:defaulted?PBS_DEFAULTED:PBS_NORMAL inRect:rect])
    [super drawPushButtonNormalInRect:rect defaulted:defaulted];
}

-(void)drawPushButtonPressedInRect:(NSRect)rect {
   if(![self drawButtonPartId:BP_PUSHBUTTON stateId:PBS_PRESSED inRect:rect])
    [super drawPushButtonPressedInRect:rect];
}

-(BOOL)getPartId:(int *)partId stateId:(int *)stateId forButtonImage:(NSImage *)image enabled:(BOOL)enabled mixed:(BOOL)mixed {
   BOOL valid=NO;

   if([[image name] isEqual:@"NSSwitch"]){
    *partId=BP_CHECKBOX;
    *stateId=enabled?CBS_UNCHECKEDNORMAL:CBS_UNCHECKEDDISABLED;
    valid=YES;
   }
   else if([[image name] isEqual:@"NSHighlightedSwitch"]){
    *partId=BP_CHECKBOX;
    *stateId=mixed?(enabled?CBS_MIXEDNORMAL:CBS_MIXEDDISABLED):(enabled?CBS_CHECKEDNORMAL:CBS_CHECKEDDISABLED);
    valid=YES;
   }
   else if([[image name] isEqual:@"NSRadioButton"]){
    *partId=BP_RADIOBUTTON;
    *stateId=enabled?RBS_UNCHECKEDNORMAL:RBS_UNCHECKEDDISABLED;
    valid=YES;
   }
   else if([[image name] isEqual:@"NSHighlightedRadioButton"]){
    *partId=BP_RADIOBUTTON;
    *stateId=enabled?RBS_CHECKEDNORMAL:RBS_CHECKEDDISABLED;
    valid=YES;
   }
   return valid;
}

-(NSSize)sizeOfButtonImage:(NSImage *)image enabled:(BOOL)enabled mixed:(BOOL)mixed {
   int partId,stateId;
   
   if([self getPartId:&partId stateId:&stateId forButtonImage:image enabled:enabled mixed:mixed]){
    NSSize result;
    
    if([self sizeOfPartId:partId stateId:stateId uxthClassId:uxthBUTTON size:&result])
     return result;
   }
   
   return [super sizeOfButtonImage:image enabled:enabled mixed:mixed];
}

-(void)drawButtonImage:(NSImage *)image inRect:(NSRect)rect enabled:(BOOL)enabled mixed:(BOOL)mixed {
   int partId,stateId;
   
   if([self getPartId:&partId stateId:&stateId forButtonImage:image enabled:enabled mixed:mixed])
    if([self drawButtonPartId:partId stateId:stateId inRect:rect])
     return;

   [super drawButtonImage:image inRect:rect enabled:enabled mixed:mixed];
}

@end

@implementation NSGraphicsStyle_uxtheme (NSPopUpButton)

-(void)drawPopUpButtonWindowBackgroundInRect:(NSRect)rect {
#if 0
   if(![self drawPartId:MENU_POPUPBORDERS stateId:0 uxthClassId:uxthMENU inRect:rect])
    [super drawPopUpButtonWindowBackgroundInRect:rect];
#else
   [[NSColor menuBackgroundColor] setFill];
   NSRectFill(rect);
   [[NSColor blackColor] setStroke];
   NSFrameRect(rect);
#endif
}

@end

@implementation NSGraphicsStyle_uxtheme (NSOutlineView)

-(void)drawOutlineViewBranchInRect:(NSRect)rect expanded:(BOOL)expanded {
   if(![self drawPartId:TVP_GLYPH stateId:expanded?GLPS_OPENED:GLPS_CLOSED uxthClassId:uxthTREEVIEW inRect:rect])
    [super drawOutlineViewBranchInRect:rect expanded:expanded];
}

-(NSRect)drawProgressIndicatorBackground:(NSRect)rect clipRect:(NSRect)clipRect bezeled:(BOOL)bezeled {
   if(bezeled){
    if([self drawPartId:PP_BAR stateId:0 uxthClassId:uxthPROGRESS inRect:rect])
     return NSInsetRect(rect,3,3);
   }

   return [super drawProgressIndicatorBackground:rect clipRect:clipRect bezeled:bezeled];
}

@end

@implementation NSGraphicsStyle_uxtheme (NSProgressIndicator)

-(void)drawProgressIndicatorChunk:(NSRect)rect {
   [self drawPartId:PP_CHUNK stateId:0 uxthClassId:uxthPROGRESS inRect:rect];
}

@end

@implementation NSGraphicsStyle_uxtheme (NSScroller)

-(void)drawScrollerButtonInRect:(NSRect)rect enabled:(BOOL)enabled pressed:(BOOL)pressed vertical:(BOOL)vertical upOrLeft:(BOOL)upOrLeft {
   int stateId;
   
   if(vertical){
    if(upOrLeft)
     stateId=enabled?(pressed?ABS_UPPRESSED:ABS_UPNORMAL):ABS_UPDISABLED;
    else
     stateId=enabled?(pressed?ABS_DOWNPRESSED:ABS_DOWNNORMAL):ABS_DOWNDISABLED;
    }
   else {
    if(upOrLeft)
     stateId=enabled?(pressed?ABS_LEFTPRESSED:ABS_LEFTNORMAL):ABS_LEFTDISABLED;
    else
     stateId=enabled?(pressed?ABS_RIGHTPRESSED:ABS_RIGHTNORMAL):ABS_RIGHTDISABLED;
   }
   
   if(![self drawPartId:SBP_ARROWBTN stateId:stateId uxthClassId:uxthSCROLLBAR inRect:rect])
    [super drawScrollerButtonInRect:rect enabled:enabled pressed:pressed vertical:vertical upOrLeft:upOrLeft];
}

-(void)drawScrollerKnobInRect:(NSRect)rect vertical:(BOOL)vertical highlight:(BOOL)highlight {
   if(![self drawPartId:vertical?SBP_THUMBBTNVERT:SBP_THUMBBTNHORZ stateId:highlight?SCRBS_PRESSED:SCRBS_NORMAL uxthClassId:uxthSCROLLBAR inRect:rect])
    [super drawScrollerKnobInRect:rect vertical:vertical highlight:highlight];

   [self drawPartId:vertical?SBP_GRIPPERVERT:SBP_GRIPPERHORZ stateId:0 uxthClassId:uxthSCROLLBAR inRect:rect];
}

-(void)drawScrollerTrackInRect:(NSRect)rect vertical:(BOOL)vertical upOrLeft:(BOOL)upOrLeft {
   int partId=vertical?(upOrLeft?SBP_UPPERTRACKVERT:SBP_LOWERTRACKVERT):(upOrLeft?SBP_UPPERTRACKHORZ:SBP_LOWERTRACKHORZ);
   
   if(![self drawPartId:partId stateId:SCRBS_NORMAL uxthClassId:uxthSCROLLBAR inRect:rect])
    [super drawScrollerTrackInRect:rect vertical:vertical upOrLeft:upOrLeft];
}

@end

@implementation NSGraphicsStyle_uxtheme (NSTableView)

-(void)drawTableViewHeaderInRect:(NSRect)rect highlighted:(BOOL)highlighted {
   rect.origin.y    -= 1.0;
   rect.size.height += 1.0;
   if(![self drawPartId:HP_HEADERITEM stateId:highlighted?HIS_PRESSED:HIS_NORMAL uxthClassId:uxthHEADER inRect:rect])
    [super drawTableViewHeaderInRect:rect highlighted:highlighted];
}

-(void)drawTableViewCornerInRect:(NSRect)rect {
   rect.origin.y    -= 1.0;
   rect.size.height += 1.0;
   if(![self drawPartId:HP_HEADERITEM stateId:HIS_NORMAL uxthClassId:uxthHEADER inRect:rect])
    [super drawTableViewCornerInRect:rect];
}

@end

@implementation NSGraphicsStyle_uxtheme (NSComboBox)

-(void)drawComboBoxButtonInRect:(NSRect)rect enabled:(BOOL)enabled bordered:(BOOL)bordered pressed:(BOOL)pressed {
   if(![self drawPartId:CP_DROPDOWNBUTTON stateId:enabled?(pressed?CBXS_PRESSED:CBXS_NORMAL):CBXS_DISABLED uxthClassId:uxthCOMBOBOX inRect:rect])
    [super drawComboBoxButtonInRect:rect enabled:(BOOL)enabled bordered:bordered pressed:pressed];
}

@end

@implementation NSGraphicsStyle_uxtheme (NSSlider)

-(void)drawSliderKnobInRect:(NSRect)rect vertical:(BOOL)vertical highlighted:(BOOL)highlighted hasTickMarks:(BOOL)hasTickMarks tickMarkPosition:(NSTickMarkPosition)tickMarkPosition {
   int partId;
   
   if(vertical){
    if(!hasTickMarks)
     partId=TKP_THUMBVERT;
    else if(tickMarkPosition==NSTickMarkLeft)
     partId=TKP_THUMBLEFT;
    else
     partId=TKP_THUMBRIGHT;
   }
   else {
    if(!hasTickMarks)
     partId=TKP_THUMB;
    else if(tickMarkPosition==NSTickMarkAbove)
     partId=TKP_THUMBTOP;
    else
     partId=TKP_THUMBBOTTOM;
   }
      
   if(![self drawPartId:partId stateId:highlighted?TUS_PRESSED:TUS_NORMAL uxthClassId:uxthTRACKBAR inRect:rect])
    [super drawSliderKnobInRect:rect vertical:vertical highlighted:highlighted hasTickMarks:hasTickMarks tickMarkPosition:tickMarkPosition];
}

-(void)drawSliderTrackInRect:(NSRect)rect vertical:(BOOL)vertical hasTickMarks:(BOOL)hasTickMarks {
   NSRect thin=rect;
   
   if(hasTickMarks){
    if(vertical){
     thin.origin.x+=(thin.size.width-4)/2;
     thin.size.width=4;
    }
    else {
     thin.origin.y+=(thin.size.height-4)/2;
     thin.size.height=4;
    }
   }
   
   if(![self drawPartId:vertical?TKP_TRACKVERT:TKP_TRACK stateId:TRS_NORMAL uxthClassId:uxthTRACKBAR inRect:thin])
    [super drawSliderTrackInRect:rect vertical:vertical hasTickMarks:hasTickMarks];
}

@end

@implementation NSGraphicsStyle_uxtheme (NSStepper)


-(void)drawStepperButtonInRect:(NSRect)rect clipRect:(NSRect)clipRect enabled:(BOOL)enabled highlighted:(BOOL)highlighted upNotDown:(BOOL)upNotDown {
   if(![self drawPartId:upNotDown?SPNP_UP:SPNP_DOWN stateId:enabled?DNS_NORMAL:DNS_DISABLED uxthClassId:uxthSPIN inRect:rect])
    [super drawStepperButtonInRect:rect clipRect:(NSRect)clipRect enabled:enabled highlighted:highlighted upNotDown:upNotDown];
}

@end

@implementation NSGraphicsStyle_uxtheme (NSTabView)

-(void)drawTabInRect:(NSRect)rect clipRect:(NSRect)clipRect color:(NSColor *)color selected:(BOOL)selected {   
   rect.origin.y-=1;
   
   if(!selected)
    rect.origin.y-=2;
    
   if(![self drawPartId:TABP_TABITEM stateId:selected?TIS_SELECTED:TIS_NORMAL uxthClassId:uxthTAB inRect:rect])
    [super drawTabInRect:rect clipRect:clipRect color:color selected:selected];
}

-(void)drawTabPaneInRect:(NSRect)rect {
   if(![self drawPartId:TABP_PANE stateId:TIS_NORMAL uxthClassId:uxthTAB inRect:rect])
    [super drawTabPaneInRect:rect];
}

-(void)drawTabViewBackgroundInRect:(NSRect)rect {
   if(![self drawPartId:TABP_BODY stateId:TIS_NORMAL uxthClassId:uxthTAB inRect:rect])
    [super drawTabPaneInRect:rect];
}

@end

@implementation NSGraphicsStyle_uxtheme (NSTextField)

-(void)drawTextFieldBorderInRect:(NSRect)rect bezeledNotLine:(BOOL)bezeledNotLine {
   if(![self drawPartId:EP_EDITTEXT stateId:ETS_NORMAL uxthClassId:uxthEDIT inRect:rect])
    [super drawTextFieldBorderInRect:rect bezeledNotLine:bezeledNotLine];
}

-(void)drawBoxWithBezelInRect:(NSRect)rect clipRect:(NSRect)clipRect {
   if(![self drawPartId:BP_GROUPBOX stateId:GBS_NORMAL uxthClassId:uxthBUTTON inRect:rect])
    [super drawBoxWithBezelInRect:rect clipRect:clipRect];
}

-(void)drawBoxWithGrooveInRect:(NSRect)rect clipRect:(NSRect)clipRect {
   if(![self drawPartId:BP_GROUPBOX stateId:GBS_NORMAL uxthClassId:uxthBUTTON inRect:rect])
    [super drawBoxWithGrooveInRect:rect clipRect:clipRect];
}

@end
