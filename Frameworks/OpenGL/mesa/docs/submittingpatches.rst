Submitting Patches
==================

.. _guidelines:

Basic guidelines
----------------

-  Patches should not mix code changes with code formatting changes
   (except, perhaps, in very trivial cases.)
-  Code patches should follow Mesa :doc:`coding
   conventions <codingstyle>`.
-  Whenever possible, patches should only affect individual Mesa/Gallium
   components.
-  Patches should never introduce build breaks and should be bisectable
   (see ``Git bisect``.)
-  Patches should be properly :ref:`formatted <formatting>`.
-  Patches should be sufficiently :ref:`tested <testing>` before
   submitting.
-  Patches should be :ref:`submitted <submit>` via a merge request for
   :ref:`review <reviewing>`.

.. _formatting:

Patch formatting
----------------

-  Lines should be limited to 75 characters or less so that Git logs
   displayed in 80-column terminals avoid line wrapping. Note that
   ``git log`` uses 4 spaces of indentation (4 + 75 < 80).
-  The first line should be a short, concise summary of the change
   prefixed with a module name. Examples:

   ::

      mesa: Add support for querying GL_VERTEX_ATTRIB_ARRAY_LONG

      gallium: add PIPE_CAP_DEVICE_RESET_STATUS_QUERY

      i965: Fix missing type in local variable declaration.

-  Subsequent patch comments should describe the change in more detail,
   if needed. For example:

   ::

      i965: Remove end-of-thread SEND alignment code.

      This was present in Eric's initial implementation of the compaction code
      for Sandybridge (commit 077d01b6). There is no documentation saying this
      is necessary, and removing it causes no regressions in piglit on any
      platform.

-  A "Signed-off-by:" line is not required, but not discouraged either.
-  If a patch addresses an issue in GitLab, use the Closes: tag For
   example:

   ::

      Closes: https://gitlab.freedesktop.org/mesa/mesa/-/issues/1

   Prefer the full URL to just ``Closes: #1``, since the URL makes it
   easier to get to the bug page from ``git log``

   **Do not use the ``Fixes:`` tag for this!** Mesa already uses
   ``Fixes:`` for something else.
   See :ref:`below <fixes>`.

-  If there have been several revisions to a patch during the review
   process, they should be noted such as in this example:

   ::

      st/mesa: add ARB_texture_stencil8 support (v4)

      if we support stencil texturing, enable texture_stencil8
      there is no requirement to support native S8 for this,
      the texture can be converted to x24s8 fine.

      v2: fold fixes from Marek in:
         a) put S8 last in the list
         b) fix renderable to always test for d/s renderable
           fixup the texture case to use a stencil only format
           for picking the format for the texture view.
      v3: hit fallback for getteximage
      v4: put s8 back in front, it shouldn't get picked now (Ilia)

-  If someone tested your patch, document it with a line like this:

   ::

      Tested-by: Joe Hacker <jhacker@foo.com>

-  If the patch was reviewed (usually the case) or acked by someone,
   that should be documented with:

   ::

      Reviewed-by: Joe Hacker <jhacker@foo.com>
      Acked-by: Joe Hacker <jhacker@foo.com>

-  When updating a merge request add all the tags (``Acked-by:``, ``Reviewed-by:``,
   ``Fixes:``, ``Backport-to:`` and/or other) to the commit messages.
   This provides reviewers with quick feedback if the patch has already
   been reviewed.

.. _fixes:

The ``Fixes:`` tag
------------------

If a patch addresses a issue introduced with earlier commit, that
should be noted in the commit message. For example::

    Fixes: d7b3707c612 ("util/disk_cache: use stat() to check if entry is a directory")

You can produce those fixes lines by running this command once::

    git config --global alias.fixes "show -s --pretty='format:Fixes: %h (\"%s\")'"

After that, using ``git fixes <sha1>`` will print the full line for you.

The stable tag
~~~~~~~~~~~~~~

If you want a commit to be applied to a stable branch, you should add an
appropriate note to the commit message.

Using a ``Fixes:`` tag as described in :ref:`Patch formatting <formatting>`
is the preferred way to nominate a commit that should be backported.
There are scripts that will figure out which releases to apply the patch
to automatically, so you don't need to figure it out.

Alternatively, you may use the ``Backport-to:`` tag, as presented in the
following example::

    Backport-to: 21.0

Multiple ``Backport-to:`` lines are allowed.

The last option is deprecated and mostly here for historical reasons
dating back to when patch submision was done via emails: using a ``Cc:``
tag. Support for this tag will be removed at some point.
Here are some examples of such a note::

    Cc: mesa-stable
    Cc: 20.0 <mesa-stable>
    CC: 20.0 19.3 <mesa-stable>

