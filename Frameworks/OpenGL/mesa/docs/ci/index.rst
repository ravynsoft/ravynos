Continuous Integration
======================

GitLab CI
---------

GitLab provides a convenient framework for running commands in response to Git pushes.
We use it to test merge requests (MRs) before merging them (pre-merge testing),
as well as post-merge testing, for everything that hits ``main``
(this is necessary because we still allow commits to be pushed outside of MRs,
and even then the MR CI runs in the forked repository, which might have been
modified and thus is unreliable).

The CI runs a number of tests, from trivial build-testing to complex GPU rendering:

- Build testing for a number of configurations and platforms
- Sanity checks (``meson test``)
- Most drivers are also tested using several test suites, such as the
  `Vulkan/GL/GLES conformance test suite <https://github.com/KhronosGroup/VK-GL-CTS>`__,
  `Piglit <https://gitlab.freedesktop.org/mesa/piglit>`__, and others.
- Replay of application traces

A typical run takes between 20 and 30 minutes, although it can go up very quickly
if the GitLab runners are overwhelmed, which happens sometimes. When it does happen,
not much can be done besides waiting it out, or cancel it.
You can do your part by only running the jobs you care about by using `our
tool <#running-specific-ci-jobs>`__.

Due to limited resources, we currently do not run the CI automatically
on every push; instead, we only run it automatically once the MR has
been assigned to ``Marge``, our merge bot.

If you're interested in the details, the main configuration file is ``.gitlab-ci.yml``,
and it references a number of other files in ``.gitlab-ci/``.

If the GitLab CI doesn't seem to be running on your fork (or MRs, as they run
in the context of your fork), you should check the "Settings" of your fork.
Under "CI / CD" → "General pipelines", make sure "Custom CI config path" is
empty (or set to the default ``.gitlab-ci.yml``), and that the
"Public pipelines" box is checked.

If you're having issues with the GitLab CI, your best bet is to ask
about it on ``#freedesktop`` on OFTC and tag `Daniel Stone
<https://gitlab.freedesktop.org/daniels>`__ (``daniels`` on IRC) or
`Emma Anholt <https://gitlab.freedesktop.org/anholt>`__ (``anholt`` on
IRC).

The three GitLab CI systems currently integrated are:


.. toctree::
   :maxdepth: 1

   bare-metal
   LAVA
   docker

Farm management
---------------

.. note::
   Never mix disabling/re-enabling a farm with any change that can affect a job
   that runs in another farm!

When the farm starts failing for any reason (power, network, out-of-space), it needs to be disabled by pushing separate MR with

.. code-block:: console

   git mv .ci-farms{,-disabled}/$farm_name

After farm restore functionality can be enabled by pushing a new merge request, which contains

.. code-block:: console

   git mv .ci-farms{-disabled,}/$farm_name

.. warning::
   Pushing (``git push``) directly to ``main`` is forbidden; this change must
   be sent as a :ref:`Merge Request <merging>`.

Application traces replay
-------------------------

The CI replays application traces with various drivers in two different jobs. The first
job replays traces listed in ``src/<driver>/ci/traces-<driver>.yml`` files and if any
of those traces fail the pipeline fails as well. The second job replays traces listed in
``src/<driver>/ci/restricted-traces-<driver>.yml`` and it is allowed to fail. This second
job is only created when the pipeline is triggered by ``marge-bot`` or any other user that
has been granted access to these traces.

A traces YAML file also includes a ``download-url`` pointing to a MinIO
instance where to download the traces from. While the first job should always work with
publicly accessible traces, the second job could point to an URL with restricted access.

Restricted traces are those that have been made available to Mesa developers without a
license to redistribute at will, and thus should not be exposed to the public. Failing to
access that URL would not prevent the pipeline to pass, therefore forks made by
contributors without permissions to download non-redistributable traces can be merged
without friction.

As an aside, only maintainers of such non-redistributable traces are responsible for
ensuring that replays are successful, since other contributors would not be able to
download and test them by themselves.

Those Mesa contributors that believe they could have permission to access such
non-redistributable traces can request permission to Daniel Stone <daniels@collabora.com>.

gitlab.freedesktop.org accounts that are to be granted access to these traces will be
added to the OPA policy for the MinIO repository as per
https://gitlab.freedesktop.org/freedesktop/helm-gitlab-infra/-/commit/a3cd632743019f68ac8a829267deb262d9670958 .

So the jobs are created in personal repositories, the name of the user's account needs
to be added to the rules attribute of the GitLab CI job that accesses the restricted
accounts.

.. toctree::
   :maxdepth: 1

   local-traces

Intel CI
--------

