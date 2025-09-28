# This file is part of avahi.
#
# avahi is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# avahi is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
# License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with avahi; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA.

EXTRA_DIST = __init__.py ServiceTypeDatabase.py.in
EXTRA_DIST += test.py

TESTS =
TEST_EXTENSIONS = .py
PY_LOG_COMPILER = $(PYTHON)

pkglibdatadir=$(libdir)/avahi

if HAVE_PYTHON

avahidir = $(pythondir)/avahi

if HAVE_GDBM
nodist_avahi_SCRIPTS = ServiceTypeDatabase.py
endif

if HAVE_DBM
nodist_avahi_SCRIPTS = ServiceTypeDatabase.py
endif

ServiceTypeDatabase.py: ServiceTypeDatabase.py.in
	$(AM_V_GEN)sed -e 's,@PYTHON\@,$(PYTHON),g' \
		-e 's,@pkglibdatadir\@,$(pkglibdatadir),g' $< > $@ && \
	chmod +x $@

avahi_PYTHON = $(avahi_SCRIPTS)

if HAVE_PYTHON_DBUS

avahi_PYTHON += __init__.py

TESTS += test.py

endif
endif

CLEANFILES=*.pyc *.pyo ServiceTypeDatabase.py
