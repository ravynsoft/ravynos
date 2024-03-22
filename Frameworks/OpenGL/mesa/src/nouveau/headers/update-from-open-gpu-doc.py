#!/usr/bin/env python3

import argparse
import git
from git import Repo
import glob
import os
import shutil
import sys

# where should files go
dirs = {
  '/classes/3d/': 'classes/',
  '/classes/compute/': 'classes/',
  '/classes/dma-copy/': 'classes/',
  '/classes/host/': 'classes/',
  '/classes/memory-to-memory-format/': 'classes/',
  '/classes/twod/': 'classes/',
}
branch = 'master'
target = os.path.abspath(os.path.dirname(__file__)) + "/"

parser = argparse.ArgumentParser(description='Updates Nvidia header files from git.')
parser.add_argument('git_path', type=str, help='Path to the open-gpu-doc repo')

args = parser.parse_args()
repo_path = os.path.abspath(args.git_path)

# 1. create repo object
try:
    repo = Repo(repo_path)
    assert not repo.bare
except git.exc.NoSuchPathError:
    print("{} doesn't point to a git repository".format(repo_path))
    sys.exit(-1)

# 2. update repo
repo.remotes.origin.fetch()
repo.git.checkout(branch)
repo.git.rebase('origin/' + branch)

# 3. check if all needed directories exist
for dir in dirs.keys():
    path = repo_path + dir
    if not os.path.isdir(path):
        print(dir + " does not exist in repository. Was the correct repository choosen?")
        sys.exit(-1)

# 4. copy over files
for src, dest in dirs.items():
    src = repo_path + src
    dest = target + dest
    for header in glob.glob(src + "*.h"):
        print(header + " => " + dest)
        shutil.copy(header, dest)
