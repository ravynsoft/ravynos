# Building Airyx for the Impatient

## Full/initial build
Building the whole system needs about 100 GB of free disk space and will take a _long_ time.

The build runs as a normal user whenever possible. However, some steps need `root` permissions, and the Makefile uses `sudo` in those places. Setting up passwordless `sudo` for the user that runs the build is highly recommended. 

As of v0.1.1, Airyx has a _self-hosted build_ -- i.e. it must be built on a running Airyx system and will no longer compile on generic FreeBSD 12. This is (in part) because Airyx patches the `clang` compiler toolchain to support Framework bundles, which are needed to build Airyx.

1. Install Airyx using the Atomic (0.2.X) or later image
2. `sudo pkg install git`
3. `git clone https://github.com/mszoek/airyx.git`
4. Set up your username and email for `git commit`
 * `git config --global user.email you@your.tld`
 * `git config --global user.name 'Your Name'`
5. `cd airyx`
6. `sudo sh install-dev-pkgs.sh`
7. `make world`
8. Have a drink and relax for a while. An installation ISO will appear in `airyx/dist` when the build finishes. 

## Incremental/update builds
If there are no changes to the BSD sources, subsequent builds are a lot faster as they do not build the BSD `world`. After making changes to the Airyx components, a new CD image can be produced with just `make airyx release`. This will build & package the Airyx components, and generate a new install CD re-using the existing FreeBSD build objects. An incremental build of FreeBSD is also available as `make freebsd-noclean`. This can be followed by the previous command to quickly generate an updated install CD. See the `Makefile` for additional targets.

## Update a running system
It is also possible to update the running Airyx system from the latest source. Running `make airyx installairyx` from the git source tree will build just the Airyx components into `airyx.txz`, then install the archive onto the running system. This method is quicker but may not upgrade cleanly.

The only truly supported way to upgrade at present is to reinstall the system from the CD. Proper upgrades are on my to-do list :)
