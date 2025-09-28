# Introduction to XKB {#xkb-intro}

__XKB__ stands for “X Keyboard Extension”. It may refer to either:

- a [protocol](@ref xkb-the-protocol)
- a [keyboard layout configuration](@ref xkb-the-config)
- a [text format](@ref xkb-the-text-format)

## XKB the protocol {#xkb-the-protocol}

A __protocol__ for the [X Windows System], that extends the core protocol.

_xkbcommon’s_ API is somehow derived from this API, but has been
substantially reworked to function as a library instead of a protocol,
and exposes fewer internal details to clients.

_xkbcommon_ does not depend on a particular windows system; for instance
it is used by the [Wayland] protocol.

_xkbcommon_ provides the <code>[xkbcommon-x11]</code> module to interface
a client with an X server using the XKB protocol. Relevant links:

- [The X Window System Protocol][X Protocol]
- [The X Keyboard Extension: Protocol Specification][XKB Protocol]
- [xkbcommon-x11]


## XKB the keyboard keymap configuration {#xkb-the-config}

In order to use [the protocol](@ref xkb-the-protocol), one must first load a
[complete keymap]. The keymap usually comes from the OS _layout database_,
which is commonly [xkeyboard-config]. Since keymaps may have definitions in
common, the database actually stores their basic components separately to allow
maximum composability and coherence. A recipe to compose a keymap from its
components is called a _keymap configuration_.

In XKB, there are several ways to define a keymap configuration. They all aim to
produce a [complete keymap]. The following diagram presents an overview.
Then they are presented hereinafter, ordered from end user to low-level
implementation.

@dotfile xkb-configuration "XKB keymap configurations"
<dl>
  <dt>
    RMLVO: <u>R</u>ules, <u>M</u>odel, <u>L</u>ayout, <u>V</u>ariant,
    <u>O</u>ptions @anchor RMLVO-intro
  </dt>
  <dd>
    This is the configuration the end user usually faces in the UI.
    The idea is to expose high level concepts such as [keyboard model] and
    [keyboard layout] to the user, then to _map_ them to the corresponding set
    of low-level configuration files (see [KcCGST]).

    @note The RMLVO configurations actually available to the end user is managed
    by the `xkbregistry`. It uses an XML file, the _registry_, which exposes and
    documents the set of RMLVO settings in the layout database.

    The RMLVO configuration consists of the following components:

    <dl>
      <dt>Rules</dt>
      <dd>
      The rules define the _mapping_ from high to low level components.
      The rules _component_ is the file containing the set of rules to use.
      It is usually implicit and set by the system.

      See the [rules file format](doc/rules-format.md) for further details.
      </dd>
      <dt>Model</dt>
      <dd>
      The name of the model of the keyboard hardware in use.
      It may depend on:

      - The _location_ and _language_ of the user, because languages may
        require [specific keys][language input keys] for their input methods,
        such as the _muhenkan_ key on Japanese keyboard and the _Hanja_ key
        for Korean keyboards. The keyboard are usually classified by the
        [standard][keyboard standard] it is based on, e.g. ANSI, ISO, JIS,
        ABNT.
      - The keyboard _vendor:_ keyboard may have a set of keys that are not
        standard, or may be specific to an OS.
      </dd>
      <dt>Layout</dt>
      <dd>
      The identifier of the general layout to use. It usually refers to a
      country or a language.
      </dd>
      <dt>Variant</dt>
      <dd>
      Any minor variants on the general layout. It may be national variants
      </dd>
      <dt>Options</dt>
      <dd>
      Set of extra options to customize the standard layouts.

      Examples: switch modifiers keys, location of the compose key, etc.
      </dd>
    </dl>
  </dd>
  <dt>
    KcCGST: <u>K</u>ey<u>c</u>odes, <u>C</u>ompat, <u>G</u>eometry,
    <u>S</u>ymbols, <u>T</u>ypes @anchor KcCGST-intro
  </dt>
  <dd>
    This is the low-level configuration of XKB and how the files are actually
    organized in the _layout database_.
    It is not really intuitive or straight-forward for the uninitiated.

    @note _xkbcommon_ [does not offer an API for KcCGST](@ref KcCGST-support):
    it is considered an implementation detail.
    Instead, [RMLVO] is the preferred way for the user to configure XKB.

    The KcCGST configuration consists of the following components:

    <dl>
      <dt>Key codes</dt>
      <dd>
      A translation of the raw [key codes] from the keyboard into
      symbolic names.
      </dd>
      <dt>Compatibility</dt>
      <dd>
      A specification of what internal actions modifiers and various
      special-purpose keys produce.
      </dd>
      <dt>Geometry</dt>
      <dd>
      A description of the physical layout of a keyboard.

      @attention This legacy feature is [not supported](@ref geometry-support)
      by _xkbcommon_.
      </dd>
      <dt>Key symbols</dt>
      <dd>
      A translation of symbolic key codes into actual [key symbols] (keysyms).
      </dd>
      <dt>Key types</dt>
      <dd>
      Types describe how a pressed key is affected by active [modifiers]
      such as Shift, Control, Alt, etc.
      </dd>
    </dl>
  </dd>
  <dt>Complete Keymap @anchor keymap-intro</dt>
  <dd>
  A complete keymap is a _self-contained_ text file with all the [KcCGST]
  components needed to configure a keyboard. This is the result of the
  _resolution_ of the [RMLVO] and [KcCGST] configurations. This is also the
  format used by X11 and Wayland when prompted to _serialize_ the keymap in use.

  @note This is a low-level configuration. [RMLVO] is the preferred way for the
  end user to configure XKB, but some _power users_ may need it for _avanced_
  configurations.

  See the [XKB text format] for further details.
  </dd>
