# Building Helium for the Impatient

## Full/initial build
Building the whole system needs about 100 GB of free disk space and will take a _long_ time.

The build runs as a normal user whenever possible. However, some steps need `root` permissions, and the Makefile uses `sudo` in those places. Setting up passwordless sudo for the user that runs the build is highly recommended.

1. Install FreeBSD 12.2 somewhere. This is your Helium build system.
2. `pkg update`
3. `pkg install git cmake`
4. `git clone https://github.com/mszoek/helium.git`
5. `cd helium`
6. `make world`
7. Have a drink and relax for a while. An installation CD (disc1.iso) will appear in `helium/dist` when the build finishes. 

## Incremental/update builds
If there are no changes to the BSD sources, subsequent builds are a lot faster as they do not build the BSD `world`. After making changes to the Helium components, a new CD image can be produced with just `make helium release`. This will build & package the Helium components, and generate a new install CD re-using the existing FreeBSD build objects. An incremental build of FreeBSD is also available as `make freebsd-noclean`. This can be followed by the previous command to quickly generate an updated install CD.

## Update a running system
It is also possible to overlay Helium onto a running FreeBSD 12.2 system. The commands below will build just the Helium components into `helium.txz`, then install the archive onto the running system. This method is quicker but may not upgrade cleanly.

1. Install FreBSD 12.2 somewhere. This will be your Helium system.
2. `pkg update && pkg install git cmake`
3. `git clone https://github.com/mszoek/helium.git`
4. `cd helium`
5. `make helium installhelium`