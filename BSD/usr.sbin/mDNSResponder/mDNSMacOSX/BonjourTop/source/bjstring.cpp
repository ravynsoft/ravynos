//
//  bjstring.cpp
//  TestTB
//
//  Created by Terrin Eager on 9/26/12.
//
//

#include "bjstring.h"
#include <time.h>


BJString::BJString()
{
    buffer = NULL;
    length = 0;
}

BJString::BJString(const BJString& scr)
{
    buffer = NULL;
    length = 0;
    Set(scr.GetBuffer());
}
BJString::BJString(const char* str)
{
    buffer = NULL;
    length = 0;
    Set(str);
}

BJString::~BJString()
{
    delete[] buffer;
    buffer = NULL;
}


BJString& BJString::operator=(const char* str)
{
    Set(str);
    return *this;
}

BJString& BJString::operator=(const BJString& str)
{
    if (&str != this)
        Set(str.GetBuffer());
    return *this;
}
bool BJString::operator==(const char* str)
{
    if (buffer == NULL && str == NULL)
        return true;
    if (buffer == NULL || str == NULL)
        return false;

    return (strcmp(str,buffer) == 0);
}
bool BJString::operator==(const BJString& str)
{
    if (buffer == NULL && str.GetBuffer() == NULL)
        return true;
    if (buffer == NULL || str.GetBuffer() == NULL)
        return false;
    return (strcmp(str.GetBuffer(),buffer) == 0);
}

bool BJString::operator<(const BJString& str) const
{
    const char* myBuff = GetBuffer();
    const char* otherBuff = str.GetBuffer();

    if (myBuff == NULL && otherBuff == NULL)
        return false;
    if (myBuff != NULL && otherBuff == NULL)
        return false;
    if (myBuff == NULL && otherBuff != NULL)
        return true;

    int cmp = strcmp(myBuff, otherBuff);

    if (cmp < 0)
        return true;
    else
        return false;

}

BJ_COMPARE BJString::Compare(const BJString& str)
{
    const char* myBuff = GetBuffer();
    const char* otherBuff = str.GetBuffer();

    if (myBuff == NULL && otherBuff == NULL)
        return BJ_EQUAL;
    if (myBuff != NULL && otherBuff == NULL)
        return BJ_GT;
    if (myBuff == NULL && otherBuff != NULL)
        return BJ_LT;

    int cmp = strcmp(myBuff, otherBuff);

    if (cmp > 0)
        return (BJ_GT);
    else if (cmp < 0)
        return (BJ_LT);
    else
        return (BJ_EQUAL);

}

BJString& BJString::operator+=(const char* str)
{
    if (buffer == NULL)
        return operator=(str);
    if (str == NULL)
        return *this;

    BJString temp = buffer;
    Create((BJ_UINT32)(strlen(buffer) + strlen(str)));
    strlcpy(buffer, temp.GetBuffer(), length + 1);
    strlcat(buffer, str, length + 1);
    return *this;
}
BJString& BJString::operator+=(const BJString&str)
{
    operator+=(str.GetBuffer());
    return *this;
}


const char* BJString::GetBuffer() const
{
    return buffer;
}

void BJString::Set(const char* str)
{

    BJ_UINT32 len = str?(BJ_UINT32)strlen(str):0;
    if (len > 255)
        len = 250;
    Create(len);
    if (buffer && str)
        strlcpy(buffer, str, length + 1);

}
void BJString::Set(const char* str, BJ_UINT32 len)
{
    Create(len);
    if (buffer)
    {
        if (str)
            strncpy(buffer, str, len);
        else
            memset(buffer, 0, length);
    }
}

void BJString::Append(const char* str, BJ_UINT32 len)
{
    if (length < (strlen(buffer) + strlen(str)))
    {
        BJString temp = buffer;
        Create((BJ_UINT32)(strlen(buffer) + strlen(str)));
        if (buffer && temp.buffer)
            strlcpy(buffer, temp.GetBuffer(), length + 1);
    }
    strncat(buffer,str,len);
}

bool BJString::Contains(const char* str)
{
    if (buffer == NULL && str == NULL)
        return true;
    if (buffer == NULL ||  str == NULL)
        return false;
    return (strstr(buffer,str) != NULL);
}

BJ_UINT32 BJString::GetUINT32()
{
    if (buffer == NULL)
        return 0;

    return atoi(buffer);
}

void BJString::Format(BJ_UINT64 number,BJ_FORMAT_STYLE style)
{
    switch (style) {
        case BJSS_BYTE:
            Create(32);
            sprintf(buffer,"%llu",number);
            break;
        case BJSS_TIME:
        {
            char formatedTime[24];
            time_t timeValue = number;
            struct tm* timeStruct = localtime(&timeValue);
            strftime(formatedTime, sizeof(formatedTime), "%Y-%m-%d_%T_%a", timeStruct);
            Set(formatedTime);
            break;
        }
        default:
            break;
    }
}


void BJString::Create(BJ_UINT32 len)
{
    if (buffer)
    {
        if (length >= len)
        {
            memset(buffer, 0, len + 1);
            return;
        }
        delete buffer;
        buffer = NULL;
        length = 0;
    }

    buffer = new char[len+1];
    if (buffer)
    {
        memset(buffer, 0, len+1);
        length = len;
    }
}

BJ_UINT32 BJString::GetLength()
{
    return  buffer?(BJ_UINT32)strlen(buffer):0;
}
