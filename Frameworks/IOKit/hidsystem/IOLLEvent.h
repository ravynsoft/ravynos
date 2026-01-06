/*
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Copyright (c) 1999-2009 Apple Computer, Inc.  All Rights Reserved.
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
/******************************************************************************
	event.h (PostScript side version)
	
	CONFIDENTIAL
	Copyright (c) 1988 NeXT, Inc. as an unpublished work.
	All Rights Reserved.

	Created Leo 01Mar88

	Modified:
	04May88 Leo  Final event types and record
	22Aug88 Leo  Change short -> int for window, add reserved
	26May90 Ted  Added NX_UNDIMMASK to correct triggering of UndoAutoDim
	12Dec91 Mike Brought into sync with dpsclient/event.h, and fixed
		     the #ifndef interlock with dpsclient/event.h that was
		     broken during the Great Header Revision.

	The PostScript version of this file differs from the
	Window Kit version in that the coordinates here are
	ints instead of floats.
******************************************************************************/

#ifndef _DEV_EVENT_H
#define _DEV_EVENT_H

#include <libkern/OSTypes.h>
#include <IOKit/hidsystem/IOHIDTypes.h>
#include <mach/mach_types.h>

#ifdef EVENT_H		/* Interlock with dpsclient/event.h */
#if !defined(_NXSIZE_)	/* Work around patch for old event.h in Phase 3 projs*/
#define _NXSIZE_	1	/* NXCoord, NXPoint, NXSize decl seen */
#define _NXSize_	NXSize
#endif /* _NXSIZE_ */
#else  /* EVENT_H */		/* Haven't seen dpsclient/event.h, so define away */
#define EVENT_H

#ifdef	KERNEL
#else	/* KERNEL */

#if !defined(_NXSIZE_)	/* Work around patch for old event.h in Phase 3 projs*/
#define _NXSIZE_	1	/* NXCoord, NXPoint, NXSize decl seen */
typedef float   NXCoord;

typedef struct _NXPoint {	/* point */
    NXCoord         x, y;
} NXPoint;

typedef struct _NXSize {	/* size */
    NXCoord         width, height;
} NXSize;
#define _NXSize_	NXSize	/* Correct usage in event_status_driver.h */
#endif /* _NXSIZE_ */

#endif	/* KERNEL */

/* Event types */

#define NX_NULLEVENT		0	/* internal use */

/* mouse events */

#define NX_LMOUSEDOWN		1	/* left mouse-down event */
#define NX_LMOUSEUP		2	/* left mouse-up event */
#define NX_RMOUSEDOWN		3	/* right mouse-down event */
#define NX_RMOUSEUP		4	/* right mouse-up event */
#define NX_MOUSEMOVED		5	/* mouse-moved event */
#define NX_LMOUSEDRAGGED	6	/* left mouse-dragged event */
#define NX_RMOUSEDRAGGED	7	/* right mouse-dragged event */
#define NX_MOUSEENTERED		8	/* mouse-entered event */
#define NX_MOUSEEXITED		9	/* mouse-exited event */

/* other mouse events
 *
 * event.data.mouse.buttonNumber should contain the 
 * button number (2-31) changing state.
 */
#define NX_OMOUSEDOWN		25	/* other mouse-down event */
#define NX_OMOUSEUP		26	/* other mouse-up event */
#define NX_OMOUSEDRAGGED	27	/* other mouse-dragged event */

/* keyboard events */

#define NX_KEYDOWN		10	/* key-down event */
#define NX_KEYUP		11	/* key-up event */
#define NX_FLAGSCHANGED		12	/* flags-changed event */

/* composite events */

#define NX_KITDEFINED		13	/* application-kit-defined event */
#define NX_SYSDEFINED		14	/* system-defined event */
#define NX_APPDEFINED		15	/* application-defined event */
/* There are additional DPS client defined events past this point. */

/* Scroll wheel events */

