===========================
TBD Format Reference Manual
===========================

.. contents::
   :local:
   :depth: 4

.. _tbd_current_abstract:

Abstract
========
Text-based Dynamic Library Stubs (.tbd) are a new representation for dynamic
libraries and frameworks in Apple's SDKs. They provide the same benefit as the
legacy Mach-O Dynamic Library Stubs, which is to reduce the SDK size, but they
are even more compact. Furthermore, they enable several new features, such as
private framework inlining, InstallAPI, etc.

The new dynamic library stub file format is a human readable and editable YAML
text file. Those text-based stub files contain the same exported symbols as the
original dynamic library.


.. _tbd_current_format:

Format
======

.. _tbd_current_TBDv4:

TBD Version 4
-------------

The TBD version 4 file uses the YAML tag *!tapi-tbd*, which is required. This
version of the format has several fundamental changes from the previous
formats. This change was necessary to support new features and platforms.

This is an example of an TBD version 4 file:

.. code::

  --- !tapi-tbd
  tbd-version: 4
  targets: [ i386-macos, x86_64-macos, x86_64-maccatalyst]
  uuids:
    - target: i386-macos
      value: xxx
    - target: x86_64-macos
      value: xxx
    - target: x86_64-maccatalyst
      value: xxx
  flags: [ flat_namespace ]
  install-name: /u/l/libfoo.dylib
  current-version: 1.2.3
  compatibility-version: 1.1
  swift-abi-version: 5
  parent-umbrella:
    - targets: [ i386-macos, x86_64-macos, x86_64-maccatalyst]
      umbrella: System
  allowable-clients:
    - targets: [ i386-macos, x86_64-macos, x86_64-maccatalyst]
      clients: [ ClientA, ClientB ]
  reexported-libraries:
    - targets: [ x86_64-macos, x86_64-maccatalyst]
      library: [ /System/Library/Frameworks/Foo.framework/Foo ]
    - targets: [ i386-macos]
      library: [ /System/Library/Frameworks/Bar.framework/Bar ]
  exports:
    - targets: [ x86_64-macos ]
      symbols: [ _symA ]
      objc-classes: []
      objc-eh-types: []
      objc-ivars: []
      weak-symbols: []
      thread-local-symbols: []
    - targets: [ x86_64-maccatalyst ]
      symbols: [ _symB ]
    - targets: [ x86_64-macos, x86_64-maccatalyst ]
      symbols: [ _symAB ]
  re-exports:
    - targets: [ i386-macos ]
      symbols: [ _symC ]
      objc-classes: []
      objc-eh-types: []
      objc-ivars: []
      weak-symbols: []
      thread-local-symbols: []
  undefineds:
    - targets: [ i386-macos ]
      symbols: [ _symD ]
      objc-classes: []
      objc-eh-types: []
      objc-ivars: []
      weak-symbols: []
      thread-local-symbols: []
  ...

Keys:

- :ref:`tbd-version <tbd_current_tbd_version>`
- :ref:`targets <tbd_current_targets>`
- :ref:`uuids <tbd_current_uuids>`
- :ref:`flags <tbd_current_flags>`
- :ref:`install-name <tbd_current_install_name>`
- :ref:`current-version <tbd_current_current_version>`
- :ref:`compatibility-version <tbd_current_compatibility_version>`
- :ref:`swift-abi-version <tbd_current_swift_abi_version>`
- :ref:`parent-umbrella <tbd_current_parent_umbrella>`
- :ref:`allowable-clients <tbd_current_allowable_clients>`
- :ref:`reexported-libraries <tbd_current_reexported_libraries>`
- :ref:`Symbol Sections <tbd_current_symbol_sections>`


.. _tbd_current_tbd_version:

TBD File Version
~~~~~~~~~~~~~~~~

The key *tbd-version* is required and specifies the TBD file version.

Example:

.. code::

  tbd-version: 4

Currently the only valid value is 4.


.. _tbd_current_targets:

Targets
~~~~~~~

The key *targets* is required and specifies a list of supported
architecture/platform tuples.

Example:

