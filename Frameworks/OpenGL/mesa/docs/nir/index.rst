NIR Intermediate Representation (NIR)
=====================================

The NIR Intermediate Representation (NIR) is the optimizing compiler stack that
sits at the core of most Mesa drivers' shader compilers.  It consists of a set
of enums and data structures that make up the IR as well as a suite of helper
functions, optimization passes, and lowering passes for building a compiler
stack.

.. toctree::
   :maxdepth: 2

   alu
   tex