#define NX_SCROLLWHEELMOVED	22

/* Zoom events */
#define NX_ZOOM             28

/* tablet events */

#define NX_TABLETPOINTER	23	/* for non-mousing transducers */
#define NX_TABLETPROXIMITY	24  /* for non-mousing transducers */

/* event range */

#define NX_FIRSTEVENT		0
#define NX_LASTEVENT		28
#define NX_NUMPROCS		(NX_LASTEVENT-NX_FIRSTEVENT+1)

/* Event masks */
#define NX_NULLEVENTMASK        (1 << NX_NULLEVENT)     /* NULL event */
#define NX_LMOUSEDOWNMASK       (1 << NX_LMOUSEDOWN)	/* left mouse-down */
#define NX_LMOUSEUPMASK         (1 << NX_LMOUSEUP)      /* left mouse-up */
#define NX_RMOUSEDOWNMASK       (1 << NX_RMOUSEDOWN)	/* right mouse-down */
#define NX_RMOUSEUPMASK         (1 << NX_RMOUSEUP)      /* right mouse-up */
#define NX_OMOUSEDOWNMASK       (1 << NX_OMOUSEDOWN)	/* other mouse-down */
#define NX_OMOUSEUPMASK         (1 << NX_OMOUSEUP)      /* other mouse-up  */
#define NX_MOUSEMOVEDMASK       (1 << NX_MOUSEMOVED)	/* mouse-moved */
#define NX_LMOUSEDRAGGEDMASK	(1 << NX_LMOUSEDRAGGED)	/* left-dragged */
#define NX_RMOUSEDRAGGEDMASK	(1 << NX_RMOUSEDRAGGED)	/* right-dragged */
#define NX_OMOUSEDRAGGEDMASK	(1 << NX_OMOUSEDRAGGED)	/* other-dragged */
#define NX_MOUSEENTEREDMASK     (1 << NX_MOUSEENTERED)	/* mouse-entered */
#define NX_MOUSEEXITEDMASK      (1 << NX_MOUSEEXITED)	/* mouse-exited */
#define NX_KEYDOWNMASK          (1 << NX_KEYDOWN)       /* key-down */
#define NX_KEYUPMASK            (1 << NX_KEYUP)         /* key-up */
#define NX_FLAGSCHANGEDMASK     (1 << NX_FLAGSCHANGED)	/* flags-changed */
#define NX_KITDEFINEDMASK       (1 << NX_KITDEFINED)	/* kit-defined */
#define NX_SYSDEFINEDMASK       (1 << NX_SYSDEFINED)	/* system-defined */
#define NX_APPDEFINEDMASK       (1 << NX_APPDEFINED)	/* app-defined */
#define NX_SCROLLWHEELMOVEDMASK	(1 << NX_SCROLLWHEELMOVED)	/* scroll wheel moved */
#define NX_ZOOMMASK             (1 << NX_ZOOM)          /* Zoom */
#define NX_TABLETPOINTERMASK	(1 << NX_TABLETPOINTER)	/* tablet pointer moved */
#define NX_TABLETPROXIMITYMASK	(1 << NX_TABLETPROXIMITY)	/* tablet pointer proximity */

#define EventCodeMask(type)	(1 << (type))
#define NX_ALLEVENTS		-1	/* Check for all events */

/* sub types for mouse and move events */

#define NX_SUBTYPE_DEFAULT					0
#define NX_SUBTYPE_TABLET_POINT				1
#define NX_SUBTYPE_TABLET_PROXIMITY			2
#define NX_SUBTYPE_MOUSE_TOUCH              3

/* sub types for system defined events */

#define NX_SUBTYPE_POWER_KEY				1
#define NX_SUBTYPE_AUX_MOUSE_BUTTONS		7

