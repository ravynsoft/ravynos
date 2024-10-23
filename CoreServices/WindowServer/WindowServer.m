/*
 * Copyright (C) 2022-2024 Zoe Knox <zoe@pixin.net>
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

#define WINDOWSERVER 1

#import <Onyx2D/O2Context.h>
#import <Onyx2D/O2Surface.h>
#import <Onyx2D/O2Image.h>
#import <Onyx2D/O2ImageSource.h>
#import <AppKit/NSAttributedString.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSWindow.h>
#import "common.h"
#import "WindowServer.h"

#undef direction // defined in mach.h
#include <linux/input.h>

#include <poll.h>
#include <kvm.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/user.h>
#include <sys/shm.h>
#include <sys/ipc.h>

#import "rpc.h"

/* This lock prevents other threads from messing with the graphics context while we
 * are in the rendering loop
 */
pthread_mutex_t renderLock;

@implementation WindowServer

-init {
    ready = NO;
    logLevel = WS_ERROR;
    envp = NULL;
    curShell = LOGINWINDOW;
    curApp = nil;
    curWindow = nil;
    pthread_mutex_init(&renderLock, NULL);
    cursorHideCount = 0;

    kern_return_t kr;
    if((kr = bootstrap_check_in(bootstrap_port, WINDOWSERVER_SVC_NAME, &_servicePort)) != KERN_SUCCESS) {
        NSLog(@"Failed to check-in service: %d", kr);
        return nil;
    }

    _kq = kqueue();
    if(_kq < 0) {
        perror("kqueue");
        return nil;
    }

    kvm = kvm_open(NULL, "/dev/null", NULL, O_RDONLY, "WindowServer(kvm): ");

    displays = [NSMutableArray new];
    apps = [NSMutableDictionary new];

    input = [WSInput new];
    [input setLogLevel:logLevel];

    stopOnErr = NO;
    NSString *s_stopOnErr = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"DebugExitOnError"];
    if(s_stopOnErr && [s_stopOnErr isEqualToString:@"YES"])
        stopOnErr = YES;

    struct passwd *passwd = getpwnam("nobody");
    if(!passwd) {
        perror("getpwnam(nobody)");
        return nil;
    }
    nobodyUID = passwd->pw_uid;

    struct group *group = getgrnam("video");
    if(!group) {
        perror("getgrnam(video)");
        return nil;
    }
    videoGID = group->gr_gid;

    // FIXME: try drm/kms first then fall back
    fb = [BSDFramebuffer new];
    if([fb openFramebuffer:"/dev/console"] < 0)
        return nil;
    _geometry = [fb geometry];
    [displays addObject:fb];

    [fb clear];

    // this is to keep our X,Y from leaving the screen bounds and eventually can be used to find
    // edges when there are multiple screens
    [input setGeometry:_geometry];
    [input setPointerPos:NSMakePoint(_geometry.size.width / 2, _geometry.size.height / 2)];

    ready = YES;
    return self;
}

-(void)dealloc {
    curShell = NONE;
    //pthread_cancel(curShellThread);
    fb = nil;
    input = nil;
    if(kvm)
        kvm_close(kvm);
}

-(void)setLogLevel:(int)level {
    logLevel = level;
}

-(BOOL)isReady {
    return ready;
}

-(O2BitmapContext *)context {
    return [fb context];
}

-(NSRect)geometry {
    return _geometry;
}

-(void)draw {
    return [fb draw];
}

-(WSDisplay *)displayWithID:(uint32_t)ID {
    for(int i = 0; i < [displays count]; ++i) {
        WSDisplay *d = [displays objectAtIndex:i];
        if([d getDisplayID] == ID)
            return d;
    }
    return nil;
}

-(BOOL)setUpEnviron:(uid_t)uid {
    struct passwd *pw = getpwuid(uid);
    if(!pw)
        return NO;
    int entries = 7;
    envp = malloc(sizeof(char *) * entries);
    asprintf(&envp[0], "HOME=%s", pw->pw_dir);
    asprintf(&envp[1], "SHELL=%s", pw->pw_shell);
    asprintf(&envp[2], "USER=%s", pw->pw_name);
    asprintf(&envp[3], "LOGNAME=%s", pw->pw_name);
    asprintf(&envp[4], "PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin");
    asprintf(&envp[6], "TERM=xterm");
    envp[entries - 1] = NULL;
    return YES;
}

-(void)freeEnviron {
    if(envp == NULL)
        return;

    while(*envp != NULL)
        free(*envp++);
    free(envp);
}

-(void *)launchShell {
    int status;
    NSString *lwPath = nil;

    while(curShell != NONE) {
        if(ready == NO) {
            sleep(1);
            continue;
        }

        switch(curShell) {
            case LOGINWINDOW:
                if(seteuid(0) != 0) { // re-assert privileges
                    perror("seteuid");
                    exit(-1);
                }

                if(setresgid(videoGID, videoGID, 0) != 0) {
                    perror("setresgid");
                    exit(-1);
                }
                if(setresuid(nobodyUID, nobodyUID, 0) != 0) {
                    perror("setresuid");
                    exit(-1);
                }

                int uid = nobodyUID;
                int gid = videoGID;

                int fds[2];
                if(pipe(fds) != 0) {
                    perror("pipe");
                    exit(-1);
                }
                char fdbuf[8];
                sprintf(fdbuf, "%d", fds[1]);
                int status = -1;

                lwPath = [[NSBundle mainBundle] pathForResource:@"LoginWindow" ofType:@"app"];
                if(!lwPath) {
                    NSLog(@"missing LoginWindow.app!");
                    break;
                }
                lwPath = [[NSBundle bundleWithPath:lwPath] executablePath];
                if(!lwPath) {
                    NSLog(@"missing LoginWindow.app!");
                    break;
                }

                if([self setUpEnviron:nobodyUID] == NO) {
                    NSLog(@"Unable to set up environment for LoginWindow!");
                    return NO;
                }

                pid_t pid = fork();
                if(!pid) { // child
                    close(fds[0]);
                    seteuid(0);
                    execle([lwPath UTF8String], [[lwPath lastPathComponent] UTF8String], fdbuf, NULL, envp);
                    exit(-1);
                } else {
                    close(fds[1]);
                    read(fds[0], &uid, sizeof(int));
                    waitpid(pid, &status, 0);
                }
                [self freeEnviron];
                close(fds[0]);
                if(logLevel >= WS_INFO)
                    NSLog(@"received uid %d", uid);

                if(uid < 500) {
                    NSLog(@"UID below minimum");
                    break;
                }

                struct passwd *pw = getpwuid(uid);
                if(!pw || pw->pw_uid != uid) {
                    NSLog(@"no such uid %d", uid);
                    break;
                }
                gid = pw->pw_gid;

                if(seteuid(0) != 0) { // re-assert privileges
                    perror("seteuid");
                    return NO;
                }

                // ensure our helper is owned correctly
                {
                    NSString *path = [[NSBundle mainBundle] pathForResource:@"SystemUIServer" ofType:@"app"];
                    if(path)
                        path = [[NSBundle bundleWithPath:path] pathForResource:@"shutdown" ofType:@""];
                    if(path) {
                        chown([path UTF8String], 0, videoGID);
                        chmod([path UTF8String], 04550);
                    }
                }

                curShell = DESKTOP;
                break;
            case DESKTOP: {
                [self setUpEnviron:uid];
                pid_t pid = fork();
                if(pid == 0) {
                    setlogin(pw->pw_name);
                    chdir(pw->pw_dir);

                    login_cap_t *lc = login_getpwclass(pw);
                    if (setusercontext(lc, pw, pw->pw_uid,
                        LOGIN_SETALL & ~(LOGIN_SETLOGIN)) != 0) {
                            perror("setusercontext");
                            exit(-1);
                    }
                    login_close(lc);

                    NSString *path = [[NSBundle mainBundle] pathForResource:@"SystemUIServer" ofType:@"app"];
                    if(path)
                        path = [[NSBundle bundleWithPath:path] executablePath];

                    if(path)
                        execle([path UTF8String], [[path lastPathComponent] UTF8String], NULL, envp);

                    perror("execl");
                    exit(-1);
                } else if(pid < 0) {
                    perror("fork");
                    sleep(3);
                    curShell = LOGINWINDOW;
                    break;
                }
                [self freeEnviron];
                waitpid(pid, &status, 0);
                curShell = LOGINWINDOW;
                // safety valve for debugging
                if(stopOnErr)
                    execl("/bin/launchctl", "launchctl", "remove", "com.ravynos.WindowServer", NULL);
                break;
            }
        }
    }
    pthread_exit(NULL);
}