The Intel CI is not yet integrated into the GitLab CI.
For now, special access must be manually given (file a issue in
`the Intel CI configuration repo <https://gitlab.freedesktop.org/Mesa_CI/mesa_jenkins>`__
if you think you or Mesa would benefit from you having access to the Intel CI).
Results can be seen on `mesa-ci.01.org <https://mesa-ci.01.org>`__
if you are *not* an Intel employee, but if you are you
can access a better interface on
`mesa-ci-results.jf.intel.com <http://mesa-ci-results.jf.intel.com>`__.

The Intel CI runs a much larger array of tests, on a number of generations
of Intel hardware and on multiple platforms (X11, Wayland, DRM & Android),
with the purpose of detecting regressions.
Tests include
`Crucible <https://gitlab.freedesktop.org/mesa/crucible>`__,
`VK-GL-CTS <https://github.com/KhronosGroup/VK-GL-CTS>`__,
`dEQP <https://android.googlesource.com/platform/external/deqp>`__,
`Piglit <https://gitlab.freedesktop.org/mesa/piglit>`__,
`Skia <https://skia.googlesource.com/skia>`__,
`VkRunner <https://github.com/Igalia/vkrunner>`__,
`WebGL <https://github.com/KhronosGroup/WebGL>`__,
and a few other tools.
A typical run takes between 30 minutes and an hour.

If you're having issues with the Intel CI, your best bet is to ask about
it on ``#dri-devel`` on OFTC and tag `Nico Cortes
<https://gitlab.freedesktop.org/ngcortes>`__ (``ngcortes`` on IRC).

.. _CI-job-user-expectations:

CI job user expectations
------------------------

To make sure that testing of one vendor's drivers doesn't block
unrelated work by other vendors, we require that a given driver's test
farm produces a spurious failure no more than once a week.  If every
driver had CI and failed once a week, we would be seeing someone's
code getting blocked on a spurious failure daily, which is an
unacceptable cost to the project.

To ensure that, driver maintainers with CI enabled should watch the Flakes panel
of the `CI flakes dashboard
<https://ci-stats-grafana.freedesktop.org/d/Ae_TLIwVk/mesa-ci-quality-false-positives?orgId=1>`__,
particularly the "Flake jobs" pane, to inspect jobs in their driver where the
automatic retry of a failing job produced a success a second time.
Additionally, most CI reports test-level flakes to an IRC channel, and flakes
reported as NEW are not expected and could cause spurious failures in jobs.
Please track the NEW reports in jobs and add them as appropriate to the
``-flakes.txt`` file for your driver.

Additionally, the test farm needs to be able to provide a short enough
turnaround time that we can get our MRs through marge-bot without the pipeline
backing up.  As a result, we require that the test farm be able to handle a
whole pipeline's worth of jobs in less than 15 minutes (to compare, the build
stage is about 10 minutes).  Given boot times and intermittent network delays,
this generally means that the test runtime as reported by deqp-runner should be
kept to 10 minutes.

If a test farm is short the HW to provide these guarantees, consider dropping
tests to reduce runtime.  dEQP job logs print the slowest tests at the end of
the run, and Piglit logs the runtime of tests in the results.json.bz2 in the
artifacts.  Or, you can add the following to your job to only run some fraction
(in this case, 1/10th) of the dEQP tests.

.. code-block:: yaml

   variables:
      DEQP_FRACTION: 10

to just run 1/10th of the test list.

For Collabora's LAVA farm, the `device types
<https://lava.collabora.dev/scheduler/device_types>`__ page can tell you how
many boards of a specific tag are currently available by adding the "Idle" and
"Busy" columns.  For bare-metal, a gitlab admin can look at the `runners
<https://gitlab.freedesktop.org/admin/runners>`__ page.  A pipeline should
probably not create more jobs for a board type than there are boards, unless you
clearly have some short-runtime jobs.

If a HW CI farm goes offline (network dies and all CI pipelines end up
stalled) or its runners are consistently spuriously failing (disk
full?), and the maintainer is not immediately available to fix the
issue, please push through an MR disabling that farm's jobs according
to the `Farm Management <#farm-management>`__ instructions.

Personal runners
----------------

Mesa's CI is currently run primarily on packet.net's m1xlarge nodes
(2.2Ghz Sandy Bridge), with each job getting 8 cores allocated.  You
can speed up your personal CI builds (and marge-bot merges) by using a
faster personal machine as a runner.  You can find the gitlab-runner
package in Debian, or use GitLab's own builds.

To do so, follow `GitLab's instructions
<https://docs.gitlab.com/ee/ci/runners/runners_scope.html#create-a-project-runner-with-a-runner-authentication-token>`__
to register your personal GitLab runner in your Mesa fork.  Then, tell
Mesa how many jobs it should serve (``concurrent=``) and how many
cores those jobs should use (``FDO_CI_CONCURRENT=``) by editing these
lines in ``/etc/gitlab-runner/config.toml``, for example:

