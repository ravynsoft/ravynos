/* For GLSL in OpenGL ES, an undefined macro appearing in an #if or #elif
 * expression, (other than as an argument to defined) is an error.
 *
 * Except in the case of a short-circuiting && or || operator, where the
 * specification explicitly mandates that there be no error.
 */
#version 300 es

/* These yield errors */
#if NOT_DEFINED
#endif

#if 0
#elif ALSO_NOT_DEFINED
#endif

/* But these yield no errors */
#if 1 || STILL_NOT_DEFINED
Success
#endif

#if 0
#elif 0 && WILL_ANYONE_DEFINE_ANYTHING
#else
More success
#endif

