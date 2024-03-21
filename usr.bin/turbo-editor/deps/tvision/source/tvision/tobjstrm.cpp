/*------------------------------------------------------------*/
/* filename -       tobjstrm.cpp                              */
/*                                                            */
/* function(s)                                                */
/*                  Member function(s) of following classes:  */
/*                     TParamText                             */
/*                     TStreamable                            */
/*                     TStreamableClass                       */
/*                     TStreamableTypes                       */
/*                     TPWrittenObjects                       */
/*                     TPReadObjects                          */
/*                     pstream                                */
/*                     ipstream                               */
/*                     opstream                               */
/*                     iopstream                              */
/*                     fpbase                                 */
/*                     ifpstream                              */
/*                     ofpstream                              */
/*                     fpstream                               */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TStreamable
#define Uses_TStreamableClass
#define Uses_TStreamableTypes
#define Uses_TPWrittenObjects
#define Uses_TPReadObjects
#define Uses_pstream
#define Uses_ipstream
#define Uses_opstream
#define Uses_iopstream
#define Uses_fpbase
#define Uses_ifpstream
#define Uses_ofpstream
#define Uses_fpstream
#include <tvision/tv.h>

#if !defined( __LIMITS_H )
#include <limits.h>
#endif  // __LIMITS_H

#if !defined( __STRING_H )
#include <string.h>
#endif  // __STRING_H

#if !defined( __FSTREAM_H )
#include <fstream.h>
#endif  // __FSTREAM_H

#if !defined( __STAT_H )
#include <sys/stat.h>
#endif  // __STAT_H

#if !defined( __FCNTL_H )
#include <fcntl.h>
#endif  // __FCNTL_H

#if !defined( __STDLIB_H )
#include <stdlib.h>
#endif  // __STDLIB_H

#if !defined( __ASSERT_H )
#include <assert.h>
#endif  // __ASSERT_H

#ifdef __FLAT__
#define _HUGE
#else
#define _HUGE huge
#endif

// ios::open_mode and ios::seek_dir are deprecated in modern C++ and are
// incompatible with the implementation-defined equivalents ios::openmode
// and ios::seekdir. We simply redirect to the appropiate type on each platform.
#ifdef __BORLANDC__
    typedef ios::open_mode openmode;
    typedef ios::seek_dir seekdir;
#else
    typedef ios::openmode openmode;
    typedef ios::seekdir seekdir;
#endif

const uchar nullStringLen = UCHAR_MAX;

TStreamableClass* volatile fLink::forceLink;

TStreamableClass::TStreamableClass( const char *n, BUILDER b, int ) noexcept :
    name( n ),
    build( b )
{
    pstream::initTypes();
    pstream::registerType( this );
}

TStreamableTypes::TStreamableTypes() noexcept : TNSSortedCollection( 5, 5 )
{
}

void *TStreamableTypes::operator new( size_t, void * arena )
{
    return arena;
}

TStreamableTypes::~TStreamableTypes()
{
}

void TStreamableTypes::registerType( const TStreamableClass *d )
{
    insert( (void *)d );
}

const TStreamableClass *TStreamableTypes::lookup( const char *name )
{
    ccIndex loc;
    if( search( (void *)name, loc ) )
        return (TStreamableClass *)at( loc );
    else
        return 0;
}

void *TStreamableTypes::keyOf( void *d )
{
    return (void *)((TStreamableClass *)d)->name;
}

int TStreamableTypes::compare( void *d1, void *d2 )
{
    return strcmp( (char *)d1, (char *)d2 );
}

TPWrittenObjects::TPWrittenObjects() noexcept : TNSSortedCollection( 5, 5 ), curId( 0 )
{
}

TPWrittenObjects::~TPWrittenObjects()
{
}

void TPWrittenObjects::registerObject( const void *adr )
{
    TPWObj *o = new TPWObj( adr, curId++ );
    insert( o );
}

