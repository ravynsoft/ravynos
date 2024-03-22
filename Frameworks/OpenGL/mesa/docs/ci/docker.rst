Docker CI
=========

For LLVMpipe and Softpipe CI, we run tests in a container containing
VK-GL-CTS, on the shared GitLab runners provided by `freedesktop
<https://www.freedesktop.org>`__

Software architecture
---------------------

The Docker containers are rebuilt using the shell scripts under
.gitlab-ci/container/ when the FDO\_DISTRIBUTION\_TAG changes in
.gitlab-ci.yml. The resulting images are around 1 GB, and are
expected to change approximately weekly (though an individual
developer working on them may produce many more images while trying to
come up with a working MR!).

gitlab-runner is a client that polls gitlab.freedesktop.org for
available jobs, with no inbound networking requirements.  Jobs can
have tags, so we can have DUT-specific jobs that only run on runners
with that tag marked in the GitLab UI.

Since dEQP takes a long time to run, we mark the job as "parallel" at
some level, which spawns multiple jobs from one definition, and then
deqp-runner.sh takes the corresponding fraction of the test list for
that job.

To reduce dEQP runtime (or avoid tests with unreliable results), a
deqp-runner.sh invocation can provide a list of tests to skip.  If
your driver is not yet conformant, you can pass a list of expected
failures, and the job will only fail on tests that aren't listed (look
at the job's log for which specific tests failed).

DUT requirements
----------------

In addition to the general :ref:`CI-job-user-expectations`, using
Docker requires:

* DUTs must have a stable kernel and GPU reset (if applicable).

If the system goes down during a test run, that job will eventually
time out and fail (default 1 hour).  However, if the kernel can't
reliably reset the GPU on failure, bugs in one MR may leak into
spurious failures in another MR.  This would be an unacceptable impact
on Mesa developers working on other drivers.

* DUTs must be able to run Docker

The Mesa gitlab-runner based test architecture is built around Docker,
so that we can cache the Debian package installation and CTS build
step across multiple test runs.  Since the images are large and change
approximately weekly, the DUTs also need to be running some script to
prune stale Docker images periodically in order to not run out of disk
space as we rev those containers (perhaps `this script
<https://gitlab.com/gitlab-org/gitlab-runner/-/issues/2980#note_169233611>`__).

Note that Docker doesn't allow containers to be stored on NFS, and
doesn't allow multiple Docker daemons to interact with the same
network block device, so you will probably need some sort of physical
storage on your DUTs.

* DUTs must be public

By including your device in .gitlab-ci.yml, you're effectively letting
anyone on the internet run code on your device.  Docker containers may
provide some limited protection, but how much you trust that and what
you do to mitigate hostile access is up to you.

* DUTs must expose the DRI device nodes to the containers.

Obviously, to get access to the HW, we need to pass the render node
through.  This is done by adding ``devices = ["/dev/dri"]`` to the
``runners.docker`` section of /etc/gitlab-runner/config.toml.
