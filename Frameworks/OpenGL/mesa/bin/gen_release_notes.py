#!/usr/bin/env python3
# Copyright © 2019-2020 Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

"""Generates release notes for a given version of mesa."""

import asyncio
import datetime
import os
import pathlib
import re
import subprocess
import sys
import textwrap
import typing
import urllib.parse

import aiohttp
from mako.template import Template
from mako import exceptions

import docutils.utils
import docutils.parsers.rst.states as states

CURRENT_GL_VERSION = '4.6'
CURRENT_VK_VERSION = '1.3'

TEMPLATE = Template(textwrap.dedent("""\
    ${header}
    ${header_underline}

    %if not bugfix:
    Mesa ${this_version} is a new development release. People who are concerned
    with stability and reliability should stick with a previous release or
    wait for Mesa ${this_version[:-1]}1.
    %else:
    Mesa ${this_version} is a bug fix release which fixes bugs found since the ${previous_version} release.
    %endif

    Mesa ${this_version} implements the OpenGL ${gl_version} API, but the version reported by
    glGetString(GL_VERSION) or glGetIntegerv(GL_MAJOR_VERSION) /
    glGetIntegerv(GL_MINOR_VERSION) depends on the particular driver being used.
    Some drivers don't support all the features required in OpenGL ${gl_version}. OpenGL
    ${gl_version} is **only** available if requested at context creation.
    Compatibility contexts may report a lower version depending on each driver.

    Mesa ${this_version} implements the Vulkan ${vk_version} API, but the version reported by
    the apiVersion property of the VkPhysicalDeviceProperties struct
    depends on the particular driver being used.

    SHA256 checksum
    ---------------

    ::

        TBD.


    New features
    ------------

    %for f in features:
    - ${rst_escape(f)}
    %endfor


    Bug fixes
    ---------

    %for b in bugs:
    - ${rst_escape(b)}
    %endfor


    Changes
    -------
    %for c, author_line in changes:
      %if author_line:

    ${rst_escape(c)}

      %else:
    - ${rst_escape(c)}
      %endif
    %endfor
    """))


# copied from https://docutils.sourceforge.io/sandbox/xml2rst/xml2rstlib/markup.py
class Inliner(states.Inliner):
    """
    Recognizer for inline markup. Derive this from the original inline
    markup parser for best results.
    """

    # Copy static attributes from super class
    vars().update(vars(states.Inliner))

    def quoteInline(self, text):
        """
        `text`: ``str``
          Return `text` with inline markup quoted.
        """
        # Method inspired by `states.Inliner.parse`
        self.document = docutils.utils.new_document("<string>")
        self.document.settings.trim_footnote_reference_space = False
        self.document.settings.character_level_inline_markup = False
        self.document.settings.pep_references = False
        self.document.settings.rfc_references = False

        self.init_customizations(self.document.settings)

        self.reporter = self.document.reporter
        self.reporter.stream = None
        self.language = None
        self.parent = self.document
        remaining = docutils.utils.escape2null(text)
        checked = ""
        processed = []
        unprocessed = []
        messages = []
        while remaining:
            original = remaining
            match = self.patterns.initial.search(remaining)
            if match:
                groups = match.groupdict()
                method = self.dispatch[groups['start'] or groups['backquote']
                                       or groups['refend'] or groups['fnend']]
                before, inlines, remaining, sysmessages = method(self, match, 0)
                checked += before
                if inlines:
                    assert len(inlines) == 1, "More than one inline found"
                    inline = original[len(before)
                                      :len(original) - len(remaining)]
                    rolePfx = re.search("^:" + self.simplename + ":(?=`)",
                                        inline)
                    refSfx = re.search("_+$", inline)
                    if rolePfx:
                        # Prefixed roles need to be quoted in the middle
                        checked += (inline[:rolePfx.end()] + "\\"
                                    + inline[rolePfx.end():])
                    elif refSfx and not re.search("^`", inline):
                        # Pure reference markup needs to be quoted at the end
                        checked += (inline[:refSfx.start()] + "\\"
                                    + inline[refSfx.start():])
                    else:
                        # Quote other inlines by prefixing
                        checked += "\\" + inline
            else:
                checked += remaining
                break
        # Quote all original backslashes
        checked = re.sub('\x00', "\\\x00", checked)
        checked = re.sub('@', '\\@', checked)
        return docutils.utils.unescape(checked, 1)

