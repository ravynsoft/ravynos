/* vdir.h
 *
 * (c) 1999 Microsoft Corporation. All rights reserved. 
 * Portions (c) 1999 ActiveState Tool Corp, http://www.ActiveState.com/
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 */

#ifndef ___VDir_H___
#define ___VDir_H___

/*
 * Allow one slot for each possible drive letter
 * and one additional slot for a UNC name
 */
const int driveCount = ('Z'-'A')+1+1;
const int driveLetterCount = ('Z'-'A')+1;

class VDir
{
public:
    VDir(int bManageDir = 1);
    ~VDir() {};

    void Init(VDir* pDir, VMem *pMem);
    void SetDefaultA(char const *pDefault);
    void SetDefaultW(WCHAR const *pDefault);
    char* MapPathA(const char *pInName);
    WCHAR* MapPathW(const WCHAR *pInName);
    int SetCurrentDirectoryA(char *lpBuffer);
    int SetCurrentDirectoryW(WCHAR *lpBuffer);
    inline int GetDefault(void) { return nDefault; };

    inline char* GetCurrentDirectoryA(int dwBufSize, char *lpBuffer)
    {
        char* ptr = dirTableA[nDefault];
        while (--dwBufSize)
        {
            if ((*lpBuffer++ = *ptr++) == '\0')
                break;
        }
        *lpBuffer = '\0';
        return /* unused */ NULL;
    };
    inline WCHAR* GetCurrentDirectoryW(int dwBufSize, WCHAR *lpBuffer)
    {
        WCHAR* ptr = dirTableW[nDefault];
        while (--dwBufSize)
        {
            if ((*lpBuffer++ = *ptr++) == '\0')
                break;
        }
        *lpBuffer = '\0';
        return /* unused */ NULL;
    };

    DWORD CalculateEnvironmentSpace(void);
    LPSTR BuildEnvironmentSpace(LPSTR lpStr);

protected:
    int SetDirA(char const *pPath, int index);
    int SetDirW(WCHAR const *pPath, int index);
    void FromEnvA(char *pEnv, int index);
    void FromEnvW(WCHAR *pEnv, int index);

    inline const char *GetDefaultDirA(void)
    {
        return dirTableA[nDefault];
    };
    inline void SetDefaultDirA(char const *pPath, int index)
    {
        SetDirA(pPath, index);
        nDefault = index;
    };
    inline const WCHAR *GetDefaultDirW(void)
    {
        return dirTableW[nDefault];
    };
    inline void SetDefaultDirW(WCHAR const *pPath, int index)
    {
        SetDirW(pPath, index);
        nDefault = index;
    };
    inline const char *GetDirA(int index)
    {
        char *ptr = dirTableA[index];
        if (!ptr) {
            /* simulate the existence of this drive */
            ptr = szLocalBufferA;
            ptr[0] = 'A' + index;
            ptr[1] = ':';
            ptr[2] = '\\';
            ptr[3] = 0;
        }
        return ptr;
    };
    inline const WCHAR *GetDirW(int index)
    {
        WCHAR *ptr = dirTableW[index];
        if (!ptr) {
            /* simulate the existence of this drive */
            ptr = szLocalBufferW;
            ptr[0] = 'A' + index;
            ptr[1] = ':';
            ptr[2] = '\\';
            ptr[3] = 0;
        }
        return ptr;
    };

    inline int DriveIndex(char chr)
    {
        if (chr == '\\' || chr == '/')
            return ('Z'-'A')+1;
        return (chr | 0x20)-'a';
    };

    VMem *pMem;
    int nDefault, bManageDirectory;
    char *dirTableA[driveCount];
    char szLocalBufferA[MAX_PATH+1];
    WCHAR *dirTableW[driveCount];
    WCHAR szLocalBufferW[MAX_PATH+1];
};


VDir::VDir(int bManageDir /* = 1 */)
{
    nDefault = 0;
    bManageDirectory = bManageDir;
    memset(dirTableA, 0, sizeof(dirTableA));
    memset(dirTableW, 0, sizeof(dirTableW));
}

