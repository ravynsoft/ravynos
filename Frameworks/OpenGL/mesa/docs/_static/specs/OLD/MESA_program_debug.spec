Name

    MESA_program_debug

Name Strings

    GL_MESA_program_debug

Contact

    Brian Paul (brian.paul 'at' tungstengraphics.com)

Status

    Obsolete.

Version

    Last Modified Date: July 20, 2003
    Author Revision: 1.0

Number

    TBD

Dependencies

    OpenGL 1.4 is required
    The extension is written against the OpenGL 1.4 specification.
    ARB_vertex_program or ARB_fragment_program or NV_vertex_program
    or NV_fragment_program is required.

Overview

    The extension provides facilities for implementing debuggers for
    vertex and fragment programs.

    The concept is that vertex and fragment program debuggers will be
    implemented outside of the GL as a utility package.  This extension
    only provides the minimal hooks required to implement a debugger.

    There are facilities to do the following:
    1. Have the GL call a user-specified function prior to executing
       each vertex or fragment instruction.
    2. Query the current program string's execution position.
    3. Query the current values of intermediate program values.

    The main feature is the ProgramCallbackMESA function.  It allows the
    user to register a callback function with the GL.  The callback will
    be called prior to executing each vertex or fragment program instruction.

    From within the callback, the user may issue Get* commands to
    query current GL state.  The GetProgramRegisterfvMESA function allows
    current program values to be queried (such as temporaries, input
    attributes, and result registers).

    There are flags for enabling/disabling the program callbacks.

    The current execution position (as an offset from the start of the
    program string) can be queried with
    GetIntegerv(GL_FRAGMENT_PROGRAM_POSITION_MESA, &pos) or
    GetIntegerv(GL_VERTEX_PROGRAM_POSITION_MESA, &pos).


IP Status

    None

Issues

    1. Is this the right model for a debugger?

       It seems prudent to minimize the scope of this extension and leave
       it up to the developer (or developer community) to write debuggers
       that layer on top of this extension.

       If the debugger were fully implemented within the GL it's not
       clear how terminal and GUI-based interfaces would work, for
       example.

    2. There aren't any other extensions that register callbacks with
       the GL.  Isn't there another solution?

       If we want to be able to single-step through vertex/fragment
       programs I don't see another way to do it.

    3. How do we prevent the user from doing something crazy in the
       callback function, like trying to call glBegin (leading to
       recursion)?

       The rule is that the callback function can only issue glGet*()
       functions and no other GL commands.  It could be difficult to
       enforce this, however.  Therefore, calling any non-get GL
       command from within the callback will result in undefined
       results.    

    4. Is this extension amenable to hardware implementation?

       Hopefully, but if not, the GL implementation will have to fall
       back to a software path when debugging.  This may be acceptable
       for debugging.

    5. What's the <data> parameter to ProgramCallbackMESA for?

       It's a common programming practice to associate a user-supplied
       value with callback functions.

    6. Debuggers often allow one to modify intermediate program values,
       then continue.  Does this extension support that?

       No.


New Procedures and Functions (and datatypes)

    typedef void (*programcallbackMESA)(enum target, void *data)

    void ProgramCallbackMESA(enum target, programcallbackMESA callback,
                             void *data)

    void GetProgramRegisterfvMESA(enum target, sizei len,
                                  const ubyte *registerName, float *v)

New Tokens

    Accepted by the <cap> parameter of Enable, Disable, IsEnabled,
    GetBooleanv, GetDoublev, GetFloatv and GetIntegerv:

        FRAGMENT_PROGRAM_CALLBACK_MESA      0x8bb1
        VERTEX_PROGRAM_CALLBACK_MESA        0x8bb4

    Accepted by the <pname> parameter GetBooleanv, GetDoublev,
    GetFloatv and GetIntegerv:

        FRAGMENT_PROGRAM_POSITION_MESA      0x8bb0
        VERTEX_PROGRAM_POSITION_MESA        0x8bb5

    Accepted by the <pname> parameter of GetPointerv:

        FRAGMENT_PROGRAM_CALLBACK_FUNC_MESA 0x8bb2
        FRAGMENT_PROGRAM_CALLBACK_DATA_MESA 0x8bb3
        VERTEX_PROGRAM_CALLBACK_FUNC_MESA   0x8bb6
        VERTEX_PROGRAM_CALLBACK_DATA_MESA   0x8bb7

Additions to Chapter 2 of the OpenGL 1.4 Specification (OpenGL Operation)

    None.

Additions to Chapter 3 of the OpenGL 1.4 Specification (Rasterization)

    None.

Additions to Chapter 4 of the OpenGL 1.4 Specification (Per-Fragment
Operations and the Frame Buffer)

    None.

Additions to Chapter 5 of the OpenGL 1.4 Specification (Special Functions)

    In section 5.4 "Display Lists", page 202, add the following command
    to the list of those that are not compiled into display lists:

        ProgramCallbackMESA.


    Add a new section 5.7 "Callback Functions"

    The function

        void ProgramCallbackMESA(enum target, programcallbackMESA callback,
                                 void *data)

    registers a user-defined callback function with the GL.  <target>
    may be FRAGMENT_PROGRAM_ARB or VERTEX_PROGRAM_ARB.  The enabled
    callback functions registered with these targets will be called
    prior to executing each instruction in the current fragment or
    vertex program, respectively.  The callbacks are enabled and
    disabled by calling Enable or Disable with <cap>
    FRAGMENT_PROGRAM_ARB or VERTEX_PROGRAM_ARB.

    The callback function's signature must match the typedef

        typedef void (*programcallbackMESA)(enum target, void *data)

    When the callback function is called, <target> will either be
    FRAGMENT_PROGRAM_ARB or VERTEX_PROGRAM_ARB to indicate which
    program is currently executing and <data> will be the value
    specified when ProgramCallbackMESA was called.

    From within the callback function, only the following GL commands
    may be called:

        GetBooleanv
        GetDoublev
        GetFloatv
        GetIntegerv
        GetProgramLocalParameter
        GetProgramEnvParameter
        GetProgramRegisterfvMESA
        GetProgramivARB
        GetProgramStringARB
        GetError

    Calling any other command from within the callback results in
    undefined behaviour.


