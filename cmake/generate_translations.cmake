# Generates or updates translation files from a pot file using msginit or msgmerge.
#
# Usage:
#   cmake -DPOT_FILE=<pot_file_name> \
#         -DLANG_FILE=<lang_file_name> \
#         -DLANG=<language> \
#         -DMSGMERGE_EXE=<path_to_msgmerge> \
#         -DMSGINIT_EXE=<path_to_msginit> \
#         -P generate_translations.cmake

if (EXISTS ${LANG_FILE})
    message(STATUS "Updating ${LANG_FILE}")
    execute_process(COMMAND ${MSGMERGE_EXE} -U ${LANG_FILE} ${POT_FILE})
else()
    execute_process(COMMAND ${MSGINIT_EXE} --no-translator -l ${LANG}.UTF-8 -o ${LANG_FILE} -i ${POT_FILE})
endif()

