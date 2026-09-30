# cmake/CcoreconfModels.cmake
#
# ccoreconfAddModel(<name>
#     SID_FILE    <path-to-.sid-file>          # required
#     [INSTANCE   <path-to-instance.json>]    # optional
#     [OUT_DIR    <out-dir>]                  # default: ${CMAKE_CURRENT_BINARY_DIR}/ccoreconf-gen
#     [TARGET     <target-to-inherit>]        # default: ccoreconf::ccoreconf
#     [PYTHON     <python-interpreter>]        # default: Python3
# )
#
# Each model lands as its own OBJECT library (ccoreconf_model_<name>) so
# multi-model binaries have no name collisions. Consumers link both
# ccoreconf and the per-model OBJECT library.
#
# NOTE: this function invokes tools/prepareModel.py (step 1 of the
# multi-model refactor, currently on hold). The function is correct
# CMake infrastructure but the build will fail to generate a model
# until that tool returns. Use only when the generator is present.

if(DEFINED CCORECONF_MODELS_CMAKE_LOADED)
    return()
endif()
set(CCORECONF_MODELS_CMAKE_LOADED TRUE)

function(ccoreconfAddModel name)
    cmake_parse_arguments(CCM
        ""
        "SID_FILE;INSTANCE;OUT_DIR;TARGET;PYTHON"
        ""
        ${ARGN}
    )

    if(NOT CCM_SID_FILE)
        message(FATAL_ERROR "ccoreconf_add_model(${name}): SID_FILE is required")
    endif()
    if(NOT IS_ABSOLUTE "${CCM_SID_FILE}")
        set(CCM_SID_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${CCM_SID_FILE}")
    endif()
    if(NOT EXISTS "${CCM_SID_FILE}")
        message(FATAL_ERROR "ccoreconf_add_model(${name}): SID_FILE not found: ${CCM_SID_FILE}")
    endif()

    if(CCM_INSTANCE AND NOT IS_ABSOLUTE "${CCM_INSTANCE}")
        set(CCM_INSTANCE "${CMAKE_CURRENT_SOURCE_DIR}/${CCM_INSTANCE}")
    endif()

    if(NOT CCM_OUT_DIR)
        set(CCM_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/ccoreconf-gen")
    endif()
    file(MAKE_DIRECTORY "${CCM_OUT_DIR}")

    if(NOT CCM_TARGET)
        set(CCM_TARGET "ccoreconf")
    endif()
    if(NOT TARGET "${CCM_TARGET}")
        message(FATAL_ERROR "ccoreconf_add_model(${name}): target '${CCM_TARGET}' does not exist")
    endif()

    if(NOT CCM_PYTHON)
        find_package(Python3 REQUIRED COMPONENTS Interpreter)
        set(CCM_PYTHON "Python3::Interpreter")
    endif()

    set(_gen_py_args
        "${CCORECONF_SOURCE_DIR}/tools/prepareModel.py"
        "model"
        "--sid-files" "${CCM_SID_FILE}"
        "--name"      "${name}"
        "--output-dir" "${CCM_OUT_DIR}"
    )
    if(CCM_INSTANCE)
        list(APPEND _gen_py_args "--instance" "${CCM_INSTANCE}")
    endif()

    add_custom_command(
        OUTPUT  "${CCM_OUT_DIR}/${name}.c"
                "${CCM_OUT_DIR}/${name}.h"
        COMMAND "${CCM_PYTHON}" ${_gen_py_args}
        DEPENDS "${CCM_SID_FILE}" "${CCORECONF_SOURCE_DIR}/tools/prepareModel.py"
        COMMENT "ccoreconf: generating model '${name}'"
        VERBATIM
    )

    # The model is its own OBJECT library.  Linking it into ccoreconf would
    # create a circular dependency.  Consumers explicitly link both.
    add_library(ccoreconf_model_${name} OBJECT "${CCM_OUT_DIR}/${name}.c")
    target_include_directories(ccoreconf_model_${name}
        PUBLIC "${CCM_OUT_DIR}"
               "${CCORECONF_SOURCE_DIR}/include")
    set_target_properties(ccoreconf_model_${name}
        PROPERTIES POSITION_INDEPENDENT_CODE ON)
endfunction()