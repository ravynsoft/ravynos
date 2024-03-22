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

"""Urwid UI for pick script."""

import asyncio
import itertools
import textwrap
import typing

import attr
import urwid

from . import core

if typing.TYPE_CHECKING:
    WidgetType = typing.TypeVar('WidgetType', bound=urwid.Widget)

PALETTE = [
    ('a', 'black', 'light gray'),
    ('b', 'black', 'dark red'),
    ('bg', 'black', 'dark blue'),
    ('reversed', 'standout', ''),
]


class RootWidget(urwid.Frame):

    def __init__(self, *args, ui: 'UI', **kwargs):
        super().__init__(*args, **kwargs)
        self.ui = ui


class CommitList(urwid.ListBox):

    def __init__(self, *args, ui: 'UI', **kwargs):
        super().__init__(*args, **kwargs)
        self.ui = ui

    def keypress(self, size: int, key: str) -> typing.Optional[str]:
        if key == 'q':
            raise urwid.ExitMainLoop()
        elif key == 'u':
            asyncio.ensure_future(self.ui.update())
        elif key == 'a':
            self.ui.add()
        else:
            return super().keypress(size, key)
        return None


class CommitWidget(urwid.Text):

    # urwid.Text is normally not interactable, this is required to tell urwid
    # to use our keypress method
    _selectable = True

    def __init__(self, ui: 'UI', commit: 'core.Commit'):
        reason = commit.nomination_type.name.ljust(6)
        super().__init__(f'{commit.date()} {reason} {commit.sha[:10]} {commit.description}')
        self.ui = ui
        self.commit = commit

    async def apply(self) -> None:
        async with self.ui.git_lock:
            result, err = await self.commit.apply(self.ui)
            if not result:
                self.ui.chp_failed(self, err)
            else:
                self.ui.remove_commit(self)

    async def denominate(self) -> None:
        async with self.ui.git_lock:
            await self.commit.denominate(self.ui)
            self.ui.remove_commit(self)

    async def backport(self) -> None:
        async with self.ui.git_lock:
            await self.commit.backport(self.ui)
            self.ui.remove_commit(self)

    def keypress(self, size: int, key: str) -> typing.Optional[str]:
        if key == 'c':
            asyncio.ensure_future(self.apply())
        elif key == 'd':
            asyncio.ensure_future(self.denominate())
        elif key == 'b':
            asyncio.ensure_future(self.backport())
        else:
            return key
        return None


class FocusAwareEdit(urwid.Edit):

    """An Edit type that signals when it comes into and leaves focus."""

    signals = urwid.Edit.signals + ['focus_changed']

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.__is_focus = False

    def render(self, size: typing.Tuple[int], focus: bool = False) -> urwid.Canvas:
        if focus != self.__is_focus:
            self._emit("focus_changed", focus)
            self.__is_focus = focus
        return super().render(size, focus)


