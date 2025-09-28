/*-----------------------------------------------------*/
/*                                                     */
/*   Turbo Vision TVHC header file                     */
/*                                                     */
/*-----------------------------------------------------*/

/* $Copyright: 1994 */

#if !defined( __TVHC_H )
#define __TVHC_H

#define Uses_fstream
#define Uses_TSortedCollection
#define Uses_TObject
#define Uses_TPoint
#include <tvision/tv.h>

#if !defined( __HELPBASE_H )
#include "tvision/helpbase.h"
#endif  // __HELPBASE_H

#if !defined( __DIR_H )
#include <dir.h>
#endif  // __DIR_H

const int MAXSTRSIZE=256;
const int MAXHELPTOPICID=16379;
const char commandChar[] = ".";

enum State { undefined, wrapping, notWrapping };

class TProtectedStream : public fstream
{

public:

#ifdef __BORLANDC__
    typedef ios::open_mode openmode;
#else
    typedef ios::openmode openmode;
#endif

    TProtectedStream( const char *aFileName, openmode aMode );

private:

    char fileName[MAXPATH];
    openmode mode;

};

// Topic Reference

struct TFixUp
{

    streampos pos;
    TFixUp *next;

};

union Content
{

    uint value;
    TFixUp *fixUpList;

};

struct TReference
{

    char *topic;
    Boolean resolved;
    Content val;

};

class TRefTable : public TSortedCollection
{

public:

    TRefTable( ccIndex aLimit, ccIndex aDelta );

    virtual int compare( void *key1,void *key2 );
    virtual void freeItem( void *item );
    TReference *getReference( const char *topic );
    virtual void *keyOf( void *item );

private:

    virtual void *readItem( ipstream& ) { return 0; };
    virtual void writeItem( void *, opstream& ) {};

};

struct TCrossRefNode
{
    char *topic;
    int offset;
    uchar length;
    TCrossRefNode *next;

};

class TTopicDefinition : public TObject
{

public:

    TTopicDefinition(const char *aTopic, uint aValue);
    ~TTopicDefinition(void);

    char *topic;
    uint value;
    TTopicDefinition *next;

};

#endif  // __TVHC_H
