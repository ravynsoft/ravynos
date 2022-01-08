/*
 * airyxOS Application Launcher & Status Bar
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

#include "WindowTracker.h"
#include "Dock.h"
#include <unistd.h>
#include <X11/Xlib.h>
#include <KWindowInfo>

extern Dock *g_dock;

static int pathForPID(unsigned int pid, char *buf, int len)
{
    char path[32];
    if(buf == NULL || len < 0)
        return -1;

    snprintf(path, sizeof(path), "/proc/%u/file", pid);
    int bytes = readlink(path, buf, len-1);
    if(bytes <= 0)
        return -1;

    buf[bytes] = 0;
    return 0;
}

WindowTracker::WindowTracker()
    : QObject()
{
    connect(KWindowSystem::self(), &KWindowSystem::windowAdded, this,
        &WindowTracker::windowWasAdded);
    connect(KWindowSystem::self(), &KWindowSystem::windowRemoved, this,
        &WindowTracker::windowWasRemoved);
    connect(KWindowSystem::self(),
        static_cast<void (KWindowSystem::*)(WId, NET::Properties,
        NET::Properties2)>(&KWindowSystem::windowChanged), this,
        &WindowTracker::windowWasChanged);
}

void WindowTracker::windowWasAdded(WId window)
{
    NSLog(@"windowWasAdded: %u", window);
}

void WindowTracker::windowWasRemoved(WId window)
{
    NSLog(@"windowWasRemoved: %u", window);
    g_dock->removeWindowFromAll(window);
}

void WindowTracker::windowWasChanged(WId window, NET::Properties props,
    NET::Properties2 props2)
{

    if(!props && !(props2 & NET::WM2TransientFor))
        return;
    if(!(props & (NET::WMPid | NET::WMName | NET::WMVisibleName |
        NET::DemandsAttention | NET::WMState | NET::XAWMState)))
        return;

    KWindowInfo info(window, NET::WMPid, 0);
    if(!info.valid())
        return;

    NSLog(@"windowWasChanged: %u -> PID %u", window, info.pid());
    char path[PATH_MAX];
    if(pathForPID(info.pid(), path, PATH_MAX-1) != 0)
        return;

    NSLog(@"Path for %u is %s", info.pid(), path);
    DockItem *di = g_dock->findDockItemForPath(path);
    if(di == nil)
        return;

    g_dock->removeWindowFromAll(window); // just in case owner changed
    [di addWindow:window];
    NSLog(@"Added window to DockItem %@", [di label]);
}
