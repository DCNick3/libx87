
set(LIBX87_SOURCES
        src/softfloat.cpp)

add_library(x87 STATIC ${LIBX87_SOURCES})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(x87 PUBLIC -DLIBX87_DEBUG)
endif()

target_include_directories(x87 PUBLIC include)
target_include_directories(x87 PRIVATE src)
