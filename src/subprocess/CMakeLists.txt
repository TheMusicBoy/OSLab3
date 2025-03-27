set(INCROOT "${PROJECT_SOURCE_DIR}/include/subprocess")
set(SRCROOT "${PROJECT_SOURCE_DIR}/src/subprocess")

set(SRC
    ${INCROOT}/subprocess.h
    ${SRCROOT}/subprocess.cpp
)

if (NOT "${SRC}" STREQUAL "")
    message(STATUS "Building subprocess library...")
    add_library(subprocess ${SRC})
    target_include_directories(subprocess PUBLIC ${INCROOT})
    set_target_properties(subprocess PROPERTIES LINKER_LANGUAGE CXX)
else()
    message(WARNING "Subprocess library not built")
endif()
