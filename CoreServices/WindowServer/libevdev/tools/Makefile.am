noinst_PROGRAMS = libevdev-events
bin_PROGRAMS = \
	       touchpad-edge-detector \
	       mouse-dpi-tool \
	       libevdev-tweak-device

AM_CPPFLAGS = $(GCC_CFLAGS) -I$(top_srcdir) -I$(top_srcdir)/include -I$(top_srcdir)/libevdev
libevdev_ldadd = $(top_builddir)/libevdev/libevdev.la

libevdev_events_SOURCES = libevdev-events.c
libevdev_events_LDADD = $(libevdev_ldadd)

touchpad_edge_detector_SOURCES = touchpad-edge-detector.c
touchpad_edge_detector_LDADD = $(libevdev_ldadd)

mouse_dpi_tool_SOURCES = mouse-dpi-tool.c
mouse_dpi_tool_LDADD = $(libevdev_ldadd)

libevdev_tweak_device_SOURCES = libevdev-tweak-device.c
libevdev_tweak_device_LDADD = $(libevdev_ldadd)

dist_man_MANS = \
		libevdev-tweak-device.1 \
		touchpad-edge-detector.1 \
		$(NULL)
