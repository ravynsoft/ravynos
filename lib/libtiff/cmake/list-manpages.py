#!/usr/bin/env python

# Find manual pages to be generated
# argv1 = doc dir containing conf.py
# argv2 = manpage directory to contain generated manpages

import importlib.util
import os.path
import pathlib
import sys

if __name__ == "__main__":

    if len(sys.argv) != 3:
        print("Usage: {} sphinx-srcdir sphinx-builddir".format(sys.argv[0]), file=sys.stderr)
        sys.exit(2)

    conf_dir = os.path.abspath(sys.argv[1])
    conf_path = os.path.join(conf_dir, 'conf.py')
    spec = importlib.util.spec_from_file_location('conf', conf_path)
    conf = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(conf)

    if hasattr(conf, 'man_pages'):
        for man in conf.man_pages:
            man_path = os.path.join(sys.argv[2], "%s.%s" % (man[1], man[4]))
            man_path_posix = pathlib.PureWindowsPath(man_path).as_posix()
            print(man_path_posix)
    else:
        print("No man_pages array in {}".format(conf_path), file=sys.stderr)
        sys.exit(3)
