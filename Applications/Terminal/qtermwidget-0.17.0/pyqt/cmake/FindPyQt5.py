# Copyright (c) 2014, Simon Edwards <simon@simonzone.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

import PyQt5.Qt
import sys
import os.path
import site

print("pyqt_version:%06.0x" % PyQt5.Qt.PYQT_VERSION)
print("pyqt_version_str:%s" % PyQt5.Qt.PYQT_VERSION_STR)

pyqt_version_tag = ""
in_t = False
for item in PyQt5.Qt.PYQT_CONFIGURATION["sip_flags"].split(' '):
    if item=="-t":
        in_t = True
    elif in_t:
        if item.startswith("Qt_5"):
            pyqt_version_tag = item
    else:
        in_t = False
print("pyqt_version_tag:%s" % pyqt_version_tag)

# FIXME Is there a way to query the path from sip instead of hardcoding it?
candidates = [
    os.path.join(site.getsitepackages()[0], "PyQt5", "bindings"),   # sip 5.x
    os.path.join(sys.prefix, "share", "sip", "PyQt5"),              # sip 4.x
]
try:
    pyqt_sip_dir = next(
        p for p in candidates
        if os.path.exists(os.path.join(p, "QtCore", "QtCoremod.sip"))
    )
except StopIteration:
    raise RuntimeError("Cannot find QtCore/QtCoremod.sip from candidates: " + repr(candidates))
print("pyqt_sip_dir:%s" % pyqt_sip_dir)

print("pyqt_sip_flags:%s" % PyQt5.Qt.PYQT_CONFIGURATION["sip_flags"])
