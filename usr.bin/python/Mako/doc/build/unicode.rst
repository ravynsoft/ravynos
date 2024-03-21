.. _unicode_toplevel:

===================
The Unicode Chapter
===================

In normal Mako operation, all parsed template constructs and
output streams are handled internally as Python 3 ``str`` (Unicode)
objects. It's only at the point of :meth:`~.Template.render` that this stream of Unicode objects may be rendered into whatever the desired output encoding
is. The implication here is that the template developer must
:ensure that :ref:`the encoding of all non-ASCII templates is explicit
<set_template_file_encoding>` (still required in Python 3, although Mako defaults to ``utf-8``),
that :ref:`all non-ASCII-encoded expressions are in one way or another
converted to unicode <handling_non_ascii_expressions>`
(not much of a burden in Python 3), and that :ref:`the output stream of the
template is handled as a unicode stream being encoded to some
encoding <defining_output_encoding>` (still required in Python 3).

.. _set_template_file_encoding:

Specifying the Encoding of a Template File
==========================================

.. versionchanged:: 1.1.3

    As of Mako 1.1.3, the default template encoding is "utf-8".  Previously, a
    Python "magic encoding comment" was required for templates that were not
    using ASCII.

Mako templates support Python's "magic encoding comment" syntax
described in  `pep-0263 <http://www.python.org/dev/peps/pep-0263/>`_:

.. sourcecode:: mako

    ## -*- coding: utf-8 -*-

    Alors vous imaginez ma surprise, au lever du jour, quand
    une drôle de petite voix m’a réveillé. Elle disait:
     « S’il vous plaît… dessine-moi un mouton! »

As an alternative, the template encoding can be specified
programmatically to either :class:`.Template` or :class:`.TemplateLookup` via
the ``input_encoding`` parameter:

.. sourcecode:: python

    t = TemplateLookup(directories=['./'], input_encoding='utf-8')

The above will assume all located templates specify ``utf-8``
encoding, unless the template itself contains its own magic
encoding comment, which takes precedence.

.. _handling_non_ascii_expressions:

Handling Expressions
====================

The next area that encoding comes into play is in expression
constructs. By default, Mako's treatment of an expression like
this:

.. sourcecode:: mako

    ${"hello world"}

looks something like this:

.. sourcecode:: python

    context.write(str("hello world"))

That is, **the output of all expressions is run through the
``str`` built-in**. This is the default setting, and can be
modified to expect various encodings. The ``str`` step serves
both the purpose of rendering non-string expressions into
strings (such as integers or objects which contain ``__str()__``
methods), and to ensure that the final output stream is
constructed as a Unicode object. The main implication of this is
that **any raw byte-strings that contain an encoding other than
ASCII must first be decoded to a Python unicode object**.

Similarly, if you are reading data from a file that is streaming
bytes, or returning data from some object that is returning a
Python byte-string containing a non-ASCII encoding, you have to
explicitly decode to Unicode first, such as:

.. sourcecode:: mako

    ${call_my_object().decode('utf-8')}

Note that filehandles acquired by ``open()`` in Python 3 default
to returning "text": that is, the decoding is done for you. See
Python 3's documentation for the ``open()`` built-in for details on
this.

If you want a certain encoding applied to *all* expressions,
override the ``str`` builtin with the ``decode`` built-in at the
:class:`.Template` or :class:`.TemplateLookup` level:

.. sourcecode:: python

    t = Template(templatetext, default_filters=['decode.utf8'])

Note that the built-in ``decode`` object is slower than the
``str`` function, since unlike ``str`` it's not a Python
built-in, and it also checks the type of the incoming data to
determine if string conversion is needed first.

The ``default_filters`` argument can be used to entirely customize
the filtering process of expressions. This argument is described
in :ref:`filtering_default_filters`.

.. _defining_output_encoding:

Defining Output Encoding
========================

Now that we have a template which produces a pure Unicode output
stream, all the hard work is done. We can take the output and do
anything with it.

As stated in the :doc:`"Usage" chapter <usage>`, both :class:`.Template` and
:class:`.TemplateLookup` accept ``output_encoding`` and ``encoding_errors``
parameters which can be used to encode the output in any Python
supported codec:

.. sourcecode:: python

    from mako.template import Template
    from mako.lookup import TemplateLookup

    mylookup = TemplateLookup(directories=['/docs'], output_encoding='utf-8', encoding_errors='replace')

    mytemplate = mylookup.get_template("foo.txt")
    print(mytemplate.render())

:meth:`~.Template.render` will return a ``bytes`` object in Python 3 if an output
encoding is specified. By default it performs no encoding and
returns a native string.

:meth:`~.Template.render_unicode` will return the template output as a Python
``str`` object:

.. sourcecode:: python

    print(mytemplate.render_unicode())

The above method disgards the output encoding keyword argument;
you can encode yourself by saying:

.. sourcecode:: python

    print(mytemplate.render_unicode().encode('utf-8', 'replace'))