/* 
 * NX_SUBTYPE_AUX_CONTROL_BUTTONS usage
 *
 * The incoming NXEvent for other mouse button down/up has event.type 
 * NX_SYSDEFINED and event.data.compound.subtype NX_SUBTYPE_AUX_MOUSE_BUTTONS.
 * Within the event.data.compound.misc.L[0] contains bits for all the buttons 
 * that have changed state, and event.data.compound.misc.L[1] contains the 
 * current button state as a bitmask, with 1 representing down, and 0
 * representing up.  Bit 0 is the left button, bit one is the right button, 
 * bit 2 is the center button and so forth.
 */
#define NX_SUBTYPE_AUX_CONTROL_BUTTONS		8

#define NX_SUBTYPE_EJECT_KEY				10
#define NX_SUBTYPE_SLEEP_EVENT				11
#define NX_SUBTYPE_RESTART_EVENT			12
#define NX_SUBTYPE_SHUTDOWN_EVENT			13
#define NX_SUBTYPE_MENU               16
#define NX_SUBTYPE_ACCESSIBILITY      17



#define NX_SUBTYPE_STICKYKEYS_ON			100
#define NX_SUBTYPE_STICKYKEYS_OFF			101
#define NX_SUBTYPE_STICKYKEYS_SHIFT			102
#define NX_SUBTYPE_STICKYKEYS_CONTROL			103
#define NX_SUBTYPE_STICKYKEYS_ALTERNATE			104
#define NX_SUBTYPE_STICKYKEYS_COMMAND			105
#define NX_SUBTYPE_STICKYKEYS_RELEASE			106
#define NX_SUBTYPE_STICKYKEYS_TOGGLEMOUSEDRIVING	107

// New stickykeys key events
// These were created to send an event describing the 
// different state of the modifiers
#define NX_SUBTYPE_STICKYKEYS_SHIFT_DOWN		110
#define NX_SUBTYPE_STICKYKEYS_CONTROL_DOWN		111
#define NX_SUBTYPE_STICKYKEYS_ALTERNATE_DOWN		112
#define NX_SUBTYPE_STICKYKEYS_COMMAND_DOWN		113
#define NX_SUBTYPE_STICKYKEYS_FN_DOWN			114

#define NX_SUBTYPE_STICKYKEYS_SHIFT_LOCK		120
#define NX_SUBTYPE_STICKYKEYS_CONTROL_LOCK		121
#define NX_SUBTYPE_STICKYKEYS_ALTERNATE_LOCK		122
#define NX_SUBTYPE_STICKYKEYS_COMMAND_LOCK		123
#define NX_SUBTYPE_STICKYKEYS_FN_LOCK			124

#define NX_SUBTYPE_STICKYKEYS_SHIFT_UP			130
#define NX_SUBTYPE_STICKYKEYS_CONTROL_UP		131
#define NX_SUBTYPE_STICKYKEYS_ALTERNATE_UP		132
#define NX_SUBTYPE_STICKYKEYS_COMMAND_UP		133
#define NX_SUBTYPE_STICKYKEYS_FN_UP			134



// SlowKeys 
#define NX_SUBTYPE_SLOWKEYS_START			200
#define NX_SUBTYPE_SLOWKEYS_ABORT			201
#define NX_SUBTYPE_SLOWKEYS_END				202

// HID Parameter Property Modified
#define NX_SUBTYPE_HIDPARAMETER_MODIFIED		210

/* Masks for the bits in event.flags */

/* device-independent */

#define	NX_ALPHASHIFTMASK	0x00010000
#define	NX_SHIFTMASK		0x00020000
#define	NX_CONTROLMASK		0x00040000
#define	NX_ALTERNATEMASK	0x00080000
#define	NX_COMMANDMASK		0x00100000
#define	NX_NUMERICPADMASK	0x00200000
#define	NX_HELPMASK		0x00400000
#define	NX_SECONDARYFNMASK	0x00800000
#define NX_ALPHASHIFT_STATELESS_MASK    0x01000000

/* device-dependent (really?) */

