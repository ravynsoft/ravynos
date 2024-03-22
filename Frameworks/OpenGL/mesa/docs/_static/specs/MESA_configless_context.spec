Name

    MESA_configless_context

Name Strings

    EGL_MESA_configless_context

Contact

    Neil Roberts <neil.s.roberts@intel.com>

Status

    Superseded by the functionally identical EGL_KHR_no_config_context
    extension.

Version

    Version 2, September 9, 2016

Number

    EGL Extension #not assigned

Dependencies

    Requires EGL 1.4 or later.  This extension is written against the
    wording of the EGL 1.4 specification.

Overview

    This extension provides a means to use a single context to render to
    multiple surfaces which have different EGLConfigs. Without this extension
    the EGLConfig for every surface used by the context must be compatible
    with the one used by the context. The only way to render to surfaces with
    different formats would be to create multiple contexts but this is
    inefficient with modern GPUs where this restriction is unnecessary.

IP Status

    Open-source; freely implementable.

New Procedures and Functions

    None.

New Tokens

    Accepted as <config> in eglCreateContext

        EGL_NO_CONFIG_MESA                  ((EGLConfig)0)

Additions to the EGL Specification section "2.2 Rendering Contexts and Drawing
Surfaces"

    Add the following to the 3rd paragraph:

   "EGLContexts can also optionally be created with respect to an EGLConfig
    depending on the parameters used at creation time. If a config is provided
    then additional restrictions apply on what surfaces can be used with the
    context."

    Replace the last sentence of the 6th paragraph with:

   "In order for a context to be compatible with a surface they both must have
    been created with respect to the same EGLDisplay. If the context was
    created without respect to an EGLConfig then there are no further
    constraints. Otherwise they are only compatible if:"

    Remove the last bullet point in the list of constraints.

Additions to the EGL Specification section "3.7.1 Creating Rendering Contexts"

    Replace the paragraph starting "If config is not a valid EGLConfig..."
    with

   "The config argument can either be a valid EGLConfig or EGL_NO_CONFIG_MESA.
    If it is neither of these then an EGL_BAD_CONFIG error is generated. If a
    valid config is passed then the error will also be generated if the config
    does not support the requested client API (this includes requesting
    creation of an OpenGL ES 1.x context when the EGL_RENDERABLE_TYPE
    attribute of config does not contain EGL_OPENGL_ES_BIT, or creation of an
    OpenGL ES 2.x context when the attribute does not contain
    EGL_OPENGL_ES2_BIT).

    Passing EGL_NO_CONFIG_MESA will create a configless context. When a
    configless context is used with the OpenGL API it can be assumed that the
    initial values of the context's state will be decided when the context is
    first made current. In particular this means that the decision of whether
    to use GL_BACK or GL_FRONT for the initial value of the first output in
    glDrawBuffers will be decided based on the config of the draw surface when
    it is first bound."

Additions to the EGL Specification section "3.7.3 Binding Contexts and
Drawables"

    Replace the first bullet point with the following:

   "* If draw or read are not compatible with ctx as described in section 2.2,
      then an EGL_BAD_MATCH error is generated."

    Add a second bullet point after that:

   "* If draw and read are not compatible with each other as described in
      section 2.2, then an EGL_BAD_MATCH error is generated."

Issues

    1.  What happens when an OpenGL context with a double-buffered surface and
        draw buffer set to GL_BACK is made current with a single-buffered
        surface?

        NOT RESOLVED: There are a few options here.  An implementation can
        raise an error, change the drawbuffer state to GL_FRONT or just do
        nothing, expecting the application to set GL_FRONT drawbuffer before
        drawing.  However, this extension deliberately does not specify any
        required behavior in this corner case and applications should avoid
        mixing single- and double-buffered surfaces with configless contexts.

        Future extensions may specify required behavior in this case.

Revision History

    Version 2, September 9, 2016
        Defer to EGL_KHR_no_config_context (Adam Jackson)

    Version 1, February 28, 2014
        Initial draft (Neil Roberts)
