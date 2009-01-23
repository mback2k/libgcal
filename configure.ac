dnl Process this file with autoconf to produce a configure script.

# (C) 2005-2006 Mandriva Conectiva S.A.
# (C) 2006 Ademar de Souza Reis Jr. <ademar@ademar.org>
#
# Based on sniffdet configure.ac: http://sniffdet.sourceforge.net
# Licensed under GNU-GPL

AC_INIT(Library gcalendar, 0.8.5,[], libgcal)
AC_PREREQ(2.58)

AC_CONFIG_SRCDIR([src/gcal.c])
AC_CONFIG_AUX_DIR([scripts])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.7.9])

AC_CONFIG_HEADERS([config.h])

AC_GNU_SOURCE
AC_CANONICAL_HOST

dnl Checks for programs.
AC_PROG_MAKE_SET
AC_PROG_INSTALL
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
AC_PROG_CC

dnl Check if pkg-config is installed
AC_CHECK_PROG(PKGCONFIG, pkg-config, yes, no)
if test $PKGCONFIG = "no"; then
	AC_MSG_ERROR("*** pkg-config not found! ***")
fi

dnl Checks for header files.
AC_CHECK_HEADERS([limits.h netdb.h netinet/in.h stdlib.h string.h sys/time.h unistd.h dlfcn.h syslog.h sys/param.h])
AC_CHECK_HEADERS([pcap.h])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_TIME
AC_TYPE_SIGNAL
AC_STRUCT_TM

AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([alarm gettimeofday memset strndup inet_ntoa])
AC_CHECK_FUNCS([strstr select strerror])

dnl Checks for libraries.

# if the library supports pkg-config, it's nice and easy
PKG_CHECK_MODULES(LIBCURL, libcurl,, \
	AC_MSG_ERROR("*** libcurl not found! You need it to build $PACKAGE_NAME. ***"))
AC_SUBST(LIBCURL_CFLAGS)
AC_SUBST(LIBCURL_LIBS)


PKG_CHECK_MODULES(LIBXML, libxml-2.0,, \
	AC_MSG_ERROR("*** libxml2 not found! You need it to build $PACKAGE_NAME. ***"))
AC_SUBST(LIBXML_CFLAGS)
AC_SUBST(LIBXML_LIBS)

# if configuring with debug code for CURL
AC_ARG_ENABLE(curldebug, AS_HELP_STRING([--enable-curldebug],[Enable CURL debug, printing requests and data]),,[enable_curldebug=no])
if test "x$enable_curldebug" = "xyes"; then
   	AC_DEFINE(GCAL_DEBUG_CURL, [], [Define if curl debug code is active])
	echo "curl debug mode... yes"
else
	echo "curl debug mode... no"
fi

AM_CONDITIONAL(GCAL_DEBUG_CURL, test "x$enable_curldebug" = "xyes")

# enable configure options and automake conditionals useful for developers
# look at m4/auxdevel.m4 for detailed documentation
AC_SUBST(csourcedir, $srcdir/src)
AC_SUBST(headerdir, $srcdir/inc)
AC_SUBST(utestdir, $srcdir/utests)

AC_DEVEL_MACROS
AC_DEVEL_ABSDIRS
AC_DEVEL_DEFINE_INSTALL_DIRS

AC_CONFIG_FILES([Makefile
                 Doxyfile
                 libgcal.pc
                 libgcal-uninstalled.pc])
AC_OUTPUT

echo "
$PACKAGE_NAME $PACKAGE_VERSION configuration:
-----------------------------
  Source code location:       ${srcdir}
  Host System Type:           ${host}
  Compiler:                   ${CC}
  Standard CFLAGS:            ${CFLAGS} ${ac_devel_default_warnings} ${LIBCURL_CFLAGS} ${LIBXML_CFLAGS}
  Libraries:                  ${LIBCURL_LIBS} ${LIBXML_LIBS}
  Install path (prefix):      ${prefix}


  Now type 'make' to build $PACKAGE_NAME $PACKAGE_VERSION.
"
