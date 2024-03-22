Name

     MESA_trace

Name Strings

     GL_MESA_trace

Contact
    
    Bernd Kreimeier, Loki Entertainment, bk 'at' lokigames.com
    Brian Paul, VA Linux Systems, Inc., brianp 'at' valinux.com

Status

    Obsolete.

Version


Number

    none yet

Dependencies

    OpenGL 1.2 is required.
    The extension is written against the OpenGL 1.2 Specification

Overview

    Provides the application with means to enable and disable logging
    of GL calls including parameters as readable text. The verbosity
    of the generated log can be controlled. The resulting logs are
    valid (but possibly incomplete) C code and can be compiled and 
    linked for standalone test programs. The set of calls and the 
    amount of static data that is logged can be controlled at runtime. 
    The application can add comments and enable or disable tracing of GL 
    operations at any time. The data flow from the application to GL
    and back is unaffected except for timing.

    Application-side implementation of these features raises namespace
    and linkage issues. In the driver dispatch table a simple
    "chain of responsibility" pattern (aka "composable piepline")
    can be added.

IP Status

    The extension spec is in the public domain.  The current implementation
    in Mesa is covered by Mesa's XFree86-style copyright by the authors above.
    This extension is partially inspired by the Quake2 QGL wrapper.

Issues

 
    (1) Is this Extension obsolete because it can
    be implemented as a wrapper DLL?

      RESOLVED: No. While certain operating systems (Win32) provide linkers 
      that facilitate this kind of solution, other operating systems
      (Linux) do not support hierarchical linking, so a wrapper solution
      would result in symbol collisions.
      Further, IHV's might have builtin support for tracing GL execution 
      that enjoys privileged access, or that they do not wish to separate
      the tracing code from their driver code base.

    (2) Should the Trace API explicitly support the notion of "frames?
    This would require hooking into glXSwapBuffers calls as well.

      RESOLVED: No. The application can use NewTraceMESA/EndTraceMESA
      and TraceComment along with external parsing tools to split the 
      trace into frames, in whatever way considered adequate.

    (2a) Should GLX calls be traced?

      PBuffers and other render-to-texture solutions demonstrate that
      context level commands beyond SwapBuffers might have to be
      traced. The GL DLL exports the entry points, so this would not
      be out of the question. 

    (3) Should the specification mandate the actual output format?

      RESOLVED: No. It is sufficient to guarantee that all data and commands 
      will be traced as requested by Enable/DisableTraceMESA, in the order
      encountered. Whether the resulting trace is available as a readable 
      text file, binary metafile, compilable source code, much less which 
      indentation and formatting has been used, is up to the implementation. 
      For the same reason this specification does not enforce or prohibit
      additional information added to the trace (statistics, profiling/timing, 
      warnings on possible error conditions).

    (4) Should the comment strings associated with names and pointer (ranges) 
    be considered persistent state?

      RESOLVED: No. The implementation is not forced to use this information 
      on subsequent occurrences of name/pointer, and is free to consider it
      transient state.
 
    (5) Should comment commands be prohibited between Begin/End?

      RESOLVED: Yes, with the exception of TraceCommentMESA. TraceCommentMESA 
      is transient, the other commands might cause storage of persistent
      data in the context. There is no need to have the ability mark names 
      or pointers between Begin and End.


New Procedures and Functions
 
    void NewTraceMESA( bitfield mask, const ubyte * traceName )

    void EndTraceMESA( void )

    void EnableTraceMESA( bitfield mask )

    void DisableTraceMESA( bitfield mask )

    void TraceAssertAttribMESA( bitfield attribMask )

    void TraceCommentMESA( const ubyte* comment )

    void TraceTextureMESA( uint name, const ubyte* comment )

    void TraceListMESA( uint name, const ubyte* comment )

    void TracePointerMESA( void* pointer, const ubyte* comment )

    void TracePointerRangeMESA( const void* first, 
                                const void* last, 
                                const ubyte* comment ) 

