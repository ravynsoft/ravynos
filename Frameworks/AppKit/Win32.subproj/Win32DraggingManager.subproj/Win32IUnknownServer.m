/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <AppKit/Win32IUnknownServer.h>
#import <Foundation/NSString_win32.h>

typedef struct Win32COMObject {
   void *vtable;
   id    object;
   IID   iid;
} Win32COMObject;

static id COMObjectToSelf(void *This) {
   Win32COMObject *comObject=This;
   return comObject->object;
}

static HRESULT STDMETHODCALLTYPE __RPC_FAR QueryInterface(IUnknown __RPC_FAR *This, REFIID riid,void __RPC_FAR *__RPC_FAR *ppvObject){
   return [COMObjectToSelf(This) QueryInterface:riid:ppvObject];
}

static ULONG STDMETHODCALLTYPE __RPC_FAR AddRef(IUnknown __RPC_FAR *This){
   return [COMObjectToSelf(This) AddRef];
}

static ULONG STDMETHODCALLTYPE __RPC_FAR Release(IUnknown __RPC_FAR *This){
   return [COMObjectToSelf(This) Release];
}

// IDropTarget
static HRESULT STDMETHODCALLTYPE __RPC_FAR DragEnter(IDropTarget __RPC_FAR *This,IDataObject __RPC_FAR *dataObject,DWORD grfKeyState,POINTL pt,DWORD __RPC_FAR *pdwEffect){
   return [COMObjectToSelf(This) DragEnter:dataObject:grfKeyState:pt:pdwEffect];
}

static HRESULT STDMETHODCALLTYPE __RPC_FAR DragOver(IDropTarget __RPC_FAR *This,DWORD grfKeyState,POINTL pt,DWORD __RPC_FAR *pdwEffect){
   return [COMObjectToSelf(This) DragOver:grfKeyState:pt:pdwEffect];
}

static HRESULT STDMETHODCALLTYPE __RPC_FAR DragLeave(IDropTarget __RPC_FAR *This){
   return [COMObjectToSelf(This) DragLeave];
}

static HRESULT STDMETHODCALLTYPE __RPC_FAR Drop(IDropTarget __RPC_FAR *This,IDataObject __RPC_FAR *dataObject,DWORD grfKeyState,POINTL pt,DWORD __RPC_FAR *pdwEffect){
   return [COMObjectToSelf(This) Drop:dataObject:grfKeyState:pt:pdwEffect];
}

static IDropTargetVtbl IDropTargetVTable={
 (void *)QueryInterface,(void *)AddRef,(void *)Release,
 DragEnter,DragOver,DragLeave,Drop,
};

static HRESULT STDMETHODCALLTYPE __RPC_FAR QueryContinueDrag(IDropSource __RPC_FAR *This,int fEscapePressed,DWORD grfKeyState) {
   return [COMObjectToSelf(This) QueryContinueDrag:fEscapePressed:grfKeyState];
}

static HRESULT STDMETHODCALLTYPE __RPC_FAR GiveFeedback(IDropSource __RPC_FAR *This,DWORD dwEffect) {
   return [COMObjectToSelf(This) GiveFeedback:dwEffect];
}

static IDropSourceVtbl IDropSourceVTable={
 (void *)QueryInterface,(void *)AddRef,(void *)Release,
 (void *)QueryContinueDrag,(void *)GiveFeedback,
};

static HRESULT STDMETHODCALLTYPE __RPC_FAR GetData(IDataObject __RPC_FAR *This, FORMATETC __RPC_FAR *pformatetcIn,STGMEDIUM __RPC_FAR *pmedium) {
   return [COMObjectToSelf(This) GetData:pformatetcIn:pmedium];
}
  
static HRESULT STDMETHODCALLTYPE __RPC_FAR GetDataHere(IDataObject __RPC_FAR * This, FORMATETC __RPC_FAR *pformatetc,STGMEDIUM __RPC_FAR *pmedium) {
   return [COMObjectToSelf(This) GetDataHere:pformatetc:pmedium];
}
        
static HRESULT STDMETHODCALLTYPE __RPC_FAR QueryGetData(IDataObject __RPC_FAR * This,FORMATETC __RPC_FAR *pformatetc) {
   return [COMObjectToSelf(This) QueryGetData:pformatetc];
}
        
static HRESULT STDMETHODCALLTYPE __RPC_FAR GetCanonicalFormatEtc(IDataObject __RPC_FAR * This,FORMATETC __RPC_FAR *pformatectIn, FORMATETC __RPC_FAR *pformatetcOut) {
   return [COMObjectToSelf(This) GetCanonicalFormatEtc:pformatectIn:pformatetcOut];
}

static HRESULT STDMETHODCALLTYPE __RPC_FAR SetData(IDataObject __RPC_FAR * This,FORMATETC __RPC_FAR *pformatetc,STGMEDIUM __RPC_FAR *pmedium,int fRelease) {
   return [COMObjectToSelf(This) SetData:pformatetc:pmedium:fRelease];
}

static HRESULT STDMETHODCALLTYPE __RPC_FAR EnumFormatEtc(IDataObject __RPC_FAR * This,DWORD dwDirection,IEnumFORMATETC __RPC_FAR *__RPC_FAR *ppenumFormatEtc) {
   return [COMObjectToSelf(This) EnumFormatEtc:dwDirection:ppenumFormatEtc];
}
        
