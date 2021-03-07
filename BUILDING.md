# Building Helium for the Impatient

Building the whole system needs about 100 GB of free disk space.

The build runs as a normal user whenever possible. However, some steps need `root` permissions, and the Makefile uses `sudo` in those places. Setting up passwordless sudo for the user that runs the build is highly recommended.

1. Install FreeBSD 12.2 somewhere. This is your Helium build system.
2. `pkg update`
3. `pkg install git`
4. `git clone https://github.com/mszoek/helium.git`
5. `cd helium`
6. `make world`
7. Have a drink and relax for a while. An installation CD (disc1.iso) will appear in `helium/dist` when the build finishes. 

