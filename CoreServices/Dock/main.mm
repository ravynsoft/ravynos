/*
 * ravynOS Application Launcher & Status Bar
 *
 * Copyright (C) 2021-2022 Zoe Knox <zoe@pixin.net>
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

#include "Dock.h"
#include "WindowTracker.h"
#include <QThread>

int kqPIDs = 0;
Dock *g_dock = 0;

void pidMonitorLoop(void) {
    struct kevent out[128], in[128];
    char buf[PATH_MAX];
    int inCount = 0;

    pid_t myPID = getpid();

    while(1) {
        int count = kevent(kqPIDs, in, inCount, out, 128, NULL);
        inCount = 0;

        for(int i = 0; i < count; ++i) {
            switch(out[i].filter) {
                case EVFILT_READ:
                    inCount = read(out[i].ident, &in[inCount], out[i].data)
                        / sizeof(struct kevent);
                    break;
                case EVFILT_PROC:
                    // ignore FORK notices for this PID - we only care about children
                    if(myPID == out[i].ident)
                        break;
                    if((out[i].fflags & (NOTE_FORK|NOTE_EXEC|NOTE_CHILD))
                        && ((out[i].fflags & NOTE_EXIT) == 0)) {

                        NSString *path = [NSString
                            stringWithFormat:@"/proc/%lu/file", out[i].ident];
                        int len = readlink([path UTF8String], buf, PATH_MAX-1);
                        if(len <= 0)
                            break;

                        buf[len] = 0;
                        DockItem *item = g_dock->findDockItemForPath(buf);
                        NSDebugLog(@"New process: %d %s %@",out[i].ident,
                            buf, item);
                        if(item == nil) {
                            g_dock->emitAddNonResident(out[i].ident, buf);
                            break;
                        }

                        BOOL wasRunning = [item isRunning];
                        [item addPID:out[i].ident];
                        if(!wasRunning) {
                            NSDebugLog(@"Item Started: %@", [item label]);
                            g_dock->emitStarted((__bridge void *)item);
                        }
                    }
                    if((out[i].fflags & NOTE_EXIT)) {
                        NSDebugLog(@"PID %lu exited", out[i].ident);
                        DockItem *item = (__bridge DockItem *)(out[i].udata);
                        [item removePID:out[i].ident];
                        if(![item isRunning]) {
                            NSDebugLog(@"Item Stopped: %@", [item label]);
                            g_dock->emitStopped((__bridge void *)item);
                        }
                    }
                    break;
                default:
                    NSLog(@"unknown filter");
            }
        }
    }
}

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    kqPIDs = kqueue();
    g_dock = new Dock();

    WindowTracker tracker;

    QThread *pidMonitor = QThread::create(pidMonitorLoop);
    pidMonitor->start();

    int ret = app.exec();

    pidMonitor->terminate();
    return ret;
}

