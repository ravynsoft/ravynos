#ifdef WINDOWS
/* Copyright (c) 2012 Glenn Ganz
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSRaise.h>

#define _NS_FRAME_ADDRESS_BACKTRACE(x) case x: array[level] = __builtin_frame_address(x + 1); break;

int backtrace(void** array, int size)
{
    int level = 0;
    
    for (;level < size;level++) {
        switch (level) {
                _NS_FRAME_ADDRESS_BACKTRACE(0); 
                _NS_FRAME_ADDRESS_BACKTRACE(1); 
                _NS_FRAME_ADDRESS_BACKTRACE(2);
                _NS_FRAME_ADDRESS_BACKTRACE(3); 
                _NS_FRAME_ADDRESS_BACKTRACE(4); 
                _NS_FRAME_ADDRESS_BACKTRACE(5);
                _NS_FRAME_ADDRESS_BACKTRACE(6);
                _NS_FRAME_ADDRESS_BACKTRACE(7); 
                _NS_FRAME_ADDRESS_BACKTRACE(8);
                _NS_FRAME_ADDRESS_BACKTRACE(9); 
                _NS_FRAME_ADDRESS_BACKTRACE(10); 
                _NS_FRAME_ADDRESS_BACKTRACE(11);
                _NS_FRAME_ADDRESS_BACKTRACE(12); 
                _NS_FRAME_ADDRESS_BACKTRACE(13); 
                _NS_FRAME_ADDRESS_BACKTRACE(14);
                _NS_FRAME_ADDRESS_BACKTRACE(15); 
                _NS_FRAME_ADDRESS_BACKTRACE(16); 
                _NS_FRAME_ADDRESS_BACKTRACE(17);
                _NS_FRAME_ADDRESS_BACKTRACE(18); 
                _NS_FRAME_ADDRESS_BACKTRACE(19); 
                _NS_FRAME_ADDRESS_BACKTRACE(20);
                _NS_FRAME_ADDRESS_BACKTRACE(21); 
                _NS_FRAME_ADDRESS_BACKTRACE(22); 
                _NS_FRAME_ADDRESS_BACKTRACE(23);
                _NS_FRAME_ADDRESS_BACKTRACE(24); 
                _NS_FRAME_ADDRESS_BACKTRACE(25); 
                _NS_FRAME_ADDRESS_BACKTRACE(26);
                _NS_FRAME_ADDRESS_BACKTRACE(27); 
                _NS_FRAME_ADDRESS_BACKTRACE(28); 
                _NS_FRAME_ADDRESS_BACKTRACE(29);
                _NS_FRAME_ADDRESS_BACKTRACE(30); 
                _NS_FRAME_ADDRESS_BACKTRACE(31); 
                _NS_FRAME_ADDRESS_BACKTRACE(32);
                _NS_FRAME_ADDRESS_BACKTRACE(33); 
                _NS_FRAME_ADDRESS_BACKTRACE(34); 
                _NS_FRAME_ADDRESS_BACKTRACE(35);
                _NS_FRAME_ADDRESS_BACKTRACE(36); 
                _NS_FRAME_ADDRESS_BACKTRACE(37); 
                _NS_FRAME_ADDRESS_BACKTRACE(38);
                _NS_FRAME_ADDRESS_BACKTRACE(39); 
                _NS_FRAME_ADDRESS_BACKTRACE(40); 
                _NS_FRAME_ADDRESS_BACKTRACE(41);
                _NS_FRAME_ADDRESS_BACKTRACE(42); 
                _NS_FRAME_ADDRESS_BACKTRACE(43); 
                _NS_FRAME_ADDRESS_BACKTRACE(44);
                _NS_FRAME_ADDRESS_BACKTRACE(45); 
                _NS_FRAME_ADDRESS_BACKTRACE(46); 
                _NS_FRAME_ADDRESS_BACKTRACE(47);
                _NS_FRAME_ADDRESS_BACKTRACE(48); 
                _NS_FRAME_ADDRESS_BACKTRACE(49); 
                _NS_FRAME_ADDRESS_BACKTRACE(50);
                _NS_FRAME_ADDRESS_BACKTRACE(51);
                _NS_FRAME_ADDRESS_BACKTRACE(52);
                _NS_FRAME_ADDRESS_BACKTRACE(53);
                _NS_FRAME_ADDRESS_BACKTRACE(54);
                _NS_FRAME_ADDRESS_BACKTRACE(55);
                _NS_FRAME_ADDRESS_BACKTRACE(56);
                _NS_FRAME_ADDRESS_BACKTRACE(57);
                _NS_FRAME_ADDRESS_BACKTRACE(58);
                _NS_FRAME_ADDRESS_BACKTRACE(59);
                _NS_FRAME_ADDRESS_BACKTRACE(60);
                _NS_FRAME_ADDRESS_BACKTRACE(61);
                _NS_FRAME_ADDRESS_BACKTRACE(62);
                _NS_FRAME_ADDRESS_BACKTRACE(63);
                _NS_FRAME_ADDRESS_BACKTRACE(64);
                _NS_FRAME_ADDRESS_BACKTRACE(65);
                _NS_FRAME_ADDRESS_BACKTRACE(66);
                _NS_FRAME_ADDRESS_BACKTRACE(67);
                _NS_FRAME_ADDRESS_BACKTRACE(68);
                _NS_FRAME_ADDRESS_BACKTRACE(69);
                _NS_FRAME_ADDRESS_BACKTRACE(70);
                _NS_FRAME_ADDRESS_BACKTRACE(71);
                _NS_FRAME_ADDRESS_BACKTRACE(72);
                _NS_FRAME_ADDRESS_BACKTRACE(73);
                _NS_FRAME_ADDRESS_BACKTRACE(74);
                _NS_FRAME_ADDRESS_BACKTRACE(75);
                _NS_FRAME_ADDRESS_BACKTRACE(76);
                _NS_FRAME_ADDRESS_BACKTRACE(77);
                _NS_FRAME_ADDRESS_BACKTRACE(78);
                _NS_FRAME_ADDRESS_BACKTRACE(79);
                _NS_FRAME_ADDRESS_BACKTRACE(80);
                _NS_FRAME_ADDRESS_BACKTRACE(81);
                _NS_FRAME_ADDRESS_BACKTRACE(82);
                _NS_FRAME_ADDRESS_BACKTRACE(83);
                _NS_FRAME_ADDRESS_BACKTRACE(84);
                _NS_FRAME_ADDRESS_BACKTRACE(85);
                _NS_FRAME_ADDRESS_BACKTRACE(86);
                _NS_FRAME_ADDRESS_BACKTRACE(87);
                _NS_FRAME_ADDRESS_BACKTRACE(88);
                _NS_FRAME_ADDRESS_BACKTRACE(89);
                _NS_FRAME_ADDRESS_BACKTRACE(90);
                _NS_FRAME_ADDRESS_BACKTRACE(91);
                _NS_FRAME_ADDRESS_BACKTRACE(92);
                _NS_FRAME_ADDRESS_BACKTRACE(93);
                _NS_FRAME_ADDRESS_BACKTRACE(94);
                _NS_FRAME_ADDRESS_BACKTRACE(95);
                _NS_FRAME_ADDRESS_BACKTRACE(96);
                _NS_FRAME_ADDRESS_BACKTRACE(97);
                _NS_FRAME_ADDRESS_BACKTRACE(98);
                _NS_FRAME_ADDRESS_BACKTRACE(99);

            default: return size;
        }
        
        if (array[level] == 0) {
            return level - 1;
        }
    }
    
    return level;
}

char** backtrace_symbols(void* const* array, int size) {
    NSUnimplementedFunction();
    return 0;
}
#endif

