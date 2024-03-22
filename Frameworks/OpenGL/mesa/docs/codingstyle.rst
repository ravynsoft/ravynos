Coding Style
============

Mesa is over 20 years old and the coding style has evolved over time.
Some old parts use a style that's a bit out of date. Different sections
of mesa can use different coding style as set in the local EditorConfig
(.editorconfig) and/or Emacs (.dir-locals.el) file. Alternatively the
following is applicable. If the guidelines below don't cover something,
try following the format of existing, neighboring code.

``clang-format``
----------------

A growing number of drivers and components are adopting ``clang-format``
to standardize the formatting and make it easy for everyone to apply it.

You can re-format the code for the components that have opted-in to the
formatting enforcement (listed in ``.clang-format-include``) by simply
running ``ninja -C build/ clang-format``.

Since mass-reformatting commits can be an annoying extra jump to go
through when looking at ``git blame``, you can configure it to ignore
them by running::

  git config blame.ignoreRevsFile .git-blame-ignore-revs

Most code editors also support automatically formatting code as you
write it; check your editor or its pluggins to see how to enable this.

Vim
***

Add this to your ``.vimrc`` to automatically format any C & C++ file
(that has a .clang-format config) when you save it:

.. code:: vim

   augroup ClangFormatOnSave
     au!

     function! ClangFormatOnSave()
       " Only format files that have a .clang-format in a parent folder
       if !empty(findfile('.clang-format', '.;'))
         let l:formatdiff = 1 " Only format lines that have changed
         py3f /usr/share/clang/clang-format.py
       endif
     endfunction

     autocmd BufWritePre *.h,*.c,*.cc,*.cpp call ClangFormatOnSave()
   augroup END

If ``/usr/share/clang/clang-format.py`` doesn't exist, try
``/usr/share/clang/clang-format-$CLANG_VERSION/clang-format.py``
(replacing ``$CLANG_VERSION`` with your clang version). If your distro
has put the file somewhere else, look through the files in the package
providing ``clang-format``.

Emacs
*****

Add this to your ``.emacs`` to automatically format any C & C++ file
(that has a .clang-format config) when you save it:

.. code:: emacs

   (load "/usr/share/clang/clang-format.el")

   (defun clang-format-save-hook-for-this-buffer ()
     "Create a buffer local save hook."
     (add-hook 'before-save-hook
               (lambda ()
                 (when (locate-dominating-file "." ".clang-format")
                   (clang-format-buffer))
                 ;; Continue to save.
                 nil)
               nil
               ;; Buffer local hook.
               t))

   ;; Run this for each mode you want to use the hook.
   (add-hook 'c-mode-hook (lambda () (clang-format-save-hook-for-this-buffer)))
   (add-hook 'c++-mode-hook (lambda () (clang-format-save-hook-for-this-buffer)))

If ``/usr/share/clang/clang-format.el`` doesn't exist, look through the
files in the package providing ``clang-format`` in your distro. If you
can't find anything (eg. on Debian/Ubuntu), refer to `this StackOverflow
answer <https://stackoverflow.com/questions/59690583/how-do-you-use-clang-format-on-emacs-ubuntu/59850773#59850773>`__
to install clang-format through Emacs instead.

git ``pre-commit`` hook
***********************

If your editor doesn't support this, or if you don't want to enable it, you
can always just run ``ninja clang-format`` to format everything, or add
a ``pre-commit`` hook that runs this automatically whenever you ``git
commit`` by adding the following in your ``.git/hooks/pre-commit``:

.. code:: sh

   shopt -s globstar
   git clang-format $upstream -- $(grep -E '^[^#]' .clang-format-include)
   # replace $upstream with the name of the remote tracking upstream mesa
   # if you don't know, it's probably `origin`


Basic formatting guidelines
---------------------------

-  3-space indentation, no tabs.
-  Limit lines to 78 or fewer characters. The idea is to prevent line
   wrapping in 80-column editors and terminals. There are exceptions,
   such as if you're defining a large, static table of information.
-  Opening braces go on the same line as the if/for/while statement. For
   example:

   .. code-block:: c

      if (condition) {
         foo;
      } else {
         bar;
      }

-  Put a space before/after operators. For example, ``a = b + c;`` and
   not ``a=b+c;``
-  This GNU indent command generally does the right thing for
   formatting:

   .. code-block:: console

      indent -br -i3 -npcs --no-tabs infile.c -o outfile.c

-  Use comments wherever you think it would be helpful for other
   developers. Several specific cases and style examples follow. Note
   that we roughly follow `Doxygen <https://www.doxygen.nl>`__
   conventions.

   Single-line comments:

   .. code-block:: c

      /* null-out pointer to prevent dangling reference below */
      bufferObj = NULL;

   Or,

   .. code-block:: c

      bufferObj = NULL;  /* prevent dangling reference below */

   Multi-line comment:

   .. code-block:: c

      /* If this is a new buffer object id, or one which was generated but
       * never used before, allocate a buffer object now.
       */

   We try to quote the OpenGL specification where prudent:

   .. code-block:: c

      /* Page 38 of the PDF of the OpenGL ES 3.0 spec says:
       *
       *     "An INVALID_OPERATION error is generated for any of the following
       *     conditions:
       *
       *     * <length> is zero."
       *
       * Additionally, page 94 of the PDF of the OpenGL 4.5 core spec
       * (30.10.2014) also says this, so it's no longer allowed for desktop GL,
       * either.
       */

   Function comment example:

   .. code-block:: c

      /**
       * Create and initialize a new buffer object.  Called via the
       * ctx->Driver.CreateObject() driver callback function.
       * \param  name  integer name of the object
       * \param  type  one of GL_FOO, GL_BAR, etc.
       * \return  pointer to new object or NULL if error
       */
      struct gl_object *
      _mesa_create_object(GLuint name, GLenum type)
      {
         /* function body */
      }

-  Put the function return type and qualifiers on one line and the
   function name and parameters on the next, as seen above. This makes
   it easy to use ``grep ^function_name dir/*`` to find function
   definitions. Also, the opening brace goes on the next line by itself
   (see above.)
-  Function names follow various conventions depending on the type of
   function:

   +---------------------+------------------------------------------+
   | Convention          | Explanation                              |
   +=====================+==========================================+
   | ``glFooBar()``      | a public GL entry point (in              |
   |                     | :file:`glapi_dispatch.c`)                |
   +---------------------+------------------------------------------+
   | ``_mesa_FooBar()``  | the internal immediate mode function     |
   +---------------------+------------------------------------------+
   | ``save_FooBar()``   | retained mode (display list) function in |
   |                     | :file:`dlist.c`                          |
   +---------------------+------------------------------------------+
   | ``foo_bar()``       | a static (private) function              |
   +---------------------+------------------------------------------+
   | ``_mesa_foo_bar()`` | an internal non-static Mesa function     |
   +---------------------+------------------------------------------+

-  Constants, macros and enum names are ``ALL_UPPERCASE``, with \_
   between words.
-  Mesa usually uses camel case for local variables (Ex:
   ``localVarname``) while Gallium typically uses underscores (Ex:
   ``local_var_name``).
-  Global variables are almost never used because Mesa should be
   thread-safe.
-  Booleans. Places that are not directly visible to the GL API should
   prefer the use of ``bool``, ``true``, and ``false`` over
   ``GLboolean``, ``GL_TRUE``, and ``GL_FALSE``. In C code, this may
   mean that ``#include <stdbool.h>`` needs to be added. The
   ``try_emit_*`` method ``src/mesa/state_tracker/st_glsl_to_tgsi.cpp``
   can serve as an example.
