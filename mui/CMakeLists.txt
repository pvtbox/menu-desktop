cmake_minimum_required(VERSION 3.0.2)
include(../fixInstallPrefix.cmake)

set (DOMAIN PvtboxMenu)

# search all supported languages according to exsitins sub-directories
MACRO (SUBDIRLIST result curdir)
    FILE (GLOB children RELATIVE ${curdir} ${curdir}/*)
    SET (dirlist "")
    FOREACH (child ${children})
        IF (IS_DIRECTORY ${curdir}/${child})
            LIST (APPEND dirlist ${child})
        ENDIF ()
    ENDFOREACH ()
    SET (${result} ${dirlist})
ENDMACRO ()

SUBDIRLIST (LANGS ${CMAKE_CURRENT_SOURCE_DIR})

# Specify template name *.pot.
set( POT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/messages.pot )

set (SRC_FILES common/MenuDescription.cpp)

add_custom_target (translation ALL DEPENDS)
add_custom_command (TARGET translation
                    COMMAND xgettext -D ${CMAKE_CURRENT_SOURCE_DIR}/.. -o ${POT_FILE} ${SRC_FILES})

set(PREFIX "")
if (${UNIX})
    set(PREFIX /usr/share)
endif()

# Merge new template with exsitins translations and install it
foreach (LANG ${LANGS})
    file (GLOB _poFile ${LANG}/*.po)
    if (EXISTS ${_poFile})
        set (GMO_FILE_NEW ${CMAKE_CURRENT_BINARY_DIR}/${LANG}.gmo)
        add_custom_command (TARGET translation
                            COMMAND msgmerge -U ${_poFile} ${POT_FILE}
                            COMMAND msgfmt -o ${GMO_FILE_NEW} ${_poFile})

        message("To install: ${CMAKE_INSTALL_PREFIX}${PRFIX}/locale/${LANG}/LC_MESSAGES/${DOMAIN}.mo")
        install (FILES ${GMO_FILE_NEW}
                 DESTINATION ${CMAKE_INSTALL_PREFIX}${PREFIX}/locale/${LANG}/LC_MESSAGES
                 RENAME ${DOMAIN}.mo)

    endif (EXISTS ${_poFile})
endforeach (LANG)
