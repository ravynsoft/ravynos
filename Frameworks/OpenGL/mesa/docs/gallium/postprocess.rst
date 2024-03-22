Gallium Post-processing
=======================

The Gallium drivers support user-defined image post-processing. At the
end of drawing a frame a post-processing filter can be applied to the
rendered image. Example filters include morphological antialiasing and
cell shading.

The filters can be toggled per-app via driconf, or per-session via the
corresponding environment variables.

Multiple filters can be used together.

PP environment variables
------------------------

-  PP_DEBUG - If defined debug information will be printed to stderr.

Current filters
---------------

-  pp_nored, pp_nogreen, pp_noblue - set to 1 to remove the
   corresponding color channel. These are basic filters for easy testing
   of the PP queue.
-  pp_jimenezmlaa, pp_jimenezmlaa_color - `Jimenez's
   MLAA <https://www.iryoku.com/mlaa/>`__ is a morphological
   antialiasing filter. The two versions use depth and color data,
   respectively. Which works better depends on the app - depth will not
   blur text, but it will miss transparent textures for example. Set to
   a number from 2 to 32, roughly corresponding to quality. Numbers
   higher than 8 see minimizing gains.
-  pp_celshade - set to 1 to enable cell shading (a more complex color
   filter).
