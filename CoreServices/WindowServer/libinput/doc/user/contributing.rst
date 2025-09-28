
.. _contributing:

==============================================================================
Contributing to libinput
==============================================================================


So you want to contribute to libinput? Great! We'd love to help you be a part
of our community. Here is some important information to help you.

.. contents::
    :local:

------------------------------------------------------------------------------
Code of Conduct
------------------------------------------------------------------------------

As a freedesktop.org project, libinput follows the `freedesktop.org
Contributor Covenant <https://www.freedesktop.org/wiki/CodeOfConduct>`_.

Please conduct yourself in a respectful and civilised manner when
interacting with community members on mailing lists, IRC, or bug trackers.
The community represents the project as a whole, and abusive or bullying
behaviour is not tolerated by the project.

------------------------------------------------------------------------------
Contact
------------------------------------------------------------------------------

Questions can be asked on ``#wayland`` on oftc or on the
`wayland-devel@lists.freedesktop.org
<https://lists.freedesktop.org/mailman/listinfo/wayland-devel>`_ mailing
list.

For IRC, ping user ``whot`` (Peter Hutterer, the libinput maintainer) though
note that he lives on UTC+10 and thus the rest of the world is out of sync
by default ;)

For anything that appears to be device specific and/or related to a new
feature, just file `an issue in our issue tracker
<https://gitlab.freedesktop.org/libinput/libinput/issues>`_. It's usually the
most efficient way to get answers.

------------------------------------------------------------------------------
What to work on?
------------------------------------------------------------------------------

If you don't already know what you want to improve or fix with libinput,
then a good way of finding something is to search for the ``help needed``
tag in our `issue tracker <https://gitlab.freedesktop.org/libinput/libinput/issues?label_name%5B%5D=help+needed>`_.
These are issues that have been triaged to some degree and deemed to be a
possible future feature to libinput.

.. note:: Some of these issue may require specific hardware to reproduce.

Another good place to help out with is the documentation. For anything you
find in these pages that isn't clear enough please feel free to reword it
and add what is missing.

------------------------------------------------------------------------------
Getting the code
------------------------------------------------------------------------------

The :ref:`building_libinput` have all the details but the short solution
will be:

::

     $> git clone https://gitlab.freedesktop.org/libinput/libinput
     $> cd libinput
     $> meson setup --prefix=/usr builddir/
     $> ninja -C builddir/
     $> sudo ninja -C builddir/ install

You can omit the last step if you only want to test locally.

------------------------------------------------------------------------------
Working on the code
------------------------------------------------------------------------------

If you are planning to send patches, it's a good idea to set up
`pre-commit <https://pre-commit.com/>`_ with these commands::

     $> pre-commit install
     $> pre-commit install --hook-type pre-push

This will check a few things before you commit and/or push to your repos to
reduce the turnaround time for some common mistakes.

libinput has a roughly three-parts architecture:

-  the front-end code which handles the ``libinput_some_function()`` API calls in ``libinput.c``
-  the generic evdev interface handling which maps those API calls to the
   backend calls (``evdev.c``).
- there are device-specific backends which do most of the actual work -
  ``evdev-mt-touchpad.c`` is the one for touchpads for example.

In general, things that only affect the internal workings of a device only
get implemented in the device-specific backend. You only need to touch the
API when you are adding configuration options. For more details, please read
the :ref:`architecture` document. There's also a `blog post describing the
building blocks
<https://who-t.blogspot.com/2019/03/libinputs-internal-building-blocks.html>`_
that may help to understand how it all fits together.

Documentation is in ``/doc/api`` for the doxygen-generated API documentation.
These are extracted from the libinput source code directly. The
documentation you're reading right now is in ``/doc/user`` and generated with
sphinx. Simply running ``ninja -C builddir`` will rebuild it and the final
product ends up in ``builddir/Documentation``.

------------------------------------------------------------------------------
Testing the code
------------------------------------------------------------------------------

libinput provides a bunch of :ref:`tools` to debug any changes - without
having to install libinput.

The two most useful ones are :ref:`libinput debug-events
<libinput-debug-events>` and :ref:`libinput debug-gui <libinput-debug-gui>`.
Both tools can be run from the build directory directly and are great for
quick test iterations::

  $> sudo ./builddir/libinput-debug-events --verbose
  $> sudo ./builddir/libinput-debug-gui --verbose

The former provides purely textual output and is useful for verifying event
streams from buttons, etc. The latter is particularly useful when you are
trying to debug pointer movement or placement. ``libinput debug-gui`` will
also visualize the raw data from the device so you can compare pointer
behavior with what comes from the kernel.

