Name

    MESA_shader_debug

Name Strings

    GL_MESA_shader_debug

Contact

    Brian Paul (brian.paul 'at' tungstengraphics.com)
    Michal Krol (mjkrol 'at' gmail.com)

Status

    Obsolete.

Version

    Last Modified Date: July 30, 2006
    Author Revision: 0.2

Number

    TBD

Dependencies

    OpenGL 1.0 is required.

    The ARB_shader_objects extension is required.

    The ARB_shading_language_100 extension is required.

    The extension is written against the OpenGL 1.5 specification.

    The extension is written against the OpenGL Shading Language 1.10
    Specification.

Overview

    This extension introduces a debug object that can be attached to
    a program object to enable debugging. Vertex and/or fragment shader,
    during execution, issue diagnostic function calls that are logged
    to the debug object's log. A separate debug log for each shader type
    is maintained. A debug object can be attached, detached and queried
    at any time outside the Begin/End pair. Multiple debug objects can
    be attached to a single program object.

IP Status

    None

Issues

    None

New Procedures and Functions

    handleARB CreateDebugObjectMESA(void)
    void ClearDebugLogMESA(handleARB obj, enum logType, enum shaderType)
    void GetDebugLogMESA(handleARB obj, enum logType, enum shaderType,
                         sizei maxLength, sizei *length,
                         charARB *debugLog)
    sizei GetDebugLogLengthMESA(handleARB obj, enum logType,
                                enum shaderType)

New Types

    None

New Tokens

    Returned by the <params> parameter of GetObjectParameter{fi}vARB:

        DEBUG_OBJECT_MESA                               0x8759

    Accepted by the <logType> argument of ClearDebugLogMESA,
    GetDebugLogLengthMESA and GetDebugLogMESA:

        DEBUG_PRINT_MESA                                0x875A
        DEBUG_ASSERT_MESA                               0x875B

Additions to Chapter 2 of the OpenGL 1.5 Specification
(OpenGL Operation)

    None

Additions to Chapter 3 of the OpenGL 1.5 Specification (Rasterization)

    None

Additions to Chapter 4 of the OpenGL 1.5 Specification (Per-Fragment
Operations and the Frame Buffer)

    None

Additions to Chapter 5 of the OpenGL 1.5 Specification
(Special Functions)

    None

Additions to Chapter 6 of the OpenGL 1.5 Specification (State and State
Requests)

    None

Additions to Appendix A of the OpenGL 1.5 Specification (Invariance)

    None

Additions to Chapter 1 of the OpenGL Shading Language 1.10 Specification
(Introduction)

    None

Additions to Chapter 2 of the OpenGL Shading Language 1.10 Specification
(Overview of OpenGL Shading)

    None

Additions to Chapter 3 of the OpenGL Shading Language 1.10 Specification
(Basics)

    None

Additions to Chapter 4 of the OpenGL Shading Language 1.10 Specification
(Variables and Types)

    None

Additions to Chapter 5 of the OpenGL Shading Language 1.10 Specification
(Operators and Expressions)

    None

Additions to Chapter 6 of the OpenGL Shading Language 1.10 Specification
(Statements and Structure)

    None

Additions to Chapter 7 of the OpenGL Shading Language 1.10 Specification
(Built-in Variables)

    None

Additions to Chapter 8 of the OpenGL Shading Language 1.10 Specification
(Built-in Functions)

    Add a new section 8.10 "Debug Functions":

    Debug functions are available to both fragment and vertex shaders.
    They are used to track the execution of a shader by logging
    passed-in arguments to the debug object's log. Those values can be
    retrieved by the application for inspection after shader execution
    is complete.

    The text, if any, produced by any of these functions is appended
    to each debug object that is attached to the program object.
    There are different debug log types

    Add a new section 8.10.1 "Print Function":

    The following printMESA prototypes are available.

        void printMESA(const float value)
        void printMESA(const int value)
        void printMESA(const bool value)
        void printMESA(const vec2 value)
        void printMESA(const vec3 value)
        void printMESA(const vec4 value)
        void printMESA(const ivec2 value)
        void printMESA(const ivec3 value)
        void printMESA(const ivec4 value)
        void printMESA(const bvec2 value)
        void printMESA(const bvec3 value)
        void printMESA(const bvec4 value)
        void printMESA(const mat2 value)
        void printMESA(const mat3 value)
        void printMESA(const mat4 value)
        void printMESA(const sampler1D value)
        void printMESA(const sampler2D value)
        void printMESA(const sampler3D value)
        void printMESA(const samplerCube value)
        void printMESA(const sampler1DShadow value)
        void printMESA(const sampler2DShadow value)

    The printMESA function writes the argument <value> to the "debug
    print log" (XXX DEBUG_PRINT_MESA?). Each component is written in
    text format (XXX format!) and is delimited by a white space (XXX 1
    or more?).

    Add a new section 8.10.2 "Assert Function":

    The following assertMESA prototypes are available.

        void assertMESA(const bool condition)
        void assertMESA(const bool condition, const int cookie)
        void assertMESA(const bool condition, const int cookie,
                        const int file, const int line)

    The assertMESA function checks if the argument <condition> is
    true or false. If it is true, nothing happens. If it is false,
    a diagnostic message is written to the "debug assert log".
    The message contains the argument <file>, <line>, <cookie> and
    implementation dependent double-quoted string, each of this
    delimited by a white space. If the argument <cookie> is not present,
    it is meant as if it was of value 0. If the arguments <file> and
    <line> are not present, they are meant as if they were of values
    __FILE__ and __LINE__, respectively. The following three calls
    produce the same output, assuming they were issued from the same
    file and line.

        assertMESA (false);
        assertMESA (false, 0);
        assertMESA (false, 0, __FILE__, __LINE__);

    The diagnostic message examples follow.

        1 89 0 ""
        1 45 333 "all (lessThanEqual (fragColor, vec4 (1.0)))"
        1 66 1 "assertion failed in file 1, line 66, cookie 1"

Additions to Chapter 9 of the OpenGL Shading Language 1.10 Specification
(Shading Language Grammar)

    None

Additions to Chapter 10 of the OpenGL Shading Language 1.10
Specification (Issues)

    None

Additions to the AGL/EGL/GLX/WGL Specifications

    None

GLX Protocol

    None

Errors

    TBD

New State

    TBD

New Implementation Dependent State

    TBD

Sample Code

    TBD

Revision History

    29 May 2006
        Initial draft. (Michal Krol)
    30 July 2006
        Add Overview, New Procedures and Functions, New Tokens sections.
        Add sections 8.10.1, 8.10.2 to GLSL spec.