static HRESULT STDMETHODCALLTYPE __RPC_FAR DAdvise(IDataObject __RPC_FAR * This, FORMATETC __RPC_FAR *pformatetc,DWORD advf,IAdviseSink __RPC_FAR *pAdvSink,DWORD __RPC_FAR *pdwConnection) {
   return [COMObjectToSelf(This) DAdvise:pformatetc:advf:pAdvSink:pdwConnection];
}

static HRESULT STDMETHODCALLTYPE __RPC_FAR DUnadvise(IDataObject __RPC_FAR * This,DWORD dwConnection) {
   return [COMObjectToSelf(This) DUnadvise: dwConnection];
}

static HRESULT STDMETHODCALLTYPE __RPC_FAR EnumDAdvise( IDataObject __RPC_FAR * This,IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenumAdvise) {
   return [COMObjectToSelf(This) EnumDAdvise:ppenumAdvise];
}

static IDataObjectVtbl IDataObjectVTable={
 (void *)QueryInterface,(void *)AddRef,(void *)Release,
 (void *)GetData,(void *)GetDataHere,(void *)QueryGetData, (void *)GetCanonicalFormatEtc, (void *)SetData, (void *)EnumFormatEtc, (void *)DAdvise, (void *)DUnadvise, (void *)EnumDAdvise
};

HRESULT STDMETHODCALLTYPE __RPC_FAR Next(IEnumFORMATETC __RPC_FAR *This,ULONG celt,FORMATETC __RPC_FAR *rgelt,ULONG __RPC_FAR *pceltFetched) {
   return [COMObjectToSelf(This) Next:celt:rgelt:pceltFetched];
}
        
HRESULT STDMETHODCALLTYPE __RPC_FAR Skip(IEnumFORMATETC __RPC_FAR *This, ULONG celt) {
   return [COMObjectToSelf(This) Skip:celt];
}
        
HRESULT STDMETHODCALLTYPE __RPC_FAR Reset(IEnumFORMATETC __RPC_FAR *This) {
   return [COMObjectToSelf(This) Reset];
}
        
HRESULT STDMETHODCALLTYPE __RPC_FAR Clone(IEnumFORMATETC __RPC_FAR *This,IEnumFORMATETC __RPC_FAR *__RPC_FAR *ppenum) {
   return [COMObjectToSelf(This) Clone:ppenum];
}

static IEnumFORMATETCVtbl IEnumFORMATETCVTable={
 (void *)QueryInterface,(void *)AddRef,(void *)Release,
 Next, Skip, Reset, Clone
};

@implementation Win32IUnknownServer

-initWithVtable:(void *)vtable interfaceIdentifier:(LPOLESTR)identifier {
   _capacity=1;
   _count=1;
   _interfaces=NSZoneMalloc(NULL,sizeof(Win32COMObject)*_capacity);
   _interfaces[0].vtable=vtable;
   _interfaces[0].object=self;
   if(IIDFromString(identifier,&_interfaces[0].iid)!=S_OK){
    NSLog(@"IIDFromString failed %s",identifier);
   }
   return self;
}

-(void)dealloc {
   if(_interfaces!=NULL)
    NSZoneFree(NULL,_interfaces);
   [super dealloc];
}

-initAsIDropTarget {
   return [self initWithVtable:&IDropTargetVTable
           interfaceIdentifier:OLESTR("{00000122-0000-0000-C000-000000000046}")];
}

-initAsIDropSource {
   return [self initWithVtable:&IDropSourceVTable
           interfaceIdentifier:OLESTR("{00000121-0000-0000-C000-000000000046}")];
}

-initAsIDataObject {
   return [self initWithVtable:&IDataObjectVTable
           interfaceIdentifier:OLESTR("{0000010E-0000-0000-C000-000000000046}")];
}

-initAsIEnumFORMATETC {
   return [self initWithVtable:&IEnumFORMATETCVTable
           interfaceIdentifier:OLESTR("{00000103-0000-0000-C000-000000000046}")];
}

-(void *)iUknown {
   return _interfaces+0;
}

-(HRESULT)QueryInterface:(REFIID)riid:(void **)ppvObject {
   IID      temp;
   int      i;

  // NSLog(@"-[%@ %s] %d",isa,sel_getName(_cmd),riid);

   IIDFromString(OLESTR("{00000000-0000-0000-C000-000000000046}"),&temp);
   if(IsEqualIID(riid,&temp)){
    [self retain];
    *ppvObject=[self iUknown];
    return S_OK;
   }

   for(i=0;i<_count;i++){
    if(IsEqualIID(riid,&_interfaces[i].iid)){
     [self retain];
     *ppvObject=_interfaces+i;
     return S_OK;
    }
   }

#if 0
   {
    unichar *str;
    unsigned length=0;

    StringFromIID(riid,&str);
    while(str[length++]!=0x0000)
     ;
    NSLog(@"REQUESTED IID=%@", NSStringFromNullTerminatedUnicode(str));
    CoTaskMemFree(str);
   }
#endif

   *ppvObject=NULL;
   return E_NOINTERFACE;
}

-(ULONG)AddRef {
  // NSLog(@"-[%@ %s]",isa,sel_getName(_cmd));
   [self retain];
   return [self retainCount];
}

-(ULONG)Release {
   ULONG result=[self retainCount]-1;
  // NSLog(@"-[%@ %s]",isa,sel_getName(_cmd));
   [self release];
   return result;
}

@end

