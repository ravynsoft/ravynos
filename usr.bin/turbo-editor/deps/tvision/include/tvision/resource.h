/* ------------------------------------------------------------------------*/
/*                                                                         */
/*   RESOURCE.H                                                            */
/*                                                                         */
/*   defines the classes TStringCollection, TResourceCollection,           */
/*   TResourceFile, TStrListMaker, and TStringList                         */
/*                                                                         */
/* ------------------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if defined( __BORLANDC__ )
#pragma option -Vo-
#endif
#if defined( __BCOPT__ ) && !defined (__FLAT__)
#pragma option -po-
#endif

#if defined( Uses_TStringCollection ) && !defined( __TStringCollection )
#define __TStringCollection

class TStringCollection : public TSortedCollection
{

public:

    TStringCollection( short aLimit, short aDelta ) noexcept;

private:

    virtual int compare( void *key1, void *key2 );
    virtual void freeItem( void *item );

    virtual const char *streamableName() const
        { return name; }
    virtual void *readItem( ipstream& );
    virtual void writeItem( void *, opstream& );

protected:

    TStringCollection( StreamableInit ) noexcept : TSortedCollection ( streamableInit ) {};

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TStringCollection& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TStringCollection*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TStringCollection& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TStringCollection* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TStringCollection

#if defined( Uses_TResourceItem ) && !defined( __TResourceItem )
#define __TResourceItem

struct TResourceItem
{

    int32_t pos;
    int32_t size;
    char *key;
};

#endif  // Uses_TResourceItem

#if defined( Uses_TResourceCollection ) && !defined( __TResourceCollection )
#define __TResourceCollection

class TResourceCollection: public TStringCollection
{

public:

    TResourceCollection( StreamableInit) noexcept : TStringCollection( streamableInit ) {};
    TResourceCollection( short aLimit, short aDelta ) noexcept;

    virtual void *keyOf( void *item );

private:

    virtual void freeItem( void *item );

    virtual const char *streamableName() const
        { return name; }
    virtual void *readItem( ipstream& );
    virtual void writeItem( void *, opstream& );

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TResourceCollection& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TResourceCollection*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TResourceCollection& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TResourceCollection* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TResourceCollection

#if defined( Uses_TResourceFile ) && !defined( __TResourceFile )
#define __TResourceFile

class _FAR TResourceCollection;
class _FAR fpstream;

class TResourceFile: public TObject
{

public:

    TResourceFile( fpstream *aStream );
    ~TResourceFile();
    short count();
    void remove( const char *key );
    void flush();
    void *get( const char *key );
    const char *keyAt( short i );
    void put( TStreamable *item, const char *key );
    fpstream *switchTo( fpstream *aStream, Boolean pack );

protected:

    fpstream *stream;
    Boolean modified;
    int32_t basePos;
    int32_t indexPos;
    TResourceCollection *index;
};

#endif  // Uses_TResourceFile

#if defined( Uses_TStrIndexRec ) && !defined( __TStrIndexRec )
#define __TStrIndexRec

class TStrIndexRec
{

public:

    TStrIndexRec() noexcept;

    ushort key;
    ushort count;
    ushort offset;

};

#endif  // Uses_TStrIndexRec

#if defined( Uses_TStringList ) && !defined( __TStringList )
#define __TStringList

class _FAR TStrIndexRec;

class TStringList : public TObject, public TStreamable
{

public:

    ~TStringList();

    void get( char *dest, ushort key );

private:

    ipstream *ip;
    int32_t basePos;
    short indexSize;
    TStrIndexRec *index;

    virtual const char *streamableName() const
        { return name; }

protected:

    TStringList( StreamableInit ) noexcept;
    virtual void write( opstream& ) {}
    virtual void *read( ipstream& );

public:

    static const char * const _NEAR name;
    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TStringList& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TStringList*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TStringList& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TStringList* cl )
    { return os << (TStreamable *)cl; }

#endif  // Uses_TStringList


#if defined( Uses_TStrListMaker ) && !defined( __TStrListMaker )
#define __TStrListMaker

class TStrListMaker : public TObject, public TStreamable
{

public:

    TStrListMaker( ushort aStrSize, ushort aIndexSize ) noexcept;
    ~TStrListMaker();

    void put( ushort key, char *str );

private:

    ushort strPos;
    ushort strSize;
    char *strings;
    ushort indexPos;
    ushort indexSize;
    TStrIndexRec *index;
    TStrIndexRec cur;
    void closeCurrent();

    virtual const char *streamableName() const
        { return TStringList::name; }

protected:

    TStrListMaker( StreamableInit ) noexcept;
    virtual void write( opstream& );
    virtual void *read( ipstream& ) { return 0; }

public:

    static TStreamable *build();

};

inline ipstream& operator >> ( ipstream& is, TStrListMaker& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TStrListMaker*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TStrListMaker& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TStrListMaker* cl )
    { return os << (TStreamable *)cl; }


#endif  // Uses_TStrListMaker

#if defined( __BORLANDC__ )
#pragma option -Vo.
#endif
#if defined( __BCOPT__ ) && !defined (__FLAT__)
#pragma option -po.
#endif
