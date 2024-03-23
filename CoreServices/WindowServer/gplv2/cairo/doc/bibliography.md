# Bibliography

Here's an effort to document some of the academic work that was
referenced during the implementation of cairo. It is presented in the
context of operations as they would be performed by either
`cairo_stroke()` or `cairo_fill()`:

Given a Bézier path, approximate it with line segments:

- The deCasteljau algorithm
  "Outillages methodes calcul", P de Casteljau, technical
  report, - Andre Citroen Automobiles SA, Paris, 1959

Note: That technical report might be "hard" to find, but fortunately this
algorithm will be described in any reasonable textbook on computational
geometry. Two that have been recommended by cairo contributors are:

- "Computational Geometry, Algorithms and Applications", M. de
  Berg, M. van Kreveld, M. Overmars, M. Schwarzkopf;
  Springer-Verlag, ISBN: 3-540-65620-0.

- "Computational Geometry in C (Second Edition)", Joseph O'Rourke,
   Cambridge University Press, ISBN 0521640105.

Then, if stroking, construct a polygonal representation of the pen
approximating a circle (if filling skip three steps):

- "Good approximation of circles by curvature-continuous Bezier
  curves", Tor Dokken and Morten Daehlen, Computer Aided
  Geometric Design 8 (1990) 22-41.

Add points to that pen based on the initial/final path faces and take the
convex hull:

- Convex hull algorithm

[Again, see your favorite computational geometry textbook. Should cite the
name of the algorithm cairo uses here, if it has a name.]

Now, "convolve" the "tracing" of the pen with the tracing of the path:

- "A Kinetic Framework for Computational Geometry", Leonidas
  J. Guibas, Lyle Ramshaw, and Jorge Stolfi, Proceedings of the
  24th IEEE Annual Symposium on Foundations of Computer Science
  (FOCS), November 1983, 100-111.

The result of the convolution is a polygon that must be filled. A fill
operations begins here. We use a very conventional Bentley-Ottmann
pass for computing the intersections, informed by some hints on robust
implementation courtesy of John Hobby:

- John D. Hobby, Practical Segment Intersection with Finite
  Precision Output, Computation Geometry Theory and
  Applications, 13(4), 1999.
  <http://cm.bell-labs.com/who/hobby/93_2-27.pdf>

Hobby's primary contribution in that paper is his "tolerance square"
algorithm for robustness against edges being "bent" due to restricting
intersection coordinates to the grid available by finite-precision
arithmetic. This is one algorithm we have not implemented yet.

We use a data-structure called Skiplists in the our implementation of
Bentley-Ottmann:

- W. Pugh, Skip Lists: a Probabilistic Alternative to Balanced Trees,
  Communications of the ACM, vol. 33, no. 6, pp.668-676, 1990.
  <http://citeseer.ist.psu.edu/pugh90skip.html>

The random number generator used in our skip list implementation is a very
small generator by Hars and Petruska.  The generator is based on an
invertable function on Z_{2^32} with full period and is described in

- Hars L. and Petruska G.,
  Pseudorandom Recursions: Small and Fast Pseurodandom
  Number Generators for Embedded Applications,
  Hindawi Publishing Corporation
  EURASIP Journal on Embedded Systems
  Volume 2007, Article ID 98417, 13 pages
  doi:10.1155/2007/98417
  <http://www.hindawi.com/getarticle.aspx?doi=10.1155/2007/98417&e=cta>

From the result of the intersection-finding pass, we are currently computing
a tessellation of trapezoids, (the exact manner is undergoing some work
right now with some important speedup), but we may want to rasterize
directly from those edges at some point.

Given the set of tessellated trapezoids, we currently execute a
straightforward, (and slow), point-sampled rasterization, (and currently
with a near-pessimal regular 15x17 grid).

We've now computed a mask which gets fed along with the source and
destination into cairo's fundamental rendering equation. The most basic form
of this equation is:

    destination = (source IN mask) OP destination

with the restriction that no part of the destination outside the current
clip region is affected. In this equation, IN refers to the Porter-Duff "in"
operation, while OP refers to a any user-selected Porter-Duff operator:

- T. Porter & T. Duff, Compositing Digital Images Computer
  Graphics Volume 18, Number 3 July 1984 pp 253-259
  <http://keithp.com/~keithp/porterduff/p253-porter.pdf>
