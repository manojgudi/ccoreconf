# cmake/CcoreconfModels.cmake
#
# ccoreconfAddModel(<name>
#     SID_FILES   <a.sid> [<b.sid> ...]          # required, from pyang --sid-extension
#     INSTANCE    <path-to-instance.json>        # required
#     [OUT_DIR    <out-dir>]                     # default: ${CMAKE_CURRENT_BINARY_DIR}/ccoreconf-gen
#     [TARGET     <target-to-inherit>]           # default: ccoreconf
#     [PYTHON     <python-interpreter>]          # default: CCORECONF_PYTHON, else Python3
# )
#
# Runs tools/prepareModel.py to generate <name>.c/.h, which define
# `const CoreconfModelDescT <name>ModelDesc` (pass it to ccoreconfModelLoadDesc).
# The Python interpreter needs the packages in tools/requirements.txt; point
# CCORECONF_PYTHON (or PYTHON) at a venv that has them.
#
# Each model lands as its own OBJECT library (ccoreconf_model_<name>) so
# multi-model binaries have no name collisions. Consumers link both
# ccoreconf and the per-model OBJECT library.

if(DEFINED CCORECONF_MODELS_CMAKE_LOADED)
    return()
endif()
set(CCORECONF_MODELS_CMAKE_LOADED TRUE)

# Cached so ccoreconfAddModel() also works from a parent project that
# pulled ccoreconf in with add_subdirectory() (normal variables are scoped)
get_filename_component(_ccoreconf_tools_dir "${CMAKE_CURRENT_LIST_DIR}/../tools" ABSOLUTE)
set(CCORECONF_TOOLS_DIR "${_ccoreconf_tools_dir}" CACHE INTERNAL "ccoreconf tools/ directory")

set(CCORECONF_PYTHON "" CACHE FILEPATH
    "Python interpreter with the tools/requirements.txt packages (e.g. a venv's bin/python)")

function(ccoreconfAddModel name)
    cmake_parse_arguments(CCM
        ""
        "INSTANCE;OUT_DIR;TARGET;PYTHON"
        "SID_FILES"
        ${ARGN}
    )

    if(NOT CCM_SID_FILES)
        message(FATAL_ERROR "ccoreconfAddModel(${name}): SID_FILES is required")
    endif()
    set(_sid_files "")
    foreach(_sid IN LISTS CCM_SID_FILES)
        if(NOT IS_ABSOLUTE "${_sid}")
            set(_sid "${CMAKE_CURRENT_SOURCE_DIR}/${_sid}")
        endif()
        if(NOT EXISTS "${_sid}")
            message(FATAL_ERROR "ccoreconfAddModel(${name}): SID file not found: ${_sid}")
        endif()
        list(APPEND _sid_files "${_sid}")
    endforeach()

    if(NOT CCM_INSTANCE)
        message(FATAL_ERROR "ccoreconfAddModel(${name}): INSTANCE is required")
    endif()
    if(NOT IS_ABSOLUTE "${CCM_INSTANCE}")
        set(CCM_INSTANCE "${CMAKE_CURRENT_SOURCE_DIR}/${CCM_INSTANCE}")
    endif()
    if(NOT EXISTS "${CCM_INSTANCE}")
        message(FATAL_ERROR "ccoreconfAddModel(${name}): INSTANCE not found: ${CCM_INSTANCE}")
    endif()

    if(NOT CCM_OUT_DIR)
        set(CCM_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/ccoreconf-gen")
    endif()
    file(MAKE_DIRECTORY "${CCM_OUT_DIR}")

    if(NOT CCM_TARGET)
        set(CCM_TARGET "ccoreconf")
    endif()
    if(NOT TARGET "${CCM_TARGET}")
        message(FATAL_ERROR "ccoreconfAddModel(${name}): target '${CCM_TARGET}' does not exist")
    endif()

    if(NOT CCM_PYTHON)
        if(CCORECONF_PYTHON)
            set(CCM_PYTHON "${CCORECONF_PYTHON}")
        else()
            find_package(Python3 REQUIRED COMPONENTS Interpreter)
            set(CCM_PYTHON "${Python3_EXECUTABLE}")
        endif()
    endif()

    set(_prepare_model "${CCORECONF_TOOLS_DIR}/prepareModel.py")
    add_custom_command(
        OUTPUT  "${CCM_OUT_DIR}/${name}.c"
                "${CCM_OUT_DIR}/${name}.h"
        COMMAND "${CCM_PYTHON}" "${_prepare_model}"
                --sid-files ${_sid_files}
                --instance "${CCM_INSTANCE}"
                --name "${name}"
                --output-dir "${CCM_OUT_DIR}"
        DEPENDS ${_sid_files} "${CCM_INSTANCE}" "${_prepare_model}"
                "${CCORECONF_TOOLS_DIR}/templates/model.c.jinja"
                "${CCORECONF_TOOLS_DIR}/templates/model.h.jinja"
        COMMENT "ccoreconf: generating model '${name}'"
        VERBATIM
    )

    # The model is its own OBJECT library.  Linking it into ccoreconf would
    # create a circular dependency.  Consumers explicitly link both.
    add_library(ccoreconf_model_${name} OBJECT "${CCM_OUT_DIR}/${name}.c")
    target_include_directories(ccoreconf_model_${name} PUBLIC "${CCM_OUT_DIR}")
    target_link_libraries(ccoreconf_model_${name} PUBLIC "${CCM_TARGET}")
    set_target_properties(ccoreconf_model_${name}
        PROPERTIES POSITION_INDEPENDENT_CODE ON)
endfunction()