These tools create a new libinput context and will not affect your session's
behavior. Only once you've installed libinput and restarted your session
will your changes affect the X server/Wayland compositor.

Once everything seems to be correct, it's time to run the
:ref:`test-suite`::

  $> sudo ./builddir/libinput-test-suite

This test suite can take test names etc. as arguments, have a look at
:ref:`test-suite` for more info. There are a bunch of other tests that are
run by the CI on merge requests, you can run those locally with ::

  $> sudo ninja -C builddir check

So it always pays to run that before submitting. This will also run the code
through valgrind and pick up any memory leaks.

.. _contributing_submitting_code:

------------------------------------------------------------------------------
Submitting Code
------------------------------------------------------------------------------

Any patches should be sent via a Merge Request (see the `GitLab docs
<https://docs.gitlab.com/ce/gitlab-basics/add-merge-request.htm>`_)
in the `libinput GitLab instance hosted by freedesktop.org
<https://gitlab.freedesktop.org/libinput/libinput>`_.

Below are the steps required to submit a merge request. They do not
replace `learning git <https://git-scm.com/doc>`__ but they should be
sufficient to make some of the more confusing steps obvious.

- `Register an account <https://gitlab.freedesktop.org/users/sign_in>`_ in
  the freedesktop.org GitLab instance.
- `Fork libinput <https://gitlab.freedesktop.org/libinput/libinput/-/forks/new>`_
  into your username's namespace. Select public visibility.
- Get libinput's main repository. git will call this repository ``origin``. ::

    git clone https://gitlab.freedesktop.org/libinput/libinput.git

- Add the forked git repository to your remotes (replace ``USERNAME``
  with your username). git will call this repository ``gitlab``. ::

    cd /path/to/libinput.git
    git remote add gitlab git@gitlab.freedesktop.org:USERNAME/libinput.git
    git fetch gitlab

- Create a new branch and commit your changes to that branch. ::

    git switch -C mynewbranch
    # edit files, make changes
    git add file1 file2
    git commit -s
    # edit commit message in the editor

  Replace ``mynewbranch`` (here and in the commands below) with a meaningful
  name. See :ref:`contributing_commit_messages` for details on the commit
  message format.

- Push your changes to your fork and submit a merge request ::

    git push gitlab mynewbranch

  This command will print the URL to file a merge request, you then only
  have to click through. Alternatively you can go to:

    https://gitlab.freedesktop.org/USERNAME/libinput/merge_requests

  Select your branch name to merge and ``libinput/libinput`` ``main`` as target branch.

- Verify that the CI completes successfully by visiting the merge request
  page. A successful pipeline shows only green ticks, failure is indicated
  by a red cross or a yellow exclamation mark (see
  the `GitLab Docs
  <https://docs.gitlab.com/ee/ci/pipelines/#pipeline-mini-graphs>`__). For
  details about the failures, click on the failed jobs in the pipelines
  and/or click the ``Expand`` button in the box for the test summaries.

  A merge request without a successful pipeline may never be looked at by a
  maintainer.

- If changes are requested by the maintainers, please **amend** the
  commit(s) and **force-push** the updated branch. ::

    # edits in file foo.c
    git add foo.c
    git commit --amend
    git push -f gitlab mynewbranch

  A force-push will re-trigger the CI and notify the merge request that new
  changes are available.

  If the branch contains more than one commit, please look at
  `git interactive rebases
  <https://git-scm.com/book/en/v2/Git-Tools-Rewriting-History>`__
  to learn how to change multiple commits, or squash new changes into older
  commits.

------------------------------------------------------------------------------
Commit History
------------------------------------------------------------------------------

libinput strives to have a
`linear, 'recipe' style history <http://www.bitsnbites.eu/git-history-work-log-vs-recipe/>`_
This means that every commit should be small, digestible, stand-alone, and
functional. Rather than a purely chronological commit history like this: ::

	doc: final docs for view transforms
	fix tests when disabled, redo broken doc formatting
	better transformed-view iteration (thanks Hannah!)
	try to catch more cases in tests
	tests: add new spline test
	fix compilation on splines
	doc: notes on reticulating splines
	compositor: add spline reticulation for view transforms

We aim to have a clean history which only reflects the final state, broken up
into functional groupings: ::

	compositor: add spline reticulation for view transforms
	compositor: new iterator for view transforms
	tests: add view-transform correctness tests
	doc: fix Doxygen formatting for view transforms

This ensures that the final patch series only contains the final state,
without the changes and missteps taken along the development process.

The first line of a commit message should contain a prefix indicating
what part is affected by the patch followed by one sentence that
describes the change. For example: ::

	touchpad: add software button behavior
	fallback: disable button debouncing on device foo

