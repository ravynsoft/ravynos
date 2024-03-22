Contributing to Wayland
=======================

Sending patches
---------------

Patches should be sent via
[GitLab merge requests](https://docs.gitlab.com/ce/gitlab-basics/add-merge-request.html).
Wayland is
[hosted on freedesktop.org's GitLab](https://gitlab.freedesktop.org/wayland/wayland/):
in order to submit code, you should create an account on this GitLab instance,
fork the core Wayland repository, push your changes to a branch in your new
repository, and then submit these patches for review through a merge request.

Wayland formerly accepted patches via `git-send-email`, sent to
**wayland-devel@lists.freedesktop.org**; these were
[tracked using Patchwork](https://patchwork.freedesktop.org/project/wayland/).
Some old patches continue to be sent this way, and we may accept small new
patches sent to the list, but please send all new patches through GitLab merge
requests.


Formatting and separating commits
---------------------------------

Unlike many projects using GitHub and GitLab, Wayland has a
[linear, 'recipe' style history](http://www.bitsnbites.eu/git-history-work-log-vs-recipe/).
This means that every commit should be small, digestible, stand-alone, and
functional. Rather than a purely chronological commit history like this:

    connection: plug a fd leak
    plug another fd leak
    connection: init fds to -1
    close all fds
    refactor checks into a new function
    don't close fds we handed out

we aim to have a clean history which only reflects the final state, broken up
into functional groupings:

    connection: Refactor out closure allocation
    connection: Clear fds we shouldn't close to -1
    connection: Make wl_closure_destroy() close fds of undispatched closures

This ensures that the final patch series only contains the final state,
without the changes and missteps taken along the development process.

The first line of a commit message should contain a prefix indicating
what part is affected by the patch followed by one sentence that
describes the change. For examples:

    protocol: Support scaled outputs and surfaces

and

    doc: generate server documentation from XML too

If in doubt what prefix to use, look at other commits that change the
same file(s) as the patch being sent.

The body of the commit message should describe what the patch changes
and why, and also note any particular side effects. This shouldn't be
empty on most of the cases. It shouldn't take a lot of effort to write
a commit message for an obvious change, so an empty commit message
body is only acceptable if the questions "What?" and "Why?" are already
answered on the one-line summary.

The lines of the commit message should have at most 76 characters, to
cope with the way git log presents them.

See [notes on commit messages] for a recommended reading on writing commit
messages.

Your patches should also include a Signed-off-by line with your name and
email address.  If you're not the patch's original author, you should
also gather S-o-b's by them (and/or whomever gave the patch to you.) The
significance of this is that it certifies that you created the patch,
that it was created under an appropriate open source license, or
provided to you under those terms.  This lets us indicate a chain of
responsibility for the copyright status of the code.

We won't reject patches that lack S-o-b, but it is strongly recommended.

When you re-send patches, revised or not, it would be very good to document the
changes compared to the previous revision in the commit message and/or the
merge request. If you have already received Reviewed-by or Acked-by tags, you
should evaluate whether they still apply and include them in the respective
commit messages. Otherwise the tags may be lost, reviewers miss the credit they
deserve, and the patches may cause redundant review effort.


Tracking patches and following up
---------------------------------

Once submitted to GitLab, your patches will be reviewed by the Wayland
development team on GitLab. Review may be entirely positive and result in your
code landing instantly, in which case, great! You're done. However, we may ask
you to make some revisions: fixing some bugs we've noticed, working to a
slightly different design, or adding documentation and tests.

If you do get asked to revise the patches, please bear in mind the notes above.
You should use `git rebase -i` to make revisions, so that your patches follow
the clear linear split documented above. Following that split makes it easier
for reviewers to understand your work, and to verify that the code you're
submitting is correct.

A common request is to split single large patch into multiple patches. This can
happen, for example, if when adding a new feature you notice a bug elsewhere
which you need to fix to progress. Separating these changes into separate
commits will allow us to verify and land the bugfix quickly, pushing part of
your work for the good of everyone, whilst revision and discussion continues on
the larger feature part. It also allows us to direct you towards reviewers who
best understand the different areas you are working on.

When you have made any requested changes, please rebase the commits, verify
that they still individually look good, then force-push your new branch to
GitLab. This will update the merge request and notify everyone subscribed to
your merge request, so they can review it again.

There are also
[many GitLab CLI clients](https://about.gitlab.com/applications/#cli-clients),
if you prefer to avoid the web interface. It may be difficult to follow review
comments without using the web interface though, so we do recommend using this
to go through the review process, even if you use other clients to track the
list of available patches.


Coding style
------------

You should follow the style of the file you're editing. In general, we
try to follow the rules below.

**Note: this file uses spaces due to markdown rendering issues for tabs.
  Code must be implemented using tabs.**

- indent with tabs, and a tab is always 8 characters wide
- opening braces are on the same line as the if statement;
- no braces in an if-body with just one statement;
- if one of the branches of an if-else condition has braces, then the
  other branch should also have braces;
- there is always an empty line between variable declarations and the
  code;

```c
static int
my_function(void)
{
        int a = 0;

        if (a)
                b();
        else
                c();

        if (a) {
                b();
                c();
        } else {
                d();
        }
}
```

- lines should be less than 80 characters wide;
- when breaking lines with functions calls, the parameters are aligned
  with the opening parentheses;
- when assigning a variable with the result of a function call, if the
  line would be longer we break it around the equal '=' sign if it makes
  sense;

```c
        long_variable_name =
                function_with_a_really_long_name(parameter1, parameter2,
                                                 parameter3, parameter4);

        x = function_with_a_really_long_name(parameter1, parameter2,
                                             parameter3, parameter4);
```

Conduct
=======

As a freedesktop.org project, Wayland follows the Contributor Covenant,
found at:
https://www.freedesktop.org/wiki/CodeOfConduct

Please conduct yourself in a respectful and civilised manner when
interacting with community members on mailing lists, IRC, or bug
trackers. The community represents the project as a whole, and abusive
or bullying behaviour is not tolerated by the project.


Licensing
=========

Wayland is licensed with the intention to be usable anywhere X.org is.
Originally, X.org was covered under the MIT X11 license, but changed to
the MIT Expat license.  Similarly, Wayland was covered initially as MIT
X11 licensed, but changed to the MIT Expat license, following in X.org's
footsteps.  Other than wording, the two licenses are substantially the
same, with the exception of a no-advertising clause in X11 not included
in Expat.

New source code files should specify the MIT Expat license in their
boilerplate, as part of the copyright statement.


Review
======

All patches, even trivial ones, require at least one positive review
(Reviewed-by). Additionally, if no Reviewed-by's have been given by
people with commit access, there needs to be at least one Acked-by from
someone with commit access. A person with commit access is expected to be
able to evaluate the patch with respect to the project scope and architecture.

The below review guidelines are intended to be interpreted in spirit, not by
the letter. There may be circumstances where some guidelines are better
ignored. We rely very much on the judgement of reviewers and commit rights
holders.

During review, the following matters should be checked:

- The commit message explains why the change is being made.

- The code fits the project's scope.

- The code license is the same MIT licence the project generally uses.

- Stable ABI or API is not broken.

- Stable ABI or API additions must be justified by actual use cases, not only
by speculation. They must also be documented, and it is strongly recommended to
include tests exercising the additions in the test suite.

- The code fits the existing software architecture, e.g. no layering
violations.

- The code is correct and does not introduce new failures for existing users,
does not add new corner-case bugs, and does not introduce new compiler
warnings.

- The patch does what it says in the commit message and changes nothing else.

- The patch is a single logical change. If the commit message addresses
multiple points, it is a hint that the commit might need splitting up.

- A bug fix should target the underlying root cause instead of hiding symptoms.
If a complete fix is not practical, partial fixes are acceptable if they come
with code comments and filed Gitlab issues for the remaining bugs.

- The bug root cause rule applies to external software components as well, e.g.
do not work around kernel driver issues in userspace.

- The test suite passes.

- The code does not depend on API or ABI which has no working free open source
implementation.

- The code is not dead or untestable. E.g. if there are no free open source
software users for it then it is effectively dead code.

- The code is written to be easy to understand, or if code cannot be clear
enough on its own there are code comments to explain it.

- The code is minimal, i.e. prefer refactor and re-use when possible unless
clarity suffers.

- The code adheres to the style guidelines.

- In a patch series, every intermediate step adheres to the above guidelines.


Commit rights
=============

Commit rights will be granted to anyone who requests them and fulfills the
below criteria:

- Submitted some (10 as a rule of thumb) non-trivial (not just simple
  spelling fixes and whitespace adjustment) patches that have been merged
  already.

- Are actively participating in public discussions about their work (on the
  mailing list or IRC). This should not be interpreted as a requirement to
  review other peoples patches but just make sure that patch submission isn't
  one-way communication. Cross-review is still highly encouraged.

- Will be regularly contributing further patches. This includes regular
  contributors to other parts of the open source graphics stack who only
  do the occasional development in this project.

- Agrees to use their commit rights in accordance with the documented merge
  criteria, tools, and processes.

To apply for commit rights, create a new issue in gitlab for the respective
project and give it the "accounts" label.

Committers are encouraged to request their commit rights get removed when they
no longer contribute to the project. Commit rights will be reinstated when they
come back to the project.

Maintainers and committers should encourage contributors to request commit
rights, especially junior contributors tend to underestimate their skills.


Stabilising for releases
========================

A release cycle ends with a stable release which also starts a new cycle and
lifts any code freezes. Gradual code freezing towards a stable release starts
with an alpha release. The release stages of a cycle are:

- **Alpha release**:
    Signified by version number #.#.91.
    Major features must have landed before this. Major features include
    invasive code motion and refactoring, high risk changes, and new stable
    library ABI.

- **Beta release**:
    Signified by version number #.#.92.
    Minor features must have landed before this. Minor features include all
    new features that are not major, low risk changes, clean-ups, and
    documentation. Stable ABI that was new in the alpha release can be removed
    before a beta release if necessary.

- **Release candidates (RC)**:
    Signified by version number #.#.93 and up to #.#.99.
    Bug fixes that are not release critical must have landed before this.
    Release critical bug fixes can still be landed after this, but they may
    call for another RC.

- **Stable release**:
    Signified by version number #.#.0.
    Ideally no changes since the last RC.

Mind that version #.#.90 is never released. It is used during development when
no code freeze is in effect. Stable branches and point releases are not covered
by the above.


[git documentation]: http://git-scm.com/documentation
[notes on commit messages]: http://who-t.blogspot.de/2009/12/on-commit-messages.html
