Rusticl
=======

Enabling
--------

In order to use Rusticl on any platform the environment variable
:envvar:`RUSTICL_ENABLE` has to be used. Rusticl does not advertise devices
for any driver by default yet as doing so can impact system stability until
remaining core issues are ironed out.

Building
--------

To build Rusticl you need to satisfy the following build dependencies:

-  ``rustc``
-  ``rustfmt`` (highly recommended, but only *required* for CI builds
   or when authoring patches)
-  ``bindgen``
-  `LLVM <https://github.com/llvm/llvm-project/>`__ built with
   ``libclc`` and ``-DLLVM_ENABLE_DUMP=ON``.
-  `SPIRV-Tools <https://github.com/KhronosGroup/SPIRV-Tools>`__
-  `SPIRV-LLVM-Translator
   <https://github.com/KhronosGroup/SPIRV-LLVM-Translator>`__ for a
   ``libLLVMSPIRVLib.so`` matching your version of LLVM, i.e. if you're
   using LLVM 15 (``libLLVM.so.15``), then you need a
   ``libLLVMSPIRVLib.so.15``.

The minimum versions to build Rusticl are:

-  Rust: 1.66
-  Meson: 1.3.1
-  Bindgen: 0.62.0
-  LLVM: 11.0.0 (recommended: 15.0.0)
-  Clang: 11.0.0 (recommended: 15.0.0)
   Updating clang requires a rebuilt of mesa and rusticl if and only if the value of
   ``CLANG_RESOURCE_DIR`` changes. It is defined through ``clang/Config/config.h``.
-  SPIRV-Tools: any version (recommended: v2022.3)

Afterwards you only need to add ``-Dgallium-rusticl=true -Dllvm=enabled
-Drust_std=2021`` to your build options.

Most of the code related to Mesa's C code lives inside ``/mesa``, with
the occasional use of enums, structs or constants through the code base.

If you need help ping ``karolherbst`` either in ``#dri-devel`` or
``#rusticl`` on OFTC.

Rust Update Policy
------------------

Given that for some distributions it's not feasible to keep up with the
pace of Rust, we promise to only bump the minimum required Rust version
following those rules:

-  Only up to the Rust requirement of other major Linux desktop
   components, e.g.:

   -  `Firefox ESR <https://whattrainisitnow.com/release/?version=esr>`__:
      `Minimum Supported Rust Version:
      <https://firefox-source-docs.mozilla.org/writing-rust-code/update-policy.html#schedule>`__

   -  latest `Linux Kernel Rust requirement
      <https://docs.kernel.org/process/changes.html#current-minimal-requirements>`__

-  Only require a newer Rust version than stated by other rules if and only
   if it's required to get around a bug inside rustc.

As bug fixes might run into rustc compiler bugs, a rust version bump _can_
happen on a stable branch as well.

Contributing 
------------

The minimum configuration you need to start developing with rust
is ``RUSTC=clippy-driver meson configure -Dgallium-rusticl=true
-Dllvm=enabled -Drust_std=2021``. In addition you probably want to enable
any device drivers on your platform. Some device drivers as well as some
features are locked behind flags during runtime. See
:ref:`Rusticl environment variables <rusticl-env-var>` for
more info.

All patches that are potentially conformance breaking and also patches
that add new features should be ran against the appropriate conformance
tests.

Also, make sure the formatting is in order before submitting code. That
can easily be done via ``git ls-files */{lib,main}.rs | xargs rustfmt``.

When submitting Merge Requests or filing bugs related to Rusticl, make
sure to add the ``Rusticl`` label so people subscribed to that Label get
pinged.

Known issues
------------

One issue you might come across is, that the Rust edition meson sets is
not right. This is a known `meson bug
<https://github.com/mesonbuild/meson/issues/10664>`__ and in order to
fix it, simply run ``meson configure $your_build_dir -Drust_std=2021``