#define	NX_DEVICELCTLKEYMASK	0x00000001
#define	NX_DEVICELSHIFTKEYMASK	0x00000002
#define	NX_DEVICERSHIFTKEYMASK	0x00000004
#define	NX_DEVICELCMDKEYMASK	0x00000008
#define	NX_DEVICERCMDKEYMASK	0x00000010
#define	NX_DEVICELALTKEYMASK	0x00000020
#define	NX_DEVICERALTKEYMASK	0x00000040
#define NX_DEVICE_ALPHASHIFT_STATELESS_MASK 0x00000080
#define NX_DEVICERCTLKEYMASK	0x00002000

/* 
 * Additional reserved bits in event.flags
 */

#define NX_STYLUSPROXIMITYMASK	0x00000080	/* deprecated */
#define NX_NONCOALSESCEDMASK	0x00000100

/* click state values
 * If you have the following events in close succession, the click
 * field has the indicated value:
 *	
 *  Event	Click Value	Comments
 *  mouse-down	1		Not part of any click yet
 *  mouse-up	1		Aha! A click!
 *  mouse-down	2		Doing a double-click
 *  mouse-up	2		It's finished
 *  mouse-down	3		A triple
 *  mouse-up	3
 */

/* Values for the character set in event.data.key.charSet */

#define	NX_ASCIISET		0
#define NX_SYMBOLSET		1
#define	NX_DINGBATSSET		2

/* tablet button masks
 * Mask bits for the tablet barrel buttons placed in tablet.buttons.
 * The buttons field uses adopts the following convention:
 *
 * Bit      Comments
 * 0        Left Mouse Button ( kHIDUsage_Button_1 )
 * 1        Right Mouse Button ( kHIDUsage_Button_2 )
 * 2        Middle Mouse Button ( kHIDUsage_Button_3 )
 * 3        4th Mouse Button ( kHIDUsage_Button_4 )
 * ...
 * 15       15th Mouse Button
 *
 * For your convenience, the following mask bits have been defined
 * for tablet specific application:
 */
 
#define NX_TABLET_BUTTON_PENTIPMASK             0x0001
#define NX_TABLET_BUTTON_PENLOWERSIDEMASK       0x0002
#define NX_TABLET_BUTTON_PENUPPERSIDEMASK       0x0004


/* tablet capability masks
 * Mask bits for the tablet capabilities field.   Use these 
 * masks with the capabilities field of a proximity event to 
 * determine what fields in a Tablet Event are valid for this 
 * device.
 */
#define NX_TABLET_CAPABILITY_DEVICEIDMASK           0x0001
#define NX_TABLET_CAPABILITY_ABSXMASK               0x0002
#define NX_TABLET_CAPABILITY_ABSYMASK               0x0004
#define NX_TABLET_CAPABILITY_VENDOR1MASK            0x0008
#define NX_TABLET_CAPABILITY_VENDOR2MASK            0x0010
#define NX_TABLET_CAPABILITY_VENDOR3MASK            0x0020
#define NX_TABLET_CAPABILITY_BUTTONSMASK            0x0040
#define NX_TABLET_CAPABILITY_TILTXMASK              0x0080
#define NX_TABLET_CAPABILITY_TILTYMASK              0x0100
#define NX_TABLET_CAPABILITY_ABSZMASK               0x0200
#define NX_TABLET_CAPABILITY_PRESSUREMASK           0x0400
#define NX_TABLET_CAPABILITY_TANGENTIALPRESSUREMASK 0x0800
#define NX_TABLET_CAPABILITY_ORIENTINFOMASK         0x1000
#define NX_TABLET_CAPABILITY_ROTATIONMASK           0x2000

/* proximity pointer types
 * Value that describes the type of pointing device placed in
 * proximity.pointerType.
 */
 
#define NX_TABLET_POINTER_UNKNOWN               0
#define NX_TABLET_POINTER_PEN                   1
#define NX_TABLET_POINTER_CURSOR                2
#define NX_TABLET_POINTER_ERASER                3

