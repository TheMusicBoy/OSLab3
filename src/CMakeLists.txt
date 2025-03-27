add_subdirectory(common)
add_subdirectory(ipc)

set(INCROOT "${PROJECT_SOURCE_DIR}/include")
set(SRCROOT "${PROJECT_SOURCE_DIR}/src")

set(SRC
    ${SRCROOT}/service.cpp
    ${INCROOT}/service.h

    ${SRCROOT}/main.cpp
)

if (NOT "${SRC}" STREQUAL "")
    message(STATUS "Building main executable...")
    add_executable(main ${SRC})
    target_include_directories(main PUBLIC ${INCROOT})
    target_link_libraries(main PUBLIC common ipc)
    set_target_properties(main PROPERTIES LINKER_LANGUAGE CXX)
else()
    message(WARNING "Main executable is not built")
endif()