void VDir::Init(VDir* pDir, VMem *p)
{
    int index;

    pMem = p;
    if (pDir) {
        for (index = 0; index < driveCount; ++index) {
            SetDirW(pDir->GetDirW(index), index);
        }
        nDefault = pDir->GetDefault();
    }
    else {
        int bSave = bManageDirectory;
        DWORD driveBits = GetLogicalDrives();

        bManageDirectory = 0;
        WCHAR szBuffer[MAX_PATH*driveCount];
        if (GetLogicalDriveStringsW(sizeof(szBuffer), szBuffer)) {
            WCHAR* pEnv = GetEnvironmentStringsW();
            WCHAR* ptr = szBuffer;
            for (index = 0; index < driveCount; ++index) {
                if (driveBits & (1<<index)) {
                    ptr += SetDirW(ptr, index) + 1;
                    FromEnvW(pEnv, index);
                }
            }
            FreeEnvironmentStringsW(pEnv);
        }
        SetDefaultW(L".");
        bManageDirectory = bSave;
  }
}

int VDir::SetDirA(char const *pPath, int index)
{
    char chr, *ptr;
    int length = 0;
    WCHAR wBuffer[MAX_PATH+1];
    if (index < driveCount && pPath != NULL) {
        length = strlen(pPath);
        pMem->Free(dirTableA[index]);
        ptr = dirTableA[index] = (char*)pMem->Malloc(length+2);
        if (ptr != NULL) {
            strcpy(ptr, pPath);
            ptr += length-1;
            chr = *ptr++;
            if (chr != '\\' && chr != '/') {
                *ptr++ = '\\';
                *ptr = '\0';
            }
            MultiByteToWideChar(CP_ACP, 0, dirTableA[index], -1,
                    wBuffer, (sizeof(wBuffer)/sizeof(WCHAR)));
            length = wcslen(wBuffer);
            pMem->Free(dirTableW[index]);
            dirTableW[index] = (WCHAR*)pMem->Malloc((length+1)*2);
            if (dirTableW[index] != NULL) {
                wcscpy(dirTableW[index], wBuffer);
            }
        }
    }

    if(bManageDirectory)
        ::SetCurrentDirectoryA(pPath);

    return length;
}

void VDir::FromEnvA(char *pEnv, int index)
{   /* gets the directory for index from the environment variable. */
    while (*pEnv != '\0') {
        if ((pEnv[0] == '=') && (DriveIndex(pEnv[1]) == index)
            && pEnv[2] == ':' && pEnv[3] == '=') {
            SetDirA(&pEnv[4], index);
            break;
        }
        else
            pEnv += strlen(pEnv)+1;
    }
}

void VDir::FromEnvW(WCHAR *pEnv, int index)
{   /* gets the directory for index from the environment variable. */
    while (*pEnv != '\0') {
        if ((pEnv[0] == '=') && (DriveIndex((char)pEnv[1]) == index)
            && pEnv[2] == ':' && pEnv[3] == '=') {
            SetDirW(&pEnv[4], index);
            break;
        }
        else
            pEnv += wcslen(pEnv)+1;
    }
}

void VDir::SetDefaultA(char const *pDefault)
{
    char szBuffer[MAX_PATH+1];
    char *pPtr;

    if (GetFullPathNameA(pDefault, sizeof(szBuffer), szBuffer, &pPtr)) {
        if (*pDefault != '.' && pPtr != NULL)
            *pPtr = '\0';

        SetDefaultDirA(szBuffer, DriveIndex(szBuffer[0]));
    }
}

int VDir::SetDirW(WCHAR const *pPath, int index)
{
    WCHAR chr, *ptr;
    int length = 0;
    if (index < driveCount && pPath != NULL) {
        length = wcslen(pPath);
        pMem->Free(dirTableW[index]);
        ptr = dirTableW[index] = (WCHAR*)pMem->Malloc((length+2)*2);
        if (ptr != NULL) {
            char *ansi;
            wcscpy(ptr, pPath);
            ptr += length-1;
            chr = *ptr++;
            if (chr != '\\' && chr != '/') {
                *ptr++ = '\\';
                *ptr = '\0';
            }
            ansi = win32_ansipath(dirTableW[index]);
            length = strlen(ansi);
            pMem->Free(dirTableA[index]);
            dirTableA[index] = (char*)pMem->Malloc(length+1);
            if (dirTableA[index] != NULL) {
                strcpy(dirTableA[index], ansi);
            }
            win32_free(ansi);
        }
    }

    if(bManageDirectory)
        ::SetCurrentDirectoryW(pPath);

    return length;
}

