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

"""Tests for pick's core data structures and routines."""

from unittest import mock
import textwrap
import typing

import attr
import pytest

from . import core


class TestCommit:

    @pytest.fixture
    def unnominated_commit(self) -> 'core.Commit':
        return core.Commit('abc123', 'sub: A commit', main_sha='45678')

    @pytest.fixture
    def nominated_commit(self) -> 'core.Commit':
        return core.Commit('abc123', 'sub: A commit', True,
                           core.NominationType.CC, core.Resolution.UNRESOLVED)

    class TestToJson:

        def test_not_nominated(self, unnominated_commit: 'core.Commit'):
            c = unnominated_commit
            v = c.to_json()
            assert v == {'sha': 'abc123', 'description': 'sub: A commit', 'nominated': False,
                         'nomination_type': None, 'resolution': core.Resolution.UNRESOLVED.value,
                         'main_sha': '45678', 'because_sha': None}

        def test_nominated(self, nominated_commit: 'core.Commit'):
            c = nominated_commit
            v = c.to_json()
            assert v == {'sha': 'abc123',
                         'description': 'sub: A commit',
                         'nominated': True,
                         'nomination_type': core.NominationType.CC.value,
                         'resolution': core.Resolution.UNRESOLVED.value,
                         'main_sha': None,
                         'because_sha': None}

    class TestFromJson:

        def test_not_nominated(self, unnominated_commit: 'core.Commit'):
            c = unnominated_commit
            v = c.to_json()
            c2 = core.Commit.from_json(v)
            assert c == c2

        def test_nominated(self, nominated_commit: 'core.Commit'):
            c = nominated_commit
            v = c.to_json()
            c2 = core.Commit.from_json(v)
            assert c == c2


