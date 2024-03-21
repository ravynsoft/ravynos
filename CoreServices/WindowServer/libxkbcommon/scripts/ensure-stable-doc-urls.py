#!/usr/bin/env python3

# Doc URLs may change with time because they depend on Doxygen machinery.
# This is unfortunate because it is good practice to keep valid URLs.
# See: “Cool URIs don’t change” at https://www.w3.org/Provider/Style/URI.html.
#
# There is no built-in solution in Doxygen that we are aware of.
# The solution proposed here is to maintain a registry of all URLs and manage
# legacy URLs as redirections to their canonical page.

import argparse
from enum import IntFlag
import glob
from itertools import chain
from pathlib import Path
from string import Template
from typing import NamedTuple, Sequence

import yaml


class Update(NamedTuple):
    new: str
    old: str


class ExitCode(IntFlag):
    NORMAL = 0
    INVALID_UPDATES = 1 << 4
    MISSING_UPDATES = 1 << 5


THIS_SCRIPT_PATH = Path(__file__)
RELATIVE_SCRIPT_PATH = THIS_SCRIPT_PATH.relative_to(THIS_SCRIPT_PATH.parent.parent)

REDIRECTION_DELAY = 6  # in seconds. Note: at least 6s for accessibility

# NOTE: The redirection works with the HTML tag: <meta http-equiv="refresh">.
# See: https://developer.mozilla.org/en-US/docs/Web/HTML/Element/meta#http-equiv
#
# NOTE: This page is a simplified version of the Doxygen-generated ones.
# It does use the current stylesheets, but it may break if the theme is updated.
# Ideally, we would just let Doxygen generate them, but I (Wismill) could not
# find a way to do this with the redirection feature.
REDIRECTION_PAGE_TEMPLATE = Template(
    """<!DOCTYPE HTML>
<html lang="en-US">
    <head>
        <meta charset="UTF-8">
        <meta http-equiv="refresh" content="${delay}; url=${canonical}">
        <link href="doxygen.css" rel="stylesheet" type="text/css">
        <link href="doxygen-extra.css" rel="stylesheet" type="text/css">
        <title>xkbcommon: Page Redirection</title>
    </head>
    <body>
        <div id="top">
            <div id="titlearea" style="padding: 1em 0 1em 0.5em;">
                <div id="projectname">
                    libxkbcommon
                </div>
            </div>
        </div>
        <div>
            <div class="header">
                <div class="headertitle">
                    <div class="title">🔀 Redirection</div>
                </div>
            </div>
            <div class="contents">
                <p>This page has been moved.</p>
                <p>
                    If you are not redirected automatically,
                    follow the <a href="${canonical}">link to the current page</a>.
                </p>
            </div>
        </div>
    </body>
</html>
"""
)


def parse_page_update(update: str) -> Update:
    updateʹ = Update(*update.split("="))
    if updateʹ.new == updateʹ.old:
        raise ValueError(f"Invalid update: {updateʹ}")
    return updateʹ


def update_registry(registry_path: Path, doc_dir: Path, updates: Sequence[str]):
    """
    Update the URL registry by:
    • Adding new pages
    • Updating page aliases
    """
    # Parse updates
    updates_ = dict(map(parse_page_update, updates))
    # Load previous registry
    with registry_path.open("rt", encoding="utf-8") as fd:
        registry = yaml.safe_load(fd) or {}
    # Expected updates
    missing_updates = set(file for file in registry if not (doc_dir / file).is_file())
    # Update
    invalid_updates = set(updates_)
    redirections = frozenset(chain(*registry.values()))
    for file in glob.iglob("**/*.html", root_dir=doc_dir, recursive=True):
        # Skip redirection pages
        if file in redirections:
            continue
        # Get previous entry and potential update
        old = updates_.get(file)
        if old:
            # Update old entry
            invalid_updates.remove(file)
            entry = registry.get(old)
            if entry is None:
                raise ValueError(f"Invalid update: {file}<-{old}")
            else:
                del registry[old]
                missing_updates.remove(old)
                registry[file] = [e for e in [old] + entry if e != file]
                print(f"[INFO] Updated: “{old}” to “{file}”")
        else:
            entry = registry.get(file)
            if entry is None:
                # New entry
                registry[file] = []
                print(f"[INFO] Added: {file}")
            else:
                # Keep previous entry
                pass
    exit_code = ExitCode.NORMAL
    # Check
    if invalid_updates:
        for update in invalid_updates:
            print(f"[ERROR] Update not processed: {update}")
        exit_code |= ExitCode.INVALID_UPDATES
    if missing_updates:
        for old in missing_updates:
            print(f"[ERROR] “{old}” not found and has no update.")
        exit_code |= ExitCode.MISSING_UPDATES
    if exit_code:
        print("[ERROR] Processing interrupted: please fix the errors above.")
        exit(exit_code.value)
    # Write changes
    with registry_path.open("wt", encoding="utf-8") as fd:
        fd.write(f"# WARNING: This file is autogenerated by: {RELATIVE_SCRIPT_PATH}\n")
        fd.write("#          Do not edit manually.\n")
        yaml.dump(
            registry,
            fd,
        )


def generate_redirections(registry_path: Path, doc_dir: Path):
    """
    Create redirection pages using the aliases in the given URL registry.
    """
    cool = True
    # Load registry
    with registry_path.open("rt", encoding="utf-8") as fd:
        registry = yaml.safe_load(fd) or {}
    for canonical, aliases in registry.items():
        # Check canonical path is up-to-date
        if not (doc_dir / canonical).is_file():
            cool = False
            print(
                f"ERROR: missing canonical documentation page “{canonical}”. "
                f"Please update “{registry_path}” using {RELATIVE_SCRIPT_PATH}”."
            )
        # Add a redirection page
        for alias in aliases:
            path = doc_dir / alias
            with path.open("wt", encoding="utf-8") as fd:
                fd.write(
                    REDIRECTION_PAGE_TEMPLATE.substitute(
                        canonical=canonical, delay=REDIRECTION_DELAY
                    )
                )
    if not cool:
        exit(1)


def add_registry_argument(parser):
    parser.add_argument(
        "registry",
        type=Path,
        help="Path to the doc URI registry.",
    )


def add_docdir_argument(parser):
    parser.add_argument(
        "docdir",
        type=Path,
        metavar="DOC_DIR",
        help="Path to the generated HTML documentation directory.",
    )


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Tool to ensure HTML documentation has stable URLs"
    )
    subparsers = parser.add_subparsers()

    parser_registry = subparsers.add_parser(
        "update-registry", help="Update the registry of URIs"
    )
    add_registry_argument(parser_registry)
    add_docdir_argument(parser_registry)
    parser_registry.add_argument(
        "updates",
        nargs="*",
        type=str,
        help="Update: new=previous entries",
    )
    parser_registry.set_defaults(
        run=lambda args: update_registry(args.registry, args.docdir, args.updates)
    )

    parser_redirections = subparsers.add_parser(
        "generate-redirections", help="Generate URIs redirections"
    )
    add_registry_argument(parser_redirections)
    add_docdir_argument(parser_redirections)
    parser_redirections.set_defaults(
        run=lambda args: generate_redirections(args.registry, args.docdir)
    )

    args = parser.parse_args()
    args.run(args)