P_id_type TPWrittenObjects::find( const void *d )
{
    ccIndex loc;
    if( search( (void *)d, loc ) )
        return ((TPWObj *)at( loc ))->ident;
    else
        return P_id_notFound;
}

void *TPWrittenObjects::keyOf( void *d )
{
    return (void *)((TPWObj *)d)->address;
}

int TPWrittenObjects::compare( void *o1, void *o2 )
{
    if( o1 == o2 )
        return 0;
    else if( ((char _HUGE *)o1)+1 < ((char _HUGE *)o2)+1 ) // force normalization
        return -1;
    else
        return 1;
}

TPWObj::TPWObj( const void *adr, P_id_type id ) noexcept : address( adr ), ident( id )
{
}

TPReadObjects::TPReadObjects() noexcept : TNSCollection( 5, 5 ), curId( 0 )
{
}

TPReadObjects::~TPReadObjects()
{
}

void TPReadObjects::registerObject( const void *adr )
{
    ccIndex loc = insert( (void *)adr );
    (void) loc;
    assert( (uint) loc == curId );  // to be sure that TNSCollection
                                    // continues to work the way
                                    // it does now...
    ++curId;
}

const void *TPReadObjects::find( P_id_type id )
{
    return at( id );
}

pstream::pstream( streambuf _FAR *sb ) noexcept
{
    init( sb );
}

pstream::~pstream()
{
}

void pstream::initTypes() noexcept
{
    if( types == 0 )
        types = new TStreamableTypes;
}

int pstream::rdstate() const noexcept
{
    return state;
}

int pstream::eof() const noexcept
{
    return state & ios::eofbit;
}

int pstream::fail() const noexcept
{
    return state & (ios::failbit | ios::badbit
#ifdef __BORLANDC__
        | ios::hardfail
#endif
    );
}

int pstream::bad() const noexcept
{
    return state & (ios::badbit
#ifdef __BORLANDC__
        | ios::hardfail
#endif
    );
}

int pstream::good() const noexcept
{
    return state == 0;
}

void pstream::clear( int i ) noexcept
{
    state = (i & 0xFF)
#ifdef __BORLANDC__
        | (state & ios::hardfail)
#endif
    ;
}

void pstream::registerType( TStreamableClass *ts ) noexcept
{
    types->registerType( ts );
}

pstream::operator void _FAR *() const noexcept
{
    return fail() ? 0 : (void *)this;
}

int pstream::operator! () const noexcept
{
    return fail();
}

streambuf _FAR * pstream::rdbuf() const noexcept
{
    return bp;
}

pstream::pstream() noexcept
{
}

void pstream::error( StreamableError e ) noexcept
{
    if( e == peInvalidType )
        cerr << "pstream error: invalid type encountered" << endl;
    else if( e == peNotRegistered )
        cerr << "pstream error: type not registered" << endl;
    abort();
}

void pstream::error( StreamableError e, const TStreamable &t ) noexcept
{
    if( e == peNotRegistered )
        cerr << "pstream error: type '" << t.streamableName() << "' not registered" << endl;
    else
        error( e );
    abort();
}

void pstream::init( streambuf *sbp ) noexcept
{
    state = 0;
    bp = sbp;
}

void pstream::setstate( int b ) noexcept
{
    state |= (b&0xFF);
}

ipstream::ipstream( streambuf _FAR *sb ) noexcept
{
    pstream::init( sb );
}

ipstream::~ipstream()
{
    objs.shouldDelete = False;
    objs.shutDown();
}

streampos ipstream::tellg()
{
#ifdef __BORLANDC__
    return bp->seekoff( 0, ios::cur, ios::in );
#else
    return bp->pubseekoff( 0, ios::cur, ios::in );
#endif
}

ipstream& ipstream::seekg( streampos pos )
{
    objs.removeAll();
#ifdef __BORLANDC__
    bp->seekoff( pos, ios::beg );
#else
    bp->pubseekoff( pos, ios::beg );
#endif
    return *this;
}

