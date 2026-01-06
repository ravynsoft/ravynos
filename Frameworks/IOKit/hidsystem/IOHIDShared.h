/*
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Copyright (c) 1999-2011 Apple Computer, Inc.  All Rights Reserved.
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef _DEV_EVIO_H
#define _DEV_EVIO_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#if KERNEL
#include <IOKit/system.h>
#else /* !KERNEL */
#include <mach/message.h>
#include <IOKit/IOKitLib.h>
#endif /* KERNEL */

#include <IOKit/IOReturn.h>
#include <IOKit/graphics/IOGraphicsTypes.h>
#include <IOKit/hidsystem/IOHIDTypes.h>
#include <IOKit/hidsystem/IOLLEvent.h>

/*
 * Identify this driver as one that uses the new driverkit and messaging API
 */
#ifndef _NeXT_MACH_EVENT_DRIVER_
#define _NeXT_MACH_EVENT_DRIVER_	(1)
#endif /* _NeXT_MACH_EVENT_DRIVER_ */


/* Pressure Constants */
#define MINPRESSURE EV_MINPRESSURE
#define MAXPRESSURE EV_MAXPRESSURE

#define	LLEQSIZE 240	/* Entries in low-level event queue */

typedef struct _NXEQElStruct {
    int	next;		/* Slot of lleq for next event */
    OSSpinLock sema; /* Is high-level code reading this event now? */
    NXEvent event;	/* The event itself */
} NXEQElement;

/******************************************************************************
    SHARED MEMORY OVERVIEW
    
    PERSPECTIVE
    The ev driver and PostScript share at least one page of wired memory.
    This memory contains the low-level event queue which ev deposits events
    into and PostScript reads events from. Also, this memory contains other
    important data such as wait cursor state and some general cursor state.
    This memory is critical for speed.  That is, we avoid having to make
    system calls for common operations.
    
    SHARED MEMORY REGIONS
    There are currently three "regions" or "zones" delineated within this
    shared memory.  The first zone is the EvOffsets structure. This structure
    contains two offsets from the beginning of shared memory. The first offset
    is to the second zone, EvGlobals. The second offset is to the third
    zone, private shmem for drivers.
    
    INITIALIZATION OF SHARED MEMORY
    When the WindowServer starts up, it finds all screens that will be active.
    It then opens the ev driver and calls the EVIOSSCR ioctl repeatedly for
    each screen in use. This lets the ev driver set up the evScreen array
    and fill in each element. This ioctl also returns to PostScript a running
    total shared memory size with which to allocate. PostScript then allocates
    a region of memory this size and calls evmmap to "map in" this shared
    region.  Evmmap initializes and fills in the EvOffsets and EvGlobals.
    Next the WindowServer calls each screen in turn to register itself with
    the ev driver in the same sequence as presented to EVIOSSCR.  Each screen
    driver calls ev_register_screen() which among other things allocates a
    part of the private shmem (of the third shared memory zone) for the driver.
    
    DEBUGGING NOTES
    You can easily display and set this shared memory from kgdb, but usually
    cannot do so from within PostScript.  Gdb (or some weird interaction
    between gdb and the os) chokes on this shmem.  So if you read or write
    this area of memory, copy-on-write will occur and you'll get a completely
    new page for PostScript.  This will render the shared memory scheme
    useless and you will have to restart PostScript.  It was my understanding
    that before, we were able to "read" this area from PS, but not write to
    it (the idea behind copy-on-WRITE).  However, this seems to be broken
    in 2.0.  We think this is a kernel bug.
******************************************************************************/

typedef volatile struct _evOffsets {
    int	evGlobalsOffset;	/* Offset to EvGlobals structure */
    int evShmemOffset;		/* Offset to private shmem regions */
} EvOffsets;

/******************************************************************************
    EvGlobals
    This structures defines the portion of the events driver data structure
    that is exported to the PostScript server. It contains the event queue
    which is in memory shared between the driver and the PostScript server.
    All the variables necessary to read and process events from the queue are
    contained here.
******************************************************************************/

typedef volatile struct _evGlobals {
    OSSpinLock cursorSema; 	/* set to disable periodic code */
    int eNum;                       /* Unique id for mouse events */
    int buttons;                    /* State of the mouse buttons 1==down, 0==up */
    int eventFlags;                 /* The current value of event.flags */
    int VertRetraceClock;           /* The current value of event.time */
    IOGPoint cursorLoc;             /* The current location of the cursor, in desktop coordinates */
    int frame;                      /* current cursor frame */
    IOGBounds workBounds;           /* bounding box of all screens */
    IOGBounds mouseRect;            /* Rect for mouse-exited events */
    int version;                    /* for run time checks */
    int	structSize;                 /* for run time checks */
    int lastFrame;
                                    /* The current location of the cursor, 24.8 bit fixed point format */
    IOFixedPoint32 screenCursorFixed; /* in Screen coordinates  */
    IOFixedPoint32 desktopCursorFixed;/* in Desktop coordinates  */
    unsigned int reservedA[27];

    unsigned reserved:25;
    unsigned updateCursorPositionFromFixed:1; /* if this is set, IOHIDSystem will take any cursor position updates from desktopCursorFixed instead of cursorLoc */
    unsigned logCursorUpdates:1;    /* log cursor updates */
    unsigned wantPressure:1;        /* pressure in current mouseRect? */
    unsigned wantPrecision:1;       /* precise coordinates in current mouseRect? */
    unsigned dontWantCoalesce:1;    /* coalesce within the current mouseRect? */
    unsigned dontCoalesce:1;        /* actual flag which determines coalescing */
    unsigned mouseRectValid:1;      /* If nonzero, post a mouse-exited whenever mouse outside mouseRect. */
    int movedMask;                  /* This contains an event mask for the three events MOUSEMOVED,
                                        LMOUSEDRAGGED,  and RMOUSEDRAGGED. It says whether driver should
                                        generate those events. */
    OSSpinLock waitCursorSema; /* protects wait cursor fields */
    int AALastEventSent;            /* timestamp for wait cursor */
    int AALastEventConsumed;        /* timestamp for wait cursor */	
    int waitCursorUp;               /* Is wait cursor up? */
    char ctxtTimedOut;              /* Has wait cursor timer expired? */
    char waitCursorEnabled;         /* Play wait cursor game (per ctxt)? */
    char globalWaitCursorEnabled;   /* Play wait cursor game (global)? */
    int waitThreshold;              /* time before wait cursor appears */

    int LLEHead;                    /* The next event to be read */
    int LLETail;                    /* Where the next event will go */
    int LLELast;                    /* The last event entered */
    NXEQElement lleq[LLEQSIZE];     /* The event queue itself */
} EvGlobals;