/* TabletPointData type: defines the tablet data for points included
 * in mouse events created by a tablet driver.
 */

typedef struct _NXTabletPointData {
	SInt32  x;  /* absolute x coordinate in tablet space at full tablet resolution */
    SInt32  y;  /* absolute y coordinate in tablet space at full tablet resolution */
	SInt32  z;  /* absolute z coordinate in tablet space at full tablet resolution */
    UInt16  buttons;   /* one bit per button - bit 0 is first button - 1 = closed  */
    UInt16  pressure;  /* scaled pressure value; MAX=(2^16)-1, MIN=0 */
    struct {           /* tilt range is -((2^15)-1) to (2^15)-1 (-32767 to 32767)  */
        SInt16 x;      /* scaled tilt x value */
        SInt16 y;      /* scaled tilt y value */
	} tilt;
	UInt16  rotation;  /* Fixed-point representation of device rotation in a 10.6 format */
	SInt16  tangentialPressure; /* tangential pressure on the device; same range as tilt */
	UInt16  deviceID;  /* system-assigned unique device ID */
	SInt16  vendor1;   /* vendor-defined signed 16-bit integer */
	SInt16  vendor2;   /* vendor-defined signed 16-bit integer */
	SInt16  vendor3;   /* vendor-defined signed 16-bit integer */
} NXTabletPointData, *NXTabletPointDataPtr;

/* TabletProximityData type: defines the tablet data for proximity
 * events included in mouse events created by a tablet driver.
 */

typedef struct _NXTabletProximityData {
    UInt16  vendorID;             /* vendor-defined ID - typically the USB vendor ID */
	UInt16  tabletID;             /* vendor-defined tablet ID - typically the USB product ID */
	UInt16  pointerID;            /* vendor-defined ID of the specific pointing device */
	UInt16  deviceID;             /* system-assigned unique device ID */
	UInt16  systemTabletID;       /* system-assigned unique tablet ID */
	UInt16  vendorPointerType;    /* vendor-defined pointer type */
	UInt32  pointerSerialNumber;  /* vendor-defined serial number */
	UInt64  uniqueID __attribute__ ((packed));             /* vendor-defined unique ID */
	UInt32  capabilityMask;       /* capabilities mask of the device */
	UInt8   pointerType;          /* type of pointing device */
	UInt8   enterProximity;       /* non-zero = entering; zero = leaving */
	SInt16  reserved1;
} NXTabletProximityData, *NXTabletProximityDataPtr;

/* EventData type: defines the data field of an event */

