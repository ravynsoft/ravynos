/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSSavePanel-Win32.h>
#import <AppKit/NSApplication.h>
#import <AppKit/Win32Display.h>
#import <AppKit/Win32Window.h>
#import <AppKit/NSWindow-Private.h>
#import <malloc.h>

#import <Foundation/NSString_win32.h>

@implementation NSSavePanel(Win32)

static unsigned *saveFileHook(HWND hdlg,UINT uiMsg,WPARAM wParam,LPARAM lParam) {
   static BOOL pastInitialFolderChange;

   if(uiMsg==WM_INITDIALOG)
    pastInitialFolderChange=NO;

   if(uiMsg==WM_NOTIFY){
    OFNOTIFYW *notify=(void *)lParam;

    if(notify->hdr.code==CDN_FOLDERCHANGE){
     if(pastInitialFolderChange){ // we get one FOLDERCHANGE right after init, ignore it
      NSString *type=(NSString *)notify->lpOFN->lCustData;
      unichar      folder[MAX_PATH+1];
      int       length=SendMessage(GetParent(hdlg),CDM_GETFOLDERPATH,MAX_PATH,(LPARAM)folder)-1;

      if(length>0){
       NSString *file=[NSString stringWithCharacters:folder length:length];
       NSString *extension=[file pathExtension];

       if([type length]>0 && [type isEqualToString:extension]){
        int result=NSRunAlertPanel(nil,@"%@ already exists. Do you want to replace it?",@"Yes",@"No",nil,file);

        if(result==NSAlertDefaultReturn){
         notify->lpOFN->lCustData=0xFFFFFFFF;
         wcscpy(notify->lpOFN->lpstrFile,folder);
         PostMessage(GetParent(hdlg),WM_SYSCOMMAND,SC_CLOSE,0); 
        }
       }
      }
     }
     pastInitialFolderChange=YES;
    }
   }

   return NULL;
}

-(int)_GetOpenFileName {
	OPENFILENAMEW openFileName={0};
	int          check;

	@synchronized(self)
	{
		unichar filename[1025]=L"";
		NSString *ext = (_requiredFileType && [_requiredFileType length] > 0) ? _requiredFileType : (NSString *)@"*";
		NSString *first = [[[NSString alloc] initWithFormat:@"Document (*.%@)", ext] autorelease];
		NSString *second = [[[NSString alloc] initWithFormat:@"*.%@", ext] autorelease];
		size_t firstLength = [first cStringLength];
		size_t secondLength = [second cStringLength];
		size_t fileTypesLength = (firstLength + secondLength + 3);
		unichar *fileTypes = (unichar *) alloca(sizeof(unichar) * fileTypesLength);

		fileTypes[firstLength] = 0;
		fileTypes[fileTypesLength - 2] = 0;
		fileTypes[fileTypesLength - 1] = 0;
		wcscpy(fileTypes, [first fileSystemRepresentationW]);
		wcscpy(fileTypes + firstLength + 1, [second fileSystemRepresentationW]);

		openFileName.lStructSize=sizeof(OPENFILENAMEW);
		openFileName.hwndOwner=[(Win32Window *)[[NSApp keyWindow] platformWindow] windowHandle];
		openFileName.hInstance=NULL;
		openFileName.lpstrFilter=fileTypes;
		openFileName.lpstrCustomFilter=NULL;
		openFileName.nMaxCustFilter=0;
		openFileName.nFilterIndex=1;
		wcsncpy(filename,[_filename fileSystemRepresentationW],1024);
		openFileName.lpstrFile=filename;
		openFileName.nMaxFile=1024;
		openFileName.lpstrFileTitle=NULL;
		openFileName.nMaxFileTitle=0;
		openFileName.lpstrInitialDir=[_directory fileSystemRepresentationW];
		openFileName.lpstrTitle=(const unichar *)[_dialogTitle cStringUsingEncoding:NSUnicodeStringEncoding];
		openFileName.Flags=
		OFN_CREATEPROMPT|
		OFN_NOTESTFILECREATE|
		OFN_EXPLORER|
		OFN_HIDEREADONLY|
		OFN_OVERWRITEPROMPT|
		OFN_ENABLEHOOK|
        OFN_ENABLESIZING
		;
		openFileName.nFileOffset=0;
		openFileName.nFileExtension=0;
        if ([_requiredFileType length] > 0) {
            openFileName.lpstrDefExt = (const unichar *)[_requiredFileType cStringUsingEncoding:NSUnicodeStringEncoding];            
        } else {
            openFileName.lpstrDefExt = NULL;
        }
		openFileName.lCustData=(LPARAM)_requiredFileType;
		openFileName.lpfnHook=(void *)saveFileHook;
		openFileName.lpTemplateName=NULL;
	}

   [(Win32Display *)[NSDisplay currentDisplay] stopWaitCursor];
   check=GetSaveFileNameW(&openFileName);
   [(Win32Display *)[NSDisplay currentDisplay] startWaitCursor];

	@synchronized(self)
	{
		if(!check && openFileName.lCustData!=0xFFFFFFFF){
			return NSCancelButton;
		}
		
		[_filename release];
		_filename=[[NSString stringWithCharacters:openFileName.lpstrFile length:wcslen(openFileName.lpstrFile)] copy];
		if(![[_filename pathExtension] isEqualToString:_requiredFileType]){
			[_filename autorelease];
			_filename=[[_filename stringByAppendingPathExtension:_requiredFileType] copy];
		}
	}

   return NSOKButton;
}

@end
