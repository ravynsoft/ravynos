# Understanding & Using Turbo Vision's Palette

```
TI813C.txt   Understanding & Using Turbo Vision's Palette.
Category    :General
Platform    :All
Product     :Borland C++  3.1

7/2/98 10:39:22 AM
```

## Description

The palette system in Turbo Vision is designed to make it easy
for either the programmer or the user to customize the colors of
an application.  The system uses an object oriented approach
which relies on an implementation where the colors of a
particular view are dependent on the colors of the owner view.
So, if there is a view `X` that can appear inside either of two
`TWindow` objects `A` and `B`, the colors seen by the user will depend
on whether it is a child of `A` or `B` at that moment.

There are two ways of thinking about colors in Turbo Vision: One
is to consider a palette entry as a particular color.  The other
is to envision it as representing the color currently used for a
particular type of object, like selected text or normal text.
Which of these is used will depend upon the nature of the object
being drawn.  The latter method should be employed when
considering the palettes for pre-defined Turbo Vision objects.

When the programmer is writing a draw function for a view, he/she
will want to be able to select a particular color or style for
drawing.  It may be desirable to have similar components of
unrelated views to be drawn in the same color.  It may also be
desirable to give the user a method of changing at runtime the
colors used for the application. The palette system in Turbo
Vision will allow both of these possibilities.

A further benefit
of the Turbo Vision palettes is that the system automatically
detects the type of display being used (color, black and white,
or monochrome) and sets up the palette accordingly.  If the
programmer chooses to modify the system palette, this should be
taken into account when the new palette is designed.
See the <a href="#construction">Construction</a> section for further details.

## How it works

So, how does one actually get a particular color from the palette
inside the draw member function of a view?  The answer lies in
understanding in greater detail how a given palette is related to
its parents.

Every class derived from `TView` (which means *every visible Turbo
Vision object*) has a color palette.  The palette may be inherited
(like `TApplication`) or it may be `NULL` (like `TDeskTop`) but all
views do have one.  The virtual function `getPalette()` is used to
supply the palette for each view. It is important to realize that
this particular function is never called explicitly by the
programmer; it will be called by Turbo Vision when necessary.

The member functions of `TView` that actually do write to the view,
with one exception, all take a color index as one of their
parameters.  This parameter must be thought of as the value of an
index to a style, not the actual color.

For example, When using `TView::writeStr()`, one parameter
specifies a color index.  Inside of `writeStr()` the following
procedure is applied to convert this index into an actual color
value.  The current view's palette is examined at the specified
index entry and a value `X` is found there.  Next, the view's
owner's palette is retrieved and `X` is used as an index into this
palette where another value `Y` resides.  This process continues
until the view being examined no longer has an owner.  At this
time, the value at the current index is returned and interpreted
as a standard PC color attribute byte.

## Example

Here are the palettes for the classes in the enclosed example and
how they map into each other:
```
                          x01   x02   x03   x04   x05   x06
   ┌────────────────────┬─────┬─────┬─────┬─────┬─────┬─────┐
   │TTextView           │ x09 │ x0A │ x0B │ x0C │ x0D │ x0E │
   └────────────────────┴─────┴─────┴─────┴─────┴─────┴─────┘
                           ▼     ▼     ▼     ▼     ▼     ▼
                 x01-x08  x09   x0A   x0B   x0C   x0D   x0E
   ┌────────────┬───────┬─────┬─────┬─────┬─────┬─────┬─────┐
   │TTextWindow │  ...  │ x40 │ x41 │ x42 │ x43 │ x44 │ x45 │
   └────────────┴───────┴─────┴─────┴─────┴─────┴─────┴─────┘
                           ▼     ▼     ▼     ▼     ▼     ▼
                 x01-x3F  x40   x41   x42   x43   x44   x45
   ┌────────────┬───────┬─────┬─────┬─────┬─────┬─────┬─────┐
   │TTestApp    │  ...  │ x3E │ x2D │ x72 │ x5F │ x68 │ x4E │
   └────────────┴───────┴─────┴─────┴─────┴─────┴─────┴─────┘

                                Table I
```

All numbers used in the palettes in this document are in
hexadecimal (it is easier to understand attributes in that base).

Here is a concrete case of a "palette" walk, taken from the code
supplied with this document.  Suppose there is a view of type
`TTestView` inserted inside a window of type `TTestWindow`, itself
inserted in the desktop.  The palettes for these views are shown
above.  To draw something with color `0x01`, simply use `0x01` as the
parameter to the write functions used in `TTestView::draw()`.
`TTestView`'s palette contains the number `0x09` at index `0x01` and
`TTestView`'s owner is `TTestWindow`.  Index `0x09` in `TTestWindow`'s
palette contains `0x40`.  `TTestWindow`'s owner is `TDesktop` which has
a `NULL` palette and is skipped (see notes). `TDesktop`'s owner is
`TTestApp` and it's palette contains `0x3E` at index `0x40`.  So the
color that will be used is `0x3E` or yellow on cyan.