void VDir::SetDefaultW(WCHAR const *pDefault)
{
    WCHAR szBuffer[MAX_PATH+1];
    WCHAR *pPtr;

    if (GetFullPathNameW(pDefault, (sizeof(szBuffer)/sizeof(WCHAR)), szBuffer, &pPtr)) {
        if (*pDefault != '.' && pPtr != NULL)
            *pPtr = '\0';

        SetDefaultDirW(szBuffer, DriveIndex((char)szBuffer[0]));
    }
}

inline BOOL IsPathSep(char ch)
{
    return (ch == '\\' || ch == '/');
}

inline void DoGetFullPathNameA(char* lpBuffer, DWORD dwSize, char* Dest)
{
    char *pPtr;

    /*
     * On WinNT GetFullPathName does not fail, (or at least always
     * succeeds when the drive is valid) WinNT does set *Dest to NULL
     * On Win98 GetFullPathName will set last error if it fails, but
     * does not touch *Dest
     */
    *Dest = '\0';
    GetFullPathNameA(lpBuffer, dwSize, Dest, &pPtr);
}

inline bool IsSpecialFileName(const char* pName)
{
    /* specical file names are devices that the system can open
     * these include AUX, CON, NUL, PRN, COMx, LPTx, CLOCK$, CONIN$, CONOUT$
     * (x is a single digit, and names are case-insensitive)
     */
    char ch = (pName[0] & ~0x20);
    switch (ch)
    {
        case 'A': /* AUX */
            if (((pName[1] & ~0x20) == 'U')
                && ((pName[2] & ~0x20) == 'X')
                && !pName[3])
                    return true;
            break;
        case 'C': /* CLOCK$, COMx,  CON, CONIN$ CONOUT$ */
            ch = (pName[1] & ~0x20);
            switch (ch)
            {
                case 'L': /* CLOCK$ */
                    if (((pName[2] & ~0x20) == 'O')
                        && ((pName[3] & ~0x20) == 'C')
                        && ((pName[4] & ~0x20) == 'K')
                        && (pName[5] == '$')
                        && !pName[6])
                            return true;
                    break;
                case 'O': /* COMx,  CON, CONIN$ CONOUT$ */
                    if ((pName[2] & ~0x20) == 'M') {
                        if (    inRANGE(pName[3], '1', '9')
                            && !pName[4])
                            return true;
                    }
                    else if ((pName[2] & ~0x20) == 'N') {
                        if (!pName[3])
                            return true;
                        else if ((pName[3] & ~0x20) == 'I') {
                            if (((pName[4] & ~0x20) == 'N')
                                && (pName[5] == '$')
                                && !pName[6])
                            return true;
                        }
                        else if ((pName[3] & ~0x20) == 'O') {
                            if (((pName[4] & ~0x20) == 'U')
                                && ((pName[5] & ~0x20) == 'T')
                                && (pName[6] == '$')
                                && !pName[7])
                            return true;
                        }
                    }
                    break;
            }
            break;
        case 'L': /* LPTx */
            if (((pName[1] & ~0x20) == 'U')
                && ((pName[2] & ~0x20) == 'X')
                &&  inRANGE(pName[3], '1', '9')
                && !pName[4])
                    return true;
            break;
        case 'N': /* NUL */
            if (((pName[1] & ~0x20) == 'U')
                && ((pName[2] & ~0x20) == 'L')
                && !pName[3])
                    return true;
            break;
        case 'P': /* PRN */
            if (((pName[1] & ~0x20) == 'R')
                && ((pName[2] & ~0x20) == 'N')
                && !pName[3])
                    return true;
            break;
    }
    return false;
}