class TestRE:

    """Tests for the regular expressions used to identify commits."""

    class TestFixes:

        def test_simple(self):
            message = textwrap.dedent("""\
                etnaviv: fix vertex buffer state emission for single stream GPUs

                GPUs with a single supported vertex stream must use the single state
                address to program the stream.

                Fixes: 3d09bb390a39 (etnaviv: GC7000: State changes for HALTI3..5)
                Signed-off-by: Lucas Stach <l.stach@pengutronix.de>
                Reviewed-by: Jonathan Marek <jonathan@marek.ca>
            """)

            fix_for_commit = core.IS_FIX.search(message)
            assert fix_for_commit is not None
            assert fix_for_commit.group(1) == '3d09bb390a39'

    class TestCC:

        def test_single_branch(self):
            """Tests commit meant for a single branch, ie, 19.1"""
            message = textwrap.dedent("""\
                radv: fix DCC fast clear code for intensity formats

                This fixes a rendering issue with DiRT 4 on GFX10. Only GFX10 was
                affected because intensity formats are different.

                Cc: 19.2 <mesa-stable@lists.freedesktop.org>
                Closes: https://gitlab.freedesktop.org/mesa/mesa/-/issues/1923
                Signed-off-by: Samuel Pitoiset <samuel.pitoiset@gmail.com>
                Reviewed-by: Bas Nieuwenhuizen <bas@basnieuwenhuizen.nl>
            """)

            cc_to = core.IS_CC.search(message)
            assert cc_to is not None
            assert cc_to.group(1) == '19.2'

        def test_multiple_branches(self):
            """Tests commit with more than one branch specified"""
            message = textwrap.dedent("""\
                radeonsi: enable zerovram for Rocket League

                Fixes corruption on game startup.
                Closes: https://gitlab.freedesktop.org/mesa/mesa/-/issues/1888

                Cc: 19.1 19.2 <mesa-stable@lists.freedesktop.org>
                Reviewed-by: Pierre-Eric Pelloux-Prayer <pierre-eric.pelloux-prayer@amd.com>
            """)

            cc_to = core.IS_CC.search(message)
            assert cc_to is not None
            assert cc_to.group(1) == '19.1'
            assert cc_to.group(2) == '19.2'

        def test_no_branch(self):
            """Tests commit with no branch specification"""
            message = textwrap.dedent("""\
                anv/android: fix images created with external format support

                This fixes a case where user first creates image and then later binds it
                with memory created from AHW buffer.

                Cc: <mesa-stable@lists.freedesktop.org>
                Signed-off-by: Tapani Pälli <tapani.palli@intel.com>
                Reviewed-by: Lionel Landwerlin <lionel.g.landwerlin@intel.com>
            """)

            cc_to = core.IS_CC.search(message)
            assert cc_to is not None

        def test_quotes(self):
            """Tests commit with quotes around the versions"""
            message = textwrap.dedent("""\
                 anv: Always fill out the AUX table even if CCS is disabled

                 Cc: "20.0" mesa-stable@lists.freedesktop.org
                 Reviewed-by: Kenneth Graunke <kenneth@whitecape.org>
                 Tested-by: Marge Bot <https://gitlab.freedesktop.org/mesa/mesa/-/merge_requests/3454>
                 Part-of: <https://gitlab.freedesktop.org/mesa/mesa/-/merge_requests/3454>
            """)

            cc_to = core.IS_CC.search(message)
            assert cc_to is not None
            assert cc_to.group(1) == '20.0'

        def test_multiple_quotes(self):
            """Tests commit with quotes around the versions"""
            message = textwrap.dedent("""\
                 anv: Always fill out the AUX table even if CCS is disabled

                 Cc: "20.0" "20.1" mesa-stable@lists.freedesktop.org
                 Reviewed-by: Kenneth Graunke <kenneth@whitecape.org>
                 Tested-by: Marge Bot <https://gitlab.freedesktop.org/mesa/mesa/-/merge_requests/3454>
                 Part-of: <https://gitlab.freedesktop.org/mesa/mesa/-/merge_requests/3454>
            """)

            cc_to = core.IS_CC.search(message)
            assert cc_to is not None
            assert cc_to.group(1) == '20.0'
            assert cc_to.group(2) == '20.1'

        def test_single_quotes(self):
            """Tests commit with quotes around the versions"""
            message = textwrap.dedent("""\
                 anv: Always fill out the AUX table even if CCS is disabled

                 Cc: '20.0' mesa-stable@lists.freedesktop.org
                 Reviewed-by: Kenneth Graunke <kenneth@whitecape.org>
                 Tested-by: Marge Bot <https://gitlab.freedesktop.org/mesa/mesa/-/merge_requests/3454>
                 Part-of: <https://gitlab.freedesktop.org/mesa/mesa/-/merge_requests/3454>
            """)

            cc_to = core.IS_CC.search(message)
            assert cc_to is not None
            assert cc_to.group(1) == '20.0'

        def test_multiple_single_quotes(self):
            """Tests commit with quotes around the versions"""
            message = textwrap.dedent("""\
                 anv: Always fill out the AUX table even if CCS is disabled

                 Cc: '20.0' '20.1' mesa-stable@lists.freedesktop.org
                 Reviewed-by: Kenneth Graunke <kenneth@whitecape.org>
                 Tested-by: Marge Bot <https://gitlab.freedesktop.org/mesa/mesa/-/merge_requests/3454>
                 Part-of: <https://gitlab.freedesktop.org/mesa/mesa/-/merge_requests/3454>
            """)

            cc_to = core.IS_CC.search(message)
            assert cc_to is not None
            assert cc_to.group(1) == '20.0'
            assert cc_to.group(2) == '20.1'

    class TestRevert:

        def test_simple(self):
            message = textwrap.dedent("""\
                Revert "radv: do not emit PKT3_CONTEXT_CONTROL with AMDGPU 3.6.0+"

                This reverts commit 2ca8629fa9b303e24783b76a7b3b0c2513e32fbd.

                This was initially ported from RadeonSI, but in the meantime it has
                been reverted because it might hang. Be conservative and re-introduce
                this packet emission.

                Unfortunately this doesn't fix anything known.

                Cc: 19.2 <mesa-stable@lists.freedesktop.org>
                Signed-off-by: Samuel Pitoiset <samuel.pitoiset@gmail.com>
                Reviewed-by: Bas Nieuwenhuizen <bas@basnieuwenhuizen.nl>
            """)

            revert_of = core.IS_REVERT.search(message)
            assert revert_of is not None
            assert revert_of.group(1) == '2ca8629fa9b303e24783b76a7b3b0c2513e32fbd'

    class TestBackportTo:

        def test_single_release(self):
            """Tests commit meant for a single branch, ie, 19.1"""
            message = textwrap.dedent("""\
                radv: fix DCC fast clear code for intensity formats

                This fixes a rendering issue with DiRT 4 on GFX10. Only GFX10 was
                affected because intensity formats are different.

                Backport-to: 19.2
                Closes: https://gitlab.freedesktop.org/mesa/mesa/-/issues/1923
                Signed-off-by: Samuel Pitoiset <samuel.pitoiset@gmail.com>
                Reviewed-by: Bas Nieuwenhuizen <bas@basnieuwenhuizen.nl>
            """)

            backport_to = core.IS_BACKPORT.search(message)
            assert backport_to is not None
            assert backport_to.groups() == ('19.2', None)

        def test_multiple_release_space(self):
            """Tests commit with more than one branch specified"""
            message = textwrap.dedent("""\
                radeonsi: enable zerovram for Rocket League

                Fixes corruption on game startup.
                Closes: https://gitlab.freedesktop.org/mesa/mesa/-/issues/1888

                Backport-to: 19.1 19.2
                Reviewed-by: Pierre-Eric Pelloux-Prayer <pierre-eric.pelloux-prayer@amd.com>
            """)

            backport_to = core.IS_BACKPORT.search(message)
            assert backport_to is not None
            assert backport_to.groups() == ('19.1', '19.2')

        def test_multiple_release_comma(self):
            """Tests commit with more than one branch specified"""
            message = textwrap.dedent("""\
                radeonsi: enable zerovram for Rocket League

                Fixes corruption on game startup.
                Closes: https://gitlab.freedesktop.org/mesa/mesa/-/issues/1888

                Backport-to: 19.1, 19.2
                Reviewed-by: Pierre-Eric Pelloux-Prayer <pierre-eric.pelloux-prayer@amd.com>
            """)

            backport_to = core.IS_BACKPORT.search(message)
            assert backport_to is not None
            assert backport_to.groups() == ('19.1', '19.2')


