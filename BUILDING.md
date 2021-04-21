# Building Helium for the Impatient

## Full/initial build
Building the whole system needs about 100 GB of free disk space and will take a _long_ time.

The build runs as a normal user whenever possible. However, some steps need `root` permissions, and the Makefile uses `doas` in those places. Setting up passwordless `doas` for the user that runs the build is highly recommended. (`doas` is similar to `sudo` and comes from OpenBSD. It is less complex and follows BSD design paradigms more than `sudo`. For convenience, Helium provides a `sudo` symlink to `doas` which will handle most daily needs. You can install the real `sudo` with `doas pkg install sudo` if you prefer.)

As of v0.1.1, Helium has a _self-hosted build_ -- i.e. it must be built on a running Helium system and will no longer compile on generic FreeBSD 12.

1. Install Helium using the M1 (0.1.1 or later) image
2. `doas pkg install git`
3. `git clone https://github.com/mszoek/helium.git`
4. `cd helium`
5. `doas sh install-dev-pkgs.sh`
6. `make world`
7. Have a drink and relax for a while. An installation CD (disc1.iso) will appear in `helium/dist` when the build finishes. 

## Incremental/update builds
If there are no changes to the BSD sources, subsequent builds are a lot faster as they do not build the BSD `world`. After making changes to the Helium components, a new CD image can be produced with just `make helium release`. This will build & package the Helium components, and generate a new install CD re-using the existing FreeBSD build objects. An incremental build of FreeBSD is also available as `make freebsd-noclean`. This can be followed by the previous command to quickly generate an updated install CD.

## Update a running system
It is also possible to update the running Helium system from the latest source. Running `make helium installhelium` from the git source tree will build just the Helium components into `helium.txz`, then install the archive onto the running system. This method is quicker but may not upgrade cleanly.

The only truly supported way to upgrade at present is to reinstall the system from the CD.
