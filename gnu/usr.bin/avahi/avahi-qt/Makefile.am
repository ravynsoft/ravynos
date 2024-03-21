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

AM_CFLAGS=-I$(top_srcdir)

lib_LTLIBRARIES =
BUILT_SOURCES =

if HAVE_QT3

avahiqt3includedir=$(includedir)/avahi-qt3

avahiqt3include_HEADERS = \
	qt-watch.h

lib_LTLIBRARIES += \
	libavahi-qt3.la

BUILT_SOURCES += qt-watch.moc3

libavahi_qt3_la_SOURCES = \
	qt-watch.cpp

qt-watch.moc3: qt-watch.cpp
	$(AM_V_GEN)$(MOC_QT3) $^ > $@

libavahi_qt3_la_CPPFLAGS = $(AM_CFLAGS) $(QT3_CFLAGS) $(VISIBILITY_HIDDEN_CFLAGS)
libavahi_qt3_la_LIBADD = $(AM_LDADD) ../avahi-common/libavahi-common.la $(QT3_LIBS)
libavahi_qt3_la_LDFLAGS = $(AM_LDFLAGS) -export-dynamic -version-info $(LIBAVAHI_QT3_VERSION_INFO)
endif

if HAVE_QT4

avahiqt4includedir=$(includedir)/avahi-qt4
avahiqt4include_HEADERS = \
	qt-watch.h

lib_LTLIBRARIES += \
	libavahi-qt4.la

BUILT_SOURCES += qt-watch.moc4

libavahi_qt4_la_SOURCES = \
	qt-watch.cpp

qt-watch.moc4: qt-watch.cpp
	$(AM_V_GEN)$(MOC_QT4) $^ > $@

libavahi_qt4_la_CPPFLAGS = $(AM_CFLAGS) $(QT4_CFLAGS) -DQT4 $(VISIBILITY_HIDDEN_CFLAGS)
libavahi_qt4_la_LIBADD = $(AM_LDADD) ../avahi-common/libavahi-common.la $(QT4_LIBS)
libavahi_qt4_la_LDFLAGS = $(AM_LDFLAGS) -version-info $(LIBAVAHI_QT4_VERSION_INFO)
endif

if HAVE_QT5

avahiqt5includedir=$(includedir)/avahi-qt5
avahiqt5include_HEADERS = \
	qt-watch.h

lib_LTLIBRARIES += \
	libavahi-qt5.la

BUILT_SOURCES += qt-watch.moc5

libavahi_qt5_la_SOURCES = \
	qt-watch.cpp

qt-watch.moc5: qt-watch.cpp
	$(AM_V_GEN)$(MOC_QT5) $^ > $@

libavahi_qt5_la_CPPFLAGS = $(AM_CFLAGS) --std=gnu++11 $(QT5_CFLAGS) -DQT5 $(VISIBILITY_HIDDEN_CFLAGS)
libavahi_qt5_la_LIBADD = $(AM_LDADD) ../avahi-common/libavahi-common.la $(QT5_LIBS)
libavahi_qt5_la_LDFLAGS = $(AM_LDFLAGS) -version-info $(LIBAVAHI_QT5_VERSION_INFO)
endif

CLEANFILES = $(BUILT_SOURCES)