-(uint32_t)windowCreate:(struct wsRPCWindow *)data forApp:(WSAppRecord *)app {
    struct kinfo_proc *kp;

    if(data->state < 0 || data->state >= WIN_STATE_MAX) {
        NSLog(@"windowCreate called with invalid state");
        data->state = NORMAL;
    }

    if([app windowWithID:data->windowID] != nil) {
        NSLog(@"windowCreate cannot create existing window ID %u for %@", data->windowID, app);
        return 0;
    }

    WSWindowRecord *winrec = [WSWindowRecord new];
    winrec.number = data->windowID;
    winrec.state = data->state;
    winrec.styleMask = data->style;
    winrec.geometry = NSMakeRect(data->x, data->y, data->w, data->h); // FIXME: bounds check?
    int len = 0;
    while(data->title[len] != '\0' && len < sizeof(data->title)) ++len;
    winrec.title = [NSString stringWithCString:data->title length:len];
    if(len > 0) {
        CGGlyph glyphs[128];
        CTFontRef ctfont = CTFontCreateWithGraphicsFont(_titleFont, 14.0, &CGAffineTransformIdentity, nil);
        CTFontGetGlyphsForCharacters(ctfont, (const unichar *)[winrec.title
                cStringUsingEncoding:NSUnicodeStringEncoding], glyphs, [winrec.title length]);
        winrec.titleSize = [self sizeOfTitleText:winrec.title];
        [winrec setGlyphs:glyphs];
        winrec.titleSet = YES;
    } else
        winrec.titleSet = NO;
    winrec.icon = nil;

    winrec.shmPath = [NSString stringWithFormat:@"/%@/%u/win/%u", [app bundleID],
        [app pid], winrec.number];
    winrec.bufSize = ([fb getDepth]/8) * data->w * data->h;

    int shmfd = shm_open([winrec.shmPath cString], O_RDWR|O_CREAT, 0600);
    if(shmfd < 0) {
        NSLog(@"Cannot open shm fd: %s", strerror(errno));
        return 0;
    }

    if(ftruncate(shmfd, winrec.bufSize) < 0)
        NSLog(@"shmfd ftruncate failed: %s", strerror(errno));

    int count = 0;
    kp = kvm_getprocs(kvm, KERN_PROC_PID, [app pid], &count);
    if(count != 1 || kp->ki_pid != [app pid]) {
        NSLog(@"Cannot get client task info! pid %u", [app pid]);
        return 0;
    }

    if(fchown(shmfd, kp->ki_uid, kp->ki_rgid) < 0)
        NSLog(@"shmfd fchown failed: %s", strerror(errno));

    winrec.surfaceBuf = mmap(NULL, winrec.bufSize, PROT_WRITE|PROT_READ, MAP_SHARED|MAP_NOCORE, shmfd, 0);
    close(shmfd);

    if(winrec.surfaceBuf == NULL) {
        winrec.bufSize = 0;
        NSLog(@"Cannot alloc surface memory! %s", strerror(errno));
        return 0;
    }

    winrec.surface = [[O2Surface alloc] initWithBytes:winrec.surfaceBuf width:data->w
            height:data->h bitsPerComponent:8 bytesPerRow:4*(data->w)
            colorSpace:[fb colorSpace]
            bitmapInfo:kCGBitmapByteOrderDefault|kCGImageAlphaPremultipliedFirst];
    winrec.frame = WSOutsetFrame(winrec.geometry, winrec.styleMask);

    [app addWindow:winrec];
    if(curApp == app)
        curWindow = winrec; // FIXME: is this how macOS behaves?
    if(logLevel >= WS_INFO)
        NSLog(@"windowCreate: success! %@", winrec);
    return winrec.number;
}

-(void)windowModify:(struct wsRPCWindow *)data forApp:(WSAppRecord *)app {
    if(data->state < 0 || data->state >= WIN_STATE_MAX) {
        NSLog(@"windowModify called with invalid state");
        data->state = NORMAL;
    }

    WSWindowRecord *winrec = [app windowWithID:data->windowID];
    winrec.state = data->state;
    winrec.styleMask = data->style;
    // FIXME: this will probably require changing the surface and shm buffer
    winrec.geometry = NSMakeRect(data->x, data->y, data->w, data->h); // FIXME: bounds check?
    int len = 0;
    while(data->title[len] != '\0' && len < sizeof(data->title)) ++len;
    winrec.title = [NSString stringWithCString:data->title length:len];
    if(len > 0) {
        CGGlyph glyphs[128];
        CTFontRef ctfont = CTFontCreateWithGraphicsFont(_titleFont, 14.0, &CGAffineTransformIdentity, nil);
        CTFontGetGlyphsForCharacters(ctfont, (const unichar *)[winrec.title
                cStringUsingEncoding:NSUnicodeStringEncoding], glyphs, [winrec.title length]);
        winrec.titleSize = [self sizeOfTitleText:winrec.title];
        [winrec setGlyphs:glyphs];
        winrec.titleSet = YES;
    } else
        winrec.titleSet = NO;
    winrec.icon = nil;
    if(logLevel >= WS_INFO)
        NSLog(@"windowModify %@ win %@", app, winrec);
}

#define _cursor_height 24
-(void)run {
    // FIXME: lock this to vsync of actual display
    pthread_mutex_lock(&renderLock);
    O2BitmapContext *ctx = [fb context];
    pthread_mutex_unlock(&renderLock);

    NSString *path = [[NSBundle mainBundle] pathForResource:@"arrowCursor" ofType:@"png"];
    NSData *cursorData = [NSData dataWithContentsOfFile:path];
    O2ImageSource *cursorIS = [O2ImageSource newImageSourceWithData:(__bridge CFMutableDataRef)cursorData
                                                            options:NULL];
    O2ImageRef cursor = [cursorIS createImageAtIndex:0 options:nil];
    NSRect cursorRect = NSMakeRect(0, 0, _cursor_height, _cursor_height);

    _titleFont = CGFontCreateWithFontName((__bridge CFStringRef)@"Inter-Bold");
    pthread_mutex_lock(&renderLock);
    O2ContextSetFont(ctx, (__bridge O2Font *)_titleFont);
    O2ContextSetFontSize(ctx, 14.0);
    O2ContextSetTextDrawingMode(ctx, kO2TextFillStroke);
    pthread_mutex_unlock(&renderLock);

    struct pollfd fds;
    fds.fd = [input fileDescriptor];
    fds.events = POLLIN;

    while(ready == YES) {
        if(poll(&fds, 1, 50) > 0)
            [input run:self];

        cursorRect.origin = [input pointerPos];

        // FIXME: handle multiple displays here. Use a thread per display?
        pthread_mutex_lock(&renderLock);
        ctx = [fb context];
        pid_t capturedPID = [fb captured];
        if(capturedPID == 0) {
            O2ContextSetRGBFillColor(ctx, 0, 0, 0, 1);
            O2ContextFillRect(ctx, (O2Rect)_geometry);
            NSEnumerator *appEnum = [apps objectEnumerator];
            WSAppRecord *app;
            while((app = [appEnum nextObject]) != nil) {
                NSArray *wins = [app windows];
                int count = [wins count];
                for(int i = 0; i < count; ++i) {
                    WSWindowRecord *win = [wins objectAtIndex:i];
                    if(win.state == HIDDEN)
                        continue;
                    [win drawFrame:ctx pointer:cursorRect.origin];
                    [ctx drawImage:win.surface inRect:win.geometry];
                }
            }
        }

        cursorRect.origin.y -= _cursor_height; // make sure point of arrow is on actual spot

        if(capturedPID == 0) {
            if(cursorHideCount == 0) {
                O2ContextSetBlendMode(ctx, kCGBlendModeNormal);
                O2ContextDrawImage(ctx, cursorRect, cursor);
            }
            [fb draw];
        } else
            if(cursorHideCount == 0)
                [fb drawWithCursor:cursor inRect:cursorRect];
            else
                [fb draw];
        pthread_mutex_unlock(&renderLock);
    }

}

