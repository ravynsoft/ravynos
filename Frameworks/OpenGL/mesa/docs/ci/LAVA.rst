LAVA CI
=======

`LAVA <https://www.lavasoftware.org/>`__ is a system for functional
testing of boards including deploying custom bootloaders and kernels.
This is particularly relevant to testing Mesa because we often need
to change kernels for UAPI changes (and this lets us do full testing
of a new kernel during development), and our workloads can easily
take down boards when mistakes are made (kernel oopses, OOMs that
take out critical system services).

Available LAVA labs
-------------------
- Collabora `[dashboard] <https://lava.collabora.dev/scheduler/device_types>`__ (without authentication only health check jobs are displayed)
- Lima [dashboard not available]

Mesa-LAVA software architecture
-------------------------------

The gitlab-runner will run on some host that has access to the LAVA
lab, with tags like "mesa-ci-x86-64-lava-$DEVICE_TYPE" to control only
taking in jobs for the hardware that the LAVA lab contains.  The
gitlab-runner spawns a Docker container with lavacli in it, and
connects to the LAVA lab using a predefined token to submit jobs under
a specific device type.

The LAVA instance manages scheduling those jobs to the boards present.
For a job, it will deploy the kernel, device tree, and the ramdisk
containing the CTS.

Deploying a new Mesa-LAVA lab
-----------------------------

You'll want to start with setting up your LAVA instance and getting
some boards booting using test jobs.  Start with the stock QEMU
examples to make sure your instance works at all.  Then, you'll need
to define your actual boards.

The device type in lava-gitlab-ci.yml is the device type you create in
your LAVA instance, which doesn't have to match the board's name in
``/etc/lava-dispatcher/device-types``.  You create your boards under
that device type and the Mesa jobs will be scheduled to any of them.
Instantiate your boards by creating them in the UI or at the command
line attached to that device type, then populate their dictionary
(using an "extends" line probably referencing the board's template in
``/etc/lava-dispatcher/device-types``).  Now, go find a relevant
health check job for your board as a test job definition, or cobble
something together from a board that boots using the same boot_method
and some public images, and figure out how to get your boards booting.

Once you can boot your board using a custom job definition, it's time
to connect Mesa CI to it.  Install gitlab-runner and register as a
shared runner (you'll need a GitLab admin for help with this).  The
runner *must* have a tag (like "mesa-ci-x86-64-lava-rk3399-gru-kevin")
to restrict the jobs it takes or it will grab random jobs from tasks
across ``gitlab.freedesktop.org``, and your runner isn't ready for
that.

The Docker image will need access to the LAVA instance.  If it's on a
public network it should be fine.  If you're running the LAVA instance
on localhost, you'll need to set ``network_mode="host"`` in
``/etc/gitlab-runner/config.toml`` so it can access localhost.  Create a
gitlab-runner user in your LAVA instance, log in under that user on
the web interface, and create an API token.  Copy that into a
``lavacli.yaml``:

.. code-block:: yaml

   default:
      token: <token contents>
      uri: <URL to the instance>
      username: gitlab-runner

Add a volume mount of that ``lavacli.yaml`` to
``/etc/gitlab-runner/config.toml`` so that the Docker container can
access it.  You probably have a ``volumes = ["/cache"]`` already, so now it would be::

   volumes = ["/home/anholt/lava-config/lavacli.yaml:/root/.config/lavacli.yaml", "/cache"]

Note that this token is visible to anybody that can submit MRs to
Mesa!  It is not an actual secret.  We could just bake it into the
GitLab CI YAML, but this way the current method of connecting to the
LAVA instance is separated from the Mesa branches (particularly
relevant as we have many stable branches all using CI).

Now it's time to define your test jobs in the driver-specific
gitlab-ci.yml file, using the device-specific tags.
