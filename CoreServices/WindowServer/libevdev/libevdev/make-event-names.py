#!/usr/bin/env python3
#
# Parses linux/input.h scanning for #define KEY_FOO 134
# Prints C header files or Python files that can be used as
# mapping and lookup tables.
#

import re
import sys


class Bits(object):
    def __init__(self):
        self.max_codes = {}


prefixes = [
    "EV_",
    "REL_",
    "ABS_",
    "KEY_",
    "BTN_",
    "LED_",
    "SND_",
    "MSC_",
    "SW_",
    "FF_",
    "SYN_",
    "REP_",
    "INPUT_PROP_",
    "MT_TOOL_",
]

duplicates = [
    "EV_VERSION",
    "BTN_MISC",
    "BTN_MOUSE",
    "BTN_JOYSTICK",
    "BTN_GAMEPAD",
    "BTN_DIGI",
    "BTN_WHEEL",
    "BTN_TRIGGER_HAPPY",
    "SW_MAX",
    "REP_MAX",
]

btn_additional = [
    [0, "BTN_A"],
    [0, "BTN_B"],
    [0, "BTN_X"],
    [0, "BTN_Y"],
]

code_prefixes = [
    "REL_",
    "ABS_",
    "KEY_",
    "BTN_",
    "LED_",
    "SND_",
    "MSC_",
    "SW_",
    "FF_",
    "SYN_",
    "REP_",
]


def print_bits(bits, prefix):
    if not hasattr(bits, prefix):
        return
    print("static const char * const %s_map[%s_MAX + 1] = {" % (prefix, prefix.upper()))
    for val, name in list(getattr(bits, prefix).items()):
        print("    [%s] = \"%s\"," % (name, name))
    if prefix == "key":
        for val, name in list(getattr(bits, "btn").items()):
            print("    [%s] = \"%s\"," % (name, name))
    print("};")
    print("")


def print_map(bits):
    print("static const char * const * const event_type_map[EV_MAX + 1] = {")

    for prefix in prefixes:
        if prefix in ["BTN_", "EV_", "INPUT_PROP_", "MT_TOOL_"]:
            continue
        print("    [EV_%s] = %s_map," % (prefix[:-1], prefix[:-1].lower()))

    print("};")
    print("")

    print("#if __clang__")
    print("#pragma clang diagnostic push")
    print("#pragma clang diagnostic ignored \"-Winitializer-overrides\"")
    print("#elif __GNUC__")
    print("#pragma GCC diagnostic push")
    print("#pragma GCC diagnostic ignored \"-Woverride-init\"")
    print("#endif")
    print("static const int ev_max[EV_MAX + 1] = {")
    for val in range(bits.max_codes["EV_MAX"] + 1):
        if val in bits.ev:
            prefix = bits.ev[val][3:]
            if prefix + "_" in prefixes:
                print("    %s_MAX," % prefix)
                continue
        print("    -1,")
    print("};")
    print("#if __clang__")
    print("#pragma clang diagnostic pop /* \"-Winitializer-overrides\" */")
    print("#elif __GNUC__")
    print("#pragma GCC diagnostic pop /* \"-Woverride-init\" */")
    print("#endif")
    print("")


def print_lookup(bits, prefix):
    if not hasattr(bits, prefix):
        return

    names = list(getattr(bits, prefix).items())
    if prefix == "btn":
        names = names + btn_additional

    # We need to manually add the _MAX codes because some are
    # duplicates
    maxname = "%s_MAX" % (prefix.upper())
    if maxname in duplicates:
        names.append((bits.max_codes[maxname], maxname))

    for val, name in sorted(names, key=lambda e: e[1]):
        print("    { .name = \"%s\", .value = %s }," % (name, name))


def print_lookup_table(bits):
    print("struct name_entry {")
    print("    const char *name;")
    print("    unsigned int value;")
    print("};")
    print("")
    print("static const struct name_entry tool_type_names[] = {")
    print_lookup(bits, "mt_tool")
    print("};")
    print("")
    print("static const struct name_entry ev_names[] = {")
    print_lookup(bits, "ev")
    print("};")
    print("")

    print("static const struct name_entry code_names[] = {")
    for prefix in sorted(code_prefixes, key=lambda e: e):
        print_lookup(bits, prefix[:-1].lower())
    print("};")
    print("")
    print("static const struct name_entry prop_names[] = {")
    print_lookup(bits, "input_prop")
    print("};")
    print("")


def print_mapping_table(bits):
    print("/* THIS FILE IS GENERATED, DO NOT EDIT */")
    print("")
    print("#ifndef EVENT_NAMES_H")
    print("#define EVENT_NAMES_H")
    print("")

    for prefix in prefixes:
        if prefix == "BTN_":
            continue
        print_bits(bits, prefix[:-1].lower())

    print_map(bits)
    print_lookup_table(bits)

    print("#endif /* EVENT_NAMES_H */")


def parse_define(bits, line):
    m = re.match(r"^#define\s+(\w+)\s+(\w+)", line)
    if m is None:
        return

    name = m.group(1)

    try:
        value = int(m.group(2), 0)
    except ValueError:
        return

    for prefix in prefixes:
        if not name.startswith(prefix):
            continue

        if name.endswith("_MAX"):
            bits.max_codes[name] = value

        if name in duplicates:
            return

        attrname = prefix[:-1].lower()

        if not hasattr(bits, attrname):
            setattr(bits, attrname, {})
        b = getattr(bits, attrname)
        b[value] = name


def parse(lines):
    bits = Bits()
    for line in lines:
        if not line.startswith("#define"):
            continue
        parse_define(bits, line)

    return bits


def usage(prog):
    print("Usage: %s <files>".format(prog))


if __name__ == "__main__":
    if len(sys.argv) <= 1:
        usage(sys.argv[0])
        sys.exit(2)

    from itertools import chain
    lines = chain(*[open(f).readlines() for f in sys.argv[1:]])
    bits = parse(lines)
    print_mapping_table(bits)