- (void)rpcMainDisplayID:(PortMessage *)msg {
    uint32_t ID = [(WSDisplay *)fb getDisplayID];
    struct wsRPCSimple reply = { kCGMainDisplayID, sizeof(uint32_t)*4, ID, 0, 0, 0 };
    [self sendInlineData:&reply length:sizeof(reply) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

- (void)rpcGetOnlineDisplayList:(PortMessage *)msg {
    size_t size = sizeof(struct wsRPCBase) + sizeof(uint32_t)*[displays count];
    uint8_t *list = malloc(size);
    struct wsRPCBase *p = (struct wsRPCBase *)list;
    p->code = kCGGetOnlineDisplayList;
    p->len = 0;
    uint32_t *q = (uint32_t *)(list + sizeof(struct wsRPCBase));
    int j = 0;
    for(int i = 0; i < [displays count]; ++i) {
        WSDisplay *d = [displays objectAtIndex:i];
        if([d isOnline])
            q[j++] = [d getDisplayID];
    }
    p->len = j * sizeof(uint32_t);
    [self sendInlineData:list length:size withCode:MSG_ID_RPC toPort:msg->descriptor.name];
    free(list);
}

- (void)rpcGetActiveDisplayList:(PortMessage *)msg {
    size_t size = sizeof(struct wsRPCBase) + sizeof(uint32_t)*[displays count];
    uint8_t *list = malloc(size);
    struct wsRPCBase *p = (struct wsRPCBase *)list;
    p->code = kCGGetActiveDisplayList;
    p->len = 0;
    uint32_t *q = (uint32_t *)(list + sizeof(struct wsRPCBase));
    int j = 0;
    for(int i = 0; i < [displays count]; ++i) {
        WSDisplay *d = [displays objectAtIndex:i];
        if([d isActive])
            q[j++] = [d getDisplayID];
    }
    p->len = j * sizeof(uint32_t);
    [self sendInlineData:list length:size withCode:MSG_ID_RPC toPort:msg->descriptor.name];
    free(list);
}

- (void)rpcGetDisplaysWithOpenGLDisplayMask:(PortMessage *)msg {
    struct wsRPCSimple *args = (struct wsRPCSimple *)msg->data;
    CGOpenGLDisplayMask mask = args->val1;

    size_t size = sizeof(struct wsRPCBase) + sizeof(uint32_t)*[displays count];
    uint8_t *list = malloc(size);
    struct wsRPCBase *p = (struct wsRPCBase *)list;
    p->code = kCGGetDisplaysWithOpenGLDisplayMask;
    p->len = 0;
    uint32_t *q = (uint32_t *)(list + sizeof(struct wsRPCBase));
    int j = 0;
    for(int i = 0; i < [displays count]; ++i) {
        WSDisplay *d = [displays objectAtIndex:i];
        if([d openGLMask] & mask)
            q[j++] = [d getDisplayID];
    }
    p->len = j * sizeof(uint32_t);
    [self sendInlineData:list length:size withCode:MSG_ID_RPC toPort:msg->descriptor.name];
    free(list);
}

- (void)rpcGetDisplaysWithPoint:(PortMessage *)msg {
    struct wsRPCSimple *args = (struct wsRPCSimple *)msg->data;
    NSPoint point = NSMakePoint(args->val1, args->val2);

    size_t size = sizeof(struct wsRPCBase) + sizeof(uint32_t)*[displays count];
    uint8_t *list = malloc(size);
    struct wsRPCBase *p = (struct wsRPCBase *)list;
    p->code = kCGGetDisplaysWithPoint;
    p->len = 0;
    uint32_t *q = (uint32_t *)(list + sizeof(struct wsRPCBase));
    int j = 0;
    for(int i = 0; i < [displays count]; ++i) {
        WSDisplay *d = [displays objectAtIndex:i];
        if(NSPointInRect(point, [d geometry])) // FIXME: this should refer to global coordinates
            q[j++] = [d getDisplayID];
    }
    p->len = j * sizeof(uint32_t);
    [self sendInlineData:list length:size withCode:MSG_ID_RPC toPort:msg->descriptor.name];
    free(list);
}

- (void)rpcGetDisplaysWithRect:(PortMessage *)msg {
    struct wsRPCSimple *args = (struct wsRPCSimple *)msg->data;
    NSRect rect = NSMakeRect(args->val1, args->val2, args->val3, args->val4);

    size_t size = sizeof(struct wsRPCBase) + sizeof(uint32_t)*[displays count];
    uint8_t *list = malloc(size);
    struct wsRPCBase *p = (struct wsRPCBase *)list;
    p->code = kCGGetDisplaysWithRect;
    p->len = 0;
    uint32_t *q = (uint32_t *)(list + sizeof(struct wsRPCBase));
    int j = 0;
    for(int i = 0; i < [displays count]; ++i) {
        WSDisplay *d = [displays objectAtIndex:i];
        if(NSIntersectsRect([d geometry], rect)) // FIXME: this should refer to global coordinates
            q[j++] = [d getDisplayID];
    }
    p->len = j * sizeof(uint32_t);
    [self sendInlineData:list length:size withCode:MSG_ID_RPC toPort:msg->descriptor.name];
    free(list);
}

- (void)rpcOpenGLDisplayMaskToDisplayID:(PortMessage *)msg {
    struct wsRPCSimple *args = (struct wsRPCSimple *)msg->data;
    CGOpenGLDisplayMask mask = args->val1;

    struct wsRPCSimple reply = {0};
    reply.base.code = kCGOpenGLDisplayMaskToDisplayID;
    reply.val1 = kCGNullDirectDisplay;
    reply.base.len = 4;

    for(int i = 0; i < [displays count]; ++i) {
        WSDisplay *d = [displays objectAtIndex:i];
        if([d openGLMask] & mask)
            reply.val1 = [d getDisplayID];
    }
    [self sendInlineData:&reply length:sizeof(reply) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

- (void)rpcDisplayIDToOpenGLDisplayMask:(PortMessage *)msg {
    struct wsRPCSimple *args = (struct wsRPCSimple *)msg->data;
    CGDirectDisplayID ID = args->val1;

    struct wsRPCSimple reply = {0};
    reply.base.code = kCGDisplayIDToOpenGLDisplayMask;
    reply.base.len = 4;

    for(int i = 0; i < [displays count]; ++i) {
        WSDisplay *d = [displays objectAtIndex:i];
        if([d getDisplayID] == ID)
            reply.val1 = [d openGLMask];
    }
    [self sendInlineData:&reply length:sizeof(reply) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

- (void)rpcDisplayCapture:(PortMessage *)msg {
    struct wsRPCSimple *args = (struct wsRPCSimple *)msg->data;
    WSDisplay *display = [self displayWithID:args->val1];
    struct wsRPCSimple reply = { {kCGDisplayCaptureWithOptions, 4}, kCGErrorSuccess };
    if(!display || [display capture:msg->pid withOptions:args->val2] != YES)
        reply.val1 = kCGErrorFailure;
    [self sendInlineData:&reply length:sizeof(reply) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

- (void)rpcDisplayRelease:(PortMessage *)msg {
    struct wsRPCSimple *args = (struct wsRPCSimple *)msg->data;
    WSDisplay *display = [self displayWithID:args->val1];
    struct wsRPCSimple reply = { {kCGDisplayRelease, 4}, kCGErrorSuccess };
    if(!display)
        reply.val1 = kCGErrorFailure;
    else
        [display releaseCapture];
    [self sendInlineData:&reply length:sizeof(reply) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

- (void)rpcCaptureAllDisplays:(PortMessage *)msg {
    struct wsRPCSimple *args = (struct wsRPCSimple *)msg->data;
    WSDisplay *display = nil;
    struct wsRPCSimple reply = { {kCGCaptureAllDisplaysWithOptions, 4}, kCGErrorSuccess };
    for(int i = 0; i < [displays count]; ++i) {
        display = [displays objectAtIndex:i];
        if([display capture:msg->pid withOptions:args->val1] != YES) {
            [displays makeObjectsPerformSelector:@selector(releaseCapture)];
            reply.val1 = kCGErrorFailure;
            break;
        }
    }
    [self sendInlineData:&reply length:sizeof(reply) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

- (void)rpcReleaseAllDisplays:(PortMessage *)msg {
    [displays makeObjectsPerformSelector:@selector(releaseCapture)];
    struct wsRPCSimple reply = { {kCGReleaseAllDisplays, 4}, kCGErrorSuccess };
    [self sendInlineData:&reply length:sizeof(reply) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

/* Conceptually, this function returns a pointer to the graphics context
 * owned by WindowServer, that the client app can then write to. Apple
 * probably does this with Mach shared memory, which we don't have. We do
 * it with SysV SHM segments
 */
- (void)rpcDisplayGetDrawingContext:(PortMessage *)msg {
    struct wsRPCSimple *args = (struct wsRPCSimple *)msg->data;
    struct wsRPCSimple reply = { {kCGDisplayGetDrawingContext, 4}, 0};
    WSDisplay *display = [self displayWithID:args->val1];
    if(display) {
        reply.val1 = [display getCapturedContextID];
    }
    [self sendInlineData:&reply length:sizeof(reply) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

// FIXME: I think this is supposed to be only for apps that have captured the display?
- (void)rpcDisplayCreateImageForRect:(PortMessage *)msg {
    struct wsRPCSimple *args = (struct wsRPCSimple *)msg->data;
    struct wsRPCSimple reply = { {kCGDisplayCreateImageForRect, 4}, 0 };
    WSDisplay *display = [self displayWithID:args->val1];
    if(display) {
        O2Rect rect;
        rect.origin.x = (args->val2 & 0xFFFF0000) >> 16;
        rect.origin.y = (args->val2 & 0xFFFF);
        rect.size.width = (args->val3 & 0xFFFF0000) >> 16;
        rect.size.height = (args->val3 & 0xFFFF);
        O2ImageRef img = [display imageForRect:rect];
        int imglen = 0;
        if(img) {
            imglen = O2ImageGetBytesPerRow(img) * O2ImageGetHeight(img);
            reply.val1 = shmget(args->val1 ^ random(), imglen, IPC_CREAT|0666);
        }
        if(reply.val1) {
            uint8_t *p = shmat(reply.val1, NULL, 0);
            if(!p)
                shmctl(reply.val1, IPC_RMID, NULL);
            else {
                memcpy(p, [img directBytes], imglen);
                shmdt(p);
            }
        }
        if(img)
            O2ImageRelease(img);
    }
    [self sendInlineData:&reply length:sizeof(reply) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

// Configuring Displays
-(void)rpcCompleteDisplayConfiguration:(PortMessage *)msg {
    struct _CGDisplayConfigInner *inner = (struct _CGDisplayConfigInner *)msg->data;
    struct wsRPCSimple reply = { {kCGCompleteDisplayConfiguration, 4}, 0 };

    if(logLevel >= WS_INFO)
        NSLog(@"configuring displays. rpc={%u %u}, length %u option %u", 
                inner->rpc.code, inner->rpc.len, inner->length, inner->option);
    int i = sizeof(struct _CGDisplayConfigInner);

    CGError ret = kCGErrorSuccess; // let's be optimistic
 
    while(i < inner->length) {
        char *p = (char *)inner + i;
        int opcode = *(uint32_t *)p;
        switch(opcode) {
            case CGDISPCFG_MIRROR: {
                struct _CGDispCfgMirror *op = p;
                if(logLevel >= WS_INFO)
                    NSLog(@"mirroring: %x %x", op->display, op->primary);
                WSDisplay *display = [self displayWithID:op->display];
                WSDisplay *primary = [self displayWithID:op->primary];
                i += sizeof(struct _CGDispCfgMirror);
                if(display == nil || primary == nil) {
                    ret = kCGErrorIllegalArgument;
                    break;
                }
                if([display mirror:primary] == NO)
                    ret = kCGErrorFailure;
                break;
            }
            case CGDISPCFG_ORIGIN: {
                struct _CGDispCfgOrigin *op = p;
                if(logLevel >= WS_INFO)
                    NSLog(@"set origin: %x %d,%d", op->display, op->x, op->y);
                i += sizeof(struct _CGDispCfgOrigin);
                WSDisplay *display = [self displayWithID:op->display];
                if(display == nil) {
                    ret = kCGErrorIllegalArgument;
                    break;
                }
                if([display setOriginX:op->x Y:op->y] == NO)
                    ret = kCGErrorFailure;
                break;
            }
            case CGDISPCFG_MODE: {
                struct _CGDispCfgMode *op = p;
                if(logLevel >= WS_INFO)
                    NSLog(@"set mode: %x %u %u %.2f %08x", op->display, op->mode.width,
                            op->mode.height, op->mode.refresh, op->mode.flags);
                WSDisplay *display = [self displayWithID:op->display];
                i += sizeof(struct _CGDispCfgMode);
                if(display == nil) {
                    ret = kCGErrorIllegalArgument;
                    break;
                }
                if([display setMode:&op->mode] == NO)
                    ret = kCGErrorFailure;
                break;
            }
            default:
                 NSLog(@"Unknown display configuration command");
                 i = inner->length;
                 break;
        };

        if(ret != kCGErrorSuccess)
            break;
    }

    if(ret == kCGErrorSuccess) {
        [displays makeObjectsPerformSelector:
            inner->option == kCGConfigureForAppOnly
                ? @selector(saveAppConfig)
                : inner->option == kCGConfigureForSession
                    ? @selector(saveSessionConfig)
                    : @selector(savePermanentConfig)
            withObject:nil];
    } else {
        // FIXME: roll back any changes to our saved config if something failed
    }

    reply.val1 = ret;
    [self sendInlineData:&reply length:sizeof(reply) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

-(void)rpcRestorePermanentDisplayConfiguration:(PortMessage *)msg {
    [displays makeObjectsPerformSelector:@selector(restorePermanentConfig) withObject:nil];
}

// Getting the Display Configuration
-(void)rpcDisplayCopyColorSpace:(PortMessage *)msg {
    // FIXME: not implemented yet
}

-(void)rpcDisplayStateFlags:(PortMessage *)msg {
    struct wsRPCSimple *args = (struct wsRPCSimple *)msg->data;
    struct wsRPCSimple reply = { {kCGDisplayStateFlags, 4}, 0};
    WSDisplay *display = [self displayWithID:args->val1];
    if(display) {
        reply.val1 = [display flags];
    }
    [self sendInlineData:&reply length:sizeof(reply) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

-(void)rpcDisplayMirrorsDisplay:(PortMessage *)msg {
    struct wsRPCSimple *args = (struct wsRPCSimple *)msg->data;
    struct wsRPCSimple reply = { {kCGDisplayMirrorsDisplay, 4}, 0};
    WSDisplay *display = [self displayWithID:args->val1];
    if(display) {
        WSDisplay *primary = [display mirrorOf];
        if(primary)
            reply.val1 = [primary getDisplayID];
    }
    [self sendInlineData:&reply length:sizeof(reply) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

-(void)rpcDisplayModelNumber:(PortMessage *)msg {
    struct wsRPCSimple *args = (struct wsRPCSimple *)msg->data;
    struct wsRPCSimple reply = { {kCGDisplayModelNumber, 4}, 0};
    WSDisplay *display = [self displayWithID:args->val1];
    if(display) {
        reply.val1 = [display modelNumber];
    }
    [self sendInlineData:&reply length:sizeof(reply) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

-(void)rpcDisplayPrimaryDisplay:(PortMessage *)msg {
    struct wsRPCSimple *args = (struct wsRPCSimple *)msg->data;
    struct wsRPCSimple reply = { {kCGDisplayPrimaryDisplay, 4}, args->val1};
    WSDisplay *display = [self displayWithID:args->val1];
    if(display) {
        WSDisplay *primary = [display primaryDisplay];
        if(primary)
            reply.val1 = [primary getDisplayID];
    }
    [self sendInlineData:&reply length:sizeof(reply) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

-(void)rpcDisplayRotation:(PortMessage *)msg {
    struct wsRPCSimple *args = (struct wsRPCSimple *)msg->data;
    struct wsRPCSimple reply = { {kCGDisplayRotation, 4}, 0};
    WSDisplay *display = [self displayWithID:args->val1];
    if(display) {
        reply.val1 = (uint32_t)[display rotation];
    }
    [self sendInlineData:&reply length:sizeof(reply) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

-(void)rpcDisplayScreenSize:(PortMessage *)msg {
    struct wsRPCSimple *args = (struct wsRPCSimple *)msg->data;
    struct wsRPCSimple reply = { {kCGDisplayScreenSize, 8}, 0};
    WSDisplay *display = [self displayWithID:args->val1];
    if(display) {
        CGSize size = [display screenSizeMM];
        reply.val1 = (uint32_t)size.width;
        reply.val2 = (uint32_t)size.height;
    }
    [self sendInlineData:&reply length:sizeof(reply) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

-(void)rpcDisplaySerialNumber:(PortMessage *)msg {
    struct wsRPCSimple *args = (struct wsRPCSimple *)msg->data;
    struct wsRPCSimple reply = { {kCGDisplaySerialNumber, 4}, 0};
    WSDisplay *display = [self displayWithID:args->val1];
    if(display) {
        reply.val1 = [display serialNumber];
    }
    [self sendInlineData:&reply length:sizeof(reply) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

-(void)rpcDisplayUnitNumber:(PortMessage *)msg {
    struct wsRPCSimple *args = (struct wsRPCSimple *)msg->data;
    struct wsRPCSimple reply = { {kCGDisplayUnitNumber, 4}, 0};
    WSDisplay *display = [self displayWithID:args->val1];
    if(display) {
        reply.val1 = [displays indexOfObject:display];
    }
    [self sendInlineData:&reply length:sizeof(reply) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

-(void)rpcDisplayVendorNumber:(PortMessage *)msg {
    struct wsRPCSimple *args = (struct wsRPCSimple *)msg->data;
    struct wsRPCSimple reply = { {kCGDisplayVendorNumber, 4}, 0};
    WSDisplay *display = [self displayWithID:args->val1];
    if(display) {
        reply.val1 = [display vendorNumber];
    }
    [self sendInlineData:&reply length:sizeof(reply) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

// Retrieving Display Parameters
-(void)rpcDisplayBounds:(PortMessage *)msg {
    struct wsRPCSimple *args = (struct wsRPCSimple *)msg->data;
    struct wsRPCSimple reply = { {kCGDisplayBounds, 16}, 0, 0, 0, 0 };
    WSDisplay *display = [self displayWithID:args->val1];
    if(display) {
        CGRect rect = [display geometry];
        reply.val1 = rect.origin.x;
        reply.val2 = rect.origin.y;
        reply.val3 = rect.size.width;
        reply.val4 = rect.size.height;
    }
    [self sendInlineData:&reply length:sizeof(reply) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

// Creating and Managing Display Modes
-(void)rpcDisplayCopyDisplayMode:(PortMessage *)msg {
    struct wsRPCSimple *args = (struct wsRPCSimple *)msg->data;
    struct {
        struct wsRPCBase base;
        struct CGDisplayMode mode;
    } reply;
    reply.base.code = kCGDisplayCopyDisplayMode;
    reply.base.len = 0;
    
    WSDisplay *display = [self displayWithID:args->val1];
    if(display) {
        reply.base.len = sizeof(struct CGDisplayMode);
        struct CGDisplayMode *mode = [display currentMode];
        memcpy(&reply.mode, mode, sizeof(struct CGDisplayMode));
    }
    [self sendInlineData:&reply length:sizeof(reply) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

-(void)rpcDisplayCopyAllDisplayModes:(PortMessage *)msg {
    struct wsRPCSimple *args = (struct wsRPCSimple *)msg->data;
    struct {
        struct wsRPCBase base;
        struct CGDisplayMode mode[32];
    } reply;
    reply.base.code = kCGDisplayCopyAllDisplayModes;
    reply.base.len = 0;
    
    WSDisplay *display = [self displayWithID:args->val1];
    if(display) {
        CFArrayRef allModes = [display allModes];
        for(int i = 0; i < CFArrayGetCount(allModes); ++i) {
            memcpy(&reply.mode[i], CFArrayGetValueAtIndex(allModes, i), sizeof(struct CGDisplayMode));
            reply.base.len += sizeof(struct CGDisplayMode);
        }
    }
    [self sendInlineData:&reply length:sizeof(reply) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

-(void)rpcDisplaySetDisplayMode:(PortMessage *)msg {
    struct wsRPCSimple *args = (struct wsRPCSimple *)msg->data;
    struct wsRPCSimple reply = { {kCGDisplaySetDisplayMode, 4}, kCGErrorIllegalArgument, 0, 0, 0 };
    
    WSDisplay *display = [self displayWithID:args->val1];
    if(display) {
        struct CGDisplayMode *mode = &(args->val2);
        if([display setMode:mode])
            reply.val1 = kCGErrorSuccess;
        else
            reply.val1 = kCGErrorFailure;
    }
    [self sendInlineData:&reply length:sizeof(reply) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

// Adjusting Display Gamma
-(void)rpcSetDisplayTransferByFormula:(PortMessage *)msg {
    struct {
        struct wsRPCBase base;
        uint32_t display;
        CGGammaValue vals[9];
    } *data = msg->data;

    int ret = kCGErrorIllegalArgument;
    WSDisplay *display = [self displayWithID:data->display];
    int count = 0;
    if(display)
        count = [display gammaTableSize];

    float *red, *green, *blue;
    float redMin, redMax, redGamma;
    float greenMin, greenMax, greenGamma;
    float blueMin, blueMax, blueGamma;

    if(count > 0) {
        redMin = data->vals[0];
        redMax = data->vals[1];
        redGamma = data->vals[2];
        greenMin = data->vals[3];
        greenMax = data->vals[4];
        greenGamma = data->vals[5];
        blueMin = data->vals[6];
        blueMax = data->vals[7];
        blueGamma = data->vals[8];

        red = malloc(sizeof(float)*count);
        green = malloc(sizeof(float)*count);
        blue = malloc(sizeof(float)*count);
    }

    ret = kCGErrorFailure;
    if(red && green && blue && count > 2) {
        red[0] = redMin + ((redMax - redMin) * pow(0, redGamma)); 
        green[0] = greenMin + ((greenMax - greenMin) * pow(0.0, greenGamma)); 
        blue[0] = blueMin + ((blueMax - blueMin) * pow(0, blueGamma)); 

        float increment = 1.0/(float)(count - 2);
        int j = 1;
        for(float i = increment; j < count && i < 1.0; i += increment) {
            red[j] = redMin + ((redMax - redMin) * pow(i, redGamma));
            green[j] = greenMin + ((greenMax - greenMin) * pow(i, greenGamma));
            blue[j] = blueMin + ((blueMax - blueMin) * pow(i, blueGamma));
            ++j;
        }

        red[count] = redMin + ((redMax - redMin) * pow(1.0, redGamma)); 
        green[count] = greenMin + ((greenMax - greenMin) * pow(1.0, greenGamma)); 
        blue[count] = blueMin + ((blueMax - blueMin) * pow(1.0, blueGamma)); 
        
        ret = [display loadGammaTable:red green:green blue:blue] ? kCGErrorSuccess : kCGErrorFailure;

        free(red);
        free(green);
        free(blue);
    }

    struct wsRPCSimple reply = { {kCGSetDisplayTransferByFormula, 4}, 0, 0, 0, 0 };
    reply.val1 = ret;
    [self sendInlineData:&reply length:sizeof(reply) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

-(void)rpcGetDisplayTransferByFormula:(PortMessage *)msg {
    struct {
        struct wsRPCBase base;
        uint32_t display;
        CGGammaValue vals[9];
    } *data = msg->data;

    int ret = kCGErrorIllegalArgument;
    WSDisplay *display = [self displayWithID:data->display];
    int count = 0;
    if(display)
        count = [display gammaTableSize];

    float *red, *green, *blue;
    if(count > 0) {
        red = malloc(sizeof(float)*count);
        green = malloc(sizeof(float)*count);
        blue = malloc(sizeof(float)*count);
    }

    ret = kCGErrorFailure;
    data->vals[0] = 1; // redMin
    data->vals[1] = 0; // redMax
    data->vals[3] = 1; // greenMin
    data->vals[4] = 0; // greenMax
    data->vals[6] = 1; // blueMin
    data->vals[7] = 0; // blueMax

    float gamma;
    if(red && green && blue && count > 0) {
        for(int i = 0; i < count; ++i) {
            gamma = red[i];
            if(gamma < data->vals[0])
                data->vals[0] = gamma; // new minimum
            if(gamma > data->vals[1])
                data->vals[1] = gamma; // new maximum

            gamma = green[i];
            if(gamma < data->vals[3])
                data->vals[0] = gamma; // new minimum
            if(gamma > data->vals[4])
                data->vals[1] = gamma; // new maximum

            gamma = blue[i];
            if(gamma < data->vals[6])
                data->vals[0] = gamma; // new minimum
            if(gamma > data->vals[7])
                data->vals[1] = gamma; // new maximum
        }

        free(red);
        free(green);
        free(blue);

        [display getGammaCoefficientRed:&data->vals[2] green:&data->vals[5] blue:&data->vals[8]];
    }

    data->base.len = 3 * count  * sizeof(CGGammaValue) + 4;
    data->display = ret;
    [self sendInlineData:msg->data length:sizeof(msg->data) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

-(void)rpcSetDisplayTransferByTable:(PortMessage *)msg {
    struct wsRPCSimple *args = msg->data;
    int len = args->base.len - 4;
    int entries = len / (3*sizeof(CGGammaValue));

    int ret = kCGErrorIllegalArgument;
    WSDisplay *display = [self displayWithID:args->val1];

    if(display) {
        ret = kCGErrorFailure;
        int count = [display gammaTableSize];

        float *red = malloc(sizeof(float)*count);
        float *green = malloc(sizeof(float)*count);
        float *blue = malloc(sizeof(float)*count);

        float *p = &(args->val2);
        for(int i = 0; i < count && i < entries; ++i) {
            float r = *p++;
            float g = *p++;
            float b = *p++;
            if(red)
                red[i] = r;
            if(green)
                green[i] = g;
            if(blue)
                blue[i] = b;
        }
        if(red && green && blue)
            ret = [display loadGammaTable:red green:green blue:blue] ? kCGErrorSuccess : kCGErrorFailure;
    }

    struct wsRPCSimple reply = { {kCGSetDisplayTransferByTable, 4}, 0, 0, 0, 0 };
    reply.val1 = ret;
    [self sendInlineData:&reply length:sizeof(reply) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

-(void)rpcGetDisplayTransferByTable:(PortMessage *)msg {
    struct wsRPCSimple *args = msg->data;
    WSDisplay *display = [self displayWithID:args->val1];

    struct { CGGammaValue r; CGGammaValue g; CGGammaValue b; } *vals = &(args->val1);
    if(display) {
        int count = [display gammaTableSize];

        float *red = malloc(sizeof(float)*count);
        float *green = malloc(sizeof(float)*count);
        float *blue = malloc(sizeof(float)*count);
        if(red && green && blue && count > 0) {
            [display getGammaTablesWithCapacity:count red:red green:green blue:blue];

            int i = 0;
            while(vals < (msg->data + sizeof(msg->data))) {
                vals->r = red[i];
                vals->g = green[i];
                vals->b = blue[i];
                vals += sizeof(CGGammaValue) * 3;
                ++i;
            }
        }

        free(red);
        free(green);
        free(blue);
    }
    args->base.len = (uint8_t *)vals - msg->data;
    [self sendInlineData:msg->data length:sizeof(msg->data) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

-(void)rpcSetDisplayTransferByByteTable:(PortMessage *)msg {
    struct wsRPCSimple *args = msg->data;
    int len = args->base.len - 4;
    int entries = len / 3;

    int ret = kCGErrorIllegalArgument;
    WSDisplay *display = [self displayWithID:args->val1];

    if(display) {
        ret = kCGErrorFailure;
        int count = [display gammaTableSize];

        uint8_t *red = malloc(count);
        uint8_t *green = malloc(count);
        uint8_t *blue = malloc(count);

        float *p = &(args->val2);
        for(int i = 0; i < count && i < entries; ++i) {
            uint8_t r = *p++;
            uint8_t g = *p++;
            uint8_t b = *p++;
            if(red)
                red[i] = r;
            if(green)
                green[i] = g;
            if(blue)
                blue[i] = b;
        }
        if(red && green && blue)
            ret = [display load8BitGammaTable:red green:green blue:blue] ? kCGErrorSuccess : kCGErrorFailure;
    }

    struct wsRPCSimple reply = { {kCGSetDisplayTransferByByteTable, 4}, 0, 0, 0, 0 };
    reply.val1 = ret;
    [self sendInlineData:&reply length:sizeof(reply) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

-(void)rpcDisplayRestoreColorSyncSettings:(PortMessage *)msg {
    [displays makeObjectsPerformSelector:@selector(loadDefaultGamma) withObject:nil];
}

-(void)rpcDisplayGammaTableCapacity:(PortMessage *)msg {
    struct wsRPCSimple *args = msg->data;
    int ret = 0;
    WSDisplay *display = [self displayWithID:args->val1];
    struct wsRPCSimple reply = { {kCGDisplayGammaTableCapacity, 4}, 0, 0, 0, 0 };
    reply.val1 = display ? [display gammaTableSize] : 0;
    [self sendInlineData:&reply length:sizeof(reply) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

// Controlling the Mouse Cursor
-(void)rpcDisplayHideCursor:(PortMessage *)msg {
    int ret = kCGErrorCannotComplete;
    if(msg->pid == [curApp pid]) {
        cursorHideCount++;
        ret = kCGErrorSuccess;
    }
    struct wsRPCSimple data;
    data.base.code = kCGDisplayHideCursor;
    data.base.len = 4;
    data.val1 = ret;
    [self sendInlineData:&data length:sizeof(data) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

-(void)rpcDisplayShowCursor:(PortMessage *)msg {
    int ret = kCGErrorCannotComplete;
    if(msg->pid == [curApp pid]) {
        cursorHideCount--;
        ret = kCGErrorSuccess;
    }
    struct wsRPCSimple data;
    data.base.code = kCGDisplayShowCursor;
    data.base.len = 4;
    data.val1 = ret;
    [self sendInlineData:&data length:sizeof(data) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

-(void)rpcDisplayMoveCursorToPoint:(PortMessage *)msg {
    struct wsRPCSimple *data = msg->data;
    int ret = kCGErrorIllegalArgument;
    WSDisplay *display = [self displayWithID:data->val1];

    if(display) {
        ret = kCGErrorSuccess;
        CGRect bounds = [display geometry];
        double x = clipTo(data->val2, bounds.origin.x, bounds.size.width);
        double y = clipTo(data->val3, bounds.origin.y, bounds.size.height);
        [input setPointerPos:NSMakePoint(x, y)];
    }
    struct wsRPCSimple reply;
    reply.base.code = kCGDisplayMoveCursorToPoint;
    reply.base.len = 4;
    reply.val1 = ret;
    [self sendInlineData:&reply length:sizeof(reply) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

-(void)rpcAssociateMouseAndMouseCursorPosition:(PortMessage *)msg {
    struct wsRPCSimple data;
    int ret = kCGErrorFailure;

    WSAppRecord *app = [apps objectForKey:[NSString stringWithCString:msg->bundleID]];
    if(app) {
        ret = kCGErrorSuccess;
        [app mouseCursorConnected:((struct wsRPCSimple *)msg->data)->val1];
    }
    data.base.code = kCGAssociateMouseAndMouseCursorPosition;
    data.base.len = 4;
    data.val1 = ret;
    [self sendInlineData:&data length:sizeof(data) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

// FIXME: this should be in Global Coordinate Space (all displays)
-(void)rpcWarpMouseCursorPosition:(PortMessage *)msg {
    struct wsRPCSimple *data = msg->data;
    int ret = kCGErrorIllegalArgument;
    WSDisplay *display = [self displayWithID:data->val1];

    if(display) {
        ret = kCGErrorSuccess;
        CGRect bounds = [display geometry];
        double x = clipTo(data->val2, bounds.origin.x, bounds.size.width);
        double y = clipTo(data->val3, bounds.origin.y, bounds.size.height);
        [input setPointerPos:NSMakePoint(x, y)];
    }

    struct wsRPCSimple reply;
    reply.base.code = kCGWarpMouseCursorPosition;
    reply.base.len = 4;
    reply.val1 = ret;
    [self sendInlineData:&reply length:sizeof(reply) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

-(void)rpcGetLastMouseDelta:(PortMessage *)msg {
    struct wsRPCSimple data;
    NSPoint pos = [input pointerPos];
    data.base.code = kCGGetLastMouseDelta;
    data.base.len = 8;
    data.val1 = pos.x;
    data.val2 = pos.y;
    [self sendInlineData:&data length:sizeof(data) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

-(void)rpcWindowCreate:(PortMessage *)msg {
    struct wsRPCSimple reply = { {kWSWindowCreate, 4}, kWSErrorFailure, 0, 0, 0 };

    if(msg->len == sizeof(struct wsRPCWindow)) {
        struct wsRPCWindow *data = (struct wsRPCWindow*)msg->data;
        WSAppRecord *app = [apps objectForKey:[NSString stringWithCString:msg->bundleID]];

        if(app != nil) {
            if([self windowCreate:data forApp:app] != 0)
                reply.val1 = kWSErrorSuccess;
        } else {
            NSLog(@"No matching app for rpcWindowCreate! %s %u", msg->bundleID, msg->pid);
        }
    }
    [self sendInlineData:&reply length:sizeof(reply) withCode:MSG_ID_RPC toPort:msg->descriptor.name];
}

-(void)rpcWindowModifyState:(PortMessage *)msg {
    if(msg->len == sizeof(struct wsRPCWindow)) {
        struct wsRPCWindow *data = (struct wsRPCWindow*)msg->data;
        WSAppRecord *app = [apps objectForKey:[NSString stringWithCString:msg->bundleID]];

        if(app != nil) {
            [self windowModify:data forApp:app];
        } else {
            NSLog(@"No matching app for rpcWindowModifyState! %s %u", msg->bundleID, msg->pid);
        }
    }
}

-(void)rpcWindowDestroy:(PortMessage *)msg {
    if(msg->len == sizeof(struct wsRPCWindow)) {
        struct wsRPCWindow *data = (struct wsRPCWindow*)msg->data;
        WSAppRecord *app = [apps objectForKey:[NSString stringWithCString:msg->bundleID]];

        if(app != nil) {
            [app removeWindowWithID:data->windowID];
        } else {
            NSLog(@"No matching app for rpcWindowDestroy! %s %u", msg->bundleID, msg->pid);
        }
    }
}

- (void)receiveMachMessage {
    ReceiveMessage msg = {0};
    mach_msg_return_t result = mach_msg((mach_msg_header_t *)&msg, MACH_RCV_MSG, 0, sizeof(msg),
        _servicePort, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
    if(result != MACH_MSG_SUCCESS)
        NSLog(@"mach_msg receive error 0x%x", result);
    else {
        switch(msg.msg.header.msgh_id) {
            case MSG_ID_RPC: { // new style synchronous RPC calls
                mach_port_t reply = MACH_PORT_NULL;
                if(msg.portMsg.msgh_descriptor_count > 0)
                    reply = msg.portMsg.descriptor.name;
                pid_t pid = msg.portMsg.pid;
                NSString *bundleID = nil;
                if(msg.portMsg.bundleID[0] != '\0') {
                    msg.portMsg.bundleID[sizeof(msg.portMsg.bundleID) - 1] = '\0'; // ensure null terminated
                    bundleID = [NSString stringWithCString:msg.portMsg.bundleID];
                } else
                    bundleID = [NSString stringWithFormat:@"unix.%u", pid];
                struct wsRPCBase *base = (struct wsRPCBase *)msg.portMsg.data;

                NSLog(@"RPC[%@ %u] reply %u data len %u code %u", bundleID, pid, reply, msg.portMsg.len, base->code);
                if(base->len > sizeof(msg.portMsg.data) - sizeof(struct wsRPCBase)) {
                    NSLog(@"RPC[%@ %u] rejected code %u request with oversized data block of %u bytes",
                            base->code, base->len);
                    return;
                }

                switch(base->code) {
                    // Quartz Display Services (CoreGraphics)
                    case kCGMainDisplayID: [self rpcMainDisplayID:&msg.portMsg]; break;
                    case kCGGetOnlineDisplayList: [self rpcGetOnlineDisplayList:&msg.portMsg]; break;
                    case kCGGetActiveDisplayList: [self rpcGetActiveDisplayList:&msg.portMsg]; break;
                    case kCGGetDisplaysWithOpenGLDisplayMask: [self rpcGetDisplaysWithOpenGLDisplayMask:&msg.portMsg];
                                                              break;
                    case kCGGetDisplaysWithPoint: [self rpcGetDisplaysWithPoint:&msg.portMsg]; break;
                    case kCGGetDisplaysWithRect: [self rpcGetDisplaysWithRect:&msg.portMsg]; break;
                    case kCGOpenGLDisplayMaskToDisplayID: [self rpcOpenGLDisplayMaskToDisplayID:&msg.portMsg];
                                                          break;
                    case kCGDisplayIDToOpenGLDisplayMask: [self rpcDisplayIDToOpenGLDisplayMask:&msg.portMsg];
                                                          break;
                    case kCGDisplayCaptureWithOptions: [self rpcDisplayCapture:&msg.portMsg]; break;
                    case kCGDisplayRelease: [self rpcDisplayRelease:&msg.portMsg]; break;
                    case kCGCaptureAllDisplaysWithOptions: [self rpcCaptureAllDisplays:&msg.portMsg]; break;
                    case kCGReleaseAllDisplays: [self rpcReleaseAllDisplays:&msg.portMsg]; break;
                    case kCGDisplayGetDrawingContext: [self rpcDisplayGetDrawingContext:&msg.portMsg]; break;
                    case kCGDisplayCreateImageForRect: [self rpcDisplayCreateImageForRect:&msg.portMsg]; break;
                    case kCGDisplayBounds: [self rpcDisplayBounds:&msg.portMsg]; break;
                    case kCGDisplayStateFlags: [self rpcDisplayStateFlags:&msg.portMsg]; break; 
                    case kCGDisplayMirrorsDisplay: [self rpcDisplayMirrorsDisplay:&msg.portMsg]; break; 
                    case kCGDisplayModelNumber: [self rpcDisplayModelNumber:&msg.portMsg]; break; 
                    case kCGDisplayPrimaryDisplay: [self rpcDisplayPrimaryDisplay:&msg.portMsg]; break; 
                    case kCGDisplayRotation: [self rpcDisplayRotation:&msg.portMsg]; break; 
                    case kCGDisplayScreenSize: [self rpcDisplayScreenSize:&msg.portMsg]; break; 
                    case kCGDisplaySerialNumber: [self rpcDisplaySerialNumber:&msg.portMsg]; break; 
                    case kCGDisplayUnitNumber: [self rpcDisplayUnitNumber:&msg.portMsg]; break; 
                    case kCGDisplayVendorNumber: [self rpcDisplayVendorNumber:&msg.portMsg]; break; 
                    case kCGCompleteDisplayConfiguration: [self rpcCompleteDisplayConfiguration:&msg.portMsg]; break;
                    case kCGRestorePermanentDisplayConfiguration: [self rpcRestorePermanentDisplayConfiguration:&msg.portMsg]; break;
                    case kCGDisplayCopyDisplayMode: [self rpcDisplayCopyDisplayMode:&msg.portMsg]; break;
                    case kCGDisplayCopyAllDisplayModes: [self rpcDisplayCopyAllDisplayModes:&msg.portMsg]; break;
                    case kCGDisplaySetDisplayMode: [self rpcDisplaySetDisplayMode:&msg.portMsg]; break;
                    case kCGSetDisplayTransferByFormula: [self rpcSetDisplayTransferByFormula:&msg.portMsg]; break;
                    case kCGGetDisplayTransferByFormula: [self rpcGetDisplayTransferByFormula:&msg.portMsg]; break;
                    case kCGSetDisplayTransferByTable: [self rpcSetDisplayTransferByTable:&msg.portMsg]; break;
                    case kCGGetDisplayTransferByTable: [self rpcGetDisplayTransferByTable:&msg.portMsg]; break;
                    case kCGSetDisplayTransferByByteTable: [self rpcSetDisplayTransferByByteTable:&msg.portMsg]; break;
                    case kCGDisplayRestoreColorSyncSettings: [self rpcDisplayRestoreColorSyncSettings:&msg.portMsg]; break;
                    case kCGDisplayGammaTableCapacity: [self rpcDisplayGammaTableCapacity:&msg.portMsg]; break;
                    case kCGDisplayHideCursor: [self rpcDisplayHideCursor:&msg.portMsg]; break;
                    case kCGDisplayShowCursor: [self rpcDisplayShowCursor:&msg.portMsg]; break;
                    case kCGDisplayMoveCursorToPoint: [self rpcDisplayMoveCursorToPoint:&msg.portMsg]; break;
                    case kCGAssociateMouseAndMouseCursorPosition: [self rpcAssociateMouseAndMouseCursorPosition:&msg.portMsg]; break;
                    case kCGWarpMouseCursorPosition: [self rpcWarpMouseCursorPosition:&msg.portMsg]; break;
                    case kCGGetLastMouseDelta: [self rpcGetLastMouseDelta:&msg.portMsg]; break;
                    // Window Management (AppKit)
                    case kWSWindowCreate: [self rpcWindowCreate:&msg.portMsg]; break;
                    case kWSWindowDestroy: [self rpcWindowDestroy:&msg.portMsg]; break;
                    case kWSWindowModifyState: [self rpcWindowModifyState:&msg.portMsg]; break;
                }
                break;
            }
            case MSG_ID_PORT:
            {
                mach_port_t port = msg.portMsg.descriptor.name;
                pid_t pid = msg.portMsg.pid;
                NSString *bundleID = [NSString stringWithCString:msg.portMsg.bundleID];
                if(logLevel >= WS_INFO)
                    NSLog(@"Port registration received from %@ pid %u for port %u", bundleID, pid, port);
                WSAppRecord *rec = [apps objectForKey:bundleID];
                if(!rec) {
                    rec = [WSAppRecord new];
                    rec.bundleID = bundleID;
                    rec.port = port;
                }
                rec.pid = pid;
                if(port != rec.port && logLevel >= WS_WARNING)
                    NSLog(@"Port registration received for %@ pid %u when already registered (%u -> %u)",
                            rec.bundleID, pid, rec.port, port);
                [apps setObject:rec forKey:bundleID];
                [self watchForProcessExit:pid];

                // A newly-launched app becomes the active one
                curApp = [apps objectForKey:bundleID];
                break;
            }
            case MSG_ID_INLINE:
            {
                switch(msg.msg.code) {
                    case CODE_ADD_RECENT_ITEM:
                        // FIXME: pass to SystemUIServer
                        break;
                    case CODE_APP_BECAME_ACTIVE:
                    {
                        pid_t pid;
                        memcpy(&pid, msg.msg.data, msg.msg.len);
                        //NSLog(@"CODE_APP_BECAME_ACTIVE: pid = %d", pid);
                        // FIXME: pass to SystemUIServer
                        break;
                    }
                    case CODE_APP_BECAME_INACTIVE:
                    {
                        pid_t pid;
                        memcpy(&pid, msg.msg.data, msg.msg.len);
                        //NSLog(@"CODE_APP_BECAME_INACTIVE: pid = %d", pid);
                        //FIXME: pass to SystemUIServer
                        break;
                    }
                    case CODE_ACTIVATION_STATE:
                    {
                        struct mach_activation_data data;
                        memcpy(&data, msg.msg.data, sizeof(data));
                        NSLog(@"CODE_ACTIVATION_STATE");
#if 0
                        // FIXME: pass to SystemUIServer
                        mach_port_t port = 0; // FIXME: get from active app
                        if(port != MACH_PORT_NULL) {
                            Message activate = {0};
                            activate.header.msgh_remote_port = port;
                            activate.header.msgh_bits = MACH_MSGH_BITS_SET(MACH_MSG_TYPE_COPY_SEND, 0, 0, 0);
                            activate.header.msgh_id = MSG_ID_INLINE;
                            activate.header.msgh_size = sizeof(activate) - sizeof(mach_msg_trailer_t);
                            activate.code = msg.msg.code;
                            memcpy(activate.data, msg.msg.data+sizeof(int), sizeof(int)); // window ID
                            activate.len = sizeof(int);
                            mach_msg((mach_msg_header_t *)&activate, MACH_SEND_MSG|MACH_SEND_TIMEOUT,
                                sizeof(activate) - sizeof(mach_msg_trailer_t),
                                0, MACH_PORT_NULL, 100 /* ms timeout */, MACH_PORT_NULL);
                        }
#endif
                        break;
                    }
                    case CODE_APP_HIDE:
                    {
                        pid_t pid;
                        memcpy(&pid, msg.msg.data, sizeof(int));
                        NSLog(@"CODE_APP_HIDE: pid = %d", pid);
                        // FIXME: pass to SystemUIServer
                        mach_port_t port = 0; // FIXME: get from active app
                        if(port != MACH_PORT_NULL) {
                            Message activate = {0};
                            activate.header.msgh_remote_port = port;
                            activate.header.msgh_bits = MACH_MSGH_BITS_SET(MACH_MSG_TYPE_COPY_SEND, 0, 0, 0);
                            activate.header.msgh_id = MSG_ID_INLINE;
                            activate.header.msgh_size = sizeof(activate) - sizeof(mach_msg_trailer_t);
                            activate.code = msg.msg.code;
                            activate.len = 0;
                            mach_msg((mach_msg_header_t *)&activate, MACH_SEND_MSG|MACH_SEND_TIMEOUT,
                                sizeof(activate) - sizeof(mach_msg_trailer_t),
                                0, MACH_PORT_NULL, 100 /* ms timeout */, MACH_PORT_NULL);
                        }
                        break;
                    }
		    case CODE_ADD_STATUS_ITEM:
		    {
			NSData *data = [NSData
			    dataWithBytes:msg.msg.data length:msg.msg.len];
			NSObject *o = nil;
			@try {
			    o = [NSKeyedUnarchiver unarchiveObjectWithData:data];
			}
			@catch(NSException *localException) {
			    NSLog(@"%@",localException);
			}

			if(o == nil || [o isKindOfClass:[NSDictionary class]] == NO ||
			    [(NSDictionary *)o objectForKey:@"StatusItem"] == nil ||
			    [(NSDictionary *)o objectForKey:@"ProcessID"] == nil) {
			    fprintf(stderr, "archiver: bad input\n");
			    break;
			}

#if 0 // I don't think we need this anymore since we set up NOTE_EXIT in MSG_ID_PORT
			NSDictionary *dict = (NSDictionary *)o;
			unsigned int pid = [[dict objectForKey:@"ProcessID"] unsignedIntValue];
                        [self watchForProcessExit:pid];
#endif

                        // FIXME: send to SystemUIServer
			break;
		    }
                    default:
                        NSLog(@"Unhandled WindowServer code %u", msg.msg.code);
                }
                break;
            }
        }
    }
}

// called from our kq watcher thread
- (void)processKernelQueue {
    struct kevent out[128];
    int count = kevent(_kq, NULL, 0, out, 128, NULL);

    for(int i = 0; i < count; ++i) {
        switch(out[i].filter) {
            case EVFILT_PROC:
                if((out[i].fflags & NOTE_EXIT)) {
                    //NSLog(@"PID %lu exited", out[i].ident);
                    WSAppRecord *app = [self findAppByPID:out[i].ident];
                    if(app != nil && curApp == app) {
                        [self switchApp];
                        if(curApp == app) {
                            curApp = nil; // there was nothing to switch to
                            curWindow = nil;
                        }
                    }
                    if(app == nil)
                        NSLog(@"PID %u exited, but no matching app record", out[i].ident);
                    else {
                        [apps removeObjectForKey:app.bundleID];
                        [app removeAllWindows];
                    }
                    // FIXME: send to SystemUIServer and Dock.app
                }
                break;
            default:
                NSLog(@"unknown filter");
        }
    }
}

- (WSWindowRecord *)windowUnderPointer:(NSPoint)pos app:(WSAppRecord **)app {
    NSEnumerator *appEnum = [apps objectEnumerator];
    while((*app = [appEnum nextObject]) != nil) {
        NSArray *windows = [*app windows];
        for(int w = 0; w < [windows count]; ++w) {
            WSWindowRecord *win = [windows objectAtIndex:w];
            if(NSPointInRect(pos, win.frame))
                return win;
        }
    }
    return nil;
}

- (BOOL)sendEventToApp:(struct mach_event *)event {
    static BOOL inDrag = NO;
    static WSWindowRecord *dragWindow = nil;

    NSPoint pos = NSMakePoint(event->x, event->y);

    /* Any key input goes to active window & app, even if pointer is over something else
     * Otherwise, identify the window (if any) that is under the pointer. Windows on macOS
     * seem to receive mouse and scroll inputs when not the active window.
     */
    WSAppRecord *app = curApp;
    WSWindowRecord *window = nil;
    if(event->code == NSKeyDown || event->code == NSKeyUp)
        event->windowID = curWindow.number;
    else
        window = [self windowUnderPointer:pos app:&app];

    // First, check if we want to handle this event ourselves!
    switch(event->code) {
        case NSLeftMouseDragged: {
            // We are already dragging a window, so keep at it
            if(inDrag && dragWindow != nil) {
                [dragWindow moveByX:event->dx Y:event->dy];
                [self updateClientWindowState:dragWindow];
                return YES;
            }

            // Are we dragging a window at all?
            if(window == nil)
                return YES;

            // Are we dragging the titlebar?
            NSRect titleFrame = window.geometry;
            titleFrame.origin.y += titleFrame.size.height;
            titleFrame.size.height = WSWindowTitleHeight;
            if(NSPointInRect(pos, titleFrame)) {
                [window moveByX:event->dx Y:event->dy];
                [self updateClientWindowState:window];
                inDrag = YES;
                dragWindow = window;
                return YES;
            }

            // Handled all WS cases - send this to the window!
            event->windowID = window.number;
            break;
        }
        case NSLeftMouseDown: {
            if(window == nil)
                return YES;

            // these are just requests - the client can ignore them, so we don't
            // actually change the window until it sends us a new state message
            if(NSPointInRect(pos, window.closeButtonRect)) {
                NSLog(@"closing window %@", window);
                window.state = CLOSED;
                [self updateClientWindowState:window];
                return YES; // closing a window doesn't activate the app owning it
            } else if(NSPointInRect(pos, window.miniButtonRect)) {
                NSLog(@"miniaturizing window %@", window);
                window.state = MINIMIZED;
                [self updateClientWindowState:window];
            } else if(NSPointInRect(pos, window.zoomButtonRect)) {
                NSLog(@"zooming window %@", window);
                window.state = MAXIMIZED;
                [self updateClientWindowState:window];
            }

            // Handled all WS cases - send this to the window!
            NSLog(@"clicked %@ of app %@", window, app);
            curApp = app;
            curWindow = window;
            // FIXME: notify app of activation
            break;
        }
        case NSLeftMouseUp: {
            inDrag = NO;
            dragWindow = nil;
        }
    }

    if(app == nil)
        return YES;

    return [self sendInlineData:event
                         length:sizeof(struct mach_event)
                       withCode:CODE_INPUT_EVENT
                          toPort:[app port]];
}

- (void)updateClientWindowState:(WSWindowRecord *)window {
    struct wsRPCWindow data = {0};
    data.base.code = kWSWindowModifyState;
    data.base.len = sizeof(data) - sizeof(struct wsRPCBase);
    data.windowID = window.number;
    data.x = window.geometry.origin.x;
    data.y = window.geometry.origin.y;
    data.w = window.geometry.size.width;
    data.h = window.geometry.size.height;
    data.style = window.styleMask;
    data.state = window.state;
    // title is ignored - only client can set it

    const char *appKey = [window.shmPath cString];
    char *key = appKey + 1;
    while(*key != '/' && *key != '\0')
        key++;
    int len = key - (appKey + 1);
    key = malloc(len+1);
    memcpy(key, appKey+1, len);
    key[len] = 0;

    WSAppRecord *app = [apps objectForKey:[NSString stringWithCString:key]];
    if(app)
        [self sendInlineData:&data length:sizeof(data) withCode:CODE_WINDOW_STATE toPort:[app port]];
    else
        NSLog(@"Cannot send window state update to app: not found. %@", window);
}

- (BOOL)sendInlineData:(void *)data length:(int)length withCode:(int)code toPort:(mach_port_t)port {
    Message msg = {0};
    msg.header.msgh_remote_port = port;
    msg.header.msgh_bits = MACH_MSGH_BITS_SET(MACH_MSG_TYPE_COPY_SEND, 0, 0, 0);
    msg.header.msgh_id = MSG_ID_INLINE;
    msg.header.msgh_size = sizeof(msg) - sizeof(mach_msg_trailer_t);
    msg.code = code;
    msg.pid = getpid();
    strncpy(msg.bundleID, WINDOWSERVER_SVC_NAME, sizeof(msg.bundleID)-1);

    memcpy(msg.data, data, length);
    msg.len = length;

    int ret;
    if((ret = mach_msg((mach_msg_header_t *)&msg, MACH_SEND_MSG|MACH_SEND_TIMEOUT,
        sizeof(msg) - sizeof(mach_msg_trailer_t), 0, MACH_PORT_NULL, 50 /* ms timeout */,
        MACH_PORT_NULL)) != MACH_MSG_SUCCESS) {
        if(logLevel >= WS_WARNING)
            NSLog(@"Failed to send message to port %d: 0x%x", port, ret);
        return NO;
    }
    return YES;
}

- (void)watchForProcessExit:(unsigned int)pid {
    struct kevent kev[1];
    EV_SET(kev, pid, EVFILT_PROC, EV_ADD|EV_ONESHOT, NOTE_EXIT, 0, NULL);
    kevent(_kq, kev, 1, NULL, 0, NULL);
}

- (WSAppRecord *)findAppByPID:(unsigned int)pid {
    NSEnumerator *apprecs = [apps objectEnumerator];
    WSAppRecord *app;
    while((app = [apprecs nextObject]) != nil) {
        if(app.pid == pid)
            return app;
    }
    return nil;
}

// FIXME: do some visual magic here for the user
- (void)switchApp {
    WSAppRecord *oldApp = curApp;

    WSAppRecord *app;
    NSArray *list = [apps allValues];
    for(int i = 0; i < [list count]; ++i) {
        app = [list objectAtIndex:i];
        if(app == curApp) {
            if(i+1 >= [list count])
                curApp = [list objectAtIndex:0];
            else
                curApp = [list objectAtIndex:1+i];
            break;
        }
    }

    curWindow = nil;
    if(curApp == nil)
        return;

    // Find the first non-hidden window for the newly active app
    for(int i = 0; i < [[curApp windows] count]; ++i) {
        WSWindowRecord *win = [[curApp windows] objectAtIndex:i];
        if([win state] == HIDDEN)
            continue;
        else {
            curWindow = win;
            break;
        }
    }

    if(curApp == oldApp)
        return;

    // Inform the old app that it has become inactive
    struct mach_activation_data data = {0};
    [self sendInlineData:&data
                  length:sizeof(data)
                withCode:CODE_ACTIVATION_STATE
                  toPort:[oldApp port]];

    // Inform the now-active app of its status
    data.windowID = (curWindow == nil) ? 0 : curWindow.number;
    data.active = 1;
    [self sendInlineData:&data
                  length:sizeof(data)
                withCode:CODE_ACTIVATION_STATE
                   toPort:[curApp port]];
}

-(void)signalQuit { ready = NO; }

-(CGFontRef)titleFont {
    return _titleFont;
}

-(NSSize)sizeOfTitleText:(NSString *)title {
    NSDictionary *attrs = @{
        NSFontAttributeName : [NSFont fontWithName:@"Inter" size:14.0], // FIXME: should be titleBarFontOfSize
    };
    NSAttributedString *atitle = [[NSAttributedString alloc] initWithString:title attributes:attrs];
    NSSize size = [atitle size];

    return size;
}

@end