ipstream& ipstream::seekg( streamoff off, pstream::seekdir dir )
{
    objs.removeAll();
#ifdef __BORLANDC__
    bp->seekoff( off, ::seekdir(dir) );
#else
    bp->pubseekoff( off, ::seekdir(dir) );
#endif
    return *this;
}

uchar ipstream::readByte()
{
    return bp->sbumpc();
}

ushort ipstream::readWord()
{
    ushort temp;
    bp->sgetn( (char *)&temp, sizeof( ushort ) );
    return temp;
}

void ipstream::readBytes( void *data, size_t sz )
{
    bp->sgetn( (char *)data, sz );
}

char *ipstream::readString()
{
    uchar len = readByte();
    if( len == nullStringLen )
        return 0;
    char *buf = new char[len+1];
    if( buf == 0 )
        return 0;
    readBytes( buf, len );
    buf[len] = EOS;
    return buf;
}

char *ipstream::readString( char *buf, unsigned maxLen )
{
    assert( buf != 0 );

    uchar len = readByte();
    if( len > maxLen-1 )
        return 0;
    readBytes( buf, len );
    buf[len] = EOS;
    return buf;
}

ipstream& operator >> ( ipstream& ps, char &ch )
{
    ch = ps.readByte();
    return ps;
}

ipstream& operator >> ( ipstream& ps, signed char &ch )
{
    ch = ps.readByte();
    return ps;
}

ipstream& operator >> ( ipstream& ps, unsigned char &ch )
{
    ch = ps.readByte();
    return ps;
}

ipstream& operator >> ( ipstream& ps, signed short &sh )
{
    sh = ps.readWord();
    return ps;
}

ipstream& operator >> ( ipstream& ps, unsigned short &sh )
{
    sh = ps.readWord();
    return ps;
}

ipstream& operator >> ( ipstream& ps, signed int &i )
{
    ps.readBytes( &i, sizeof(int) );
    return ps;
}

ipstream& operator >> ( ipstream& ps, unsigned int &i )
{
    ps.readBytes( &i, sizeof(int) );
    return ps;
}

ipstream& operator >> ( ipstream& ps, signed long &l )
{
    ps.readBytes( &l, sizeof(l) );
    return ps;
}

ipstream& operator >> ( ipstream& ps, unsigned long &l )
{
    ps.readBytes( &l, sizeof(l) );
    return ps;
}

ipstream& operator >> ( ipstream& ps, float &f )
{
    ps.readBytes( &f, sizeof(f) );
    return ps;
}

ipstream& operator >> ( ipstream& ps, double &d )
{
    ps.readBytes( &d, sizeof(d) );
    return ps;
}

ipstream& operator >> ( ipstream& ps, long double &ld )
{
    ps.readBytes( &ld, sizeof(ld) );
    return ps;
}

ipstream& operator >> ( ipstream& ps, TStreamable& t )
{
    const TStreamableClass *pc = ps.readPrefix();
    ps.readData( pc, &t );
    ps.readSuffix();
    return ps;
}

ipstream& operator >> ( ipstream& ps, void *&t )
{
    char ch = ps.readByte();
    switch( ch )
        {
        case pstream::ptNull:
            t = 0;
            break;
        case pstream::ptIndexed:
            {
            P_id_type index = ps.readWord();
            t = (void *)ps.find( index );
            assert( t != 0 );
            break;
            }
        case pstream::ptObject:
            {
            const TStreamableClass *pc = ps.readPrefix();
            t = ps.readData( pc, 0 );
            ps.readSuffix();
            break;
            }
        default:
            ps.error( pstream::peInvalidType );
            break;
        }
    return ps;
}

ipstream::ipstream() noexcept
{
}

const TStreamableClass *ipstream::readPrefix()
{
    char ch = readByte();
    assert( ch == '[' );    // don't combine this with the previous line!
    (void) ch;              // We must always do the read, even if we're
                            // not checking assertions

    char name[128];
    readString( name, sizeof name );
    return types->lookup( name );
}

