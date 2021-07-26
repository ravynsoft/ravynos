/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSOpenPanel-Win32.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSDocumentController.h>
#import <AppKit/NSWindow-Private.h>
#import <AppKit/Win32Display.h>
#import <AppKit/Win32Window.h>

#import <windows.h>
#import <commdlg.h>
#import <shlobj.h>
#import <malloc.h>

#import <Foundation/NSString_win32.h>

@implementation NSDocumentController(Win32)

-(NSArray *)_allFileTypes {
   return _fileTypes;
}

@end;

@implementation NSOpenPanel(Win32)

static int CALLBACK browseFolderHook(HWND hdlg, UINT uMsg, LPARAM lParam, LPARAM lpData) {
	if(uMsg==BFFM_INITIALIZED) {
		SendMessage(hdlg, BFFM_SETSELECTIONW, YES, lpData);
	}
	
	return 0;
}

-(int)_SHBrowseForFolder:(NSString *)initialPath {
   BROWSEINFOW  browseInfo;
   ITEMIDLIST *itemIdList;
   LPMALLOC    mallocInterface;
   unichar        displayName[MAX_PATH+1];
	
   @synchronized(self)
	{
      browseInfo.hwndOwner=[(Win32Window *)[[NSApp keyWindow] platformWindow] windowHandle];
      browseInfo.pidlRoot=NULL;
      browseInfo.pszDisplayName=displayName;
      browseInfo.lpszTitle=(const unichar *)[_dialogTitle cStringUsingEncoding:NSUnicodeStringEncoding];
      browseInfo.ulFlags=BIF_NEWDIALOGSTYLE;
		if ([initialPath length]) {
			browseInfo.lpfn=browseFolderHook;
			browseInfo.lParam=(LPARAM)[initialPath fileSystemRepresentationW];
		}
		else {
			browseInfo.lpfn=NULL;
			browseInfo.lParam=0;
		}
      browseInfo.iImage=0;
   }
	
	

   [(Win32Display *)[NSDisplay currentDisplay] stopWaitCursor];
   itemIdList=SHBrowseForFolderW(&browseInfo);
   [(Win32Display *)[NSDisplay currentDisplay] startWaitCursor];

   if(itemIdList==NULL)
    return NSCancelButton;

   if(SHGetMalloc(&mallocInterface)!=NOERROR)
    NSLog(@"SHGetMalloc failed");

   if(!SHGetPathFromIDListW(itemIdList,displayName))
    NSLog(@"SHGetPathFromIDList failed");

   mallocInterface->lpVtbl->Free(mallocInterface,itemIdList);
   mallocInterface->lpVtbl->Release(mallocInterface);

   @synchronized(self)
	{
      [_filenames release];
      _filenames=[[NSArray arrayWithObject:[NSString stringWithCharacters:displayName length:wcslen(displayName)]] retain];   
      
      if([_filenames count]>0){
         [_filename release];
         _filename=[[_filenames objectAtIndex:0] copy];
         [_directory release];
         _directory=[[_filename stringByDeletingLastPathComponent] copy];
      }
   }

   return NSOKButton;
}

// I haven't figured out a way to distinguish between double-clicking a folder and
// clicking the Open button, so we are stuck with SHBrowseForFolder() for opening folders
// The hook works fine when there are file types to check against

static unsigned *openFileHook(HWND hdlg,UINT uiMsg,WPARAM wParam,LPARAM lParam) {
   static BOOL pastInitialFolderChange;

   if(uiMsg==WM_INITDIALOG)
    pastInitialFolderChange=NO;

   if(uiMsg==WM_NOTIFY){
    OFNOTIFYW *notify=(void *)lParam;

    if(notify->hdr.code==CDN_FOLDERCHANGE){
     if(pastInitialFolderChange){ // we get one FOLDERCHANGE right after init, ignore it
      NSArray  *types=(NSArray *)notify->lpOFN->lCustData;
      unichar      folder[MAX_PATH+1];
      int       length=SendMessage(GetParent(hdlg),CDM_GETFOLDERPATH,MAX_PATH,(LPARAM)folder)-1;

      if(length>0){
       NSString *file=[NSString stringWithCharacters:folder length:length];
       NSString *extension=[file pathExtension];

       if([types containsObject:extension]){
        notify->lpOFN->lCustData=0xFFFFFFFF;
        wcscpy(notify->lpOFN->lpstrFile,folder);
        PostMessage(GetParent(hdlg),WM_SYSCOMMAND,SC_CLOSE,0); 
       }
      }
     }
     pastInitialFolderChange=YES;
    }
   }

   return NULL;
}

