/* ------------------------------------------------------------------------*/
/*                                                                         */
/*   SURFACE.H                                                             */
/*                                                                         */
/*   Defines the classes TDrawSurface and TSurfaceView.                    */
/*                                                                         */
/* ------------------------------------------------------------------------*/

#if defined( Uses_TDrawSurface ) && !defined( __TDrawSurface )
#define __TDrawSurface

// A TDrawSurface holds a two-dimensional buffer of TScreenCells
// that can be freely written to.

class TDrawSurface
{

    size_t dataLength;
    TScreenCell _FAR *data;

public:

    TPoint size;

    TDrawSurface() noexcept;
    TDrawSurface(TPoint aSize) noexcept;
    ~TDrawSurface();

    void resize(TPoint aSize);
    void grow(TPoint aDelta);
    void clear();

    // Warning: no bounds checking.
    TScreenCell _FAR &at(int y, int x);
    const TScreenCell _FAR &at(int y, int x) const;

};

inline void TDrawSurface::grow(TPoint aDelta)
{
    resize(size + aDelta);
}

inline TScreenCell _FAR &TDrawSurface::at(int y, int x)
{
    return data[y*size.x + x];
}

inline const TScreenCell _FAR &TDrawSurface::at(int y, int x) const
{
    return data[y*size.x + x];
}

#endif

/* ---------------------------------------------------------------------- */
/*      class TSurfaceView                                                */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Empty area                                                  */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TSurfaceView ) && !defined( __TSurfaceView )
#define __TSurfaceView

// A TSurfaceView displays a region of a TDrawSurface between 'delta' and
// '{delta.x + size.x, delta.y + size.y}'.
// Out-of-bounds areas (or the whole view if 'surface' is null) are
// displayed as whitespaces.

// The "empty area" color maps to TWindow's and TDialog's "frame passive" color.

class TSurfaceView : public TView
{

public:

    const TDrawSurface _FAR *surface;
    TPoint delta;

    TSurfaceView(const TRect &bounds, const TDrawSurface _FAR *aSurface=0) noexcept;

    virtual void draw();
    virtual TPalette& getPalette() const;

};

#endif