Using the CC tag **should** include the stable branches you want to
nominate the patch to. If you do not provide any version it is nominated
to all active stable branches.

.. _testing:

Testing Patches
---------------

It should go without saying that patches must be tested. In general, do
whatever testing is prudent.

You should always run the Mesa test suite before submitting patches. The
test suite can be run using the 'meson test' command. All tests must
pass before patches will be accepted, this may mean you have to update
the tests themselves.

Whenever possible and applicable, test the patch with
`Piglit <https://piglit.freedesktop.org>`__ and/or
`dEQP <https://android.googlesource.com/platform/external/deqp/>`__ to
check for regressions.

As mentioned at the beginning, patches should be bisectable. A good way
to test this is to make use of the \`git rebase\` command, to run your
tests on each commit. Assuming your branch is based off
``origin/main``, you can run:

.. code-block:: console

   $ git rebase --interactive --exec "meson test -C build/" origin/main

replacing ``"meson test"`` with whatever other test you want to run.

.. _submit:

Submitting Patches
------------------

Patches are submitted to the Mesa project via a
`GitLab <https://gitlab.freedesktop.org/mesa/mesa>`__ Merge Request.

Add labels to your MR to help reviewers find it. For example:

-  Mesa changes affecting all drivers: mesa
-  Hardware vendor specific code: AMD common, intel, ...
-  Driver specific code: ANV, freedreno, i965, iris, radeonsi, RADV,
   vc4, ...
-  Other tag examples: gallium, util

Tick the following when creating the MR. It allows developers to rebase
your work on top of main.

::

   Allow commits from members who can merge to the target branch

If you revise your patches based on code review and push an update to
your branch, you should maintain a **clean** history in your patches.
There should not be "fixup" patches in the history. The series should be
buildable and functional after every commit whenever you push the
branch.

It is your responsibility to keep the MR alive and making progress, as
there are no guarantees that a Mesa dev will independently take interest
in it.

Some other notes:

-  Make changes and update your branch based on feedback
-  After an update, for the feedback you handled, close the feedback
   discussion with the "Resolve Discussion" button. This way the
   reviewers know which feedback got handled and which didn't.
-  Old, stale MR may be closed, but you can reopen it if you still want
   to pursue the changes
-  You should periodically check to see if your MR needs to be rebased
-  Make sure your MR is closed if your patches get pushed outside of
   GitLab
-  Please send MRs from a personal fork rather than from the main Mesa
   repository, as it clutters it unnecessarily.

.. _reviewing:

Reviewing Patches
-----------------

To participate in code review, you can monitor the GitLab Mesa `Merge
Requests <https://gitlab.freedesktop.org/mesa/mesa/-/merge_requests>`__
page, and/or register for notifications in your GitLab settings.

When you've reviewed a patch, please be unambiguous about your review.
That is, state either

::

   Reviewed-by: Joe Hacker <jhacker@foo.com>

or

::

   Acked-by: Joe Hacker <jhacker@foo.com>

Rather than saying just "LGTM" or "Seems OK".

If small changes are suggested, it's OK to say something like:

::

   With the above fixes, Reviewed-by: Joe Hacker <jhacker@foo.com>

which tells the patch author that the patch can be committed, as long as
the issues are resolved first.

These Reviewed-by, Acked-by, and Tested-by tags should also be amended
into commits in a MR before it is merged.

When providing a Reviewed-by, Acked-by, or Tested-by tag in a GitLab MR,
enclose the tag in backticks:

::

   `Reviewed-by: Joe Hacker <jhacker@example.com>`

This is the markdown format for literal, and will prevent GitLab from
hiding the < and > symbols.

Review by non-experts is encouraged. Understanding how someone else goes
about solving a problem is a great way to learn your way around the
project. The submitter is expected to evaluate whether they have an
appropriate amount of review feedback from people who also understand
the code before merging their patches.

.. _merging:

Merging merge requests
----------------------

Once a merge request has been appropriately reviewed, its author can decide to
merge it.

.. warning::
   Pushing (``git push``) directly to ``main`` is forbidden. This bypasses all
   the CI checks and is likely to cause issues for everyone else.

.. warning::
   Do not use the "Merge"/"Merge when pipeline succeeds"/"Set to auto-merge"
   buttons.

We use a `custom script <https://gitlab.com/marge-org/marge-bot>`__ to manage
this, triggered by **assigning the MR** to the pseudo-user `@marge-bot
<https://gitlab.freedesktop.org/marge-bot>`__.

Authors who do not have ``Developer`` access (or above) should ask on the
merge request for someone else to do it for them, or reach on
:doc:`other channels <lists>` if the MR reviewers don't have access themselves.

Do not merge someone else's MR unless you are sure they don't have a new
version that they are testing locally for instance.
**When in doubt, ask**, for instance by leaving a comment on that MR.