class TestResolveNomination:

    @attr.s(slots=True)
    class FakeSubprocess:

        """A fake asyncio.subprocess like class for use with mock."""

        out: typing.Optional[bytes] = attr.ib(None)
        returncode: int = attr.ib(0)

        async def mock(self, *_, **__):
            """A dirtly little helper for mocking."""
            return self

        async def communicate(self) -> typing.Tuple[bytes, bytes]:
            assert self.out is not None
            return self.out, b''

        async def wait(self) -> int:
            return self.returncode

    @staticmethod
    async def return_true(*_, **__) -> bool:
        return True

    @staticmethod
    async def return_false(*_, **__) -> bool:
        return False

    @pytest.mark.asyncio
    async def test_fix_is_nominated(self):
        s = self.FakeSubprocess(b'Fixes: 3d09bb390a39 (etnaviv: GC7000: State changes for HALTI3..5)')
        c = core.Commit('abcdef1234567890', 'a commit')

        with mock.patch('bin.pick.core.asyncio.create_subprocess_exec', s.mock):
            with mock.patch('bin.pick.core.is_commit_in_branch', self.return_true):
                await core.resolve_nomination(c, '')

        assert c.nominated
        assert c.nomination_type is core.NominationType.FIXES

    @pytest.mark.asyncio
    async def test_fix_is_not_nominated(self):
        s = self.FakeSubprocess(b'Fixes: 3d09bb390a39 (etnaviv: GC7000: State changes for HALTI3..5)')
        c = core.Commit('abcdef1234567890', 'a commit')

        with mock.patch('bin.pick.core.asyncio.create_subprocess_exec', s.mock):
            with mock.patch('bin.pick.core.is_commit_in_branch', self.return_false):
                await core.resolve_nomination(c, '')

        assert not c.nominated
        assert c.nomination_type is core.NominationType.FIXES

    @pytest.mark.asyncio
    async def test_cc_is_nominated(self):
        s = self.FakeSubprocess(b'Cc: 16.2 <mesa-stable@lists.freedesktop.org>')
        c = core.Commit('abcdef1234567890', 'a commit')

        with mock.patch('bin.pick.core.asyncio.create_subprocess_exec', s.mock):
            await core.resolve_nomination(c, '16.2')

        assert c.nominated
        assert c.nomination_type is core.NominationType.CC

    @pytest.mark.asyncio
    async def test_cc_is_nominated2(self):
        s = self.FakeSubprocess(b'Cc: mesa-stable@lists.freedesktop.org')
        c = core.Commit('abcdef1234567890', 'a commit')

        with mock.patch('bin.pick.core.asyncio.create_subprocess_exec', s.mock):
            await core.resolve_nomination(c, '16.2')

        assert c.nominated
        assert c.nomination_type is core.NominationType.CC

    @pytest.mark.asyncio
    async def test_cc_is_not_nominated(self):
        s = self.FakeSubprocess(b'Cc: 16.2 <mesa-stable@lists.freedesktop.org>')
        c = core.Commit('abcdef1234567890', 'a commit')

        with mock.patch('bin.pick.core.asyncio.create_subprocess_exec', s.mock):
            await core.resolve_nomination(c, '16.1')

        assert not c.nominated
        assert c.nomination_type is None

    @pytest.mark.asyncio
    async def test_backport_is_nominated(self):
        s = self.FakeSubprocess(b'Backport-to: 16.2')
        c = core.Commit('abcdef1234567890', 'a commit')

        with mock.patch('bin.pick.core.asyncio.create_subprocess_exec', s.mock):
            await core.resolve_nomination(c, '16.2')

        assert c.nominated
        assert c.nomination_type is core.NominationType.BACKPORT

    @pytest.mark.asyncio
    async def test_backport_is_not_nominated(self):
        s = self.FakeSubprocess(b'Backport-to: 16.2')
        c = core.Commit('abcdef1234567890', 'a commit')

        with mock.patch('bin.pick.core.asyncio.create_subprocess_exec', s.mock):
            await core.resolve_nomination(c, '16.1')

        assert not c.nominated
        assert c.nomination_type is None

    @pytest.mark.asyncio
    async def test_revert_is_nominated(self):
        s = self.FakeSubprocess(b'This reverts commit 1234567890123456789012345678901234567890.')
        c = core.Commit('abcdef1234567890', 'a commit')

        with mock.patch('bin.pick.core.asyncio.create_subprocess_exec', s.mock):
            with mock.patch('bin.pick.core.is_commit_in_branch', self.return_true):
                await core.resolve_nomination(c, '')

        assert c.nominated
        assert c.nomination_type is core.NominationType.REVERT

    @pytest.mark.asyncio
    async def test_revert_is_not_nominated(self):
        s = self.FakeSubprocess(b'This reverts commit 1234567890123456789012345678901234567890.')
        c = core.Commit('abcdef1234567890', 'a commit')

        with mock.patch('bin.pick.core.asyncio.create_subprocess_exec', s.mock):
            with mock.patch('bin.pick.core.is_commit_in_branch', self.return_false):
                await core.resolve_nomination(c, '')

        assert not c.nominated
        assert c.nomination_type is core.NominationType.REVERT

    @pytest.mark.asyncio
    async def test_is_fix_and_backport(self):
        s = self.FakeSubprocess(
            b'Fixes: 3d09bb390a39 (etnaviv: GC7000: State changes for HALTI3..5)\n'
            b'Backport-to: 16.1'
        )
        c = core.Commit('abcdef1234567890', 'a commit')

        with mock.patch('bin.pick.core.asyncio.create_subprocess_exec', s.mock):
            with mock.patch('bin.pick.core.is_commit_in_branch', self.return_true):
                await core.resolve_nomination(c, '16.1')

        assert c.nominated
        assert c.nomination_type is core.NominationType.FIXES

    @pytest.mark.asyncio
    async def test_is_fix_and_cc(self):
        s = self.FakeSubprocess(
            b'Fixes: 3d09bb390a39 (etnaviv: GC7000: State changes for HALTI3..5)\n'
            b'Cc: 16.1 <mesa-stable@lists.freedesktop.org>'
        )
        c = core.Commit('abcdef1234567890', 'a commit')

        with mock.patch('bin.pick.core.asyncio.create_subprocess_exec', s.mock):
            with mock.patch('bin.pick.core.is_commit_in_branch', self.return_true):
                await core.resolve_nomination(c, '16.1')

        assert c.nominated
        assert c.nomination_type is core.NominationType.FIXES

    @pytest.mark.asyncio
    async def test_is_fix_and_revert(self):
        s = self.FakeSubprocess(
            b'Fixes: 3d09bb390a39 (etnaviv: GC7000: State changes for HALTI3..5)\n'
            b'This reverts commit 1234567890123456789012345678901234567890.'
        )
        c = core.Commit('abcdef1234567890', 'a commit')

        with mock.patch('bin.pick.core.asyncio.create_subprocess_exec', s.mock):
            with mock.patch('bin.pick.core.is_commit_in_branch', self.return_true):
                await core.resolve_nomination(c, '16.1')

        assert c.nominated
        assert c.nomination_type is core.NominationType.FIXES

    @pytest.mark.asyncio
    async def test_is_cc_and_revert(self):
        s = self.FakeSubprocess(
            b'This reverts commit 1234567890123456789012345678901234567890.\n'
            b'Cc: 16.1 <mesa-stable@lists.freedesktop.org>'
        )
        c = core.Commit('abcdef1234567890', 'a commit')

        with mock.patch('bin.pick.core.asyncio.create_subprocess_exec', s.mock):
            with mock.patch('bin.pick.core.is_commit_in_branch', self.return_true):
                await core.resolve_nomination(c, '16.1')

        assert c.nominated
        assert c.nomination_type is core.NominationType.CC


