# Wayland protocols

wayland-protocols contains Wayland protocols that add functionality not
available in the Wayland core protocol. Such protocols either add
completely new functionality, or extend the functionality of some other
protocol either in Wayland core, or some other protocol in
wayland-protocols.

A protocol in wayland-protocols consists of a directory containing a set
of XML files containing the protocol specification, and a README file
containing detailed state and a list of maintainers.

## Protocol phases

Protocols in general has three phases: the development phase, the testing
phase, and the stable phase.

In the development phase, a protocol is not officially part of
wayland-protocols, but is actively being developed, for example by
iterating over it in a
[merge
request](https://gitlab.freedesktop.org/wayland/wayland-protocols/merge_requests),
or planning it in an
[issue](https://gitlab.freedesktop.org/wayland/wayland-protocols/issues).

During this phase, patches for clients and compositors are written as a test
vehicle. Such patches must not be merged in clients and compositors, because
the protocol can still change.

When a protocol has reached a stage where it is ready for wider adoption,
and after the [GOVERNANCE section
2.3](GOVERNANCE.md#2.3-introducing-new-protocols) requirements have been
met, it enters the "testing" phase. At this point, the protocol is added
to `staging/` directory of wayland-protocols and made part of a release.
What this means is that implementation is encouraged in clients and
compositors where the functionality it specifies is wanted.

Extensions in staging cannot have backward incompatible changes, in that
sense they are equal to stable extensions. However, they may be completely
replaced with a new major version, or a different protocol extension all
together, if design flaws are found in the testing phase.

After a staging protocol has been sufficiently tested in the wild and
proven adequate, its maintainers and the community at large may declare it
"stable", meaning it is unexpected to become superseded by a new major
version.

## Deprecation

A protocol may be deprecated, if it has been replaced by some other
protocol, or declared undesirable for some other reason. No more changes
will be made to a deprecated protocol.

## Legacy protocol phases

An "unstable" protocol refers to a protocol categorization policy
previously used by wayland-protocols, where protocols initially
placed in the `unstable/` directory had certain naming conventions were
applied, requiring a backward incompatible change to be declared "stable".

During this phase, protocol extension interface names were in addition to
the major version postfix also prefixed with `z` to distinguish from
stable protocols.

## Protocol directory tree structure

Depending on which stage a protocol is in, the protocol is placed within
the toplevel directory containing the protocols with the same stage.
Stable protocols are placed in the `stable/` directory, staging protocols
are placed in the `staging/` directory, and deprecated protocols are
placed in the `deprecated/` directory.

Unstable protocols (see [Legacy protocol phases](#legacy-protocol-phases))
can be found in the `unstable/` directory, but new ones should never be
placed here.

## Protocol development procedure

To propose a new protocol, create a GitLab merge request adding the
relevant files and Makefile.am entry to the repository with the
explanation and motivation in the commit message. Protocols are
organized in namespaces describing their scope ("wp", "xdg" and "ext").
There are different requirements for each namespace, see [GOVERNANCE
section 2](GOVERNANCE.md#2-protocols) for more information.

If the new protocol is just an idea, open an issue on the GitLab issue
tracker. If the protocol isn't ready for complete review yet and is an
RFC, create a merge request and add the "WIP:" prefix in the title.

To propose changes to existing protocols, create a GitLab merge request.

Please include a `Signed-off-by` line at the end of the commit to certify
that you wrote it or otherwise have the right to pass it on as an
open-source patch. See the
[Developer Certificate of Origin](https://developercertificate.org/) for
a formal definition.

## Interface naming convention

All protocols should avoid using generic namespaces or no namespaces in
the protocol interface names in order to minimize risk that the generated
C API collides with other C API. Interface names that may collide with
interface names from other protocols should also be avoided.

For generic protocols not limited to certain configurations (such as
specific desktop environment or operating system) the `wp_` prefix
should be used on all interfaces in the protocol.

For protocols allowing clients to configure how their windows are
managed, the `xdg_` prefix should be used.

For operating system specific protocols, the interfaces should be
prefixed with both `wp_` and the operating system, for example
`wp_linux_`, or `wp_freebsd_`, etc.

For more information about namespaces, see [GOVERNANCE section 2.1
](GOVERNANCE.md#21-protocol-namespaces).

Each new protocol XML file must include a major version postfix, starting
with `-v1`. The purpose of this postfix is to make it possible to
distinguish between backward incompatible major versions of the same
protocol.

The interfaces in the protocol XML file should as well have the same
major version postfix in their names.

For example, the protocol `foo-bar` may have a XML file
`foo-bar/foo-bar-v1.xml`, consisting of the interface `wp_foo_bar_v1`,
corresponding to the major version 1, as well as the newer version
`foo-bar/foo-bar-v2.xml` consisting of the interface `wp_foo_bar_v2`,
corresponding to the major version 2.

## Include a disclaimer

Include the following disclaimer:

```
Warning! The protocol described in this file is currently in the testing
phase. Backward compatible changes may be added together with the
corresponding interface version bump. Backward incompatible changes can
only be done by creating a new major version of the extension.
```

## Backward compatible protocol changes

A protocol may receive backward compatible additions and changes. This
is to be done in the general Wayland way, using `version` and `since` XML
element attributes.

## Backward incompatible protocol changes

While not preferred, a protocol may at any stage, especially during the
testing phase, when it is located in the `staging/` directory, see
backward incompatible changes.

Assuming a backward incompatible change is needed, the procedure for how to
do so is the following:

- Make a copy of the XML file with the major version increased by 1.
- Increase the major version number in the protocol XML by 1.
- Increase the major version number in all of the interfaces in the
  XML by 1.
- Reset the interface version number (interface version attribute) of all
  the interfaces to 1.
- Remove all of the `since` attributes.

## Declaring a protocol stable

Once it has been concluded that a protocol been proven adequate in
production, and that it is deemed unlikely to receive any backward
incompatible changes, it may be declared stable.

The procedure of doing this is the following:

- Create a new directory in the `stable/` toplevel directory with the
  same name as the protocol directory in the `staging/` directory.
- Copy the final version of the XML that is the version that was
  decided to be declared stable into the new directory. The target name
  should be the same name as the protocol directory but with the `.xml`
  suffix.
- Remove the disclaimer about the protocol being in the testing phase.
- Update the `README` file in the staging directory and create a new
  `README` file in the new directory.
- Replace the disclaimer in the protocol files left in the staging/
  directory with the following:

```
Disclaimer: This protocol extension has been marked stable. This copy is
no longer used and only retained for backwards compatibility. The
canonical version can be found in the stable/ directory.
```

Note that the major version of the stable protocol extension, as well as
all the interface versions and names, must remain unchanged.

There are other requirements for declaring a protocol stable, see
[GOVERNANCE section 2.3](GOVERNANCE.md#23-introducing-new-protocols).

## Releases

Each release of wayland-protocols finalizes the version of the protocols
to their state they had at that time.

## Gitlab conventions

### Triaging merge requests

New merge requests should be triaged. Doing so requires the one doing the
triage to add a set of initial labels:

~"New Protocol" - For a new protocol being added. If it's an amendment to
an existing protocol, apply the label of the corresponding protocol
instead. If none exist, create it.

~"Needs acks" - If the protocol needs one or more acknowledgements.

~"Needs implementations" - If there are not enough implementations of the
protocol.

~"Needs review" - If the protocol is in need of review.

~"In 30 day discussion period" - If the protocol needs a 30 day discussion
period.

For the meaning and requirement of acknowledgments and available
implementations, see the GOVERNANCE.md document.

### Managing merge requests

When merge requests get their needed feedback and items, remove the
corresponding label that marks it as needing something. For example, if a
merge request receives all the required acknowledgments, remove the
~"Needs acks" label, or if 30 days passed since opening, remove any
~"In 30 day discussion period" label.

### Nacking a merge request

If the inclusion of a merge request is denied due to one or more Nacks, add
the ~Nacked label.
