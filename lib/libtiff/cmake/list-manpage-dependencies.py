#!/usr/bin/env python

# Find manual page dependencies
# argv1 = doc dir containing conf.py

import importlib.util
import os.path
import pathlib
import sys

if __name__ == "__main__":

    if len(sys.argv) != 2:
        sys.exit("Usage: %s sphinx-srcdir" % (sys.argv[0]))

    conf_dir = os.path.abspath(sys.argv[1])
    conf_path = os.path.join(conf_dir, 'conf.py')
    spec = importlib.util.spec_from_file_location('conf', conf_path)
    conf = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(conf)

    if hasattr(conf, 'man_pages'):
        for man in conf.man_pages:
            man_path = os.path.join(sys.argv[1], "{}{}".format(man[0], '.rst'))
            man_path_posix = pathlib.PureWindowsPath(man_path).as_posix()
            print(man_path_posix)

    print(pathlib.PureWindowsPath(conf_path).as_posix())
