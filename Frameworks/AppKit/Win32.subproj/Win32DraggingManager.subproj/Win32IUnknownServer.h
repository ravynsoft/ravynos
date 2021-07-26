/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/Foundation.h>
#import <windows.h>
#import <wtypes.h>
#import <unknwn.h>
#import <objidl.h>
#import <ole2.h>

@protocol IUnknown
- (HRESULT)QueryInterface:(REFIID)riid:(void **)ppvObject;
- (ULONG)AddRef;
- (ULONG)Release;
@end

@protocol IDropTarget
- (HRESULT)DragEnter:(IDataObject *)pDataObj:(DWORD)grfKeyState:(POINTL)pt:(DWORD *)pdwEffect;
- (HRESULT)DragOver:(DWORD)grfKeyState:(POINTL)pt:(DWORD *)pdwEffect;
- (HRESULT)DragLeave;
- (HRESULT)Drop:(IDataObject *)pDataObj:(DWORD)grfKeyState:(POINTL)pt:(DWORD *)pdwEffect;
@end

@protocol IDropSource
- (HRESULT)QueryContinueDrag:(BOOL)fEscapedPressed:(DWORD)grfKeyState;
- (HRESULT)GiveFeedback:(DWORD)dwEffect;
@end

@protocol IDataObject
- (HRESULT)GetData:(FORMATETC *)formatEtcp:(STGMEDIUM *)pmedium;
- (HRESULT)GetDataHere:(FORMATETC *)formatEtcp:(STGMEDIUM *)pmedium;
- (HRESULT)QueryGetData:(FORMATETC *)formatEtcp;
- (HRESULT)GetCanonicalFormatEtc:(FORMATETC *)formatEtcp:(FORMATETC *)pformatetcOut;
- (HRESULT)SetData:(FORMATETC *)formatEtcp:(STGMEDIUM *)pmedium:(BOOL)fRelease;
- (HRESULT)EnumFormatEtc:(DWORD)dwDirection:(IEnumFORMATETC **)ppenumFormatEtc;
- (HRESULT)DAdvise:(FORMATETC *)pformatetc:(DWORD)advf:(IAdviseSink *)pAdvSink:(DWORD *)pdwConnection;
- (HRESULT)DUnadvise:(DWORD)dwConnection;
- (HRESULT)EnumDAdvise:(IEnumSTATDATA **)ppenumAdvise;
@end

@protocol IEnumFORMATETC
- (HRESULT)Next:(ULONG)celt:(FORMATETC *)rgelt:(ULONG *)pceltFetched;
- (HRESULT)Skip:(ULONG)celt;
- (HRESULT)Reset;
- (HRESULT)Clone:(IEnumFORMATETC **)ppenum;
@end

@interface Win32IUnknownServer : NSObject {
    unsigned _capacity, _count;
    struct Win32COMObject *_interfaces;
}

- initAsIDropTarget;
- initAsIDropSource;
- initAsIDataObject;
- initAsIEnumFORMATETC;

- (void *)iUknown;

@end
