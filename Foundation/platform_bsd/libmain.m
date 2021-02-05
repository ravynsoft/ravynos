/* Copyright (c) 2010-2011 Glenn Ganz
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */


#include <stdio.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#import <Foundation/NSProcessInfo.h>


FOUNDATION_EXPORT void __attribute__ ((constructor)) libmain(void)
{    
    char args[sysconf(_SC_ARG_MAX)];
    int managementInfoBase[4];
    size_t len;
    int argc = 0;
    unsigned int last = 0;

        
    managementInfoBase[0] = CTL_KERN;
    managementInfoBase[1] = KERN_PROC;
    managementInfoBase[2] = KERN_PROC_ARGS;
    managementInfoBase[3] = getpid();
    
    len = sizeof(args);
    if (sysctl(managementInfoBase, 4, args, &len, NULL, 0) == -1) {
        fprintf(stderr, "Error during Cocotron initialization: Unable to read argument list of the process");
        abort();
    }
    
    int allocated = 5;
    const char **argv = (const char**)calloc(allocated,sizeof(char*));
    
    for(unsigned int i = 0;i < len;i++) {
        if(args[i] == '\0') {
            
            char* arg = strdup(&args[last]);
            
            if(allocated <= argc) {
                allocated *= 2;
                argv = realloc(argv,sizeof(char*) * allocated);
            }
            
            argv[argc] = arg;                     
            last = i + 1;
            argc++;
        }
    }

    __NSInitializeProcess(argc,argv);
}