</dl>

@note Layout making use of dead keys require a [Compose](@ref compose) file. The
same applies when if using a [Compose key].

[key codes]: @ref keycode-def
[key symbols]: @ref keysym-def
[levels]: @ref level-def
[modifiers]: @ref modifier-def
[RMLVO]: @ref RMLVO-intro
[KcCGST]: @ref KcCGST-intro
[complete keymap]: @ref keymap-intro
[Compose key]: https://en.wikipedia.org/wiki/Compose_key
[XKB text format]: @ref xkb-the-text-format


## XKB the text format {#xkb-the-text-format}

A __text format__ to define keyboard keymaps. XKB 1.0 is the specification
implemented in current X servers. The format supported by _xkbcommon_
is very close to XKB 1.0, with some removals and additions. See the
[compatibility] page for further details.

The format supported by _xkbcommon_ is documented at the page
“[The XKB keymap text format, V1][keymap-format-text-v1]”.

The documentation of the _original_ XKB 1.0 format is much more scarce than
for the protocol. Some priceless resources are:

- [Ivan Pascal's XKB documentation][ivan-pascal]
- [An Unreliable Guide to XKB Configuration][unreliable-guide]
- [ArchWiki XKB page][arch-wiki]

[X Windows System]: https://en.wikipedia.org/wiki/X_Window_System
[X Protocol]: https://www.x.org/releases/current/doc/xproto/x11protocol.html#Keyboards
[XKB Protocol]: https://www.x.org/releases/current/doc/kbproto/xkbproto.html
[xkbcommon-x11]: @ref x11-overview
[Wayland]: https://wayland.freedesktop.org/docs/html/apa.html#protocol-spec-wl_keyboard
[compatibility]: @ref xkb-v1-compatibility
[keymap-format-text-v1]: doc/keymap-format-text-v1.md
[ivan-pascal]: https://web.archive.org/web/20190724015820/http://pascal.tsu.ru/en/xkb/
[unreliable-guide]: https://www.charvolant.org/doug/xkb/html/index.html
[arch-wiki]: https://wiki.archlinux.org/index.php/X_keyboard_extension
[keyboard model]: https://en.wikipedia.org/wiki/Computer_keyboard
[keymap]: https://en.wikipedia.org/wiki/Keyboard_layout
[keyboard layout]: https://en.wikipedia.org/wiki/Keyboard_layout
[xkeyboard-config]: https://gitlab.freedesktop.org/xkeyboard-config/xkeyboard-config
[keyboard standard]: https://en.wikipedia.org/wiki/Computer_keyboard#Types_and_standards
[language input keys]: https://en.wikipedia.org/wiki/Language_input_keys

@todo Explain how to configure XKB, with examples