New Tokens
 
    Accepted by the <mask> parameter of EnableTrace and DisableTrace:

       TRACE_ALL_BITS_MESA           0xFFFF
       TRACE_OPERATIONS_BIT_MESA     0x0001
       TRACE_PRIMITIVES_BIT_MESA     0x0002
       TRACE_ARRAYS_BIT_MESA         0x0004
       TRACE_TEXTURES_BIT_MESA       0x0008
       TRACE_PIXELS_BIT_MESA         0x0010
       TRACE_ERRORS_BIT_MESA         0x0020

    Accepted by the <pname> parameter of GetIntegerv, GetBooleanv,
    GetFloatv, and GetDoublev:

       TRACE_MASK_MESA               0x8755

    Accepted by the <pname> parameter to GetString:

       TRACE_NAME_MESA               0x8756


Additions to Chapter 2 of the OpenGL 1.2.1 Specification (OpenGL Operation)

    None.

Additions to Chapter 3 of the OpenGL 1.2.1 Specification (OpenGL Operation)

    None.

Additions to Chapter 4 of the OpenGL 1.2.1 Specification (OpenGL Operation)

    None.

Additions to Chapter 5 of the OpenGL 1.2.1 Specification (Special Functions)

    Add a new section:

    5.7 Tracing

    The tracing facility is used to record the execution of a GL program
    to a human-readable log.  The log appears as a sequence of GL commands
    using C syntax.  The primary intention of tracing is to aid in program
    debugging.

    A trace is started with the command

      void NewTraceMESA( bitfield mask, const GLubyte * traceName )

    <mask> may be any value accepted by PushAttrib and specifies a set of
    attribute groups.  The state values included in those attribute groups
    is written to the trace as a sequence of GL commands.

    <traceName> specifies a name or label for the trace.  It is expected
    that <traceName> will be interpreted as a filename in most implementations.

    A trace is ended by calling the command

      void EndTraceMESA( void )

    It is illegal to call NewTraceMESA or EndTraceMESA between Begin and End. 

    The commands

      void EnableTraceMESA( bitfield mask )
      void DisableTraceMESA( bitfield mask )

    enable or disable tracing of different classes of GL commands.
    <mask> may be the union of any of TRACE_OPERATIONS_BIT_MESA,
    TRACE_PRIMITIVES_BIT_MESA, TRACE_ARRAYS_BIT_MESA, TRACE_TEXTURES_BIT_MESA,
    and TRACE_PIXELS_BIT_MESA.  The special token TRACE_ALL_BITS_MESA
    indicates all classes of commands are to be logged.

    TRACE_OPERATIONS_BIT_MESA controls logging of all commands outside of
    Begin/End, including Begin/End.
  
    TRACE_PRIMITIVES_BIT_MESA controls logging of all commands inside of
    Begin/End, including Begin/End.
 
    TRACE_ARRAYS_BIT_MESA controls logging of VertexPointer, NormalPointer,
    ColorPointer, IndexPointer, TexCoordPointer and EdgeFlagPointer commands.

    TRACE_TEXTURES_BIT_MESA controls logging of texture data dereferenced by
    TexImage1D, TexImage2D, TexImage3D, TexSubImage1D, TexSubImage2D, and
    TexSubImage3D commands.

    TRACE_PIXELS_BIT_MESA controls logging of image data dereferenced by
    Bitmap and DrawPixels commands.

    TRACE_ERRORS_BIT_MESA controls logging of all errors. If this bit is 
    set, GetError will be executed wherever applicable, and the result will
    be added to the trace as a comment. The error returns are cached and 
    returned to the application on its GetError calls. If the user does not 
    wish the additional GetError calls to be performed, this bit should not
    be set.
    
    The command

      void TraceCommentMESA( const ubyte* comment )

    immediately adds the <comment> string to the trace output, surrounded
    by C-style comment delimiters.

    The commands

      void TraceTextureMESA( uint name, const ubyte* comment )
      void TraceListMESA( uint name, const ubyte* comment )

    associates <comment> with the texture object or display list specified
    by <name>.  Logged commands which reference the named texture object or
    display list will be annotated with <comment>.  If IsTexture(name) or
    IsList(name) fail (respectively) the command is quietly ignored.

    The commands

      void TracePointerMESA( void* pointer, const ubyte* comment )

      void TracePointerRangeMESA( const void* first, 
                                  const void* last,
                                  const ubyte* comment ) 

    associate <comment> with the address specified by <pointer> or with
    a range of addresses specified by <first> through <last>.
    Any logged commands which reference <pointer> or an address between
    <first> and <last> will be annotated with <comment>.

    The command

      void TraceAssertAttribMESA( bitfield attribMask )

    will add GL state queries and assertion statements to the log to
    confirm that the current state at the time TraceAssertAttrib is
    executed matches the current state when the trace log is executed
    in the future.

    <attribMask> is any value accepted by PushAttrib and specifies
    the groups of state variables which are to be asserted.

    The commands NewTraceMESA, EndTraceMESA, EnableTraceMESA, DisableTraceMESA,
    TraceAssertAttribMESA, TraceCommentMESA, TraceTextureMESA, TraceListMESA, 
    TracePointerMESA and TracePointerRangeMESA are not compiled into display lists.


    Examples:

    The command NewTraceMESA(DEPTH_BUFFER_BIT, "log") will query the state
    variables DEPTH_TEST, DEPTH_FUNC, DEPTH_WRITEMASK, and DEPTH_CLEAR_VALUE
    to get the values <test>, <func>, <mask>, and <clear> respectively.
    Statements equivalent to the following will then be logged:

       glEnable(GL_DEPTH_TEST);   (if <test> is true)
       glDisable(GL_DEPTH_TEST);  (if <test> is false)
       glDepthFunc(<func>); 
       glDepthMask(<mask>);
       glClearDepth(<clear>);
   

    The command TraceAssertAttribMESA(DEPTH_BUFFER_BIT) will query the state
    variables DEPTH_TEST, DEPTH_FUNC, DEPTH_WRITEMASK, and DEPTH_CLEAR_VALUE
    to get the values <test>, <func>, <mask>, and <clear> respectively.
    The resulting trace might then look will like this:

    {
      GLboolean b;
      GLint i;
      GLfloat f;
      b = glIsEnabled(GL_DEPTH_TEST);
      assert(b == <test>);
      glGetIntegerv(GL_DEPTH_FUNC, &i);
      assert(i == <func>);
      glGetIntegerv(GL_DEPTH_MASK, &i);
      assert(i == <mask>);
      glGetFloatv(GL_DEPTH_CLEAR_VALUE, &f);
      assert(f == <clear>);
    }


