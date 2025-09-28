/* Copyright (c) 2010 Glenn Ganz
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSProcessInfo.h>

#define _STRUCTURED_PROC 1
#import <sys/procfs.h>
#import <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


FOUNDATION_EXPORT void __attribute__ ((constructor)) libmain(void)
{
    
    char        **argValues = NULL;
    static char **nArgValues=NULL;
    char        psinfoFile[80];
    psinfo_t    pinfo;
    int         argCount = 0;
    pid_t       p=getpid();
    
    sprintf(psinfoFile, "/proc/%d/psinfo", p);    
    FILE *f = fopen(psinfoFile, "r");
    if (f == NULL) {
        fprintf(stderr, "Error during Cocotron initialization: Failed to open ps info file for pid [%d]", p);
        abort();
    }  
    
    fread(&pinfo, sizeof(pinfo), 1, f);
    fclose(f);
    
    argValues = (char **)pinfo.pr_argv;
    while(argValues[argCount] != NULL) {
        argCount++;
    }
    
    nArgValues = (char**)malloc(sizeof(char*) * (argCount));
    
    for (int i = 0; i < argCount; i++) {
        nArgValues[i] = (char *)strdup(argValues[i]);
    }
    
    __NSInitializeProcess(argCount, (const char **)nArgValues);
}