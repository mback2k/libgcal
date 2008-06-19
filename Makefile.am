AUTOMAKE_OPTIONS = foreign

# libgcal
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgcal.pc
libmandir = $(mandir)/man3
#libman_DATA = libfrob.3
lib_LTLIBRARIES = libgcal.la
include_HEADERS = $(headerdir)/internal_gcal.h \
		$(headerdir)/gcal.h $(headerdir)/atom_parser.h \
		$(headerdir)/xml_aux.h $(headerdir)/gcal_parser.h \
		$(headerdir)/gcontact.h $(headerdir)/gcal_status.h
if GCAL_DEBUG_CURL
include_HEADERS += $(headerdir)/curl_debug_gcal.h
endif
libgcal_la_SOURCES = $(csourcedir)/gcal.c $(csourcedir)/atom_parser.c \
		$(csourcedir)/xml_aux.c $(csourcedir)/gcal_parser.c \
		$(csourcedir)/gcontact.c $(csourcedir)/gcal_status.c
if GCAL_DEBUG_CURL
libgcal_la_SOURCES += $(csourcedir)/curl_debug_gcal.c
endif
libgcal_la_CPPFLAGS = -I$(headerdir)
libgcal_la_CFLAGS = $(AM_CFLAGS) $(LIBCURL_CFLAGS) $(LIBXML_CFLAGS)
libgcal_la_LIBADD = $(LIBCURL_LIBS) $(LIBXML_LIBS)



# unit tests
if CHECK
TESTS = utest
noinst_PROGRAMS = utest
utest_SOURCES = $(utestdir)/utest_gcal.h $(utestdir)/utest_gcal.c \
		$(utestdir)/utest_xpath.h $(utestdir)/utest_xpath.c \
		$(utestdir)/utest_edit.h $(utestdir)/utest_edit.c \
		$(utestdir)/utest_contact.h $(utestdir)/utest_contact.c \
		$(utestdir)/utest_debug.h $(utestdir)/utest_debug.c \
		$(utestdir)/utest_query.h $(utestdir)/utest_query.c \
		$(utestdir)/utest.c

utest_CPPFLAGS = $(CHECK_FLAGS) $(AM_CPPFLAGS) -I$(csourcedir) -I$(headerdir) \
		$(LIBXML_CFLAGS)
utest_LDADD = $(CHECK_LIBS) $(LDADD) $(lib_LTLIBRARIES)
endif


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


