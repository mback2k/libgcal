prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
libdir=${exec_prefix}/@GCAL_LIBRARY_INSTALL_DIR@
includedir=${prefix}/@GCAL_INCLUDE_INSTALL_DIR@

Name: libgcal
Description: Implements google data protocol for calendar and contacts
Version: @GCAL_VERSION@
Libs: -L${libdir} -lgcal
Cflags: -I${includedir}
Requires: libcurl libxml-2.0