class TestResolveFixes:

    @pytest.mark.asyncio
    async def test_in_new(self):
        """Because commit abcd is nominated, so f123 should be as well."""
        c = [
            core.Commit('f123', 'desc', nomination_type=core.NominationType.FIXES, because_sha='abcd'),
            core.Commit('abcd', 'desc', True),
        ]
        await core.resolve_fixes(c, [])
        assert c[1].nominated

    @pytest.mark.asyncio
    async def test_not_in_new(self):
        """Because commit abcd is not nominated, commit f123 shouldn't be either."""
        c = [
            core.Commit('f123', 'desc', nomination_type=core.NominationType.FIXES, because_sha='abcd'),
            core.Commit('abcd', 'desc'),
        ]
        await core.resolve_fixes(c, [])
        assert not c[0].nominated

    @pytest.mark.asyncio
    async def test_in_previous(self):
        """Because commit abcd is nominated, so f123 should be as well."""
        p = [
            core.Commit('abcd', 'desc', True),
        ]
        c = [
            core.Commit('f123', 'desc', nomination_type=core.NominationType.FIXES, because_sha='abcd'),
        ]
        await core.resolve_fixes(c, p)
        assert c[0].nominated

    @pytest.mark.asyncio
    async def test_not_in_previous(self):
        """Because commit abcd is not nominated, commit f123 shouldn't be either."""
        p = [
            core.Commit('abcd', 'desc'),
        ]
        c = [
            core.Commit('f123', 'desc', nomination_type=core.NominationType.FIXES, because_sha='abcd'),
        ]
        await core.resolve_fixes(c, p)
        assert not c[0].nominated


class TestIsCommitInBranch:

    @pytest.mark.asyncio
    async def test_no(self):
        # Hopefully this is never true?
        value = await core.is_commit_in_branch('ffffffffffffffffffffffffffffff')
        assert not value

    @pytest.mark.asyncio
    async def test_yes(self):
        # This commit is from 2000, it better always be in the branch
        value = await core.is_commit_in_branch('88f3b89a2cb77766d2009b9868c44e03abe2dbb2')
        assert value


class TestFullSha:

    @pytest.mark.asyncio
    async def test_basic(self):
        # This commit is from 2000, it better always be in the branch
        value = await core.full_sha('88f3b89a2cb777')
        assert value

    @pytest.mark.asyncio
    async def test_invalid(self):
        # This commit is from 2000, it better always be in the branch
        with pytest.raises(core.PickUIException):
            await core.full_sha('fffffffffffffffffffffffffffffffffff')
