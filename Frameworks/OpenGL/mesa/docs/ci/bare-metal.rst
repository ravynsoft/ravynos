Bare-metal CI
=============

The bare-metal scripts run on a system with gitlab-runner and Docker,
connected to potentially multiple bare-metal boards that run tests of
Mesa.  Currently "fastboot", "ChromeOS Servo", and POE-powered devices are
supported.

In comparison with LAVA, this doesn't involve maintaining a separate
web service with its own job scheduler and replicating jobs between the
two.  It also places more of the board support in Git, instead of
web service configuration.  On the other hand, the serial interactions
and bootloader support are more primitive.

Requirements (fastboot)
-----------------------

This testing requires power control of the DUTs by the gitlab-runner
machine, since this is what we use to reset the system and get back to
a pristine state at the start of testing.

We require access to the console output from the gitlab-runner system,
since that is how we get the final results back from the tests.  You
should probably have the console on a serial connection, so that you
can see bootloader progress.

The boards need to be able to have a kernel/initramfs supplied by the
gitlab-runner system, since Mesa often needs to update the kernel either for new
DRM functionality, or to fix kernel bugs.

The boards must have networking, so that we can extract the dEQP XML results to
artifacts on GitLab, and so that we can download traces (too large for an
initramfs) for trace replay testing.  Given that we need networking already, and
our dEQP/Piglit/etc. payload is large, we use NFS from the x86 runner system
rather than initramfs.

See ``src/freedreno/ci/gitlab-ci.yml`` for an example of fastboot on DB410c and
DB820c (freedreno-a306 and freedreno-a530).

Requirements (Servo)
--------------------

For Servo-connected boards, we can use the EC connection for power
control to reboot the board.  However, loading a kernel is not as easy
as fastboot, so we assume your bootloader can do TFTP, and that your
gitlab-runner mounts the runner's tftp directory specific to the board
at /tftp in the container.

Since we're going the TFTP route, we also use NFS root.  This avoids
packing the rootfs and sending it to the board as a ramdisk, which
means we can support larger rootfses (for Piglit testing), at the cost
of needing more storage on the runner.

Telling the board about where its TFTP and NFS should come from is
done using dnsmasq on the runner host.  For example, this snippet in
the dnsmasq.conf.d in the google farm, with the gitlab-runner host we
call "servo"::

   dhcp-host=1c:69:7a:0d:a3:d3,10.42.0.10,set:servo

   # Fixed dhcp addresses for my sanity, and setting a tag for
   # specializing other DHCP options
   dhcp-host=a0:ce:c8:c8:d9:5d,10.42.0.11,set:cheza1
   dhcp-host=a0:ce:c8:c8:d8:81,10.42.0.12,set:cheza2

   # Specify the next server, watch out for the double ',,'.  The
   # filename didn't seem to get picked up by the bootloader, so we use
   # tftp-unique-root and mount directories like
   # /srv/tftp/10.42.0.11/jwerner/cheza as /tftp in the job containers.
   tftp-unique-root
   dhcp-boot=tag:cheza1,cheza1/vmlinuz,,10.42.0.10
   dhcp-boot=tag:cheza2,cheza2/vmlinuz,,10.42.0.10

   dhcp-option=tag:cheza1,option:root-path,/srv/nfs/cheza1
   dhcp-option=tag:cheza2,option:root-path,/srv/nfs/cheza2

See ``src/freedreno/ci/gitlab-ci.yml`` for an example of Servo on cheza.  Note
that other Servo boards in CI are managed using LAVA.

Requirements (POE)
------------------

For boards with 30W or less power consumption, POE can be used for the power
control.  The parts list ends up looking something like (for example):

- x86-64 gitlab-runner machine with a mid-range CPU, and 3+ GB of SSD storage
  per board.  This can host at least 15 boards in our experience.
- Cisco 2960S gigabit ethernet switch with POE. (Cisco 3750G, 3560G, or 2960G
  were also recommended as reasonable-priced HW, but make sure the name ends in
  G, X, or S)
- POE splitters to power the boards (you can find ones that go to micro USB,
  USBC, and 5V barrel jacks at least)
- USB serial cables (Adafruit sells pretty reliable ones)
- A large powered USB hub for all the serial cables
- A pile of ethernet cables

You'll talk to the Cisco for configuration using its USB port, which provides a
serial terminal at 9600 baud.  You need to enable SNMP control, which we'll do
using a "mesaci" community name that the gitlab runner can access as its
authentication (no password) to configure.  To talk to the SNMP on the router,
you need to put an IP address on the default VLAN (VLAN 1).

Setting that up looks something like:

.. code-block: console

   Switch>
   Password:
   Switch#configure terminal
   Switch(config)#interface Vlan 1
   Switch(config-if)#ip address 10.42.0.2 255.255.0.0
   Switch(config-if)#end
   Switch(config)#snmp-server community mesaci RW
   Switch(config)#end
   Switch#copy running-config startup-config

