#import "NSSound_DirectShow.h"
#import <Foundation/NSPathUtilities.h>

@implementation NSSound_DirectShow

static GUID IID_IMediaControl ={ 0x56a868b1, 0x0ad4, 0x11ce,
    0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70 };
static GUID CLSID_FilterGraph={ 0xe436ebb3, 0x524f, 0x11ce,
    0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70};
static GUID IID_IGraphBuilder={ 0x56a868a9, 0x0ad4, 0x11ce,
    0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70};

-initWithContentsOfFile:(NSString *)path byReference:(BOOL)byReference {
    
	if ((self = [super initWithContentsOfFile:path byReference:byReference])) {
		_path = [path copy];
	}
    
    if (SUCCEEDED(CoCreateInstance(&CLSID_FilterGraph,
                                   NULL,
                                   CLSCTX_INPROC_SERVER,
                                   &IID_IGraphBuilder,
                                   (void **)&_graphBuilder)))
    {
        if(_graphBuilder!=NULL){
            _graphBuilder->lpVtbl->QueryInterface(_graphBuilder,&IID_IMediaControl, (void **)&_mediaControl);
            
            const unichar *soundPathW=[_path fileSystemRepresentationW];
            
            HRESULT hr = _graphBuilder->lpVtbl->RenderFile(_graphBuilder,soundPathW,NULL);
            if (!SUCCEEDED(hr)) {
                [self dealloc];
                return nil;
            }
        }
    }
    
	return self;
}

-(BOOL)play {
    if(_mediaControl==NULL)
        return NO;
    
    HRESULT hr = _mediaControl->lpVtbl->Run(_mediaControl);
    return (SUCCEEDED(hr)) ? YES : NO;
    
	return YES;
}

-(BOOL)pause {
    if(_mediaControl==NULL)
        return NO;
    
    HRESULT hr = _mediaControl->lpVtbl->Pause(_mediaControl);
    return (SUCCEEDED(hr)) ? YES : NO;
}

-(BOOL)resume {
    if(_mediaControl==NULL)
        return NO;
    
    HRESULT hr = _mediaControl->lpVtbl->Run(_mediaControl);
    return (SUCCEEDED(hr)) ? YES : NO;
}

-(BOOL)stop {
    if(_mediaControl==NULL)
        return NO;
    
    HRESULT hr = _mediaControl->lpVtbl->Stop(_mediaControl);
    return (SUCCEEDED(hr)) ? YES : NO;
}

-(void)dealloc {
    if(_mediaControl!=NULL)
        _mediaControl->lpVtbl->Release(_mediaControl);
    if(_graphBuilder!=NULL)
        _graphBuilder->lpVtbl->Release(_graphBuilder);
    
	[super dealloc];
}



@end