CSO
===

CSO, Constant State Objects, are a core part of Gallium's API.

CSO work on the principle of reusable state; they are created by filling
out a state object with the desired properties, then passing that object
to a context. The context returns an opaque context-specific handle which
can be bound at any time for the desired effect.

.. toctree::
   :glob:

   cso/*
