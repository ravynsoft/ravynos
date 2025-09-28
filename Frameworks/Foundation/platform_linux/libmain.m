/* Copyright (c) 2010 Glenn Ganz
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#ifdef LINUX
#import <Foundation/NSProcessInfo.h>
#import <stdio.h>
#import <string.h>

#define READ_BUFFER_SIZE 2048

FOUNDATION_EXPORT void __attribute__ ((constructor)) libmain(void)
{
    
    char **argValues=NULL;
    static char **nArgValues=NULL;
    
    FILE *commandLineFile;    
    
    char psinfofile[32] = {0};
    
    commandLineFile = fopen("/proc/self/cmdline", "r");
    if (commandLineFile == NULL) {
        fprintf(stderr, "Error during Cocotron initialization: Failed to open command line file [/proc/self/cmdline]");
        abort();
    }
    
    char buffer[READ_BUFFER_SIZE]; 
    int argCount = 0;    

    while( fgets(buffer, READ_BUFFER_SIZE, commandLineFile) ) { 
        size_t n = 0; 
        while( n < READ_BUFFER_SIZE && buffer[n] != '\0' ) { 
            argCount++;
            n += strlen(buffer+n) + 1; 
        } 
    }
    
    fseek ( commandLineFile , 0 , SEEK_SET );
    
    nArgValues = (char**)malloc(sizeof(char*) * (argCount));
    
    int c = 0;
    while( fgets(buffer, READ_BUFFER_SIZE, commandLineFile) ) { 
        size_t n = 0; 
        while( n < READ_BUFFER_SIZE && buffer[n] != '\0' ) { 
            nArgValues[c] = (char *)strdup(buffer+n);
            n += strlen(buffer+n) + 1; 
            c++;
        } 
    }   
    
    __NSInitializeProcess(argCount, (const char **)nArgValues);
}
#endif

