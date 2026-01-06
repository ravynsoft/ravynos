====================================
TBD Format Reference Manual (legacy)
====================================

.. contents::
   :local:
   :depth: 4

.. _tbd_legacy_format:

Format
======


.. _tbd_legacy_TBDv1:

TBD Version 1
-------------

The TBD v1 format only support two level address libraries and is per
definition application extension safe. It cannot represent flat namespace or
not application extension safe dynamic libraries.

The initial version of the TBD file doesn't have a YAML tag. The tag
*!tapi-tbd-v1* is optional and shouldn't be emitted to support older linker.

This is an example of an TBD version 1 file with all keys (optional and
required):

.. code::

  ---
  archs: [ armv7, armv7s, arm64 ]
  platform: ios
  install-name: /usr/lib/libfoo.dylib
  current-version: 1.2.3
  compatibility-version: 1.0
  swift-version: 1.0
  objc-constraint: none
  exports:
    - archs: [ armv7, armv7s, arm64 ]
      allowed-clients: [ clientA ]
      re-exports: [ /usr/lib/liba.dylib, /usr/lib/libb.dylib ]
      symbols: [ _sym1, _sym2 ]
      objc-classes: [ _NSString, _NSBlockPredicate ]
      objc-ivars: [ _NSBlockPredicate._block ]
      weak-def-symbols: [ _weakSym1, _weakSym2 ]
      thread-local-symbols: [ _tlvSym1, _tlvSym2 ]
    - archs: [ arm64 ]
      allowed-clients: [ clientB, clientC ]
      re-exports: [ /usr/lib/libc.dylib ]
      symbols: [ _sym3 ]
  ...

Keys:

- :ref:`archs <tbd_legacy_architectures>`
- :ref:`platform <tbd_legacy_platform>`
- :ref:`install-name <tbd_legacy_install_name>`
- :ref:`current-version <tbd_legacy_current_version>`
- :ref:`compatibility-version <tbd_legacy_compatibility_version>`
- :ref:`swift-version <tbd_legacy_swift_version>`
- :ref:`objc-constraint <tbd_legacy_objectivec_constraint>`
- :ref:`exports <tbd_legacy_exports>`


.. _tbd_legacy_TBDv2:

TBD Version 2
-------------

The TBD version 2 file uses the YAML tag *!tapi-tbd-v2*, which is required.
This version of the format adds new keys for UUIDs, flags, parent-umbrella, and
undefined symbols. This allows supporting flat namespace and non application
extension safe libraries. Furthermore, the *allowed-clients* key was renamed to
*allowable-clients* to be consistent with the static linker. The default value
for the key *objc-constraint* has been changed to *retain_release*.

This is an example of an TBD version 2 file with all keys (optional and
required):

.. code::

  --- !tapi-tbd-v2
  archs: [ armv7, armv7s, arm64 ]
  uuids: [ "armv7: 00000000-0000-0000-0000-000000000000",
           "armv7s: 11111111-1111-1111-1111-111111111111",
           "arm64: 22222222-2222-2222-2222-222222222222" ]
  platform: ios
  flags: [ flat_namespace ]
  install-name: /usr/lib/libfoo.dylib
  current-version: 1.2.3
  compatibility-version: 1.0
  swift-version: 2
  objc-constraint: retain_release
  parent-umbrella: System
  exports:
    - archs: [ armv7, armv7s, arm64 ]
      allowable-clients: [ clientA ]
      re-exports: [ /usr/lib/liba.dylib, /usr/lib/libb.dylib ]
      symbols: [ _sym1, _sym2 ]
      objc-classes: [ _NSString, _NSBlockPredicate ]
      objc-ivars: [ _NSBlockPredicate._block ]
      weak-def-symbols: [ _weakSym1, _weakSym2 ]
      thread-local-symbols: [ _tlvSym1, _tlvSym2 ]
    - archs: [ arm64 ]
      allowable-clients: [ clientB, clientC ]
      re-exports: [ /usr/lib/libc.dylib ]
      symbols: [ _sym3 ]
  undefineds:
    - archs: [ armv7, armv7s, arm64 ]
      symbols: [ _sym10, _sym11 ]
      objc-classes: [ _ClassA ]
      objc-ivars: [ _ClassA.ivar1 ]
      weak-ref-symbols: [ _weakSym5 ]
    - archs: [ arm64 ]
      symbols: [ _sym12 ]
  ...

Keys:

