#!/usr/bin/env python3

import os
import sys
import argparse
import platform
from pathlib import PurePath

if __name__=='__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('availpath')
    parser.add_argument('confpath')
    parser.add_argument('links', nargs='+')
    args = parser.parse_args()

    if os.path.isabs(args.confpath):
        destdir = os.environ.get('DESTDIR')
        if destdir:
            # c:\destdir + c:\prefix must produce c:\destdir\prefix
            confpath = str(PurePath(destdir, *PurePath(args.confpath).parts[1:]))
        else:
            confpath = args.confpath
    else:
        confpath = os.path.join(os.environ['MESON_INSTALL_DESTDIR_PREFIX'], args.confpath)

    if not os.path.exists(confpath):
        os.makedirs(confpath)

    for link in args.links:
        src = os.path.join(args.availpath, link)
        dst = os.path.join(confpath, link)
        try:
            os.remove(dst)
        except FileNotFoundError:
            pass
        try:
            os.symlink(src, dst)
        except NotImplementedError:
            # Not supported on this version of Windows
            break
        except OSError as e:
            # Symlink privileges are not available
            if platform.system().lower() == 'windows' and e.winerror == 1314:
                break
            raise