typedef	union {
    struct {    /* For mouse-down and mouse-up events */
        UInt8   subx;       /* sub-pixel position for x */
        UInt8   suby;       /* sub-pixel position for y */
        SInt16  eventNum;   /* unique identifier for this button */
        SInt32  click;      /* click state of this event */
        UInt8   pressure;   /* pressure value: 0=none, 255=full */
        UInt8   buttonNumber;/* button generating other button event (0-31) */
        UInt8   subType;
        UInt8   reserved2;
        SInt32  reserved3;
        union {
            NXTabletPointData      point;     /* tablet point data */
            NXTabletProximityData  proximity; /* tablet proximity data */
        } tablet;
    } mouse;
    struct {
        SInt32  dx;
        SInt32  dy;
        UInt8   subx;
        UInt8   suby;
        UInt8   subType;
        UInt8   reserved1;
        SInt32  reserved2;
        union {
            NXTabletPointData      point;     /* tablet point data */
            NXTabletProximityData  proximity; /* tablet proximity data */
        } tablet;
    } mouseMove;
    struct {    /* For key-down and key-up events */
        UInt16  origCharSet;    /* unmodified character set code */
        SInt16  repeat;         /* for key-down: nonzero if really a repeat */
        UInt16  charSet;        /* character set code */
        UInt16  charCode;       /* character code in that set */
        UInt16  keyCode;        /* device-dependent key number */
        UInt16  origCharCode;   /* unmodified character code */
        SInt32  reserved1;
        UInt32  keyboardType;
        SInt32  reserved2;
        SInt32  reserved3;
        SInt32  reserved4;
        SInt32  reserved5[4];
    } key;
    struct {    /* For mouse-entered and mouse-exited events */
        SInt16  reserved;
        SInt16  eventNum;       /* unique identifier from mouse down event */
        SInt32  trackingNum;    /* unique identifier from settrackingrect */
        SInt32  userData;       /* uninterpreted integer from settrackingrect */
        SInt32  reserved1;
        SInt32  reserved2;
        SInt32  reserved3;
        SInt32  reserved4;
        SInt32  reserved5;
        SInt32  reserved6[4];
    } tracking;
    struct {
        SInt16  deltaAxis1;
        SInt16  deltaAxis2;
        SInt16  deltaAxis3;
        SInt16  reserved1;
        SInt32  fixedDeltaAxis1;
        SInt32  fixedDeltaAxis2;
        SInt32  fixedDeltaAxis3;
        SInt32  pointDeltaAxis1;
        SInt32  pointDeltaAxis2;
        SInt32  pointDeltaAxis3;
        SInt32  reserved8[4];
    } scrollWheel, zoom;
    struct {    /* For window-changed, sys-defined, and app-defined events */
        SInt16  reserved;
        SInt16  subType;    /* event subtype for compound events */
        union {
            float   F[11];  /* for use in compound events */
            SInt32  L[11];  /* for use in compound events */
            SInt16  S[22];  /* for use in compound events */
            char    C[44];  /* for use in compound events */
        } misc;
    } compound;
    struct {
        SInt32  x;  /* absolute x coordinate in tablet space at full tablet resolution */
        SInt32  y;  /* absolute y coordinate in tablet space at full tablet resolution */
        SInt32  z;  /* absolute z coordinate in tablet space at full tablet resolution */
        UInt16  buttons;   /* one bit per button - bit 0 is first button - 1 = closed  */
        UInt16  pressure;  /* scaled pressure value; MAX=(2^16)-1, MIN=0 */
        struct {           /* tilt range is -((2^15)-1) to (2^15)-1 (-32767 to 32767)  */
            SInt16 x;      /* scaled tilt x value */
            SInt16 y;      /* scaled tilt y value */
        } tilt;
        UInt16  rotation;  /* Fixed-point representation of device rotation in a 10.6 format */
        SInt16  tangentialPressure; /* tangential pressure on the device; same range as tilt */
        UInt16  deviceID;  /* system-assigned unique device ID */
        SInt16  vendor1;   /* vendor-defined signed 16-bit integer */
        SInt16  vendor2;   /* vendor-defined signed 16-bit integer */
        SInt16  vendor3;   /* vendor-defined signed 16-bit integer */
        SInt32  reserved[4];
    } tablet;
    struct {
        UInt16  vendorID;  /* vendor-defined ID - typically the USB vendor ID */
        UInt16  tabletID;  /* vendor-defined tablet ID - typically the USB product ID */
        UInt16  pointerID; /* vendor-defined ID of the specific pointing device */
        UInt16  deviceID;             /* system-assigned unique device ID */
        UInt16  systemTabletID;       /* system-assigned unique tablet ID */
        UInt16  vendorPointerType;    /* vendor-defined pointer type */
        UInt32  pointerSerialNumber;  /* vendor-defined serial number */
        UInt64  uniqueID __attribute__ ((packed));             /* vendor-defined unique ID */
        UInt32  capabilityMask;       /* capabilities mask of the device */
        UInt8   pointerType;          /* type of pointing device */
        UInt8   enterProximity;       /* non-zero = entering; zero = leaving */
        SInt16  reserved1;
        SInt32  reserved2[4];
    } proximity;
} NXEventData;

