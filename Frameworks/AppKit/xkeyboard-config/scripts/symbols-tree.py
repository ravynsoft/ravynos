#!/usr/bin/env python3
#
# Builds a tree view of a symbols file (showing all includes)
#
# This file is formatted with Python Black

import argparse
import pathlib
from pyparsing import (
    Word,
    Literal,
    LineEnd,
    OneOrMore,
    oneOf,
    Or,
    And,
    QuotedString,
    Regex,
    cppStyleComment,
    alphanums,
    Optional,
    ParseException,
)

xkb_basedir = None


class XkbSymbols:
    def __init__(self, file, name):
        self.file = file  # Path to the file this section came from
        self.layout = file.name  # XKb - filename is the layout name
        self.name = name
        self.includes = []

    def __str__(self):
        return f"{self.layout}({self.name}): {self.includes}"


class XkbLoader:
    """
    Wrapper class to avoid loading the same symbols file over and over
    again.
    """

    class XkbParserException(Exception):
        pass

    _instance = None

    def __init__(self, xkb_basedir):
        self.xkb_basedir = xkb_basedir
        self.loaded = {}

    @classmethod
    def create(cls, xkb_basedir):
        assert cls._instance is None
        cls._instance = XkbLoader(xkb_basedir)

    @classmethod
    def instance(cls):
        assert cls._instance is not None
        return cls._instance

    @classmethod
    def load_symbols(cls, file):
        return cls.instance().load_symbols_file(file)

    def load_symbols_file(self, file):
        file = self.xkb_basedir / file
        try:
            return self.loaded[file]
        except KeyError:
            pass

        sections = []

        def quoted(name):
            return QuotedString(quoteChar='"', unquoteResults=True)

        # Callback, toks[0] is "foo" for xkb_symbols "foo"
        def new_symbols_section(name, loc, toks):
            assert len(toks) == 1
            sections.append(XkbSymbols(file, toks[0]))

        # Callback, toks[0] is "foo(bar)" for include "foo(bar)"
        def append_includes(name, loc, toks):
            assert len(toks) == 1
            sections[-1].includes.append(toks[0])

        EOL = LineEnd().suppress()
        SECTIONTYPE = (
            "default",
            "partial",
            "hidden",
            "alphanumeric_keys",
            "modifier_keys",
            "keypad_keys",
            "function_keys",
            "alternate_group",
        )
        NAME = quoted("name").setParseAction(new_symbols_section)
        INCLUDE = (
            lit("include") + quoted("include").setParseAction(append_includes) + EOL
        )
        # We only care about includes
        OTHERLINE = And([~lit("};"), ~lit("include") + Regex(".*")]) + EOL

        with open(file) as fd:
            types = OneOrMore(oneOf(SECTIONTYPE)).suppress()
            include_or_other = Or([INCLUDE, OTHERLINE.suppress()])
            section = (
                types
                + lit("xkb_symbols")
                + NAME
                + lit("{")
                + OneOrMore(include_or_other)
                + lit("};")
            )
            grammar = OneOrMore(section)
            grammar.ignore(cppStyleComment)
            try:
                grammar.parseFile(fd)
            except ParseException as e:
                raise XkbLoader.XkbParserException(str(e))

        self.loaded[file] = sections

        return sections


def lit(string):
    return Literal(string).suppress()


def print_section(s, filter_section=None, indent=0):
    if filter_section and s.name != filter_section:
        return

    layout = Word(alphanums + "_/").setResultsName("layout")
    variant = Optional(
        lit("(") + Word(alphanums + "_").setResultsName("variant") + lit(")")
    )
    grammar = layout + variant

    prefix = ""
    if indent > 0:
        prefix = " " * (indent - 2) + "|-> "
    print(f"{prefix}{s.layout}({s.name})")
    for include in s.includes:
        result = grammar.parseString(include)
        # Should really find the "default" section but for this script
        # hardcoding "basic" is good enough
        layout, variant = result.layout, result.variant or "basic"

        include_sections = XkbLoader.load_symbols(layout)
        for include_section in include_sections:
            print_section(include_section, filter_section=variant, indent=indent + 4)


def list_sections(sections, filter_section=None, indent=0):
    for section in sections:
        print_section(section, filter_section)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="XKB symbol tree viewer")
    parser.add_argument(
        "file",
        metavar="file-or-directory",
        type=pathlib.Path,
        help="The XKB symbols file or directory",
    )
    parser.add_argument(
        "section", type=str, default=None, nargs="?", help="The section (optional)"
    )
    ns = parser.parse_args()

    if ns.file.is_dir():
        xkb_basedir = ns.file.resolve()
        files = sorted([f for f in ns.file.iterdir() if not f.is_dir()])
    else:
        # Note: this requires that the file given on the cmdline is not one of
        # the sun_vdr/de or others inside a subdirectory. meh.
        xkb_basedir = ns.file.parent.resolve()
        files = [ns.file]

    XkbLoader.create(xkb_basedir)

    for file in files:
        try:
            sections = XkbLoader.load_symbols(file.resolve())
            list_sections(sections, filter_section=ns.section)
        except XkbLoader.XkbParserException:
            pass
