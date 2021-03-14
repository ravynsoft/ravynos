/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <AppKit/Win32IEnumFORMATETCServer.h>
#import <AppKit/Win32FORMATETC.h>

@implementation Win32IEnumFORMATETCServer

-(void)dealloc {
   [_formatEtcs release];
   [super dealloc];
}

-(void)setFormatEtcs:(NSArray *)formatEtcs {
   formatEtcs=[formatEtcs copy];
   [_formatEtcs release];
   _formatEtcs=formatEtcs;
}

-(void)setIndex:(unsigned)index {
   _index=index;
}

-(HRESULT)Next:(ULONG)celt:(FORMATETC *)formatEtc:(ULONG *)pceltFetched {
   ULONG fetched=0;

   if(formatEtc==NULL)
    return E_INVALIDARG;
   if(celt>1 && pceltFetched==NULL)
    return E_INVALIDARG;
   if(_index>=[_formatEtcs count])
    return S_FALSE;

   for(;_index<[_formatEtcs count] && fetched<celt;fetched++)
    *formatEtc=[[_formatEtcs objectAtIndex:_index++] FORMATETC];

   if(pceltFetched!=NULL)
    *pceltFetched=fetched;

   return (fetched==celt)?S_OK:S_FALSE;
}

-(HRESULT)Skip:(ULONG)celt {
   _index+=celt;

   return (_index<=[_formatEtcs count])?S_OK:S_FALSE;
}

-(HRESULT)Reset {
   _index=0;
   return S_OK;
}

-(HRESULT)Clone:(IEnumFORMATETC **)ppenum {
   Win32IEnumFORMATETCServer *server=[[Win32IEnumFORMATETCServer alloc] initAsIEnumFORMATETC];

   [server setFormatEtcs:_formatEtcs];
   [server setIndex:_index];

   *ppenum=[server iUknown];
   
   return S_OK;
}

@end