void *ipstream::readData( const TStreamableClass *c, TStreamable *mem )
{
    if( mem == 0 )
        mem = c->build();
    // Register the actual address of the object, not the address of the
    // TStreamable sub-object, so that it is returned by subsequent calls
    // to find().
    registerObject( dynamic_cast<void *>(mem) );
    return mem->read( *this );
}

void ipstream::readSuffix()
{
    char ch = readByte();
    assert( ch == ']' );    // don't combine this with the previous line!
    (void) ch;              // We must always do the write, even if we're
                            // not checking assertions
}

const void *ipstream::find( P_id_type id )
{
    return objs.find( id );
}

void ipstream::registerObject( const void *adr )
{
    objs.registerObject( adr );
}

opstream::opstream() noexcept
{
    objs = new TPWrittenObjects;
}

opstream::opstream( streambuf * sb ) noexcept
{
    objs = new TPWrittenObjects;
    pstream::init( sb );
}

opstream::~opstream()
{
    objs->shutDown();
    delete objs;
}

opstream& opstream::seekp( streampos pos )
{
    objs->freeAll();
    objs->removeAll();
#ifdef __BORLANDC__
    bp->seekoff( pos, ios::beg );
#else
    bp->pubseekoff( pos, ios::beg );
#endif
    return *this;
}

opstream& opstream::seekp( streamoff pos, pstream::seekdir dir )
{
    objs->freeAll();
    objs->removeAll();
#ifdef __BORLANDC__
    bp->seekoff( pos, ::seekdir(dir) );
#else
    bp->pubseekoff( pos, ::seekdir(dir) );
#endif
    return *this;
}

streampos opstream::tellp()
{
#ifdef __BORLANDC__
    return bp->seekoff( 0, ios::cur, ios::out );
#else
    return bp->pubseekoff( 0, ios::cur, ios::out );
#endif
}

opstream& opstream::flush()
{
#ifdef __BORLANDC__
    bp->sync();
#else
    bp->pubsync();
#endif
    return *this;
}

void opstream::writeByte( uchar ch )
{
    bp->sputc( ch );
}

void opstream::writeBytes( const void *data, size_t sz )
{
    bp->sputn( (char *)data, sz );
}

void opstream::writeWord( ushort sh )
{
    bp->sputn( (char *)&sh, sizeof( ushort ) );
}

void opstream::writeString( const char *str )
{
    if( str == 0 )
        {
        writeByte( nullStringLen );
        return;
        }
    writeString( TStringView(str) );
}

void opstream::writeString( TStringView str )
{
    str = str.substr(0, nullStringLen - 1);
    writeByte( (uchar)str.size() );
    writeBytes( str.data(), str.size() );
}

opstream& operator << ( opstream& ps, char ch )
{
    ps.writeByte( ch );
    return ps;
}

opstream& operator << ( opstream& ps, signed char ch )
{
    ps.writeByte( ch );
    return ps;
}

opstream& operator << ( opstream& ps, unsigned char ch )
{
    ps.writeByte( ch );
    return ps;
}

opstream& operator << ( opstream& ps, signed short sh )
{
    ps.writeWord( sh );
    return ps;
}

opstream& operator << ( opstream& ps, unsigned short sh )
{
    ps.writeWord( sh );
    return ps;
}

opstream& operator << ( opstream& ps, signed int i )
{
    ps.writeBytes( &i, sizeof(int) );
    return ps;
}

opstream& operator << ( opstream& ps, unsigned int i )
{
    ps.writeBytes( &i, sizeof(int) );
    return ps;
}
opstream& operator << ( opstream& ps, signed long l )
{
    ps.writeBytes( &l, sizeof(l) );
    return ps;
}

opstream& operator << ( opstream& ps, unsigned long l )
{
    ps.writeBytes( &l, sizeof(l) );
    return ps;
}

