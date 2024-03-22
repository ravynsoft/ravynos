Name

    MESA_multithread_makecurrent

Name Strings

    GLX_MESA_multithread_makecurrent

Contact

    Eric Anholt (eric@anholt.net)

Status

    Obsolete.

Version

    Last Modified Date:  21 February 2011

Number

    TBD

Dependencies

    OpenGL 1.0 or later is required.
    GLX 1.3 or later is required.

Overview

    The GLX context setup encourages multithreaded applications to
    create a context per thread which each operate on their own
    objects in parallel, and leaves synchronization for write access
    to shared objects up to the application.

    For some applications, maintaining per-thread contexts and
    ensuring that the glFlush happens in one thread before another
    thread starts working on that object is difficult.  For them,
    using the same context across multiple threads and protecting its
    usage with a mutex is both higher performance and easier to
    implement.  This extension gives those applications that option by
    relaxing the context binding requirements.

    This new behavior matches the requirements of AGL, while providing
    a feature not specified in WGL.

IP Status

    Open-source; freely implementable.

Issues

    None.

New Procedures and Functions

    None.

New Tokens

    None.

Changes to Chapter 2 of the GLX 1.3 Specification (Functions and Errors)

    Replace the following sentence from section 2.2 Rendering Contexts:
	In addition, a rendering context can be current for only one
	thread at a time.
    with:
	In addition, an indirect rendering context can be current for
	only one thread at a time.  A direct rendering context may be
	current to multiple threads, with synchronization of access to
	the context through the GL managed by the application through
	mutexes.

Changes to Chapter 3 of the GLX 1.3 Specification (Functions and Errors)

    Replace the following sentence from section 3.3.7 Rendering Contexts:
	If ctx is current to some other thread, then
	glXMakeContextCurrent will generate a BadAccess error.
    with:
	If ctx is an indirect context current to some other thread,
	then glXMakeContextCurrent will generate a BadAccess error.

    Replace the following sentence from section 3.5 Rendering Contexts:
	If ctx is current to some other thread, then
	glXMakeCurrent will generate a BadAccess error.
    with:
	If ctx is an indirect context current to some other thread,
	then glXMakeCurrent will generate a BadAccess error.

GLX Protocol

    None.  The GLX extension only extends to direct rendering contexts.

Errors

    None.

New State

    None.

Issues

    (1) What happens if the app binds a context/drawable in multiple
	threads, then binds a different context/thread in one of them?

    As with binding a new context from the current thread, the old
    context's refcount is reduced and the new context's refcount is
    increased.

    (2) What happens if the app binds a context/drawable in multiple
	threads, then binds None/None in one of them?

    The GLX context is unreferenced from that thread, and the other
    threads retain their GLX context binding.

    (3) What happens if the app binds a context/drawable in 7 threads,
	then destroys the context in one of them?

    As with GLX context destruction previously, the XID is destroyed
    but the context remains usable by threads that have the context
    current.

    (4) What happens if the app binds a new drawable/readable with
        glXMakeCurrent() when it is already bound to another thread?

    The context becomes bound to the new drawable/readable, and
    further rendering in either thread will use the new
    drawable/readable.

    (5) What requirements should be placed on the user managing contexts
        from multiple threads?

    The intention is to allow multithreaded access to the GL at the
    minimal performance cost, so requiring that the GL do general
    synchronization (beyond that already required by context sharing)
    is not an option, and synchronizing of GL's access to the GL
    context between multiple threads is left to the application to do
    across GL calls.  However, it would be unfortunate for a library
    doing multithread_makecurrent to require that other libraries
    share in synchronization for binding of their own contexts, so the
    refcounting of the contexts is required to be threadsafe.

    (6) Does this apply to indirect contexts?

    This was ignored in the initial revision of the spec.  Behavior
    for indirect contexts is left as-is.

Revision History

    20 November 2009 Eric Anholt - initial specification
    22 November 2009 Eric Anholt - added issues from Ian Romanick.
    3 February 2011 Eric Anholt - updated with resolution to issues 1-3
    3 February 2011 Eric Anholt - added issue 4, 5
    21 February 2011 Eric Anholt - Include glXMakeCurrent() sentence
    along with glXMakeContextCurrent() for removal.
