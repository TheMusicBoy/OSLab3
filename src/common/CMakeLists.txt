set(INCROOT "${PROJECT_SOURCE_DIR}/include/common")
set(SRCROOT "${PROJECT_SOURCE_DIR}/src/common")

set(SRC
    ${INCROOT}/public.h
    ${SRCROOT}/refcounted.cpp
    ${INCROOT}/refcounted.h
    ${SRCROOT}/intrusive_ptr.cpp
    ${INCROOT}/intrusive_ptr.h
    ${SRCROOT}/weak_ptr.cpp
    ${INCROOT}/weak_ptr.h
    ${SRCROOT}/threadpool.cpp
    ${INCROOT}/threadpool.h
    ${SRCROOT}/periodic_executor.cpp
    ${INCROOT}/periodic_executor.h
    ${SRCROOT}/getopts.cpp
    ${INCROOT}/getopts.h
    ${SRCROOT}/format.cpp
    ${INCROOT}/format.h
    ${SRCROOT}/logging.cpp
    ${INCROOT}/logging.h
    ${SRCROOT}/exception.cpp
    ${INCROOT}/exception.h

)

add_library(common STATIC ${SRC})

target_include_directories(common PUBLIC 
    ${PROJECT_SOURCE_DIR}/include
)

set_target_properties(common PROPERTIES LINKER_LANGUAGE CXX)
