/*
 * Copyright (C) 2022 Zoe Knox <zoe@pixin.net>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#import <Foundation/Foundation.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

int main(int argc, const char *argv[]) {
    __NSInitializeProcess(argc, argv);

    if(getenv("XDG_RUNTIME_DIR") == NULL) {
        char *buf = 0;
        asprintf(&buf, "/tmp/runtime.%u", getuid());
        setenv("XDG_RUNTIME_DIR", buf, 0);
        if(access(buf, R_OK|W_OK|X_OK) != 0) {
            switch(errno) {
                case ENOENT: mkdir(buf, 0700); break;
                default: perror("WindowServer"); exit(-1);
            }
        }
        free(buf);
    }

    NSString *exePath = [[NSBundle mainBundle] pathForResource:@"waybox" ofType:@""];
    NSArray *args = @[@"WindowServer", @"--config-file", @"ws.conf"];
    char **_argv = malloc((1 + [args count]) * sizeof(char *));
    int i;
    for(i = 0; i < [args count]; ++i)
        _argv[i] = strdup([[args objectAtIndex:i] cString]);
    _argv[i] = NULL;

    execv([exePath cString], _argv);
    perror("execv");
    return -1;
}