inliner = Inliner();


async def gather_commits(version: str) -> str:
    p = await asyncio.create_subprocess_exec(
        'git', 'log', '--oneline', f'mesa-{version}..', '-i', '--grep', r'\(Closes\|Fixes\): \(https\|#\).*',
        stdout=asyncio.subprocess.PIPE)
    out, _ = await p.communicate()
    assert p.returncode == 0, f"git log didn't work: {version}"
    return out.decode().strip()


async def parse_issues(commits: str) -> typing.List[str]:
    issues: typing.List[str] = []
    for commit in commits.split('\n'):
        sha, message = commit.split(maxsplit=1)
        p = await asyncio.create_subprocess_exec(
            'git', 'log', '--max-count', '1', r'--format=%b', sha,
            stdout=asyncio.subprocess.PIPE)
        _out, _ = await p.communicate()
        out = _out.decode().split('\n')

        for line in reversed(out):
            if not line.lower().startswith(('closes:', 'fixes:')):
                continue
            bug = line.split(':', 1)[1].strip()
            if (bug.startswith('https://gitlab.freedesktop.org/mesa/mesa')
                # Avoid parsing "merge_requests" URL. Note that a valid issue
                # URL may or may not contain the "/-/" text, so we check if
                # the word "issues" is contained in URL.
                and '/issues' in bug):
                # This means we have a bug in the form "Closes: https://..."
                issues.append(os.path.basename(urllib.parse.urlparse(bug).path))
            elif ',' in bug:
                multiple_bugs = [b.strip().lstrip('#') for b in bug.split(',')]
                if not all(b.isdigit() for b in multiple_bugs):
                    # this is likely a "Fixes" tag that refers to a commit name
                    continue
                issues.extend(multiple_bugs)
            elif bug.startswith('#'):
                issues.append(bug.lstrip('#'))

    return issues


async def gather_bugs(version: str) -> typing.List[str]:
    commits = await gather_commits(version)
    if commits:
        issues = await parse_issues(commits)
    else:
        issues = []

    loop = asyncio.get_event_loop()
    async with aiohttp.ClientSession(loop=loop) as session:
        results = await asyncio.gather(*[get_bug(session, i) for i in issues])
    typing.cast(typing.Tuple[str, ...], results)
    bugs = list(results)
    if not bugs:
        bugs = ['None']
    return bugs


async def get_bug(session: aiohttp.ClientSession, bug_id: str) -> str:
    """Query gitlab to get the name of the issue that was closed."""
    # Mesa's gitlab id is 176,
    url = 'https://gitlab.freedesktop.org/api/v4/projects/176/issues'
    params = {'iids[]': bug_id}
    async with session.get(url, params=params) as response:
        content = await response.json()
    if not content:
        # issues marked as "confidential" look like "404" page for
        # unauthorized users
        return f'Confidential issue #{bug_id}'
    else:
        return content[0]['title']


async def get_shortlog(version: str) -> str:
    """Call git shortlog."""
    p = await asyncio.create_subprocess_exec('git', 'shortlog', f'mesa-{version}..',
                                             stdout=asyncio.subprocess.PIPE)
    out, _ = await p.communicate()
    assert p.returncode == 0, 'error getting shortlog'
    assert out is not None, 'just for mypy'
    return out.decode()


def walk_shortlog(log: str) -> typing.Generator[typing.Tuple[str, bool], None, None]:
    for l in log.split('\n'):
        if l.startswith(' '): # this means we have a patch description
            yield l.lstrip(), False
        elif l.strip():
            yield l, True


