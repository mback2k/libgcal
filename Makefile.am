AUTOMAKE_OPTIONS = foreign

# libgcal
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgcal.pc
libmandir = $(mandir)/man3
#libman_DATA = libfrob.3
lib_LTLIBRARIES = libgcal.la
include_HEADERS = $(headerdir)/gcal.h
libgcal_la_SOURCES = $(csourcedir)/gcal.c
libgcal_la_CPPFLAGS = -I$(headerdir)
libgcal_la_CFLAGS = $(AM_CFLAGS) $(LIBCURL_CFLAGS)
libgcal_la_LIBADD = $(LIBCURL_LIBS)




EXTRA_DIST = $(srcdir)/m4/auxdevel.m4 \
             $(srcdir)/m4/check.m4 \
             $(srcdir)/m4/define_dirs.m4 \
             $(srcdir)/mk/auxdevel.am \
             $(srcdir)/mk/libtool-fix.am \
             $(srcdir)/README \
             $(srcdir)/COPYING \
             Doxyfile.in


DISTCHECK_CONFIGURE_FLAGS = --enable-efence --enable-debug \
                            --enable-warnings --disable-check

# this variable is scanned by autoreconf and needs to be static (no expansion)
ACLOCAL_AMFLAGS = -I m4

include $(srcdir)/mk/auxdevel.am


