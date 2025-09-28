/* Copyright (c) 2012 Glenn Ganz
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSRaise.h>

#include <dlfcn.h>
#include <ucontext.h>
#include <string.h>
#include <stdlib.h>

struct _backtracearray {
    void **array;
    int size;
    int currentPos;
};

int _fillarray(uintptr_t pc, int sig, void *usrarg) {    
    struct _backtracearray *a = (struct _backtracearray *)usrarg;
    int i = 0;
    
    if (a->currentPos >= a->size) {
        return -1;
    }
    
    a->array[a->currentPos++] = (void *)pc;
    
    return 0;
}

int backtrace(void** array, int size) {
    struct _backtracearray a;
    a.array = array;
    a.size = size;
    a.currentPos = 0;
    ucontext_t ucp;
    if(getcontext(&ucp)) {
        return 0;
    } else {
        walkcontext(&ucp, &_fillarray, &a);
    }
    
    return a.currentPos;
}

char** backtrace_symbols(void* const* array, int size) {
    NSUnimplementedFunction();
    return 0;
/*
    char** ret = malloc(size);
    
    for (int i = 0 ; i < size; ++i) {
        Dl_info info;
        dladdr(array[i], &info);
        if (info.dli_sname != NULL) {
            ret[i] = strdup(info.dli_sname);
        }
        else {
            ret[i] = NULL;
        }
    }
    
    return ret;*/
}