opstream& operator << ( opstream& ps, float f )
{
    ps.writeBytes( &f, sizeof(f) );
    return ps;
}

opstream& operator << ( opstream& ps, double d )
{
    ps.writeBytes( &d, sizeof(d) );
    return ps;
}

opstream& operator << ( opstream& ps, long double ld )
{
    ps.writeBytes( &ld, sizeof(ld) );
    return ps;
}

opstream& operator << ( opstream& ps, TStreamable& t )
{
    ps.writePrefix( t );
    ps.writeData( t );
    ps.writeSuffix( t );
    return ps;
}

opstream& operator << ( opstream& ps, TStreamable *t )
{
    P_id_type index;
    if( t == 0 )
        ps.writeByte( pstream::ptNull );
    else if( (index = ps.find( t )) != P_id_notFound )
        {
        ps.writeByte( pstream::ptIndexed );
        ps.writeWord( index );
        }
    else
        {
        ps.writeByte( pstream::ptObject );
        ps << *t;
        }
    return ps;
}

void opstream::writePrefix( const TStreamable& t )
{
    writeByte( '[' );
    writeString( t.streamableName() );
}

void opstream::writeData( TStreamable& t )
{
    if( types->lookup( t.streamableName() ) == 0 )
        error( peNotRegistered, t );
    else
        {
        registerObject( &t );
        t.write( *this );
        }
}

void opstream::writeSuffix( const TStreamable& )
{
    writeByte( ']' );
}

P_id_type opstream::find( const void *adr )
{
    return objs->find( adr );
}

void opstream::registerObject( const void *adr )
{
    objs->registerObject( adr );
}

iopstream::iopstream( streambuf * sb ) noexcept
{
    pstream::init( sb );
}

iopstream::~iopstream()
{
}

iopstream::iopstream() noexcept
{
}

fpbase::fpbase() noexcept
{
    pstream::init( &buf );
}

fpbase::fpbase( const char *name, pstream::openmode omode)
{
    pstream::init( &buf );
    open( name, omode );
}

fpbase::~fpbase()
{
}

void fpbase::open( const char *b, pstream::openmode m)
{
    if( buf.is_open() )
        clear(ios::failbit);        // fail - already open
    else if( buf.open(b, ::openmode(m)) )
        clear(ios::goodbit);        // successful open
    else
        clear(ios::badbit);     // open failed
}

void fpbase::close()
{
    if( buf.close() )
        clear(ios::goodbit);
    else
        setstate(ios::failbit);
}

filebuf *fpbase::rdbuf() noexcept
{
    return &buf;
}

ifpstream::ifpstream() noexcept
{
}

ifpstream::ifpstream( const char* name, pstream::openmode omode) :
        fpbase( name, omode | ios::in | ios::binary)
{
}

ifpstream::~ifpstream()
{
}

filebuf *ifpstream::rdbuf() noexcept
{
    return fpbase::rdbuf();
}

void ifpstream::open( const char _FAR *name, pstream::openmode omode)
{
    fpbase::open( name, omode | ios::in | ios::binary);
}

ofpstream::ofpstream() noexcept
{
}

ofpstream::ofpstream( const char* name, pstream::openmode omode) :
        fpbase( name, omode | ios::out | ios::binary)
{
}

ofpstream::~ofpstream()
{
}

filebuf *ofpstream::rdbuf() noexcept
{
    return fpbase::rdbuf();
}

void ofpstream::open( const char _FAR *name, pstream::openmode omode)
{
    fpbase::open( name, omode | ios::out | ios::binary);
}

fpstream::fpstream() noexcept
{
}

fpstream::fpstream( const char* name, pstream::openmode omode) :
        fpbase( name, omode | ios::binary)
{
}

fpstream::~fpstream()
{
}

filebuf *fpstream::rdbuf() noexcept
{
    return fpbase::rdbuf();
}

void fpstream::open( const char _FAR *name, pstream::openmode omode)
{
    fpbase::open( name, omode | ios::binary);
}


