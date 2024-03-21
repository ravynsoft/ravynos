The rules file {#rule-file-format}
==============

The purpose of the rules file is to map between configuration values
that are easy for a user to specify and understand, and the
configuration values xkbcomp uses and understands.

xkbcomp uses the `xkb_component_names` struct, which maps directly to
include statements of the appropriate sections, called for short
[KcCGST](@ref KcCGST-intro) (see the [XKB
introduction](doc/introduction-to-xkb.md);
'G' stands for "geometry", which is not supported). These are not
really intuitive or straight-forward for the uninitiated.

Instead, the user passes in a `xkb_rule_names` struct, which consists
of the name of a rules file (in Linux this is usually "evdev"), a
keyboard model (e.g. "pc105"), a set of layouts (which will end up
in different groups, e.g. "us,fr"), variants (used to alter/augment
the respective layout, e.g. "intl,dvorak"), and a set of options
(used to tweak some general behavior of the keyboard, e.g.
"ctrl:nocaps,compose:menu" to make the Caps Lock key act like Ctrl
and the Menu key like Compose). We call these
[RMLVO](@ref RMLVO-intro).

Format of the file
------------------
The file consists of rule sets, each consisting of rules (one per
line), which match the MLVO values on the left hand side, and, if
the values match to the values the user passed in, results in the
values on the right hand side being added to the resulting KcCGST.
Since some values are related and repeated often, it is possible
to group them together and refer to them by a group name in the
rules.

Along with matching values by simple string equality, and for
membership in a group defined previously, rules may also contain
"wildcard" values - "*" - which always match. These usually appear
near the end.

Grammar
-------
(It might be helpful to look at a file like rules/evdev along with
this grammar. Comments, whitespace, etc. are not shown.)

```
File         ::= { "!" (Include | Group | RuleSet) }

Include      ::= "include" <ident>

Group        ::= GroupName "=" { GroupElement } "\n"
GroupName    ::= "$"<ident>
GroupElement ::= <ident>

RuleSet      ::= Mapping { Rule }

Mapping      ::= { Mlvo } "=" { Kccgst } "\n"
Mlvo         ::= "model" | "option" | ("layout" | "variant") [ Index ]
Index        ::= "[" 1..XKB_NUM_GROUPS "]"
Kccgst       ::= "keycodes" | "symbols" | "types" | "compat" | "geometry"

Rule         ::= { MlvoValue } "=" { KccgstValue } "\n"
MlvoValue    ::= "*" | GroupName | <ident>
KccgstValue  ::= <ident>
```

Notes:

- Include processes the rules in the file path specified in the ident,
  in order. %-expansion is performed, as follows:

```
  %%:
    A literal %.

  %H:
    The value of the HOME environment variable.

  %E:
    The extra lookup path for system-wide XKB data (usually /etc/xkb/rules).

  %S:
    The system-installed rules directory (usually /usr/share/X11/xkb/rules).
```

- The order of values in a Rule must be the same as the Mapping it
  follows. The mapping line determines the meaning of the values in
  the rules which follow in the RuleSet.

- If a Rule is matched, %-expansion is performed on the KccgstValue,
  as follows:

```
  %m, %l, %v:
     The model, layout or variant, if only one was given (e.g.
     %l for "us,il" is invalid).

  %l[1], %v[1]:
     Layout or variant for the specified group Index, if more than
     one was given (e.g. %l[1] for "us" is invalid).

  %+m, %+l, %+v, %+l[1], %+v[1]
     As above, but prefixed with '+'. Similarly, '|', '-', '_' may be
     used instead of '+'.

  %(m), %(l), %(l[1]), %(v), %(v[1]):
     As above, but prefixed by '(' and suffixed by ')'.
```

  In case the expansion is invalid, as described above, it is
  skipped (the rest of the string is still processed); this includes
  the prefix and suffix (that's why you shouldn't use e.g. "(%v[1])").