If in doubt what prefix to use, look at other commits that change the
same file(s) as the patch being sent.

.. _contributing_commit_messages:

------------------------------------------------------------------------------
Commit Messages
------------------------------------------------------------------------------

When you re-send patches, revised or not, it would be very good to document the
changes compared to the previous revision in the commit message and/or the
merge request. If you have already received Reviewed-by or Acked-by tags, you
should evaluate whether they still apply and include them in the respective
commit messages. Otherwise the tags may be lost, reviewers miss the credit they
deserve, and the patches may cause redundant review effort.

For further reading, please see
`'on commit messages' <http://who-t.blogspot.de/2009/12/on-commit-messages.html>`_
as a general guideline on what commit messages should contain.

------------------------------------------------------------------------------
Coding Style
------------------------------------------------------------------------------

Please see the `CODING_STYLE.md
<https://gitlab.freedesktop.org/libinput/libinput/blob/main/CODING_STYLE.md>`_
document in the source tree.

------------------------------------------------------------------------------
Tracking patches and follow-ups
------------------------------------------------------------------------------

Once submitted to GitLab, your patches will be reviewed by the libinput
development team on GitLab. Review may be entirely positive and result in your
code landing instantly, in which case, great! You're done. However, we may ask
you to make some revisions: fixing some bugs we've noticed, working to a
slightly different design, or adding documentation and tests.

If you do get asked to revise the patches, please bear in mind the notes above.
You should use ``git rebase -i`` to make revisions, so that your patches
follow the clear linear split documented above. Following that split makes
it easier for reviewers to understand your work, and to verify that the code
you're submitting is correct.

A common request is to split single large patch into multiple patches. This can
happen, for example, if when adding a new feature you notice a bug in
libinput's core which you need to fix to progress. Separating these changes
into separate commits will allow us to verify and land the bugfix quickly,
pushing part of your work for the good of everyone, whilst revision and
discussion continues on the larger feature part. It also allows us to direct
you towards reviewers who best understand the different areas you are
working on.

When you have made any requested changes, please rebase the commits, verify
that they still individually look good, then force-push your new branch to
GitLab. This will update the merge request and notify everyone subscribed to
your merge request, so they can review it again.

There are also many GitLab CLI clients, if you prefer to avoid the web
interface. It may be difficult to follow review comments without using the
web interface though, so we do recommend using this to go through the review
process, even if you use other clients to track the list of available
patches.

------------------------------------------------------------------------------
Failed pipeline errors
------------------------------------------------------------------------------

After submitting your merge request to GitLab, you might receive an email
informing you that your pipeline failed.

Visit your merge request page and check the `pipeline mini graph
<https://docs.gitlab.com/ee/ci/pipelines/#pipeline-mini-graphs>`_ to know which
step failed.

Follow the appropriate section to fix the errors.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Committed gitlab-ci.yml differs from generated gitlab-ci.yml. Please verify
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When your merge request modifies the CI templates, you might see this error
mainly due two reasons: the wrong file was modified and/or
``ci-fairy generate-template`` wasn't run.

``.gitlab-ci.yaml`` is auto generated, changes should be made in:

- ``.gitlab-ci/ci.template``

- ``.gitlab-ci/config.yaml``

Once the changes are ready, run
`ci-fairy <https://freedesktop.pages.freedesktop.org/ci-templates/ci-fairy.html#templating-gitlab-ci-yml>`_
to update ``.gitlab-ci.yaml``: ::

  ci-fairy generate-template

Finally, force-push you changes. See :ref:`contributing_submitting_code` for
more details.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Build errors
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Usually, checking the CI log is enough to catch this errors. However, your merge
request is built using different configurations you might have not tested.

In order to fix this kind of problems, you can compile libinput using the same
flags used by the CI.

For example, if an error is found in the ``build-no-libwacom`` step, open the
log and search the build options: ::

  [...]
  + rm -rf 'build dir'
  + meson 'build dir' -Dlibwacom=false
  The Meson build system
  [...]

Use the same flags to fix the issue and force-push you changes. See
:ref:`contributing_submitting_code` for more details.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Test errors
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The test suite is run for your merge request to check for bugs, regressions and
memory leaks among other issues.

Open the CI error log and search for a message similar to: ::

  :: Failure: ../test/test-touchpad.c:465: touchpad_2fg_scroll_slow_distance(synaptics-t440)

See :ref:`test-suite` to learn how to run the failing tests.

Once the tests are fixed, force-push you changes. See
:ref:`contributing_submitting_code` for more details.
