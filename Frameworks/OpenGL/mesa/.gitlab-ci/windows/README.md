# Native Windows GitLab CI builds

Unlike Linux, Windows cannot reuse the freedesktop ci-templates as they exist
as we do not have Podman, Skopeo, or even Docker-in-Docker builds available
under Windows.

We still reuse the same model: build a base container with the core operating
system and infrequently-changed build dependencies, then execute Mesa builds
only inside that base container. This is open-coded in PowerShell scripts.

## Base container build

The base container build job executes the `mesa_container.ps1` script which
reproduces the ci-templates behaviour. It looks for the registry image in
the user's namespace, and exits if found. If not found, it tries to copy
the same image tag from the upstream Mesa repository. If that is not found,
the image is rebuilt inside the user's namespace.

The rebuild executes `docker build` which calls `mesa_deps.ps1` inside the
container to fetch and install all build dependencies. This includes Visual
Studio Community Edition (downloaded from Microsoft, under the license which
allows use by open-source projects), other build tools from Chocolatey, and
finally Meson and Python dependencies from PyPI.

This job is executed inside a Windows shell environment directly inside the
host, without Docker.

## Mesa build

The Mesa build runs inside the base container, executing `mesa_build.ps1`.
This simply compiles Mesa using Meson and Ninja, executing the build and
unit tests. Currently, no build artifacts are captured.

## Using build scripts locally

`*.ps1` scripts for building dockers are using PowerShell 7 to run