- :ref:`archs <tbd_legacy_architectures>`
- :ref:`uuids <tbd_legacy_uuids>`
- :ref:`platform <tbd_legacy_platform>`
- :ref:`flags <tbd_legacy_flags>`
- :ref:`install-name <tbd_legacy_install_name>`
- :ref:`current-version <tbd_legacy_current_version>`
- :ref:`compatibility-version <tbd_legacy_compatibility_version>`
- :ref:`swift-version <tbd_legacy_swift_version>`
- :ref:`objc-constraint <tbd_legacy_objectivec_constraint>`
- :ref:`parent-umbrella <tbd_legacy_parent_umbrella>`
- :ref:`exports <tbd_legacy_exports>`
- :ref:`undefineds <tbd_legacy_undefineds>`


.. _tbd_legacy_TBDv3:

TBD Version 3
-------------

The TBD version 3 file uses the YAML tag *!tapi-tbd-v3*, which is required.
This version of the format adds new keys for Objective-C exception type and
renames the *swift-version* key to *swift-abi-version*, which also changes the
values that are encoded with this key. Furthermore, this version support
multiple YAML documents per TBD file, which is used by the private framework
inlining feature. The encoding of Objective-C class names and instance variables
has been changed to drop the leading '_'.

This is an example of an TBD version 3 file (without some optional keys):

.. code::

  --- !tapi-tbd-v3
  archs: [ armv7, armv7s, arm64 ]
  platform: ios
  install-name: /usr/lib/libfoo.dylib
  swift-abi-version: 3
  exports:
    - archs: [ armv7, armv7s, arm64 ]
      re-exports: [ /usr/lib/internal/liba.dylib ]
      symbols: [ _sym1, _sym2 ]
      objc-classes: [ NSString, NSBlockPredicate ]
      objc-eh-types: [ NSString ]
      objc-ivars: [ NSBlockPredicate._block ]
    - archs: [ arm64 ]
      symbols: [ _sym3 ]
  --- !tapi-tbd-v3
  archs: [ armv7, armv7s, arm64 ]
  platform: ios
  install-name: /usr/lib/liba.dylib
  swift-abi-version: 3
  exports:
    - archs: [ armv7, armv7s, arm64 ]
      re-exports: [ /usr/lib/internal/liba.dylib ]
      symbols: [ _sym10, _sym11 ]
  ...

Keys:

- :ref:`archs <tbd_legacy_architectures>`
- :ref:`uuids <tbd_legacy_uuids>`
- :ref:`platform <tbd_legacy_platform>`
- :ref:`flags <tbd_legacy_flags>`
- :ref:`install-name <tbd_legacy_install_name>`
- :ref:`current-version <tbd_legacy_current_version>`
- :ref:`compatibility-version <tbd_legacy_compatibility_version>`
- :ref:`swift-abi-version <tbd_legacy_swift_abi_version>`
- :ref:`objc-constraint <tbd_legacy_objectivec_constraint>`
- :ref:`parent-umbrella <tbd_legacy_parent_umbrella>`
- :ref:`exports <tbd_legacy_exports>`
- :ref:`undefineds <tbd_legacy_undefineds>`


Common Keys
--------------------

.. _tbd_legacy_architectures:

Architectures
~~~~~~~~~~~~~

The key *archs* is required and specifies the list of architectures that are
supported by the dynamic library file.

Example:

.. code::

  archs: [ armv7, armv7s, arm64 ]

Valid architectures are: i386, x86_64, x86_64h, armv7, armv7s, armv7k, arm64


.. _tbd_legacy_uuids:

UUIDs
~~~~~

The key *uuids* is optional and specifies the list of UUIDs per architecture.

Example:

.. code::

  uuids: [ "armv7: 00000000-0000-0000-0000-000000000000",
           "armv7s: 11111111-1111-1111-1111-111111111111",
           "arm64: 22222222-2222-2222-2222-222222222222" ]


.. _tbd_legacy_platform:

Platform
~~~~~~~~

The key *platform* is required and specifies the platform that is supported by
the dynamic library file.

Example:

.. code::

  platform: macosx

Valid platforms are: macosx, ios, tvos, watchos


.. _tbd_legacy_flags:

Flags
~~~~~

The key *flags* is optional and specifies dynamic library specific flags.

Example:

.. code::

  flags: [ installapi ]

Valid flags are: flat_namespace, not_app_extension_safe, and installapi.


.. _tbd_legacy_install_name:

Install Name
~~~~~~~~~~~~

The key *install-name* is required and specifies the install name of the dynamic
library file, which is usually the path in the SDK.

Example:

.. code::

  install-name: /System/Library/Frameworks/Foundation.framework/Foundation


.. _tbd_legacy_current_version:

Current Version
~~~~~~~~~~~~~~~

The key *current-version* is optional and specifies the current version of the
dynamic library file. The default value is 1.0 if not specified.

Example:

.. code::

  current-version: 1.2.3


