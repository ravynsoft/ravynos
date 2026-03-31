//
//  bjstring.h
//  TestTB
//
//  Created by Terrin Eager on 9/26/12.
//
//

#ifndef __TestTB__bjstring__
#define __TestTB__bjstring__

#include <iostream>
#include "bjtypes.h"

class BJString
{

public:
    BJString();
    BJString(const BJString& scr);
    BJString(const char* str);
    virtual ~BJString();

    BJString& operator=(const char* str);
    BJString& operator=(const BJString& str);
    bool operator==(const char* str);
    bool operator!=(const char* str){return !operator==(str);};
    bool operator==(const BJString& str);
    bool operator!=(const BJString& str) {return !operator==(str);};
    bool operator<(const BJString& str) const;

    BJ_COMPARE Compare(const BJString& str);


    BJString& operator+=(const char* str);
    BJString& operator+=(const BJString& str);

    const char* GetBuffer() const;

    void Set(const char* str);
    void Set(const char* str,BJ_UINT32 len);

    void Append(const char* str, BJ_UINT32 len);

    bool Contains(const char* str);

    BJ_UINT32 GetUINT32();

    enum BJ_FORMAT_STYLE {BJSS_BYTE,BJSS_TIME} ;
    void Format(BJ_UINT64 number,BJ_FORMAT_STYLE style);

    BJ_UINT32 GetLength();

    BJ_UINT32 GetBufferLength(){return length;};

private:

    void Create(BJ_UINT32 len);
    char* buffer;
    BJ_UINT32 length;
};

#endif /* defined(__TestTB__bjstring__) */