The `writeXXX` functions in TView all take color index values in
the current palette except one, `writeLine(..., TDrawBuffer)`.  A
`TDrawBuffer` is a buffer for an entire line.  Once constructed, it
is drawn into the view using `writeLine()`.  `TDrawBuffer`'s member
functions for drawing are quite similar to `TView`'s with one
exception.  They do NOT use color index values.  They use
attribute bytes to determine the colors used. What this means is
that in order to use the color palettes, one must obtain the
color attribute for a member of the current palette by hand. This
is done with the function `getColor()`.  Pass it the index and it
performs the "palette walk" and returns the actual attribute
represented.  Use this value in `TDrawBuffer`'s write functions.

Note that any attribute byte can be used with a `TDrawBuffer`.
`getColor()` need not be the source of the value used.

<div id="construction"></div>

## Palette construction

Creating a new palette for a given view is quite simple, though
deciding what indexes to use may take some thought.  It requires
inheriting from the view and overriding the `getPalette()` member
function.  This function has the following prototype:

```c++
TPalette& TTestView::getPalette() const;
```

The actual palette is a character string where the bytes contain
the appropriate reference values.  These bytes are normally
written out in hex when the string is created.  For example,

```c++
#define cpTestView "\x9\xA\xB\xC\xD\xE"
```

is the definition of the palette for `TTestView`.  The `cpTestView`
symbol is then used in TPalette constructor like this:

```c++
TPalette palette( cpTestView, sizeof(cpTestView)-1 );
```

The subtraction of 1 from `sizeof()` is to remove the terminating
`NULL` that all C++ literal strings have by default.  Also, since a
reference to this palette is returned by `getPalette()`, it must
exist from the first call to `getPalette` onward and is normally
made a static local variable to function (to avoid polluting the
global name space).  So the entire function looks like this:

```c++
#define cpTestView "\x9\xA\xB\xC\xD\xE"
TPalette& TTestView::getPalette() const;
{
    static TPalette palette(cpTestView, sizeof(cpTestView)-1);
    return palette;
}
```

What if a user has a black and white display?  A palette that has
been designed with the benefit of color will usually look
terrible when viewed in either black and white or monochrome
mode.  For this reason, Turbo Vision has three completely
distinct system palettes: `cpAppColor`, `cpAppBlackWhite`, and
`cpAppMonoChrome`.  At startup, the program will detect what kind of
display is attached and use the appropriate settings.  So, when
modifying the system palette, one needs to modify all three of
the basic system palettes.

The enclosed example addresses this
issue, as well as a similar one involving windows, since they
also have three palettes (for a different reason).  The color
choices for the alternate color palettes can be select from the
lists below:

```
        cpAppBlackWhite                     cpAppMonochrome
    0x07    Light Grey on Black         0x07    White on Black
    0x0F    White on Black              0x70    Black on White
    0x70    Black on Light Grey         0x09    White on Black
                                                Underlined
    0x78    Dark Grey on Light Grey             (not recommended)
    0x7F    White on Light Grey
```

One final note on `TWindow`.  This view has three palettes, like
`TProgram`.  However, this is so that it is easy to have three
different color schemes for windows used in an application, such
as yellow on blue for an editor but black on grey for a dialog
box.  Extending each of these palettes is done in a similar
fashion to the three palettes for `TProgram` with the exception
that `TWindow`'s palettes are not in a header file and thus must be
included by the programmer in the application along with the
extensions.  See the example for further details.

## Notes

1. In some of the example programs and the User's Guide, you will
see `getColor()` being called with a value greater than `255`.  In
this case, both bytes of the word passed are mapped into colors
and returned as a word with the attributes stored in the high and
low bytes.  This is required if one is uses
`TDrawBuffer::writeCStr` to display strings with highlighted
characters (see documentation on `writeCStr()` for more details).

2. The missing entries in table I can be found in the Turbo
Vision User's Guide by looking under `TWindow` and `TProgram`.

3. If at any time the index being used is out of range for the
palette being examined, the error attribute is returned
immediately.  The error attribute is Flashing White on Red.

4. If at any time the current palette has no entries (`NULL`
palette), then the owner's palette is examined directly.
`TDeskTop` is one view that has a `NULL` palette.

5. The top of the chain will always be the application object,
for it is the only view that will not have an owner.  The palette
for the application is inherited from `TProgram`, but can be
changed by overriding `getPalette()` in the application object (the
class derived from `TApplication`).

6. Using the `TDrawBuffer` object because of its ability to
completely bypass the palette system is sometimes desirable, but
can produce unexpected side effects.  For example, running such a
program on a VGA system running video mode 2 (`BW80`) will still
produce colors, even though Turbo Vision itself will be running
black and white.  (This example will display this behaviour so
try typing `mode BW80` at the DOS prompt and then running the
demo).
