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

#include "EventFilter.h"
#include <xcb/xcb.h>

XEventFilter::XEventFilter()
{
    m_display = XOpenDisplay(":0");
}

XEventFilter::~XEventFilter()
{
    XCloseDisplay(m_display);
}

bool XEventFilter::nativeEventFilter(const QByteArray& type, void *msg,
    long *result)
{
    xcb_generic_event_t *ev = (xcb_generic_event_t *)msg;
    uint8_t evType = ev->response_type;

    switch(evType) {
        case XCB_CREATE_NOTIFY:
        {
            xcb_create_notify_event_t *e = (xcb_create_notify_event_t *)msg;
            NSLog(@"Create %u at %u,%u", e->window, e->x, e->y);
            break;
        }
        case XCB_DESTROY_NOTIFY:
        {
            xcb_destroy_notify_event_t *e = (xcb_destroy_notify_event_t *)msg;
            NSLog(@"Destroy %u", e->window);
            break;
        }
        case XCB_UNMAP_NOTIFY:
        {
            xcb_unmap_notify_event_t *e = (xcb_unmap_notify_event_t *)msg;
            NSLog(@"Unmap %u", e->window);
            break;
        }
        case XCB_MAP_NOTIFY:
        {
            xcb_map_notify_event_t *e = (xcb_map_notify_event_t *)msg;
            NSLog(@"Map %u", e->window);
            break;
        }
        case XCB_PROPERTY_NOTIFY:
        {
            xcb_property_notify_event_t *e = (xcb_property_notify_event_t *)msg;
            NSLog(@"Property %u for %u", e->window, e->atom);
            break;
        }
    }
    return false;
}