Additions to Chapter 6 of the OpenGL 1.2.1 Specification 
    (State and State Requests)

    Querying TRACE_MASK_MESA with GetIntegerv, GetFloatv, GetBooleanv or
    GetDoublev returns the current command class trace mask.

    Querying TRACE_NAME_MESA with GetString returns the current trace name.


Additions to Appendix A of the OpenGL 1.2.1 Specification (Invariance)

    The MESA_trace extension can be used in a way that does not affect data 
    flow from application to OpenGL, as well as data flow from OpenGL to 
    application, except for timing, possible print I/O. TRACE_ERRORS_BIT_MESA
    will add additional GetError queries. Setting a trace mask with NewTraceMESA
    as well as use of TraceAssertAttribMESA might cause additional state queries.
    With the possible exception of performance, OpenGL rendering should not be
    affected at all by a properly chosen logging operation.

Additions to the AGL/GLX/WGL Specifications

    None.

GLX Protocol

    None. The logging operation is carried out client-side, by exporting
    entry points to the wrapper functions that execute the logging operation.

Errors

    INVALID_OPERATION is generated if any trace command except TraceCommentMESA
    is called between Begin and End.

New State

    The current trace name and current command class mask are stored
    per-context.

New Implementation Dependent State

    None.

Revision History

  * Revision 0.1 - Initial draft from template (bk000415)
  * Revision 0.2 - Draft (bk000906)
  * Revision 0.3 - Draft (bk000913)
  * Revision 0.4 - Reworked text, fixed typos (bp000914)
  * Revision 0.5 - Assigned final GLenum values (bp001103)
  * Revision 0.6 - TRACE_ERRORS_BIT_MESA (bk000916)
  * Revision 0.7 - Added MESA postfix (bk010126)