/* These evio structs are used in various calls supported by the ev driver. */

struct evioLLEvent {
    int setCursor;
    int type;
    IOGPoint location;
    NXEventData data;
    int setFlags;
    int flags;
};

typedef struct evioLLEvent _NXLLEvent;

#ifdef mach3xxx

/*
 * On a keypress of a VOL UP or VOL DOWN key, we send a message to the 
 * sound server to notify it of the volume change.  The message includes
 * a flag to indicate which key was pressed, and the machine independant
 * flag bits to indicate which modifier keys were pressed.
 */

struct evioSpecialKeyMsg
{
	msg_header_t Head;
	msg_type_t keyType;
	int key;		// special key number, from bsd/dev/ev_keymap.h
	msg_type_t directionType;
	int direction;		// NX_KEYDOWN, NX_KEYUP from event.h
	msg_type_t flagsType;
	int flags;		// device independant flags from event.h
	msg_type_t levelType;
	int level;		// EV_AUDIO_MIN_VOLUME to EV_AUDIO_MAX_VOLUME
};
#else
struct evioSpecialKeyMsg
{
	mach_msg_header_t Head;
	int key;		// special key number, from bsd/dev/ev_keymap.h
	int direction;		// NX_KEYDOWN, NX_KEYUP from event.h
	int flags;		// device independant flags from event.h
	int level;		// EV_AUDIO_MIN_VOLUME to EV_AUDIO_MAX_VOLUME
};
#endif

#define EV_SPECIAL_KEY_MSG_ID	(('S'<<24) | ('k'<<16) | ('e'<<8) | ('y'))
typedef struct evioSpecialKeyMsg *evioSpecialKeyMsg_t;

/*
 * Volume ranges
 */
#define EV_AUDIO_MIN_VOLUME	0
#define EV_AUDIO_MAX_VOLUME	64

#define kIOHIDSystemClass	"IOHIDSystem"
#define kIOHIKeyboardClass	"IOHIKeyboard"
#define kIOHIPointingClass	"IOHIPointing"

#define IOHIDSYSTEM_CONFORMSTO	kIOHIDSystemClass

enum {
    kIOHIDEventNotification     = 0,
};
#define kIOHIDCurrentShmemVersion           4
#define kIOHIDLastCompatibleShmemVersion    3

enum {
    kIOHIDServerConnectType	= 0,
    kIOHIDParamConnectType	= 1,
    kIOHIDEventSystemConnectType = 3,
};

enum {
    kIOHIDGlobalMemory          = 0
};

enum {
    kIOHIDEventQueueTypeKernel  = 0,
    kIOHIDEventQueueTypeUser    = 1
};

#ifdef KERNEL
typedef UInt16 (*MasterVolumeUpdate)(void);
typedef bool (*MasterMuteUpdate)(void);

typedef struct {
    MasterVolumeUpdate incrementMasterVolume;
    MasterVolumeUpdate decrementMasterVolume;
    MasterMuteUpdate toggleMasterMute;
} MasterAudioFunctions;

extern MasterAudioFunctions *masterAudioFunctions;
#endif

#ifndef KERNEL
#ifndef _IOKIT_IOHIDLIB_H
#include <IOKit/hidsystem/IOHIDLib.h>
#endif
#endif /* !KERNEL */



enum {
    /*!
     @defined kIOHIDOpenedByEventSystem
     @abstract option passed to open for IOHIDInterface  if opened by IOHIDEventDriver
     */
    kIOHIDOpenedByEventSystem       = 0x10000,
    /*!
     @defined kIOHIDOpenedByFastPathClient
     @abstract option passed to open for IOHIDEventService if opened by fast path client
     */
    kIOHIDOpenedByFastPathClient    = 0x20000
};

/*!
 @defined kIOHIDMessageOpenedByEventSystem
 @abstract message to IOHIDInterface and /or IOHIDDevice if instance of corresponding  IOHIDEventService opened by event system and ready to receive events
 */

#define kIOHIDMessageOpenedByEventSystem  iokit_vendor_specific_msg(1)

/*!
 @defined kIOHIDMessageRelayServiceInterfaceActive
 @abstract message from IOHIDDevice to indicate that the IOHIDRelayService's USB interface is active.
 */

#define kIOHIDMessageRelayServiceInterfaceActive     iokit_vendor_specific_msg(2)


__END_DECLS


#endif /* !_DEV_EVIO_H */