Nominating a commit for a stable branch
---------------------------------------

There are several ways to nominate a patch for inclusion in the stable
branch and release. In order or preference:

- By adding the ``Fixes:`` tag in the commit message as described above, if you are fixing
  a specific commit.
- By adding the ``Cc: mesa-stable`` tag in the commit message as described above.
- By submitting a merge request against the ``staging/year.quarter``
  branch on GitLab. Refer to the :ref:`instructions below <backports>`.

Please **DO NOT** send patches to mesa-stable@lists.freedesktop.org, it
is not monitored actively and is a historical artifact.

If you are not the author of the original patch, please Cc: them in your
nomination request.

The current patch status can be observed in the :ref:`staging
branch <stagingbranch>`.

.. _criteria:

Criteria for accepting patches to the stable branch
---------------------------------------------------

Mesa has a designated release manager for each stable branch, and the
release manager is the only developer that should be pushing changes to
these branches. Everyone else should nominate patches using the
mechanism described above. The following rules define which patches are
accepted and which are not. The stable-release manager is also given
broad discretion in rejecting patches that have been nominated.

-  Patch must conform with the :ref:`Basic guidelines <guidelines>`
-  Patch must have landed in main first. In case where the original
   patch is too large and/or otherwise contradicts with the rules set
   within, a backport is appropriate.
-  It must not introduce a regression - be that build or runtime wise.

   .. note::
      If the regression is due to faulty Piglit/dEQP/CTS/other test
      the latter must be fixed first. A reference to the offending test(s)
      and respective fix(es) should be provided in the nominated patch.

-  Patch cannot be larger than 100 lines.
-  Patches that move code around with no functional change should be
   rejected.
-  Patch must be a bug fix and not a new feature.

   .. note::
      An exception to this rule, are hardware-enabling "features". For
      example, :ref:`backports <backports>` of new code to support a
      newly-developed hardware product can be accepted if they can be
      reasonably determined not to have effects on other hardware.

-  Patch must be reviewed, For example, the commit message has
   Reviewed-by, Signed-off-by, or Tested-by tags from someone but the
   author.
-  Performance patches are considered only if they provide information
   about the hardware, program in question and observed improvement. Use
   numbers to represent your measurements.

If the patch complies with the rules it will be
:ref:`cherry-picked <pickntest>`. Alternatively the release
manager will reply to the patch in question stating why the patch has
been rejected or would request a backport. The stable-release manager
may at times need to force-push changes to the stable branches, for
example, to drop a previously-picked patch that was later identified as
causing a regression). These force-pushes may cause changes to be lost
from the stable branch if developers push things directly. Consider
yourself warned.

.. _backports:

Sending backports for the stable branch
---------------------------------------

By default merge conflicts are resolved by the stable-release manager.
The release maintainer should resolve trivial conflicts, but for complex
conflicts they should ask the original author to provide a backport or
denominate the patch.

For patches that either need to be nominated after they've landed in
main, or that are known ahead of time to not not apply cleanly to a
stable branch (such as due to a rename), using a GitLab MR is most
appropriate. The MR should be based on and target the
``staging/year.quarter`` branch, not on the ``year.quarter`` branch,
per the stable branch policy. Assigning the MR to release maintainer for
said branch or mentioning them is helpful, but not required.

Make sure to use ``git cherry-pick -x`` when cherry-picking the commits
from the main branch. This adds the "cherry picked from commit ..." line
to the commit message, to allow the release maintainters to mark those
as backported, which in turn allows the tools to correctly report any
future ``Fixes:`` affecting the commits you backported.

Documentation patches
---------------------

Our documentation is written as `reStructuredText`_ files in the
:file:`docs` folder, and built using `Sphinx`_.

.. code-block:: sh

   # Install dependencies (adapt for your distro)
   apk add coreutils graphviz py3-clang clang-dev musl-dev linux-headers
   pip3 install sphinx===5.1.1 mako===1.2.3 hawkmoth===0.16.0

   # Build docs
   sphinx-build -W -b html docs docs-html/

The preferred language of the documentation is US English. This
doesn't mean that everyone is expected to pay close attention to
the different English variants, but it does mean someone might
suggest a spelling-change, either during review or as a follow-up
merge-request.

.. _reStructuredText: https://docutils.sourceforge.io/rst.html
.. _Sphinx: https://www.sphinx-doc.org/

Git tips
--------

-  ``git rebase -i ...`` is your friend. Don't be afraid to use it.
-  Apply a fixup to commit FOO.

   .. code-block:: console

      git add ...
      git commit --fixup=FOO
      git rebase -i --autosquash ...

-  Test for build breakage between patches e.g last 8 commits.

   .. code-block:: console

      git rebase -i --exec="ninja -C build/" HEAD~8
