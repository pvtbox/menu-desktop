cmake_minimum_required(VERSION 3.0.2)

set(LIBRARY_NAME libintl)
project(${LIBRARY_NAME})

set(PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/source)
set(INTL ${PREFIX}/gettext-runtime/intl)

set(SOURCE_FILES
    ${PREFIX}/gettext-runtime/intl/bindtextdom.c
    ${PREFIX}/gettext-runtime/intl/dcgettext.c
    ${PREFIX}/gettext-runtime/intl/dcigettext.c
    ${PREFIX}/gettext-runtime/intl/dcngettext.c
    ${PREFIX}/gettext-runtime/intl/dgettext.c
    ${PREFIX}/gettext-runtime/intl/dngettext.c
    ${PREFIX}/gettext-runtime/intl/explodename.c
    ${PREFIX}/gettext-runtime/intl/finddomain.c
    ${PREFIX}/gettext-runtime/intl/gettext.c
    ${PREFIX}/gettext-runtime/intl/hash-string.c
    ${PREFIX}/gettext-runtime/intl/intl-compat.c
    ${PREFIX}/gettext-runtime/intl/l10nflist.c
    ${PREFIX}/gettext-runtime/intl/langprefs.c
    ${PREFIX}/gettext-runtime/intl/loadmsgcat.c
    ${PREFIX}/gettext-runtime/intl/localcharset.c
    ${PREFIX}/gettext-runtime/intl/localealias.c
    ${PREFIX}/gettext-runtime/intl/localename.c
    ${PREFIX}/gettext-runtime/intl/lock.c
    ${PREFIX}/gettext-runtime/intl/log.c
    ${PREFIX}/gettext-runtime/intl/ngettext.c
    ${PREFIX}/gettext-runtime/intl/osdep.c
    ${PREFIX}/gettext-runtime/intl/plural-exp.c
    ${PREFIX}/gettext-runtime/intl/plural.c
    ${PREFIX}/gettext-runtime/intl/printf.c
    ${PREFIX}/gettext-runtime/intl/relocatable.c
    ${PREFIX}/gettext-runtime/intl/textdomain.c
    ${PREFIX}/gettext-runtime/intl/version.c
    ${PREFIX}/gettext-runtime/config.h
    ${PREFIX}/gettext-runtime/intl/libgnuintl.h
)

string(REPLACE "/" "\\\\" LOCALE_DIR "\\\"${CMAKE_BINARY_DIR}\\\"")
message(${LOCALE_DIR})

add_definitions(
    /D LOCALEDIR=${LOCALE_DIR};
    /D LOCALE_ALIAS_PATH=${LOCALE_DIR};
    /D LIBDIR=${LOCALE_DIR};
    /D IN_LIBINTL;
    /D ENABLE_RELOCATABLE=1;
    /D IN_LIBRARY;
    /D INSTALLDIR=${LOCALE_DIR};
    /D NO_XMALLOC;
    /D set_relocation_prefix=libintl_set_relocation_prefix;
    /D relocate=libintl_relocate;
    /D DEPENDS_ON_LIBICONV=1;
    /D HAVE_CONFIG_H;
    /D _CRT_SECURE_NO_WARNINGS;
)

include_directories(${PREFIX}/gettext-runtime)
include_directories(${PREFIX}/gettext-runtime/intl)


add_library(${LIBRARY_NAME} STATIC ${SOURCE_FILES})

install(TARGETS ${LIBRARY_NAME}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

install(FILES ${PREFIX}/gettext-runtime/intl/libgnuintl.h
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include
        RENAME libintl.h)
