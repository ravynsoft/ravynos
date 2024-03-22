import argparse
import pytest  # type: ignore
import subprocess

from .commit_in_branch import (
    is_commit_valid,
    branch_has_commit,
    branch_has_backport_of_commit,
    canonicalize_commit,
    validate_branch,
)


def get_upstream() -> str:
    # Let's assume main is bound to the upstream remote and not a fork
    out = subprocess.check_output(['git', 'for-each-ref',
                                   '--format=%(upstream)',
                                   'refs/heads/main'],
                                  stderr=subprocess.DEVNULL)
    return out.decode().strip().split('/')[2]


@pytest.mark.parametrize(
    'commit, expected',
    [
        ('20.1-branchpoint', True),
        ('main', True),
        ('e58a10af640ba58b6001f5c5ad750b782547da76', True),
        ('d043d24654c851f0be57dbbf48274b5373dea42b', True),
        ('dd2bd68fa69124c86cd008b256d06f44fab8e6cd', True),
        ('0000000000000000000000000000000000000000', False),
        ('not-even-a-valid-commit-format', False),
    ])
def test_canonicalize_commit(commit: str, expected: bool) -> None:
    if expected:
        assert canonicalize_commit(commit)
    else:
        try:
            assert canonicalize_commit(commit)
        except argparse.ArgumentTypeError:
            return
        assert False


@pytest.mark.parametrize(
    'commit, expected',
    [
        (get_upstream() + '/20.1', True),
        (get_upstream() + '/staging/20.1', True),
        (get_upstream() + '/main', True),
        ('20.1', False),
        ('main', False),
        ('e58a10af640ba58b6001f5c5ad750b782547da76', False),
        ('d043d24654c851f0be57dbbf48274b5373dea42b', False),
        ('dd2bd68fa69124c86cd008b256d06f44fab8e6cd', False),
        ('0000000000000000000000000000000000000000', False),
        ('not-even-a-valid-commit-format', False),
    ])
def test_validate_branch(commit: str, expected: bool) -> None:
    if expected:
        assert validate_branch(commit)
    else:
        try:
            assert validate_branch(commit)
        except argparse.ArgumentTypeError:
            return
        assert False


@pytest.mark.parametrize(
    'commit, expected',
    [
        ('main', True),
        ('20.1-branchpoint', True),
        ('20.1', False),
        (get_upstream() + '/20.1', True),
        (get_upstream() + '/staging/20.1', True),
        ('e58a10af640ba58b6001f5c5ad750b782547da76', True),
        ('d043d24654c851f0be57dbbf48274b5373dea42b', True),
        ('dd2bd68fa69124c86cd008b256d06f44fab8e6cd', True),
        ('0000000000000000000000000000000000000000', False),
        ('not-even-a-valid-commit-format', False),
    ])
def test_is_commit_valid(commit: str, expected: bool) -> None:
    assert is_commit_valid(commit) == expected


@pytest.mark.parametrize(
    'branch, commit, expected',
    [
        (get_upstream() + '/20.1', '20.1-branchpoint', True),
        (get_upstream() + '/20.1', '20.0', False),
        (get_upstream() + '/20.1', 'main', False),
        (get_upstream() + '/20.1', 'e58a10af640ba58b6001f5c5ad750b782547da76', True),
        (get_upstream() + '/20.1', 'd043d24654c851f0be57dbbf48274b5373dea42b', True),
        (get_upstream() + '/staging/20.1', 'd043d24654c851f0be57dbbf48274b5373dea42b', True),
        (get_upstream() + '/20.1', 'dd2bd68fa69124c86cd008b256d06f44fab8e6cd', False),
        (get_upstream() + '/main', 'dd2bd68fa69124c86cd008b256d06f44fab8e6cd', True),
        (get_upstream() + '/20.0', 'd043d24654c851f0be57dbbf48274b5373dea42b', False),
    ])
def test_branch_has_commit(branch: str, commit: str, expected: bool) -> None:
    assert branch_has_commit(branch, commit) == expected


@pytest.mark.parametrize(
    'branch, commit, expected',
    [
        (get_upstream() + '/20.1', 'dd2bd68fa69124c86cd008b256d06f44fab8e6cd', 'd043d24654c851f0be57dbbf48274b5373dea42b'),
        (get_upstream() + '/staging/20.1', 'dd2bd68fa69124c86cd008b256d06f44fab8e6cd', 'd043d24654c851f0be57dbbf48274b5373dea42b'),
        (get_upstream() + '/20.1', '20.1-branchpoint', ''),
        (get_upstream() + '/20.1', '20.0', ''),
        (get_upstream() + '/20.1', '20.2', 'abac4859618e02aea00f705b841a7c5c5007ad1a'),
        (get_upstream() + '/20.1', 'main', ''),
        (get_upstream() + '/20.1', 'd043d24654c851f0be57dbbf48274b5373dea42b', ''),
        (get_upstream() + '/20.0', 'dd2bd68fa69124c86cd008b256d06f44fab8e6cd', '8cd4f57381cefe69019a3282d457d5bda3644030'),
    ])
def test_branch_has_backport_of_commit(branch: str, commit: str, expected: bool) -> None:
    assert branch_has_backport_of_commit(branch, commit) == expected