.. code-block:: toml

   concurrent = 2

   [[runners]]
     environment = ["FDO_CI_CONCURRENT=16"]


Docker caching
--------------

The CI system uses Docker images extensively to cache
infrequently-updated build content like the CTS.  The `freedesktop.org
CI templates
<https://gitlab.freedesktop.org/freedesktop/ci-templates/>`__ help us
manage the building of the images to reduce how frequently rebuilds
happen, and trim down the images (stripping out manpages, cleaning the
apt cache, and other such common pitfalls of building Docker images).

When running a container job, the templates will look for an existing
build of that image in the container registry under
``MESA_IMAGE_TAG``.  If it's found it will be reused, and if
not, the associated ``.gitlab-ci/containers/<jobname>.sh`` will be run
to build it.  So, when developing any change to container build
scripts, you need to update the associated ``MESA_IMAGE_TAG`` to
a new unique string.  We recommend using the current date plus some
string related to your branch (so that if you rebase on someone else's
container update from the same day, you will get a Git conflict
instead of silently reusing their container)

When developing a given change to your Docker image, you would have to
bump the tag on each ``git commit --amend`` to your development
branch, which can get tedious.  Instead, you can navigate to the
`container registry
<https://gitlab.freedesktop.org/mesa/mesa/container_registry>`__ for
your repository and delete the tag to force a rebuild.  When your code
is eventually merged to main, a full image rebuild will occur again
(forks inherit images from the main repo, but MRs don't propagate
images from the fork into the main repo's registry).

Building locally using CI docker images
---------------------------------------

It can be frustrating to debug build failures on an environment you
don't personally have.  If you're experiencing this with the CI
builds, you can use Docker to use their build environment locally.  Go
to your job log, and at the top you'll see a line like::

   Pulling docker image registry.freedesktop.org/anholt/mesa/debian/android_build:2020-09-11

We'll use a volume mount to make our current Mesa tree be what the
Docker container uses, so they'll share everything (their build will
go in _build, according to ``meson-build.sh``).  We're going to be
using the image non-interactively so we use ``run --rm $IMAGE
command`` instead of ``run -it $IMAGE bash`` (which you may also find
useful for debug).  Extract your build setup variables from
.gitlab-ci.yml and run the CI meson build script:

.. code-block:: console

   IMAGE=registry.freedesktop.org/anholt/mesa/debian/android_build:2020-09-11
   sudo docker pull $IMAGE
   sudo docker run --rm -v `pwd`:/mesa -w /mesa $IMAGE env PKG_CONFIG_PATH=/usr/local/lib/aarch64-linux-android/pkgconfig/:/android-ndk-r21d/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/aarch64-linux-android/pkgconfig/ GALLIUM_DRIVERS=freedreno UNWIND=disabled EXTRA_OPTION="-D android-stub=true -D llvm=disabled" DRI_LOADERS="-D glx=disabled -D gbm=disabled -D egl=enabled -D platforms=android" CROSS=aarch64-linux-android ./.gitlab-ci/meson-build.sh

All you have left over from the build is its output, and a _build
directory.  You can hack on mesa and iterate testing the build with:

.. code-block:: console

   sudo docker run --rm -v `pwd`:/mesa $IMAGE meson compile -C /mesa/_build

Running specific CI jobs
------------------------

You can use ``bin/ci/ci_run_n_monitor.py`` to run specific CI jobs. It
will automatically take care of running all the jobs yours depends on,
and cancel the rest to avoid wasting resources.

See ``bin/ci/ci_run_n_monitor.py --help`` for all the options.

The ``--target`` argument takes a regex that you can use to select the
jobs names you want to run, eg. ``--target 'zink.*'`` will run all the
zink jobs, leaving the other drivers' jobs free for others to use.

Conformance Tests
-----------------

Some conformance tests require a special treatment to be maintained on GitLab CI.
This section lists their documentation pages.

.. toctree::
  :maxdepth: 1

  skqp


Updating GitLab CI Linux Kernel
-------------------------------

GitLab CI usually runs a bleeding-edge kernel. The following documentation has
instructions on how to uprev Linux Kernel in the GitLab CI ecosystem.

.. toctree::
  :maxdepth: 1

  kernel


Reusing CI scripts for other projects
--------------------------------------

The CI scripts in ``.gitlab-ci/`` can be reused for other projects, to
facilitate reuse of the infrastructure, our scripts can be used as tools
to create containers and run tests on the available farms.

.. envvar:: EXTRA_LOCAL_PACKAGES

   Define extra Debian packages to be installed in the container.
