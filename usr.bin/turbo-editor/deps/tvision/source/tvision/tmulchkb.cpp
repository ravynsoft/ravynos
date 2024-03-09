/*------------------------------------------------------------*/
/* filename -       tmulchkb.cpp                              */
/*                                                            */
/* function(s)                                                */
/*          TMultiCheckBox member functions                   */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TMultiCheckBoxes
#define Uses_ipstream
#define Uses_opstream
#include <tvision/tv.h>

TMultiCheckBoxes::TMultiCheckBoxes( TRect& bounds, TSItem* aStrings,
                                    uchar aSelRange, ushort aFlags,
                                    const char* aStates) noexcept :
    TCluster(bounds, aStrings)
{
    selRange = aSelRange;
    flags = aFlags;
    states = newStr(aStates);
}

#if !defined(NO_STREAMABLE)

TMultiCheckBoxes::TMultiCheckBoxes( StreamableInit ) noexcept :
    TCluster( streamableInit )
{
}

void* TMultiCheckBoxes::read( ipstream& is )
{
    TCluster::read(is);
    is >> selRange >> flags;
    states = is.readString();

    return this;
}

void TMultiCheckBoxes::write( opstream& os )
{
    TCluster::write( os );
    os << selRange << flags;
    os.writeString(states);
}

TStreamable* TMultiCheckBoxes::build()
{
    return new TMultiCheckBoxes( streamableInit );
}

#endif

TMultiCheckBoxes::~TMultiCheckBoxes()
{
    delete states;
}

void TMultiCheckBoxes::draw()
{
    drawMultiBox(" [ ] ", states);
}

ushort TMultiCheckBoxes::dataSize()
{
    return  sizeof(int32_t);
}

uchar TMultiCheckBoxes::multiMark(int item)
{
    int flo = flags & 0xff;
    int fhi = (flags >> 8) * item;
    return (int32_t)((value & ((int32_t)flo << fhi)) >> fhi);
}

void TMultiCheckBoxes::getData(void* p)
{
    *(uint32_t*)p = value;
    drawView();
}

void TMultiCheckBoxes::press(int item)
{
    short curState;

    int flo = flags & 0xff;
    int fhi = (flags >> 8) * item;

    curState = (short)((value & ((int32_t)flo << fhi)) >> fhi);

    curState++;

    if (curState >= selRange)
        curState = 0;

    value = (int32_t)((value & ~((int32_t)flo << fhi)) | (curState << fhi));
}

void TMultiCheckBoxes::setData(void* p)
{
    value = *(uint32_t*)p;
    drawView();
}