@attr.s(slots=True)
class UI:

    """Main management object.

    :previous_commits: A list of commits to main since this branch was created
    :new_commits: Commits added to main since the last time this script was run
    """

    commit_list: typing.List['urwid.Button'] = attr.ib(factory=lambda: urwid.SimpleFocusListWalker([]), init=False)
    feedback_box: typing.List['urwid.Text'] = attr.ib(factory=lambda: urwid.SimpleFocusListWalker([]), init=False)
    notes: 'FocusAwareEdit' = attr.ib(factory=lambda: FocusAwareEdit('', multiline=True), init=False)
    header: 'urwid.Text' = attr.ib(factory=lambda: urwid.Text('Mesa Stable Picker', align='center'), init=False)
    body: 'urwid.Columns' = attr.ib(attr.Factory(lambda s: s._make_body(), True), init=False)
    footer: 'urwid.Columns' = attr.ib(attr.Factory(lambda s: s._make_footer(), True), init=False)
    root: RootWidget = attr.ib(attr.Factory(lambda s: s._make_root(), True), init=False)
    mainloop: urwid.MainLoop = attr.ib(None, init=False)

    previous_commits: typing.List['core.Commit'] = attr.ib(factory=list, init=False)
    new_commits: typing.List['core.Commit'] = attr.ib(factory=list, init=False)
    git_lock: asyncio.Lock = attr.ib(factory=asyncio.Lock, init=False)

    def _get_current_commit(self) -> typing.Optional['core.Commit']:
        entry = self.commit_list.get_focus()[0]
        return entry.original_widget.commit if entry is not None else None

    def _change_notes_cb(self) -> None:
        commit = self._get_current_commit()
        if commit and commit.notes:
            self.notes.set_edit_text(commit.notes)
        else:
            self.notes.set_edit_text('')

    def _change_notes_focus_cb(self, notes: 'FocusAwareEdit', focus: 'bool') -> 'None':
        # in the case of coming into focus we don't want to do anything
        if focus:
            return
        commit = self._get_current_commit()
        if commit is None:
            return
        text: str = notes.get_edit_text()
        if text != commit.notes:
            asyncio.ensure_future(commit.update_notes(self, text))

    def _make_body(self) -> 'urwid.Columns':
        commits = CommitList(self.commit_list, ui=self)
        feedback = urwid.ListBox(self.feedback_box)
        urwid.connect_signal(self.commit_list, 'modified', self._change_notes_cb)
        notes = urwid.Filler(self.notes)
        urwid.connect_signal(self.notes, 'focus_changed', self._change_notes_focus_cb)

        return urwid.Columns([urwid.LineBox(commits), urwid.Pile([urwid.LineBox(notes), urwid.LineBox(feedback)])])

    def _make_footer(self) -> 'urwid.Columns':
        body = [
            urwid.Text('[U]pdate'),
            urwid.Text('[Q]uit'),
            urwid.Text('[C]herry Pick'),
            urwid.Text('[D]enominate'),
            urwid.Text('[B]ackport'),
            urwid.Text('[A]pply additional patch'),
        ]
        return urwid.Columns(body)

    def _make_root(self) -> 'RootWidget':
        return RootWidget(self.body, urwid.LineBox(self.header), urwid.LineBox(self.footer), 'body', ui=self)

    def render(self) -> 'WidgetType':
        asyncio.ensure_future(self.update())
        return self.root

    def load(self) -> None:
        self.previous_commits = core.load()

    async def update(self) -> None:
        self.load()
        with open('VERSION', 'r') as f:
            version = '.'.join(f.read().split('.')[:2])
        if self.previous_commits:
            sha = self.previous_commits[0].sha
        else:
            sha = f'{version}-branchpoint'

        new_commits = await core.get_new_commits(sha)

        if new_commits:
            pb = urwid.ProgressBar('a', 'b', done=len(new_commits))
            o = self.mainloop.widget
            self.mainloop.widget = urwid.Overlay(
                urwid.Filler(urwid.LineBox(pb)), o, 'center', ('relative', 50), 'middle', ('relative', 50))
            self.new_commits = await core.gather_commits(
                version, self.previous_commits, new_commits,
                lambda: pb.set_completion(pb.current + 1))
            self.mainloop.widget = o

        for commit in reversed(list(itertools.chain(self.new_commits, self.previous_commits))):
            if commit.nominated and commit.resolution is core.Resolution.UNRESOLVED:
                b = urwid.AttrMap(CommitWidget(self, commit), None, focus_map='reversed')
                self.commit_list.append(b)
        self.save()

    async def feedback(self, text: str) -> None:
        self.feedback_box.append(urwid.AttrMap(urwid.Text(text), None))
        latest_item_index = len(self.feedback_box) - 1
        self.feedback_box.set_focus(latest_item_index)

    def remove_commit(self, commit: CommitWidget) -> None:
        for i, c in enumerate(self.commit_list):
            if c.base_widget is commit:
                del self.commit_list[i]
                break

    def save(self):
        core.save(itertools.chain(self.new_commits, self.previous_commits))

    def add(self) -> None:
        """Add an additional commit which isn't nominated."""
        o = self.mainloop.widget

        def reset_cb(_) -> None:
            self.mainloop.widget = o

        async def apply_cb(edit: urwid.Edit) -> None:
            text: str = edit.get_edit_text()

            # In case the text is empty
            if not text:
                return

            sha = await core.full_sha(text)
            for c in reversed(list(itertools.chain(self.new_commits, self.previous_commits))):
                if c.sha == sha:
                    commit = c
                    break
            else:
                raise RuntimeError(f"Couldn't find {sha}")

            await commit.apply(self)

        q = urwid.Edit("Commit sha\n")
        ok_btn = urwid.Button('Ok')
        urwid.connect_signal(ok_btn, 'click', lambda _: asyncio.ensure_future(apply_cb(q)))
        urwid.connect_signal(ok_btn, 'click', reset_cb)

        can_btn = urwid.Button('Cancel')
        urwid.connect_signal(can_btn, 'click', reset_cb)

        cols = urwid.Columns([ok_btn, can_btn])
        pile = urwid.Pile([q, cols])
        box = urwid.LineBox(pile)

        self.mainloop.widget = urwid.Overlay(
            urwid.Filler(box), o, 'center', ('relative', 50), 'middle', ('relative', 50)
        )

    def chp_failed(self, commit: 'CommitWidget', err: str) -> None:
        o = self.mainloop.widget

        def reset_cb(_) -> None:
            self.mainloop.widget = o

        t = urwid.Text(textwrap.dedent(f"""
            Failed to apply {commit.commit.sha} {commit.commit.description} with the following error:

            {err}

            You can either cancel, or resolve the conflicts (`git mergetool`), finish the
            cherry-pick (`git cherry-pick --continue`) and select ok."""))

        can_btn = urwid.Button('Cancel')
        urwid.connect_signal(can_btn, 'click', reset_cb)
        urwid.connect_signal(
            can_btn, 'click', lambda _: asyncio.ensure_future(commit.commit.abort_cherry(self, err)))

        ok_btn = urwid.Button('Ok')
        urwid.connect_signal(ok_btn, 'click', reset_cb)
        urwid.connect_signal(
            ok_btn, 'click', lambda _: asyncio.ensure_future(commit.commit.resolve(self)))
        urwid.connect_signal(
            ok_btn, 'click', lambda _: self.remove_commit(commit))

        cols = urwid.Columns([ok_btn, can_btn])
        pile = urwid.Pile([t, cols])
        box = urwid.LineBox(pile)

        self.mainloop.widget = urwid.Overlay(
            urwid.Filler(box), o, 'center', ('relative', 50), 'middle', ('relative', 50)
        )