Additions to Chapter 6 of the OpenGL 1.4 Specification (State and
State Requests)

    Add a new section 6.1.3 "Program Value Queries":

    The command

        void GetProgramRegisterfvMESA(enum target, sizei len,
                                      const ubyte *registerName,
                                      float *v)
        
    Is used to query the value of program variables and registers
    during program execution.  GetProgramRegisterfvMESA may only be
    called from within a callback function registered with
    ProgramCallbackMESA.

    <registerName> and <len> specify the name a variable, input
    attribute, temporary, or result register in the program string.
    The current value of the named variable is returned as four
    values in <v>.  If <name> doesn't exist in the program string,
    the error INVALID_OPERATION is generated.

Additions to Appendix A of the OpenGL 1.4 Specification (Invariance)

    None.

Additions to the AGL/GLX/WGL Specifications

    None.

GLX Protocol

    XXX TBD

Dependencies on NV_vertex_program and NV_fragment_program

    If NV_vertex_program and/or NV_fragment_program are supported,
    vertex and/or fragment programs defined by those extensions may
    be debugged as well.  Register queries will use the syntax used
    by those extensions (i.e. "v[X]" to query vertex attributes,
    "o[X]" for vertex outputs, etc.)

Errors

    INVALID_OPERATION is generated if ProgramCallbackMESA is called
    between Begin and End.

    INVALID_ENUM is generated by ProgramCallbackMESA if <target> is not
    a supported vertex or fragment program type.

    Note: INVALID_OPERAION IS NOT generated by GetProgramRegisterfvMESA,
    GetBooleanv, GetDoublev, GetFloatv, or GetIntegerv if called between
    Begin and End when a vertex or fragment program is currently executing.

    INVALID_ENUM is generated by ProgramCallbackMESA,
    GetProgramRegisterfvMESA if <target> is not a program target supported
    by ARB_vertex_program, ARB_fragment_program (or NV_vertex_program or
    NV_fragment_program).

    INVALID_VALUE is generated by GetProgramRegisterfvMESA if <registerName>
    does not name a known program register or variable.

    INVALID_OPERATION is generated by GetProgramRegisterfvMESA when a
    register query is attempted for a program target that's not currently
    being executed.


New State

    XXX finish

(table 6.N, p. ###)
                                                            Initial
    Get Value                            Type Get Command   Value    Description  Sec.  Attribute
    ---------                            ---- -----------   -----    -----------  ----  ---------
    FRAGMENT_PROGRAM_CALLBACK_MESA        B   IsEnabled     FALSE    XXX          XXX   enable
    VERTEX_PROGRAM_CALLBACK_MESA          B   IsEnabled     FALSE    XXX          XXX   enable
    FRAGMENT_PROGRAM_POSITION_MESA        Z+  GetIntegerv   -1       XXX          XXX   -
    VERTEX_PROGRAM_POSITION_MESA          Z+  GetIntegerv   -1       XXX          XXX   -
    FRAGMENT_PROGRAM_CALLBACK_FUNC_MESA   P   GetPointerv   NULL     XXX          XXX   -
    VERTEX_PROGRAM_CALLBACK_FUNC_MESA     P   GetPointerv   NULL     XXX          XXX   -
    FRAGMENT_PROGRAM_CALLBACK_DATA_MESA   P   GetPointerv   NULL     XXX          XXX   -
    VERTEX_PROGRAM_CALLBACK_DATA_MESA     P   GetPointerv   NULL     XXX          XXX   -

    XXX more?

New Implementation Dependent State

    None.

Revision History

    8 July 2003
        Initial draft. (Brian Paul)
    11 July 2003
        Second draft. (Brian Paul)
    20 July 2003
        Third draft.  Lots of fundamental changes. (Brian Paul)
    23 July 2003
        Added chapter 5 and 6 spec language. (Brian Paul)

Example Usage

   The following is a very simple example of how this extension may
   be used to print the values of R0, R1, R2 and R3 while executing
   vertex programs.


    /* This is called by the GL when the vertex program is executing.
     * We can only make glGet* calls from within this function!
     */
    void DebugCallback(GLenum target, GLvoid *data)
    {
       GLint pos;
       GLuint i;

       /* Get PC and current instruction string */
       glGetIntegerv(GL_VERTEX_PROGRAM_POSITION_ARB, &pos);

       printf("Current position: %d\n", pos);

       printf("Current temporary registers:\n");
       for (i = 0; i < 4; i++) {
	  GLfloat v[4];
	  char s[10];
	  sprintf(s, "R%d", i);
	  glGetProgramRegisterfvMESA(GL_VERTEX_PROGRAM_ARB, strlen(s), s, v);
	  printf("R%d = %g, %g, %g, %g\n", i, v[0], v[1], v[2], v[3]);
       }
    }


    /*
     * elsewhere...
     */

    /* Register our debugger callback function */
    glProgramCallbackMESA(GL_VERTEX_PROGRAM_ARB, DebugCallback, NULL);
    glEnable(GL_VERTEX_PROGRAM_CALLBACK_MESA);

    /* define/bind a vertex program */

    glEnable(GL_VERTEX_PROGRAM);

    /* render something */
    glBegin(GL_POINTS);
    glVertex2f(0, 0);
    glEnd();