def calculate_next_version(version: str, is_point: bool) -> str:
    """Calculate the version about to be released."""
    if '-' in version:
        version = version.split('-')[0]
    if is_point:
        base = version.split('.')
        base[2] = str(int(base[2]) + 1)
        return '.'.join(base)
    return version


def calculate_previous_version(version: str, is_point: bool) -> str:
    """Calculate the previous version to compare to.

    In the case of -rc to final that version is the previous .0 release,
    (19.3.0 in the case of 20.0.0, for example). for point releases that is
    the last point release. This value will be the same as the input value
    for a point release, but different for a major release.
    """
    if '-' in version:
        version = version.split('-')[0]
    if is_point:
        return version
    base = version.split('.')
    if base[1] == '0':
        base[0] = str(int(base[0]) - 1)
        base[1] = '3'
    else:
        base[1] = str(int(base[1]) - 1)
    return '.'.join(base)


def get_features(is_point_release: bool) -> typing.Generator[str, None, None]:
    p = pathlib.Path('docs') / 'relnotes' / 'new_features.txt'
    if p.exists() and p.stat().st_size > 0:
        if is_point_release:
            print("WARNING: new features being introduced in a point release", file=sys.stderr)
        with p.open('rt') as f:
            for line in f:
                yield line.rstrip()
        p.unlink()
        subprocess.run(['git', 'add', p])
    else:
        yield "None"


def update_release_notes_index(version: str) -> None:
    relnotes_index_path = pathlib.Path('docs') / 'relnotes.rst'

    with relnotes_index_path.open('r') as f:
        relnotes = f.readlines()

    new_relnotes = []
    first_list = True
    second_list = True
    for line in relnotes:
        if first_list and line.startswith('-'):
            first_list = False
            new_relnotes.append(f'-  :doc:`{version} release notes <relnotes/{version}>`\n')
        if (not first_list and second_list and
            re.match(r'   \d+.\d+(.\d+)? <relnotes/\d+.\d+(.\d+)?>', line)):
            second_list = False
            new_relnotes.append(f'   {version} <relnotes/{version}>\n')
        new_relnotes.append(line)

    with relnotes_index_path.open('w', encoding='utf-8') as f:
        for line in new_relnotes:
            f.write(line)

    subprocess.run(['git', 'add', relnotes_index_path])


async def main() -> None:
    v = pathlib.Path('VERSION')
    with v.open('rt') as f:
        raw_version = f.read().strip()
    is_point_release = '-rc' not in raw_version
    assert '-devel' not in raw_version, 'Do not run this script on -devel'
    version = raw_version.split('-')[0]
    previous_version = calculate_previous_version(version, is_point_release)
    this_version = calculate_next_version(version, is_point_release)
    today = datetime.date.today()
    header = f'Mesa {this_version} Release Notes / {today}'
    header_underline = '=' * len(header)

    shortlog, bugs = await asyncio.gather(
        get_shortlog(previous_version),
        gather_bugs(previous_version),
    )

    final = pathlib.Path('docs') / 'relnotes' / f'{this_version}.rst'
    with final.open('wt', encoding='utf-8') as f:
        try:
            f.write(TEMPLATE.render(
                bugfix=is_point_release,
                bugs=bugs,
                changes=walk_shortlog(shortlog),
                features=get_features(is_point_release),
                gl_version=CURRENT_GL_VERSION,
                this_version=this_version,
                header=header,
                header_underline=header_underline,
                previous_version=previous_version,
                vk_version=CURRENT_VK_VERSION,
                rst_escape=inliner.quoteInline,
            ))
        except:
            print(exceptions.text_error_template().render())
            return

    subprocess.run(['git', 'add', final])

    update_release_notes_index(this_version)

    subprocess.run(['git', 'commit', '-m',
                    f'docs: add release notes for {this_version}'])


if __name__ == "__main__":
    loop = asyncio.get_event_loop()
    loop.run_until_complete(main())