.. _tbd_legacy_compatibility_version:

Compatibility Version
~~~~~~~~~~~~~~~~~~~~~

The key *compatibility-version* is optional and specifies the compatibility
version of the dynamic library file. The default value is 1.0 if not specified.

Example:

.. code::

  compatibility-version: 1.2


.. _tbd_legacy_swift_version:

Swift Version
~~~~~~~~~~~~~

The key *swift-version* is optional and specifies the Swift version the
dynamic library file was compiled with. The default value is 0 if not
specified.

Example:

.. code::

  swift-version: 1.0


.. _tbd_legacy_swift_abi_version:

Swift ABI Version
~~~~~~~~~~~~~~~~~

The key *swift-abi-version* is optional and specifies the Swift ABI version the
dynamic library file was compiled with. The default value is 0 if not
specified.

Example:

.. code::

  swift-abi-version: 5


.. _tbd_legacy_objectivec_constraint:

Objective-C Constraint
~~~~~~~~~~~~~~~~~~~~~~

The key *objc-constraint* is optional and specifies the Objective-C constraint
that was used to compile the dynamic library file. The default value is *none*
for TBD v1 files and *retain_release* thereafter.

Example:

.. code::

  objc-constraint: retain_release

Valid Objective-C constraints are: none, retain_release,
retain_release_for_simulator, retain_release_or_gc, or gc.


.. _tbd_legacy_parent_umbrella:

Parent Umbrella
~~~~~~~~~~~~~~~

The key *parent-umbrella* is optional and specifies the parent umbrella of the
dynamic library (if applicable).

Example:

.. code::

  parent-umbrella: System


.. _tbd_legacy_exports:

Export Section
~~~~~~~~~~~~~~

The key *exports* is optional, but it is very uncommon to have a dynamic
library that does not export any symbols. Symbol names, Objective-C Class
names, etc, are grouped into sections. Each section defines a unique
architecture set. This is an optimization to reduce the size of the file, by
grouping common symbol names into the same section.

Example (TBD v1):

.. code::

  exports:
    - archs: [ armv7, armv7s, arm64 ]
      allowed-clients: [ clientA ]
      re-exports: [ /usr/lib/liba.dylib, /usr/lib/libb.dylib ]
      symbols: [ _sym1, _sym2 ]
      objc-classes: [ _NSString, _NSBlockPredicate ]
      objc-ivars: [ _NSBlockPredicate._block ]
      weak-def-symbols: [ _weakSym1, _weakSym2 ]
      thread-local-symbols: [ _tlvSym1, _tlvSym2 ]


Example (TBD v2):

.. code::

  exports:
    - archs: [ armv7, armv7s, arm64 ]
      allowable-clients: [ clientA ]
      re-exports: [ /usr/lib/liba.dylib, /usr/lib/libb.dylib ]
      symbols: [ _sym1, _sym2 ]
      objc-classes: [ _NSString, _NSBlockPredicate ]
      objc-ivars: [ _NSBlockPredicate._block ]
      weak-def-symbols: [ _weakSym1, _weakSym2 ]
      thread-local-symbols: [ _tlvSym1, _tlvSym2 ]
  

Example (TBD v3):

.. code::

  exports:
    - archs: [ armv7, armv7s, arm64 ]
      allowable-clients: [ clientA ]
      re-exports: [ /usr/lib/liba.dylib, /usr/lib/libb.dylib ]
      symbols: [ _sym1, _sym2 ]
      objc-classes: [ NSString, NSBlockPredicate ]
      objc-eh-types: [ NSString ]
      objc-ivars: [ NSBlockPredicate._block ]
      weak-def-symbols: [ _weakSym1, _weakSym2 ]
      thread-local-symbols: [ _tlvSym1, _tlvSym2 ]


Each section has the following keys:
  - :ref:`archs <tbd_legacy_architectures>`
  - :ref:`allowed-clients <tbd_legacy_allowable_clients>` (TBD v1) or
    :ref:`allowable-clients <tbd_legacy_allowable_clients>` (TBD v2 and TBD v3)
  - :ref:`re-exports <tbd_legacy_reexported_libraries>`
  - :ref:`symbols <tbd_legacy_symbols>`
  - :ref:`objc-classes <tbd_legacy_objectivec_classes>`
  - :ref:`objc-eh-types <tbd_legacy_objectivec_eh_types>` (TBD v3 only)
  - :ref:`objc-ivars <tbd_legacy_objectivec_ivars>`
  - :ref:`weak-def-symbols <tbd_legacy_weak_symbols>`
  - :ref:`thread-local-symbols <tbd_legacy_thread_local_symbols>`


.. _tbd_legacy_undefineds:

