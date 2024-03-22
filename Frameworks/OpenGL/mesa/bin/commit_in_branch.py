#!/usr/bin/env python3

import argparse
import subprocess
import sys


def print_(args: argparse.Namespace, success: bool, message: str) -> None:
    """
    Print function with extra coloring when supported and/or requested,
    and with a "quiet" switch
    """

    COLOR_SUCCESS = '\033[32m'
    COLOR_FAILURE = '\033[31m'
    COLOR_RESET = '\033[0m'

    if args.quiet:
        return

    if args.color == 'auto':
        use_colors = sys.stdout.isatty()
    else:
        use_colors = args.color == 'always'

    s = ''
    if use_colors:
        if success:
            s += COLOR_SUCCESS
        else:
            s += COLOR_FAILURE

    s += message

    if use_colors:
        s += COLOR_RESET

    print(s)


def is_commit_valid(commit: str) -> bool:
    ret = subprocess.call(['git', 'cat-file', '-e', commit],
                          stdout=subprocess.DEVNULL,
                          stderr=subprocess.DEVNULL)
    return ret == 0


def branch_has_commit(upstream_branch: str, commit: str) -> bool:
    """
    Returns True if the commit is actually present in the branch
    """
    ret = subprocess.call(['git', 'merge-base', '--is-ancestor',
                           commit, upstream_branch],
                          stdout=subprocess.DEVNULL,
                          stderr=subprocess.DEVNULL)
    return ret == 0


def branch_has_backport_of_commit(upstream_branch: str, commit: str) -> str:
    """
    Returns the commit hash if the commit has been backported to the branch,
    or an empty string if is hasn't
    """
    upstream, _ = upstream_branch.split('/', 1)

    out = subprocess.check_output(['git', 'log', '--format=%H',
                                   upstream + '..' + upstream_branch,
                                   '--grep', 'cherry picked from commit ' + commit],
                                  stderr=subprocess.DEVNULL)
    return out.decode().strip()


def canonicalize_commit(commit: str) -> str:
    """
    Takes a commit-ish and returns a commit sha1 if the commit exists
    """

    # Make sure input is valid first
    if not is_commit_valid(commit):
        raise argparse.ArgumentTypeError('invalid commit identifier: ' + commit)

    out = subprocess.check_output(['git', 'rev-parse', commit],
                                  stderr=subprocess.DEVNULL)
    return out.decode().strip()


def validate_branch(branch: str) -> str:
    if '/' not in branch:
        raise argparse.ArgumentTypeError('must be in the form `remote/branch`')

    out = subprocess.check_output(['git', 'remote', '--verbose'],
                                  stderr=subprocess.DEVNULL)
    remotes = out.decode().splitlines()
    upstream, _ = branch.split('/', 1)
    valid_remote = False
    for line in remotes:
        if line.startswith(upstream + '\t'):
            valid_remote = True

    if not valid_remote:
        raise argparse.ArgumentTypeError('Invalid remote: ' + upstream)

    if not is_commit_valid(branch):
        raise argparse.ArgumentTypeError('Invalid branch: ' + branch)

    return branch


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="""
    Returns 0 if the commit is present in the branch,
    1 if it's not,
    and 2 if it couldn't be determined (eg. invalid commit)
    """)
    parser.add_argument('commit',
                        type=canonicalize_commit,
                        help='commit sha1')
    parser.add_argument('branch',
                        type=validate_branch,
                        help='branch to check, in the form `remote/branch`')
    parser.add_argument('--quiet',
                        action='store_true',
                        help='suppress all output; exit code can still be used')
    parser.add_argument('--color',
                        choices=['auto', 'always', 'never'],
                        default='auto',
                        help='colorize output (default: true if stdout is a terminal)')
    args = parser.parse_args()

    if branch_has_commit(args.branch, args.commit):
        print_(args, True, 'Commit ' + args.commit + ' is in branch ' + args.branch)
        exit(0)

    backport = branch_has_backport_of_commit(args.branch, args.commit)
    if backport:
        print_(args, True,
               'Commit ' + args.commit + ' was backported to branch ' + args.branch + ' as commit ' + backport)
        exit(0)

    print_(args, False, 'Commit ' + args.commit + ' is NOT in branch ' + args.branch)
    exit(1)