/* The current version number of the NXEventData structure. */

#define kNXEventDataVersion		2

/* Finally! The event record! */
#ifndef __ppc__
typedef struct _NXEvent {
	SInt32              type;		/* An event type from above */
    struct {
        SInt32	x, y;					/* Base coordinates in window, */
    } 					location;	/* from bottom left */
    UInt64              time __attribute__ ((packed));		/* time since launch */
    SInt32              flags;		/* key state flags */
    UInt32              window;		/* window number of assigned window */
    UInt64              service_id __attribute__ ((packed)); /* service id */
    SInt32              ext_pid;    /* external pid */
    NXEventData			data;		/* type-dependent data */
} NXEvent, *NXEventPtr;

#else

typedef struct _NXEvent {
	SInt32              type;		/* An event type from above */
    struct {
        SInt32	x, y;					/* Base coordinates in window, */
    } 					location;	/* from bottom left */
    UInt64              time __attribute__ ((packed));		/* time since launch */
    SInt32              flags;		/* key state flags */
    UInt32              window;		/* window number of assigned window */
    NXEventData			data;		/* type-dependent data */
    UInt64              service_id __attribute__ ((packed)); /* service id */
    SInt32              ext_pid;    /* external pid */
} NXEvent, *NXEventPtr;

#endif

/* The current version number of the NXEvent structure. */

#define kNXEventVersion		2

/* How to pick window(s) for event (for PostEvent) */
#define NX_NOWINDOW		-1
#define NX_BYTYPE		0
#define NX_BROADCAST		1
#define NX_TOPWINDOW		2
#define NX_FIRSTWINDOW		3
#define NX_MOUSEWINDOW		4
#define NX_NEXTWINDOW		5
#define NX_LASTLEFT		6
#define NX_LASTRIGHT		7
#define NX_LASTKEY		8
#define NX_EXPLICIT		9
#define NX_TRANSMIT		10
#define NX_BYPSCONTEXT		11

#endif /* EVENT_H */	/* End of defs common with dpsclient/event.h */

/* Mask of events that cause the screen to wake up */
#define NX_WAKEMASK     (   NX_KEYDOWNMASK | NX_FLAGSCHANGEDMASK | \
                            NX_LMOUSEDOWNMASK | NX_LMOUSEUPMASK | \
                            NX_RMOUSEDOWNMASK | NX_RMOUSEUPMASK | \
                            NX_OMOUSEDOWNMASK | NX_OMOUSEUPMASK   \
                        )

/* Mask of events that cause screen to undim */
#define NX_UNDIMMASK    (   NX_WAKEMASK | NX_KEYUPMASK | NX_SCROLLWHEELMOVEDMASK | \
                            NX_LMOUSEDRAGGEDMASK | NX_RMOUSEDRAGGEDMASK | NX_OMOUSEDRAGGEDMASK | \
                            NX_MOUSEMOVEDMASK | NX_MOUSEENTEREDMASK | NX_MOUSEEXITEDMASK | \
                            NX_TABLETPOINTERMASK | NX_TABLETPROXIMITYMASK \
                        )


#define  NX_EVENT_EXTENSION_LOCATION_INVALID            0x1
#define  NX_EVENT_EXTENSION_LOCATION_TYPE_FLOAT         0x2
#define  NX_EVENT_EXTENSION_LOCATION_DEVICE_SCALED      0x4
#define  NX_EVENT_EXTENSION_MOUSE_DELTA_TYPE_FLOAT      0x8
#define  NX_EVENT_EXTENSION_AUDIT_TOKEN                 0x10

typedef struct _NXEventExtension {
    UInt32              flags;
    audit_token_t       audit;
} NXEventExtension;

typedef struct _NXEventExt {
    NXEvent             payload;
    NXEventExtension    extension;
} NXEventExt;

#endif /* !_DEV_EVENT_H */

