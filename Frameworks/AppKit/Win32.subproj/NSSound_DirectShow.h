#import <AppKit/NSSound.h>

#include <windows.h>
#include <mmsystem.h>
#define IBaseFilter void
#define LPDDPIXELFORMAT int
#include <strmif.h>
#include <control.h>
#include <oaidl.h>

DECLARE_ENUMERATOR_(IEnumFilters, IBaseFilter *);

#define INTERFACE IPin
DECLARE_INTERFACE_(IPin, IUnknown) {
    STDMETHOD(QueryInterface)(THIS_ REFIID, PVOID *)PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;
    STDMETHOD(Connect)(THIS_ IPin *, const AM_MEDIA_TYPE *)PURE;
    STDMETHOD(ReceiveConnection)(THIS_ IPin *, const AM_MEDIA_TYPE *)PURE;
    STDMETHOD(Disconnect)(THIS) PURE;
    STDMETHOD(ConnectedTo)(THIS_ IPin **)PURE;
    STDMETHOD(ConnectionMediaType)(THIS_ AM_MEDIA_TYPE *)PURE;
    STDMETHOD(QueryPinInfo)(THIS_ PIN_INFO *)PURE;
    STDMETHOD(QueryDirection)(THIS_ PIN_DIRECTION *)PURE;
};
#undef INTERFACE

#define INTERFACE IGraphBuilder
DECLARE_INTERFACE_(IGraphBuilder, IUnknown) {
    // IUnknown
    STDMETHOD(QueryInterface)(THIS_ REFIID, PVOID *)PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;
    // IFilterGraph
    STDMETHOD(AddFilter)(THIS_ IBaseFilter *, LPCWSTR) PURE;
    STDMETHOD(RemoveFilter)(THIS_ IBaseFilter *)PURE;
    STDMETHOD(EnumFilters)(THIS_ IEnumFilters **)PURE;
    STDMETHOD(FindFilterByName)(THIS_ LPCWSTR, IBaseFilter **)PURE;
    STDMETHOD(ConnectDirect)(THIS_ IPin *, IPin *, const AM_MEDIA_TYPE *)PURE;
    STDMETHOD(Reconnect)(THIS_ IPin *)PURE;
    STDMETHOD(Disconnect)(THIS_ IPin *)PURE;
    STDMETHOD(SetDefaultSyncSource)(THIS) PURE;

    STDMETHOD(Connect)(THIS_ IPin *, IPin *)PURE;
    STDMETHOD(Render)(THIS_ IPin *)PURE;
    STDMETHOD(RenderFile)(THIS_ LPCWSTR, LPCWSTR) PURE;
    STDMETHOD(AddSourceFilter)(THIS_ LPCWSTR, LPCWSTR, IBaseFilter **)PURE;
    STDMETHOD(SetLogFile)(THIS_ DWORD_PTR) PURE;
    STDMETHOD(Abort)(THIS) PURE;
    STDMETHOD(ShouldOperationContinue)(THIS) PURE;
};
#undef INTERFACE

typedef long OAFilterState;

#define INTERFACE IMediaControl
DECLARE_INTERFACE_(IMediaControl, IDispatch) {
    STDMETHOD(QueryInterface)(THIS_ REFIID, PVOID *)PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    STDMETHOD(GetTypeInfoCount)(THIS_ UINT *)PURE;
    STDMETHOD(GetTypeInfo)(THIS_ UINT, LCID, LPTYPEINFO *)PURE;
    STDMETHOD(GetIDsOfNames)(THIS_ REFIID, LPOLESTR *, UINT, LCID, DISPID *)PURE;
    STDMETHOD(Invoke)(THIS_ DISPID, REFIID, LCID, WORD, DISPPARAMS *, VARIANT *, EXCEPINFO *, UINT *)PURE;

    STDMETHOD(Run)(THIS) PURE;
    STDMETHOD(Pause)(THIS) PURE;
    STDMETHOD(Stop)(THIS) PURE;
    STDMETHOD(GetState)(THIS_ LONG, OAFilterState *)PURE;
    STDMETHOD(RenderFile)(THIS_ BSTR) PURE;
    STDMETHOD(AddSourceFilter)(THIS_ BSTR, IDispatch **)PURE;
    STDMETHOD(get_FilterCollection)(THIS_ IDispatch **)PURE;
    STDMETHOD(get_RegFilterCollection)(THIS_ IDispatch **)PURE;
    STDMETHOD(StopWhenReady)(THIS) PURE;
};
#undef INTERFACE

@interface NSSound_DirectShow : NSSound {
    NSString *_path;

    struct IGraphBuilder *_graphBuilder;
    struct IMediaControl *_mediaControl;
}

@end
