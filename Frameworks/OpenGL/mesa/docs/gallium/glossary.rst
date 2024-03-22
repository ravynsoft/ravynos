Glossary
========

.. glossary::
   :sorted:

   MSAA
      Multi-Sampled Anti-Aliasing. A basic anti-aliasing technique that takes
      multiple samples of the depth buffer, and uses this information to
      smooth the edges of polygons.

   TCL
      Transform, Clipping, & Lighting. The three stages of preparation in a
      rasterizing pipeline prior to the actual rasterization of vertices into
      fragments.

   NPOT
      Non-power-of-two. Usually applied to textures which have at least one
      dimension which is not a power of two.

   LOD
      Level of Detail. Also spelled "LoD". The value that determines when the
      switches between mipmaps occur during texture sampling.

   layer
      This term is used as the name of the "3rd coordinate" of a resource.
      3D textures have zslices, cube maps have faces, 1D and 2D array textures
      have array members (other resources do not have multiple layers).
      Since the functions only take one parameter no matter what type of
      resource is used, use the term "layer" instead of a resource type
      specific one.

   GLSL
      GL Shading Language. The official, common high-level shader language used
      in GL 2.0 and above.
