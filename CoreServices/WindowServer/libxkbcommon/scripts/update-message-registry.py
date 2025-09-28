#!/usr/bin/env python3

from __future__ import annotations

import argparse
from dataclasses import astuple, dataclass
from pathlib import Path
import re
from typing import Callable, Generic, Sequence, TypeVar

import jinja2
import yaml


@dataclass(order=True)
class Version:
    """A semantic version number: MAJOR.MINOR.PATCH."""

    UNKNOWN_VERSION = "ALWAYS"
    DEFAULT_VERSION = "1.0.0"

    major: int
    minor: int
    patch: int = 0

    def __str__(self):
        return ".".join(map(str, astuple(self)))

    @classmethod
    def parse(cls, raw_version: str) -> Version:
        if raw_version == cls.UNKNOWN_VERSION:
            raw_version = cls.DEFAULT_VERSION
        version = raw_version.split(".")
        assert 2 <= len(version) <= 3 and all(
            n.isdecimal() for n in version
        ), raw_version
        return Version(*map(int, version))


@dataclass
class Example:
    """An example in a message entry."""

    name: str
    description: str
    before: str | None
    after: str | None

    @classmethod
    def parse(cls, entry) -> Example:
        name = entry.get("name")
        assert name, entry

        description = entry.get("description")
        assert description

        before = entry.get("before")
        after = entry.get("after")
        # Either none or both of them
        assert not (bool(before) ^ bool(after))

        return Example(name=name, description=description, before=before, after=after)


@dataclass
class Entry:
    """An xkbcommon message entry in the message registry"""

    VALID_TYPES = ("warning", "error")

    code: int
    """A unique strictly positive integer identifier"""
    id: str
    """A unique short human-readable string identifier"""
    type: str
    """Log level of the message"""
    description: str
    """A short description of the meaning of the message"""
    details: str
    """A long description of the meaning of the message"""
    added: Version
    """Version of xkbcommon the message has been added"""
    removed: Version | None
    """Version of xkbcommon the message has been removed"""
    examples: tuple[Example, ...]
    """
    Optional examples of situations in which the message occurs.
    If the message is an error or a warning, also provide hints on how to fix it.
    """

    @classmethod
    def parse(cls, entry) -> Entry:
        code = entry.get("code")
        assert code is not None and isinstance(code, int) and code > 0, entry

        id = entry.get("id")
        assert id is not None, entry

        type_ = entry.get("type")
        assert type_ in cls.VALID_TYPES, entry

        description = entry.get("description")
        assert description is not None, entry

        details = entry.get("details", "")

        raw_added = entry.get("added", "")
        assert raw_added, entry

        added = Version.parse(raw_added)
        assert added, entry

        if removed := entry.get("removed"):
            removed = Version.parse(removed)
            assert added < removed, entry

        if examples := entry.get("examples", ()):
            examples = tuple(map(Example.parse, examples))

        return Entry(
            code=code,
            id=id,
            type=type_,
            description=description,
            added=added,
            removed=removed,
            details=details,
            examples=examples,
        )

    @property
    def message_code(self) -> str:
        """Format the message code for display"""
        return f"XKB-{self.code:0>3}"

    @property
    def message_code_constant(self: Entry) -> str:
        """Returns the C enumeration member denoting the message code"""
        id = self.id.replace("-", "_").upper()
        return f"XKB_{self.type.upper()}_{id}"

    @property
    def message_name(self: Entry):
        """Format the message string identifier for display"""
        return self.id.replace("-", " ").capitalize()


def prepend_todo(text: str) -> str:
    if text.startswith("TODO"):
        return f"""<span class="todo">{text[:5]}</span>{text[5:]}"""
    else:
        return text


def load_message_registry(
    env: jinja2.Environment, constants: dict[str, int], path: Path
) -> Sequence[Entry]:
    # Load the message registry YAML file as a Jinja2 template
    registry_template = env.get_template(str(path))

    # Load message registry
    message_registry = sorted(
        map(Entry.parse, yaml.safe_load(registry_template.render(constants))),
        key=lambda e: e.code,
    )

    # Check message codes and identifiers are unique
    codes: set[int] = set()
    identifiers: set[str] = set()
    for n, entry in enumerate(message_registry):
        if entry.code in codes:
            raise ValueError("Duplicated code in entry #{n}: {entry.code}")
        if entry.id in identifiers:
            raise ValueError("Duplicated identifier in entry #{n}: {entry.id}")
        codes.add(entry.code)
        identifiers.add(entry.id)

    return message_registry


def generate(
    registry: Sequence[Entry],
    env: jinja2.Environment,
    root: Path,
    file: Path,
    skip_removed: bool = False,
):
    """Generate a file from its Jinja2 template and the message registry"""
    template_path = file.with_suffix(f"{file.suffix}.jinja")
    template = env.get_template(str(template_path))
    path = root / file
    script = Path(__file__).name
    with path.open("wt", encoding="utf-8") as fd:
        entries = (
            tuple(filter(lambda e: e.removed is None, registry))
            if skip_removed
            else registry
        )
        fd.writelines(template.generate(entries=entries, script=script))


T = TypeVar("T")


@dataclass
class Constant(Generic[T]):
    name: str
    pattern: re.Pattern
    conversion: Callable[[str], T]


def read_constants(path: Path, patterns: Sequence[Constant[T]]) -> dict[str, T]:
    constants: dict[str, T] = {}
    patternsʹ = list(patterns)
    with path.open("rt", encoding="utf-8") as fd:
        for line in fd:
            for k, constant in enumerate(patternsʹ):
                if m := constant.pattern.match(line):
                    constants[constant.name] = constant.conversion(m.group(1))
                    del patternsʹ[k]
                    continue  # Expect only one match per line
            if not patternsʹ:
                # No more pattern to match
                break
    for constant in patternsʹ:
        print(f"ERROR: could not find constant: {constant.name}.")
    if patternsʹ:
        raise ValueError("Some constants were not found.")
    return constants


# Root of the project
ROOT = Path(__file__).parent.parent

# Parse commands
parser = argparse.ArgumentParser(description="Generate files from the message registry")
parser.add_argument(
    "--root",
    type=Path,
    default=ROOT,
    help="Path to the root of the project (default: %(default)s)",
)

args = parser.parse_args()

# Read some constants from libxkbcommon that we need
constants = read_constants(
    Path(__file__).parent.parent / "src" / "keymap.h",
    (Constant("XKB_MAX_GROUPS", re.compile("^#define\s+XKB_MAX_GROUPS\s+(\d+)"), int),),
)

# Configure Jinja
template_loader = jinja2.FileSystemLoader(args.root, encoding="utf-8")
jinja_env = jinja2.Environment(
    loader=template_loader,
    keep_trailing_newline=True,
    trim_blocks=True,
    lstrip_blocks=True,
)
jinja_env.filters["prepend_todo"] = prepend_todo

# Load message registry
message_registry = load_message_registry(
    jinja_env, constants, Path("doc/message-registry.yaml")
)

# Generate the files
generate(
    message_registry,
    jinja_env,
    args.root,
    Path("src/messages-codes.h"),
    skip_removed=True,
)
generate(
    message_registry, jinja_env, args.root, Path("tools/messages.c"), skip_removed=True
)
generate(message_registry, jinja_env, args.root, Path("doc/message-registry.md"))
