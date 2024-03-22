Source Code Repository
======================

Mesa uses `Git <https://git-scm.com>`__ as its source code management
system.

The upstream Git repository is hosted on
`freedesktop.org <https://www.freedesktop.org>`__.

You may access the repository either as an :ref:`anonymous
user <anonymous>` (read-only) or as a :ref:`developer <developer>`
(read/write).

You may also `browse the main Mesa Git
repository <https://gitlab.freedesktop.org/mesa/mesa>`__ and the `Mesa
demos and tests Git
repository <https://gitlab.freedesktop.org/mesa/demos>`__.

.. _anonymous:

Anonymous Git Access
--------------------

To get the Mesa sources anonymously (read-only):

#. Install the Git software on your computer if needed.
#. Get an initial, local copy of the repository with:

   .. code-block:: console

      git clone https://gitlab.freedesktop.org/mesa/mesa.git

#. Later, you can update your tree from the upstream repository with:

   .. code-block:: console

      git pull origin

#. If you also want the Mesa demos/tests repository:

   .. code-block:: console

      git clone https://gitlab.freedesktop.org/mesa/demos.git

.. _developer:

Developer Git Access
--------------------

If you wish to become a Mesa developer with GitLab merge privilege,
please follow this procedure:

#. Subscribe to the
   `mesa-dev <https://lists.freedesktop.org/mailman/listinfo/mesa-dev>`__
   mailing list.
#. Start contributing to the project by :doc:`submitting
   patches <submittingpatches>`. Specifically,

   -  Use `GitLab <https://gitlab.freedesktop.org/>`__ to create your
      merge requests.
   -  Wait for someone to review the code and give you a ``Reviewed-by``
      statement.
   -  You'll have to rely on another Mesa developer to push your initial
      patches after they've been reviewed.

#. After you've demonstrated the ability to write good code and have had
   a dozen or so patches accepted, a maintainer may use their discretion
   to give you access to merge your own code.

Pushing code to your GitLab account via HTTPS
---------------------------------------------

Useful for people behind strict proxies

You can use `personal access
tokens <https://gitlab.freedesktop.org/profile/personal_access_tokens>`__
to push over HTTPS if ssh does not suit your needs. In this case, create
a token, and put it in the URL as shown here:

.. code-block:: console

   git remote set-url --push origin https://USER:TOKEN@gitlab.freedesktop.org/your~user~name/mesa.git

Windows Users
-------------

If you're `using Git on
Windows <https://git-scm.com/book/en/v2/Getting-Started-Installing-Git#_installing_on_windows>`__
you'll want to enable automatic CR/LF conversion in your local copy of
the repository:

.. code-block:: console

   git config --global core.autocrlf true

This will cause Git to convert all text files to CR+LF on checkout, and
to LF on commit.

Unix users don't need to set this option.

Development Branches
--------------------

At any given time, there may be several active branches in Mesa's
repository. Generally, ``main`` contains the latest development
(unstable) code while a branch has the latest stable code.

The command ``git branch`` will list all available branches.

Questions about branch status/activity should be posted to the mesa-dev
mailing list.

Developer Git Tips
------------------

#. Setting up to edit the main branch

   If you try to do a pull by just saying\ ``git pull`` and Git
   complains that you have not specified a branch, try:

   .. code-block:: console

      git config branch.main.remote origin
      git config branch.main.merge main

   Otherwise, you have to say\ ``git pull origin main`` each time you
   do a pull.

#. Small changes to main

   If you are an experienced Git user working on substantial
   modifications, you are probably working on a separate branch and
   would rebase your branch prior to merging with main. But for small
   changes to the main branch itself, you also need to use the rebase
   feature in order to avoid an unnecessary and distracting branch in
   main.

   If it has been awhile since you've done the initial clone, try

   .. code-block:: console

      git pull

   to get the latest files before you start working.

   Make your changes and use

   .. code-block:: console

      git add <files to commit>
      git commit

   to get your changes ready to push back into the freedesktop.org
   repository.

   It is possible (and likely) that someone has changed main since you
   did your last pull. Even if your changes do not conflict with their
   changes, Git will make a fast-forward merge branch, branching from
   the point in time where you did your last pull and merging it to a
   point after the other changes.

   To avoid this,

   .. code-block:: console

      git pull --rebase
      git push

   If you are familiar with CVS or similar system, this is similar to
   doing a ``cvs update`` in order to update your source tree to the
   current repository state, instead of the time you did the last
   update. (CVS doesn't work like Git in this respect, but this is
   easiest way to explain it.)

   In any case, your repository now looks like you made your changes
   after all the other changes.

   If the rebase resulted in conflicts or changes that could affect the
   proper operation of your changes, you'll need to investigate those
   before doing the push.

   If you want the rebase action to be the default action, then

   .. code-block:: console

      git config branch.main.rebase true
      git config --global branch.autosetuprebase=always

   See `Understanding Git
   Conceptually <https://www.cduan.com/technical/git/>`__
   for a fairly clear explanation about all of this.
