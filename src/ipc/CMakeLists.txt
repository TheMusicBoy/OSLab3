set(INCROOT "${PROJECT_SOURCE_DIR}/include/ipc")
set(SRCROOT "${PROJECT_SOURCE_DIR}/src/ipc")

set(SRC
    ${SRCROOT}/shared_memory.cpp
    ${INCROOT}/shared_memory.h

    ${SRCROOT}/subprocess.cpp
    ${INCROOT}/subprocess.h
)

add_library(ipc STATIC ${SRC})

target_include_directories(ipc PUBLIC 
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(ipc PUBLIC common rt)

set_target_properties(ipc PROPERTIES LINKER_LANGUAGE CXX)
