/*
 * Small setuid helper app to extract system manufacturer and model from
 * the SMBIOS using dmidecode. This is called from the system menu.
 * 
 * Copyright (C) 2021 Zoe Knox <zoe@pixin.net>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *manufacturer = "/usr/sbin/dmidecode -s system-manufacturer";
const char *product = "/usr/sbin/dmidecode -s system-product-name";

int main(int argc, char **argv)
{
    FILE *pipe;
    char buffer[128];

    pipe = popen(manufacturer, "r");
    if(pipe) {
        memset(buffer, 0, sizeof(buffer));
        fgets(buffer, sizeof(buffer), pipe);
        fputs(buffer, stdout);
        pclose(pipe);
    } else
        puts("Unknown");
    
    pipe = popen(product, "r");
    if(pipe) {
        memset(buffer, 0, sizeof(buffer));
        fgets(buffer, sizeof(buffer), pipe);
        fputs(buffer,stdout);
        pclose(pipe);
    } else
        puts("Unknown");
    
    return 0;
}