.. code::

  targets: [ x86_64-macos, x86_64-maccatalyst, arm64-ios, x86_64-ios-simulator ]

The following platform identifiers are supported: macos, ios, ios-simulator,
tvos, tvos-simulator, watchos, watchos-simulator, bridgeos.

Those identifiers are mapped to the platform number that is specified by the
Mach-O format for the LC_BUILD_VERSION load command. It is also possible to
encode the platform with the platform number directly (for example: x86_64-maccatalyst).

.. _tbd_current_parent_umbrella:

Parent Umbrella
~~~~~~~~~~~~~~~

The key *parent-umbrella* is optional and specifies the parent umbrella of the
dynamic library (if applicable). This key is equivalent to the LC_SUB_FRAMEWORK
load command in the Mach-O format.

Example:

.. code::

  parent-umbrella:
    - targets: [ arm64-ios ]
      umbrella: System
    - targets: [ x86_64-ios-simulator]
      umbrella: SystemSim

.. _tbd_current_allowable_clients:

Allowable Clients
~~~~~~~~~~~~~~~~~

The key *allowable-clients* is optional and specifies a list of allowable
clients that are permitted to link against the dynamic library file. This key is
equivalent to the LC_SUB_CLIENT load command in the Mach-O format.

Example:

.. code::

  allowable-clients:
    - targets: [ arm64-ios ]
      clients: [ ClientA, ClientB ]
    - targets: [ x86_64-ios-simulator ]
      clients: [ ClientC ]


.. _tbd_current_reexported_libraries:

Reexported Libraries
~~~~~~~~~~~~~~~~~~~~

The key *reexported-libraries* is optional and specifies a list of reexported
libraries. This key is equivalent to the LC_REEXPORT_DYLIB load command in the
Mach-O format.

Example:

.. code::

  reexported-libraries:
    - targets:   [ arm64-ios ]
      libraries: [ /usr/lib/libm.dylib ]
    - targets:   [ x86_64-ios-simulator ]
      libraries: [ /usr/lib/libobjc4.dylib ]


.. _tbd_current_uuids:

UUIDs
~~~~~

The key *uuids* is optional and specifies the list of UUIDs per architecture.
This key is equivalent to the LC_UUID load command in the Mach-O format.
This field is now deprecated.

Example:

.. code::

  uuids:
    - target: i386-macos
      value: xxx
    - target: x86_64-macos
      value: xxx
    - target: x86_64-maccatalyst
      value: xxx


.. _tbd_current_flags:

Flags
~~~~~

The key *flags* is optional and specifies dynamic library specific flags.

Example:

.. code::

  flags: [ installapi ]

Valid flags are: flat_namespace, not_app_extension_safe, and installapi.

flat_namespace is deprecated, but there are still some old binaries around on
macOS that depend on flat namespace linking. The default is two level
namespace linking. not_app_extension_safe indicates that the library is not safe
to be used in an Application Extension. Per default libaries are build as
application extension safe in B&I. Previously, installapi indicated that this TBD file was
generated during the installapi phase in B&I. The installapi flag is now deprecated.


.. _tbd_current_install_name:

Install Name
~~~~~~~~~~~~

The key *install-name* is required and specifies the install name of the dynamic
library file, which is usually the path in the SDK. This key is part of the
LC_ID_DYLIB load command in the Mach-O format.

Example:

.. code::

  install-name: /System/Library/Frameworks/Foundation.framework/Foundation


.. _tbd_current_current_version:

Current Version
~~~~~~~~~~~~~~~

The key *current-version* is optional and specifies the current version of the
dynamic library file. The default value is 1.0 if not specified. This key is
part of the LC_ID_DYLIB load command in the Mach-O format.

Example:

.. code::

  current-version: 1.2.3


.. _tbd_current_compatibility_version:

Compatibility Version
~~~~~~~~~~~~~~~~~~~~~

The key *compatibility-version* is optional and specifies the compatibility
version of the dynamic library file. The default value is 1.0 if not specified.
This key is part of the LC_ID_DYLIB load command in the Mach-O format.

Example:

.. code::

  compatibility-version: 1.2


.. _tbd_current_swift_abi_version:

