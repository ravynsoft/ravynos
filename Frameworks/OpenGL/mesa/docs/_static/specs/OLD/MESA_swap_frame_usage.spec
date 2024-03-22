Name

    MESA_swap_frame_usage

Name Strings

    GLX_MESA_swap_frame_usage

Contact

    Ian Romanick, IBM, idr at us.ibm.com

Status

    Obsolete.

Version

    Date: 5/1/2003   Revision: 1.1

Number

    ???

Dependencies

    GLX_SGI_swap_control affects the definition of this extension.
    GLX_MESA_swap_control affects the definition of this extension.
    GLX_OML_sync_control affects the definition of this extension.

    Based on WGL_I3D_swap_frame_usage version 1.3.

Overview

    This extension allows an application to determine what portion of the
    swap period has elapsed since the last swap operation completed.  The
    "usage" value is a floating point value on the range [0,max] which is
    calculated as follows:

                              td
                   percent = ----
                              tf

    where td is the time measured from the last completed buffer swap (or
    call to enable the statistic) to when the next buffer swap completes, tf
    is the entire time for a frame which may be multiple screen refreshes
    depending on the swap interval as set by the GLX_SGI_swap_control or
    GLX_OML_sync_control extensions. 

    The value, percent, indicates the amount of time spent between the
    completion of the two swaps.  If the value is in the range [0,1], the
    buffer swap occurred within the time period required to maintain a
    constant frame rate.  If the value is in the range (1,max], a constant
    frame rate was not achieved.  The value indicates the number of frames
    required to draw.

    This definition of "percent" differs slightly from
    WGL_I3D_swap_frame_usage.  In WGL_I3D_swap_frame_usage, the measurement
    is taken from the completion of one swap to the issuance of the next.
    This representation may not be as useful as measuring between
    completions, as a significant amount of time may pass between the
    issuance of a swap and the swap actually occurring.

    There is also a mechanism to determine whether a frame swap was
    missed.

New Procedures and Functions

    int glXGetFrameUsageMESA(Display *dpy,
                             GLXDrawable drawable,
    	                     float *usage)

    int glXBeginFrameTrackingMESA(Display *dpy,
                                  GLXDrawable drawable)

    int glXEndFrameTrackingMESA(Display *dpy,
                                GLXDrawable drawable)

    int glXQueryFrameTrackingMESA(Display *dpy,
                                  GLXDrawable drawable,
				  int64_t *swapCount,
                                  int64_t *missedFrames,
                                  float *lastMissedUsage)

New Tokens

    None

Additions to Chapter 2 of the 1.4 GL Specification (OpenGL Operation)

    None

Additions to Chapter 3 of the 1.4 GL Specification (Rasterization)

    None

Additions to Chapter 4 of the 1.4 GL Specification (Per-Fragment Operations
and the Framebuffer)

    None

Additions to Chapter 5 of the 1.4 GL Specification (Special Functions)

    None

Additions to Chapter 6 of the 1.4 GL Specification (State and State Requests)

    None

Additions to the GLX 1.3 Specification

    The frame usage is measured as the percentage of the swap period elapsed
    between two buffer-swap operations being committed.  In unextended GLX the
    swap period is the vertical refresh time.  If SGI_swap_control or
    MESA_swap_control are supported, the swap period is the vertical refresh
    time multiplied by the swap interval (or one if the swap interval is set
    to zero).
    
    If OML_sync_control is supported, the swap period is the vertical
    refresh time multiplied by the divisor parameter to
    glXSwapBuffersMscOML.  The frame usage in this case is less than 1.0 if
    the swap is committed before target_msc, and is greater than or equal to
    1.0 otherwise.  The actual usage value is based on the divisor and is
    never less than 0.0.

       int glXBeginFrameTrackingMESA(Display *dpy,
                                     GLXDrawable drawable,
				     float *usage)

    glXGetFrameUsageMESA returns a floating-point value in <usage>
    that represents the current swap usage, as defined above.

    Missed frame swaps can be tracked by calling the following function:

       int glXBeginFrameTrackingMESA(Display *dpy,
                                     GLXDrawable drawable)

    glXBeginFrameTrackingMESA resets a "missed frame" count and
    synchronizes with the next frame vertical sync before it returns.
    If a swap is missed based in the rate control specified by the
    <interval> set by glXSwapIntervalSGI or the default swap of once
    per frame, the missed frame count is incremented.

    The current missed frame count and total number of swaps since
    the last call to glXBeginFrameTrackingMESA can be obtained by
    calling the following function:

       int glXQueryFrameTrackingMESA(Display *dpy,
                                     GLXDrawable drawable,
				     int64_t *swapCount,
                                     int64_t *missedFrames,
                                     float *lastMissedUsage)

    The location pointed to by <swapCount> will be updated with the
    number of swaps that have been committed.  This value may not match the
    number of swaps that have been requested since swaps may be
    queued by the implementation.  This function can be called at any
    time and does not synchronize to vertical blank.

    The location pointed to by <missedFrames> will contain the number
    swaps that missed the specified frame.  The frame usage for the
    last missed frame is returned in the location pointed to by
    <lastMissedUsage>.

    Frame tracking is disabled by calling the function

       int glXEndFrameTrackingMESA(Display *dpy,
                                   GLXDrawable drawable)

    This function will not return until all swaps have occurred.  The
    application can call glXQueryFrameTrackingMESA for a final swap and
    missed frame count.

    If these functions are successful, zero is returned.  If the context
    associated with dpy and drawable is not a direct context,
    GLX_BAD_CONTEXT is returned.

Errors

    If the function succeeds, zero is returned.  If the function
    fails, one of the following error codes is returned:

       GLX_BAD_CONTEXT         The current rendering context is not a direct
       			       context.

GLX Protocol

    None.  This extension only extends to direct rendering contexts.

New State

    None

New Implementation Dependent State

    None

Revision History

    1.1,  5/1/03   Added contact information.
    1.0,  3/17/03  Initial version based on WGL_I3D_swap_frame_usage.