char *VDir::MapPathA(const char *pInName)
{   /*
     * possiblities -- relative path or absolute path with or without drive letter
     * OR UNC name
     */
    int driveIndex;
    char szBuffer[(MAX_PATH+1)*2];
    char szlBuf[MAX_PATH+1];
    int length = strlen(pInName);

    if (!length)
        return (char*)pInName;

    if (length > MAX_PATH) {
        strncpy(szlBuf, pInName, MAX_PATH);
        if (IsPathSep(pInName[0]) && !IsPathSep(pInName[1])) {   
            /* absolute path - reduce length by 2 for drive specifier */
            szlBuf[MAX_PATH-2] = '\0';
        }
        else
            szlBuf[MAX_PATH] = '\0';
        pInName = szlBuf;
    }
    /* strlen(pInName) is now <= MAX_PATH */

    if (length > 1 && pInName[1] == ':') {
        /* has drive letter */
        if (length > 2 && IsPathSep(pInName[2])) {
            /* absolute with drive letter */
            DoGetFullPathNameA((char*)pInName, sizeof(szLocalBufferA), szLocalBufferA);
        }
        else {
            /* relative path with drive letter */
            driveIndex = DriveIndex(*pInName);
            if (driveIndex < 0 || driveIndex >= driveLetterCount)
                return (char *)pInName;
            strcpy(szBuffer, GetDirA(driveIndex));
            strcat(szBuffer, &pInName[2]);
            if(strlen(szBuffer) > MAX_PATH)
                szBuffer[MAX_PATH] = '\0';

            DoGetFullPathNameA(szBuffer, sizeof(szLocalBufferA), szLocalBufferA);
        }
    }
    else {
        /* no drive letter */
        if (length > 1 && IsPathSep(pInName[1]) && IsPathSep(pInName[0])) {
            /* UNC name */
            DoGetFullPathNameA((char*)pInName, sizeof(szLocalBufferA), szLocalBufferA);
        }
        else {
            strcpy(szBuffer, GetDefaultDirA());
            if (IsPathSep(pInName[0])) {
                /* absolute path */
                strcpy(&szBuffer[2], pInName);
                DoGetFullPathNameA(szBuffer, sizeof(szLocalBufferA), szLocalBufferA);
            }
            else {
                /* relative path */
                if (IsSpecialFileName(pInName)) {
                    return (char*)pInName;
                }
                else {
                    strcat(szBuffer, pInName);
                    if (strlen(szBuffer) > MAX_PATH)
                        szBuffer[MAX_PATH] = '\0';

                    DoGetFullPathNameA(szBuffer, sizeof(szLocalBufferA), szLocalBufferA);
                }
            }
        }
    }

    return szLocalBufferA;
}

int VDir::SetCurrentDirectoryA(char *lpBuffer)
{
    char *pPtr;
    int length, nRet = -1;

    pPtr = MapPathA(lpBuffer);
    length = strlen(pPtr);
    if(length > 3 && IsPathSep(pPtr[length-1])) {
        /* don't remove the trailing slash from 'x:\'  */
        pPtr[length-1] = '\0';
    }

    DWORD r = GetFileAttributesA(pPtr);
    if ((r != 0xffffffff) && (r & FILE_ATTRIBUTE_DIRECTORY))
    {
        char szBuffer[(MAX_PATH+1)*2];
        DoGetFullPathNameA(pPtr, sizeof(szBuffer), szBuffer);
        SetDefaultDirA(szBuffer, DriveIndex(szBuffer[0]));
        nRet = 0;
    }

    return nRet;
}

DWORD VDir::CalculateEnvironmentSpace(void)
{   /* the current directory environment strings are stored as '=D:=d:\path' */
    int index;
    DWORD dwSize = 0;
    for (index = 0; index < driveCount; ++index) {
        if (dirTableA[index] != NULL) {
            dwSize += strlen(dirTableA[index]) + 5;  /* add 1 for trailing NULL and 4 for '=D:=' */
        }
    }
    return dwSize;
}