Swift ABI Version
~~~~~~~~~~~~~~~~~

The key *swift-abi-version* is optional and specifies the Swift ABI version the
dynamic library file was compiled with. The default value is 0 if not
specified. The Swift ABI version is encoded in the Objective-C image section,
which doesn't exist in stubbed dynamic library files.

Example:

.. code::

  swift-abi-version: 5


.. _tbd_current_symbol_sections:

Symbol Sections
~~~~~~~~~~~~~~~

The keys *exports*, *re-exports*, and *undefineds* are optional. *exports* are
regular exported symbol sections. *re-exports* are also exported symbol
sections, but the symbol is not defined by the library itself. The symbol is
coming from a different library instead. *undefineds* are undefined symbol
sections and only used for flat address space libaries.

A symbol section specifies the symbol names, Objective-C Class
names, etc. Each section defines a unique architecture/platform tuple set. This
is an optimization to reduce the size of the file, by grouping common symbol
names into the same section.

Example:

.. code::

  exports:
    - targets: [ x86_64-macos ]
      symbols: [ _symbolA ]
      objc-classes: [ NSString ]
      objc-eh-types: [ NSString ]
      objc-ivars: [ NSString.ivar1 ]
      weak-symbols: [ _weakSymol ]
      thread-local-symbols: [ _tlvSymbol ]
    - targets: [ x86_64-maccatalyst ]
      symbols: [ _symB ]
    - targets: [ x86_64-macos, x86_64-maccatalyst ]
      symbols: [ _symAB ]


Each section has the following keys:
  - :ref:`targets <tbd_current_targets>`
  - :ref:`symbols <tbd_current_symbols>`
  - :ref:`objc-classes <tbd_current_objectivec_classes>`
  - :ref:`objc-eh-types <tbd_current_objectivec_eh_types>`
  - :ref:`objc-ivars <tbd_current_objectivec_ivars>`
  - :ref:`weak-def-symbols <tbd_current_weak_symbols>`
  - :ref:`thread-local-symbols <tbd_current_thread_local_symbols>`



.. _tbd_current_symbols:

Symbol Names:
~~~~~~~~~~~~~

The key *symbols* is optional and specifies a list of exported, re-exported, or
undefined symbol names.

Example:

.. code::

  symbols: [ _sym1, _sym2, _sym3 ]


.. _tbd_current_objectivec_classes:

Objective-C Class Names:
~~~~~~~~~~~~~~~~~~~~~~~~

The key *objc-classes* is optional and specifies a list of exported,
re-exported, or undefined Objective-C class names. Objective-C classes have
different symbol mangling depending on the Objective-C ABI, which would prevent
the merging of Objective-C class symbols across architecture slices. Therefore
they are listed separately from other symbols, which avoids the mangling issue
and allows the merging across architecture slices.


Example:

.. code::

  objc-classes: [ ClassA, ClassB, ClassC ]


.. _tbd_current_objectivec_eh_types:

Objective-C Exception Type:
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The key *objc-eh-types* is optional and specifies a list of exported or
undefined Objective-C class exception types.

Example:

.. code::

  objc-eh-types: [ ClassA, ClassB ]


.. _tbd_current_objectivec_ivars:

Objective-C Instance Variables:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The key *objc-ivars* is optional and specifies a list of exported or undefined
Objective-C instance variable names.

Example:

.. code::

  objc-ivars: [ ClassA.ivar1, ClassA.ivar2, ClassC.ivar1 ]


.. _tbd_current_weak_symbols:

Weak Symbols:
~~~~~~~~~~~~~~~~~~~~~

The key *weak-def-symbols* for export sections or *weak-ref-symbols* for
undefined sections is optional and specifies a list of weak symbol names.

Example

.. code::

  weak-symbols: [ _weakSym1, _weakSym2 ]


.. _tbd_current_thread_local_symbols:

Thread Local Symbols:
~~~~~~~~~~~~~~~~~~~~~

The key *thread-local-symbols* is optional and specifies a list of thread local
exported symbol names.

Example:

.. code::

  thread-local-symbols: [ _tlv1, _tlv2 ]