-(int)_GetOpenFileNameForTypes:(NSArray *)types {
   OPENFILENAMEW openFileName;
   unichar       filename[MAX_PATH+1];
   unichar      *fileTypes,*p,*q;
   int           i,j,fileTypesLength,check;
   NSArray      *allTypes = [[NSDocumentController sharedDocumentController] _allFileTypes];
   NSDictionary *typeDict;
   NSArray      *typeExtensions;

   if(types)
   {
    types = [types sortedArrayUsingSelector:@selector(caseInsensitiveCompare:)];
    fileTypesLength=strlen("Supported (");
    for(j=0;j<[types count];j++)
     // 2 x length of *.<EXT> + semicolon
    fileTypesLength+=2*(2+[[types objectAtIndex:j] cStringLength]+1);
    fileTypesLength+=1; // space for one \0  

    for(i=0;i<[allTypes count];i++){
     typeDict=[allTypes objectAtIndex:i];
     // length of the full name of the document type + blank + opening bracket
     fileTypesLength+=[[typeDict objectForKey:@"CFBundleTypeName"] cStringLength]+2;
     typeExtensions=[typeDict objectForKey:@"CFBundleTypeExtensions"];
     for(j=0;j<[typeExtensions count];j++)
      // 2 x length of *.<EXT> + semicolon
      fileTypesLength+=2*(2+[[typeExtensions objectAtIndex:j] cStringLength]+1);
     fileTypesLength+=1; // space for one \0  
    }
    fileTypesLength++;   // the final \0

    // allocate the space and fill in the file types list
    p=fileTypes=calloc(fileTypesLength, sizeof(unichar));
    wcscpy(p,L"Supported ("); p+=wcslen(L"Supported (");
    q=p;
    for(j=0;j<[types count];j++){
     *p++ ='*'; *p++ ='.';
     [[types objectAtIndex:j] getCharacters:p]; p+=wcslen(p);
     *p++ =';';
    }
    *(p-1)=')'; // replace the last semicolon by the closing bracket
    *p++ ='\0';
    // duplicate the semicolon separated *.extension list
    wcscpy(p,q); p+=p-q-2;
    *p++ ='\0'; // replace the stray closing bracket by '\0'

    for(i=0;i<[allTypes count];i++){
     typeDict=[allTypes objectAtIndex:i];
     [[typeDict objectForKey:@"CFBundleTypeName"] getCharacters:p]; p+=wcslen(p);
     *p++ =' '; *p++ ='(';

     typeExtensions=[typeDict objectForKey:@"CFBundleTypeExtensions"];
     q=p;
     for(j=0;j<[typeExtensions count];j++){
      *p++ ='*'; *p++ ='.';
      [[typeExtensions objectAtIndex:j] getCharacters:p]; p+=wcslen(p);
      *p++ =';';
     }
     *(p-1)=')'; // replace the last semicolon by the closing bracket
     *p++ ='\0';
     // duplicate the semicolon separated *.extension list
     wcscpy(p,q); p+=p-q-2;
     *p++ ='\0'; // replace the stray closing bracket by '\0'
    }
    *p='\0';
   }
   else
    fileTypes=L"All files (*.*)\0*.*\0\0";

   @synchronized(self)
	{
      openFileName.lStructSize=sizeof(OPENFILENAME);
      openFileName.hwndOwner=[(Win32Window *)[[NSApp keyWindow] platformWindow] windowHandle];
      openFileName.hInstance=NULL;
      openFileName.lpstrFilter=fileTypes;
      openFileName.lpstrCustomFilter=NULL;
      openFileName.nMaxCustFilter=0;
      openFileName.nFilterIndex=1;
      wcsncpy(filename,[_filename fileSystemRepresentationW],MAX_PATH);
      openFileName.lpstrFile=filename;
      openFileName.nMaxFile=1024;
      openFileName.lpstrFileTitle=NULL;
      openFileName.nMaxFileTitle=0;
      openFileName.lpstrInitialDir=[_directory fileSystemRepresentationW];
      openFileName.lpstrTitle= (const unichar *)[_dialogTitle cStringUsingEncoding:NSUnicodeStringEncoding];
      openFileName.Flags=
      (_allowsMultipleSelection?OFN_ALLOWMULTISELECT:0)|
      OFN_NOTESTFILECREATE|
      OFN_EXPLORER|
      OFN_HIDEREADONLY|
      OFN_ENABLEHOOK|
      OFN_ENABLESIZING
      ;
      openFileName.nFileOffset=0;
      openFileName.nFileExtension=0;
      openFileName.lpstrDefExt=NULL;
      openFileName.lCustData=(LPARAM)types;
      openFileName.lpfnHook=(void *)openFileHook;
      openFileName.lpTemplateName=NULL;
   }
   
   [(Win32Display *)[NSDisplay currentDisplay] stopWaitCursor];
   check=GetOpenFileNameW(&openFileName);
   [(Win32Display *)[NSDisplay currentDisplay] startWaitCursor];

   if(types)
    free(fileTypes);
   if(!check && openFileName.lCustData!=0xFFFFFFFF)
    return NSCancelButton;
   
   @synchronized(self)
	{

      [_filenames release];
      {
         NSString *firstFile=[NSString stringWithCharacters:openFileName.lpstrFile length:wcslen(openFileName.lpstrFile)];
         int       offset=openFileName.nFileOffset;
         
         if(offset<[firstFile length])
            _filenames=[[NSArray arrayWithObject:firstFile] retain];
         else {
            NSMutableArray *list=[NSMutableArray array];
            
            while(YES){
               NSString *next=[NSString stringWithCharacters:openFileName.lpstrFile+offset length:wcslen(openFileName.lpstrFile+offset)];
               
               if([next length]==0)
                  break;
               
               [list addObject:[firstFile stringByAppendingPathComponent:next]];
               offset+=[next length]+1;
            }
            _filenames=[list retain];
         }
      }
      if([_filenames count]>0){
         [_filename release];
         _filename=[[_filenames objectAtIndex:0] copy];
         [_directory release];
         _directory=[[_filename stringByDeletingLastPathComponent] copy];
      }
   }
      
   return NSOKButton;
}

@end