LPSTR VDir::BuildEnvironmentSpace(LPSTR lpStr)
{   /* store the current directory environment strings as '=D:=d:\path' */
    int index, length;
    LPSTR lpDirStr;
    for (index = 0; index < driveCount; ++index) {
        lpDirStr = dirTableA[index];
        if (lpDirStr != NULL) {
            lpStr[0] = '=';
            lpStr[1] = lpDirStr[0];
            lpStr[2] = '\0';
            CharUpper(&lpStr[1]);
            lpStr[2] = ':';
            lpStr[3] = '=';
            strcpy(&lpStr[4], lpDirStr);
            length = strlen(lpDirStr);
            lpStr += length + 5; /* add 1 for trailing NULL and 4 for '=D:=' */
            if (length > 3 && IsPathSep(lpStr[-2])) {
                lpStr[-2] = '\0';   /* remove the trailing path separator */
                --lpStr;
            }
        }
    }
    return lpStr;
}

inline BOOL IsPathSep(WCHAR ch)
{
    return (ch == '\\' || ch == '/');
}

inline void DoGetFullPathNameW(WCHAR* lpBuffer, DWORD dwSize, WCHAR* Dest)
{
    WCHAR *pPtr;

    /*
     * On WinNT GetFullPathName does not fail, (or at least always
     * succeeds when the drive is valid) WinNT does set *Dest to NULL
     * On Win98 GetFullPathName will set last error if it fails, but
     * does not touch *Dest
     */
    *Dest = '\0';
    GetFullPathNameW(lpBuffer, dwSize, Dest, &pPtr);
}

inline bool IsSpecialFileName(const WCHAR* pName)
{
    /* specical file names are devices that the system can open
     * these include AUX, CON, NUL, PRN, COMx, LPTx, CLOCK$, CONIN$, CONOUT$
     * (x is a single digit, and names are case-insensitive)
     */
    WCHAR ch = (pName[0] & ~0x20);
    switch (ch)
    {
        case 'A': /* AUX */
            if (((pName[1] & ~0x20) == 'U')
                && ((pName[2] & ~0x20) == 'X')
                && !pName[3])
                    return true;
            break;
        case 'C': /* CLOCK$, COMx,  CON, CONIN$ CONOUT$ */
            ch = (pName[1] & ~0x20);
            switch (ch)
            {
                case 'L': /* CLOCK$ */
                    if (((pName[2] & ~0x20) == 'O')
                        && ((pName[3] & ~0x20) == 'C')
                        && ((pName[4] & ~0x20) == 'K')
                        && (pName[5] == '$')
                        && !pName[6])
                            return true;
                    break;
                case 'O': /* COMx,  CON, CONIN$ CONOUT$ */
                    if ((pName[2] & ~0x20) == 'M') {
                        if (    inRANGE(pName[3], '1', '9')
                            && !pName[4])
                            return true;
                    }
                    else if ((pName[2] & ~0x20) == 'N') {
                        if (!pName[3])
                            return true;
                        else if ((pName[3] & ~0x20) == 'I') {
                            if (((pName[4] & ~0x20) == 'N')
                                && (pName[5] == '$')
                                && !pName[6])
                            return true;
                        }
                        else if ((pName[3] & ~0x20) == 'O') {
                            if (((pName[4] & ~0x20) == 'U')
                                && ((pName[5] & ~0x20) == 'T')
                                && (pName[6] == '$')
                                && !pName[7])
                            return true;
                        }
                    }
                    break;
            }
            break;
        case 'L': /* LPTx */
            if (((pName[1] & ~0x20) == 'U')
                && ((pName[2] & ~0x20) == 'X')
                &&  inRANGE(pName[3], '1', '9')
                && !pName[4])
                    return true;
            break;
        case 'N': /* NUL */
            if (((pName[1] & ~0x20) == 'U')
                && ((pName[2] & ~0x20) == 'L')
                && !pName[3])
                    return true;
            break;
        case 'P': /* PRN */
            if (((pName[1] & ~0x20) == 'R')
                && ((pName[2] & ~0x20) == 'N')
                && !pName[3])
                    return true;
            break;
    }
    return false;
}