Undefined Section
~~~~~~~~~~~~~~~~~

The key *undefineds* is optional and only applies to flat namespace libraries.

Example (TBD v2):

.. code::

  undefineds:
    - archs: [ armv7, armv7s, arm64 ]
      symbols: [ _sym1, _sym2 ]
      objc-classes: [ _NSString, _NSBlockPredicate ]
      objc-ivars: [ _NSBlockPredicate._block ]
      weak-ref-symbols: [ _weakSym1, _weakSym2 ]
  
Example (TBD v3):

.. code::

  undefineds:
    - archs: [ armv7, armv7s, arm64 ]
      symbols: [ _sym1, _sym2 ]
      objc-classes: [ NSString, NSBlockPredicate ]
      objc-eh-types: [ NSString ]
      objc-ivars: [ _NSBlockPredicate._block ]
      weak-ref-symbols: [ _weakSym1, _weakSym2 ]


Each section has the following keys:
  - :ref:`archs <tbd_legacy_architectures>`
  - :ref:`symbols <tbd_legacy_symbols>`
  - :ref:`objc-classes <tbd_legacy_objectivec_classes>`
  - :ref:`objc-eh-types <tbd_legacy_objectivec_eh_types>` (TBD v3 only)
  - :ref:`objc-ivars <tbd_legacy_objectivec_ivars>`
  - :ref:`weak-ref-symbols <tbd_legacy_weak_symbols>`


  .. _tbd_legacy_allowable_clients:

Allowable Clients
~~~~~~~~~~~~~~~~~

The key *allowed-clients* in TBD format 1 or *allowable-clients* in the TBD
format 2 and later is optional and specifies a list of allowable clients that
are permitted to link against the dynamic library file.

Example (TBD v1):

.. code::

  allowed-clients: [ clientA ]

Example (TBD v2 + v3):

.. code::

  allowable-clients: [ clientA ]


.. _tbd_legacy_reexported_libraries:

Reexported Libraries
~~~~~~~~~~~~~~~~~~~~

The key *re-exports* is optional and specifies a list of re-exported library
names.

Example:

.. code::

  re-export: [ /usr/lib/libm.dylib ]


.. _tbd_legacy_symbols:

Symbol Names:
~~~~~~~~~~~~~

The key *symbols* is optional and specifies a list of exported or undefined
symbol names.

Example:

.. code::

  symbols: [ _sym1, _sym2, _sym3 ]


.. _tbd_legacy_objectivec_classes:

Objective-C Class Names:
~~~~~~~~~~~~~~~~~~~~~~~~

The key *objc-classes* is optional and specifies a list of exported or undefined
Objective-C class names. Objective-C classes have different symbol mangling
depending on the Objective-C ABI, which would prevent the merging of
Objective-C class symbols across architecture slices. Therefore they are listed
separately from other symbols, which avoids the mangling issue and allows the
merging across architecture slices.


Example (TBD v1 and TBD v2):

.. code::

  objc-classes: [ _ClassA, _ClassB, _ClassC ]

Example (TBD v3):

.. code::

  objc-classes: [ ClassA, ClassB, ClassC ]


.. _tbd_legacy_objectivec_eh_types:

Objective-C Exception Type:
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The key *objc-eh-types* is optional and specifies a list of exported or
undefined Objective-C class exception types.

Example (TBD v3):

.. code::

  objc-eh-types: [ ClassA, ClassB ]


.. _tbd_legacy_objectivec_ivars:

Objective-C Instance Variables:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The key *objc-ivars* is optional and specifies a list of exported or undefined
Objective-C instance variable names.

Example (TBD v1 and TBD v2):

.. code::

  objc-ivars: [ _ClassA.ivar1, _ClassA.ivar2, _ClassC.ivar1 ]

Example (TBD v3):

.. code::

  objc-ivars: [ ClassA.ivar1, ClassA.ivar2, ClassC.ivar1 ]


.. _tbd_legacy_weak_symbols:

Weak Symbols:
~~~~~~~~~~~~~~~~~~~~~

The key *weak-def-symbols* for export sections or *weak-ref-symbols* for
undefined sections is optional and specifies a list of weak symbol names.

Example (Export Section):

.. code::

  weak-def-symbols: [ _weakDef1, _weakDef2 ]


Example (Undefined Section):

.. code::

  weak-ref-symbols: [ _weakRef1, _weakRef2 ]


.. _tbd_legacy_thread_local_symbols:

Thread Local Symbols:
~~~~~~~~~~~~~~~~~~~~~

The key *thread-local-symbols* is optional and specifies a list of thread local
exported symbol names.

Example:

.. code::

  thread-local-symbols: [ _tlv1, _tlv2 ]