With that set up, you should be able to power on/off a port with something like:

.. code-block: console

   % snmpset -v2c -r 3 -t 30 -cmesaci 10.42.0.2 1.3.6.1.4.1.9.9.402.1.2.1.1.1.1 i 1
   % snmpset -v2c -r 3 -t 30 -cmesaci 10.42.0.2 1.3.6.1.4.1.9.9.402.1.2.1.1.1.1 i 4

Note that the "1.3.6..." SNMP OID changes between switches.  The last digit
above is the interface id (port number).  You can probably find the right OID by
google, that was easier than figuring it out from finding the switch's MIB
database.  You can query the POE status from the switch serial using the ``show
power inline`` command.

Other than that, find the dnsmasq/tftp/NFS setup for your boards "servo" above.

See ``src/broadcom/ci/gitlab-ci.yml`` and ``src/nouveau/ci/gitlab-ci.yml`` for an
examples of POE for Raspberry Pi 3/4, and Jetson Nano.

Setup
-----

Each board will be registered in freedesktop.org GitLab.  You'll want
something like this to register a fastboot board:

.. code-block:: console

   sudo gitlab-runner register \
        --url https://gitlab.freedesktop.org \
        --registration-token $1 \
        --name MY_BOARD_NAME \
        --tag-list MY_BOARD_TAG \
        --executor docker \
        --docker-image "alpine:latest" \
        --docker-volumes "/dev:/dev" \
        --docker-network-mode "host" \
        --docker-privileged \
        --non-interactive

For a Servo board, you'll need to also volume mount the board's NFS
root dir at /nfs and TFTP kernel directory at /tftp.

The registration token has to come from a freedesktop.org GitLab admin
going to https://gitlab.freedesktop.org/admin/runners

The name scheme for Google's lab is google-freedreno-boardname-n, and
our tag is something like google-freedreno-db410c.  The tag is what
identifies a board type so that board-specific jobs can be dispatched
into that pool.

We need privileged mode and the /dev bind mount in order to get at the
serial console and fastboot USB devices (--device arguments don't
apply to devices that show up after container start, which is the case
with fastboot, and the Servo serial devices are actually links to
/dev/pts).  We use host network mode so that we can spin up a nginx
server to collect XML results for fastboot.

Once you've added your boards, you're going to need to add a little
more customization in ``/etc/gitlab-runner/config.toml``.  First, add
``concurrent = <number of boards>`` at the top ("we should have up to
this many jobs running managed by this gitlab-runner").  Then for each
board's runner, set ``limit = 1`` ("only 1 job served by this board at a
time").  Finally, add the board-specific environment variables
required by your bare-metal script, something like::

   [[runners]]
     name = "google-freedreno-db410c-1"
     environment = ["BM_SERIAL=/dev/ttyDB410c8", "BM_POWERUP=google-power-up.sh 8", "BM_FASTBOOT_SERIAL=15e9e390", "FDO_CI_CONCURRENT=4"]

The ``FDO_CI_CONCURRENT`` variable should be set to the number of CPU threads on
the board, which is used for auto-tuning of job parallelism.

Once you've updated your runners' configs, restart with ``sudo service
gitlab-runner restart``

Caching downloads
-----------------

To improve the runtime for downloading traces during traces job runs, you will
want a pass-through HTTP cache.  On your runner box, install nginx:

.. code-block:: console

   sudo apt install nginx libnginx-mod-http-lua

Add the server setup files:

.. literalinclude:: fdo-cache
   :name: /etc/nginx/sites-available/fdo-cache
   :caption: /etc/nginx/sites-available/fdo-cache

.. literalinclude:: uri-caching.conf
   :name: /etc/nginx/snippets/uri-caching.conf
   :caption: /etc/nginx/snippets/uri-caching.conf

Edit the listener addresses in fdo-cache to suit the ethernet interface that
your devices are on.

Enable the site and restart nginx:

.. code-block:: console

   sudo rm /etc/nginx/sites-enabled/default
   sudo ln -s /etc/nginx/sites-available/fdo-cache /etc/nginx/sites-enabled/fdo-cache
   sudo systemctl restart nginx

   # First download will hit the internet
   wget http://localhost/cache/?uri=https://s3.freedesktop.org/mesa-tracie-public/itoral-gl-terrain-demo/demo-v2.trace
   # Second download should be cached.
   wget http://localhost/cache/?uri=https://s3.freedesktop.org/mesa-tracie-public/itoral-gl-terrain-demo/demo-v2.trace

Now, set ``download-url`` in your ``traces-*.yml`` entry to something like
``http://caching-proxy/cache/?uri=https://s3.freedesktop.org/mesa-tracie-public``
and you should have cached downloads for traces.  Add it to
``FDO_HTTP_CACHE_URI=`` in your ``config.toml`` runner environment lines and you
can use it for cached artifact downloads instead of going all the way to
freedesktop.org on each job.