WCHAR* VDir::MapPathW(const WCHAR *pInName)
{   /*
     * possiblities -- relative path or absolute path with or without drive letter
     * OR UNC name
     */
    int driveIndex;
    WCHAR szBuffer[(MAX_PATH+1)*2];
    WCHAR szlBuf[MAX_PATH+1];
    int length = wcslen(pInName);

    if (!length)
        return (WCHAR*)pInName;

    if (length > MAX_PATH) {
        wcsncpy(szlBuf, pInName, MAX_PATH);
        if (IsPathSep(pInName[0]) && !IsPathSep(pInName[1])) {   
            /* absolute path - reduce length by 2 for drive specifier */
            szlBuf[MAX_PATH-2] = '\0';
        }
        else
            szlBuf[MAX_PATH] = '\0';
        pInName = szlBuf;
    }
    /* strlen(pInName) is now <= MAX_PATH */

    if (length > 1 && pInName[1] == ':') {
        /* has drive letter */
        if (IsPathSep(pInName[2])) {
            /* absolute with drive letter */
            DoGetFullPathNameW((WCHAR*)pInName, (sizeof(szLocalBufferW)/sizeof(WCHAR)), szLocalBufferW);
        }
        else {
            /* relative path with drive letter */
            driveIndex = DriveIndex(*pInName);
            if (driveIndex < 0 || driveIndex >= driveLetterCount)
                return (WCHAR *)pInName;
            wcscpy(szBuffer, GetDirW(driveIndex));
            wcscat(szBuffer, &pInName[2]);
            if(wcslen(szBuffer) > MAX_PATH)
                szBuffer[MAX_PATH] = '\0';

            DoGetFullPathNameW(szBuffer, (sizeof(szLocalBufferW)/sizeof(WCHAR)), szLocalBufferW);
        }
    }
    else {
        /* no drive letter */
        if (length > 1 && IsPathSep(pInName[1]) && IsPathSep(pInName[0])) {
            /* UNC name */
            DoGetFullPathNameW((WCHAR*)pInName, (sizeof(szLocalBufferW)/sizeof(WCHAR)), szLocalBufferW);
        }
        else {
            wcscpy(szBuffer, GetDefaultDirW());
            if (IsPathSep(pInName[0])) {
                /* absolute path */
                wcscpy(&szBuffer[2], pInName);
                DoGetFullPathNameW(szBuffer, (sizeof(szLocalBufferW)/sizeof(WCHAR)), szLocalBufferW);
            }
            else {
                /* relative path */
                if (IsSpecialFileName(pInName)) {
                    return (WCHAR*)pInName;
                }
                else {
                    wcscat(szBuffer, pInName);
                    if (wcslen(szBuffer) > MAX_PATH)
                        szBuffer[MAX_PATH] = '\0';

                    DoGetFullPathNameW(szBuffer, (sizeof(szLocalBufferW)/sizeof(WCHAR)), szLocalBufferW);
                }
            }
        }
    }
    return szLocalBufferW;
}

int VDir::SetCurrentDirectoryW(WCHAR *lpBuffer)
{
    WCHAR *pPtr;
    int length, nRet = -1;

    pPtr = MapPathW(lpBuffer);
    length = wcslen(pPtr);
    if(length > 3 && IsPathSep(pPtr[length-1])) {
        /* don't remove the trailing slash from 'x:\'  */
        pPtr[length-1] = '\0';
    }

    DWORD r = GetFileAttributesW(pPtr);
    if ((r != 0xffffffff) && (r & FILE_ATTRIBUTE_DIRECTORY))
    {
        WCHAR wBuffer[(MAX_PATH+1)*2];
        DoGetFullPathNameW(pPtr, (sizeof(wBuffer)/sizeof(WCHAR)), wBuffer);
        SetDefaultDirW(wBuffer, DriveIndex((char)wBuffer[0]));
        nRet = 0;
    }

    return nRet;
}

#endif	/* ___VDir_H___ */
