cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

################################################################################
# Target
################################################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_ANDROID_STL_TYPE c++_static)
project(PlantsVsZombies C CXX)

################################################################################
# Global configuration types
################################################################################
set(CMAKE_CONFIGURATION_TYPES
        "Debug"
        "Release"
        CACHE STRING "List of supported configurations." FORCE
)

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "\${ORIGIN}")

# @Patoke todo: probably add MSVC counterparts?
if (MSVC)
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        set(universal_compiler_flags
                "/Ob2" "/Ox"
        )
        if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            set(universal_link_flags
                    "/MT /LTCG"
            )
            set(universal_compiler_flags
                    ${universal_compiler_flags}
                    "/GL" "/arch:AVX2"
            )
        endif ()
    else ()
        if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            set(universal_compiler_flags "/ZI")
            set(universal_link_flags "/INCREMENTAL /MTd")
        endif ()
    endif ()
else ()
    set(universal_compiler_flags
            "-Wno-unused-command-line-argument"
            "-Wno-deprecated-declarations"
            "-fdiagnostics-color=always"
            "-pipe"
            "-pthread"
            # Remove these for proper release build
            "-g"
            "-fno-omit-frame-pointer"
            "-fstack-protector-all"
            #"-fno-inline"
    )

    if (NOT ANDROID)
        list(APPEND universal_compiler_flags "-march=native")
    endif ()


    set(universal_link_flags
            "-fuse-ld=lld"
    )

    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        set(universal_compiler_flags
                "${universal_compiler_flags}"
                "-O3"
                "-flto"
        )
    else ()
        set(universal_compiler_flags
                "${universal_compiler_flags}"
                "-O0"
        )
    endif ()
endif ()
#set(universal_cxxflags "${CMAKE_CXX_FLAGS} ")
#set(universal_cflags   "${CMAKE_C_FLAGS} ${universal_compiler_flags}")

if (MSVC)
    set(warn_flags "")
else ()
    set(warn_flags
            "-Wall"
            "-Wextra"
            "-Werror"
    )

    if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND UNIX)
        set(warn_flags
                ${warn_flags}
                "-fsanitize=address,undefined"
        )
    endif ()
endif ()

add_compile_options(${universal_compiler_flags})
add_link_options(${universal_link_flags})
# set(CMAKE_CXX_FLAGS "${universal_compiler_flags}")
# set(CMAKE_C_FLAGS   "${universal_compiler_flags}")
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

if (MSVC)
    set(CMAKE_RC_COMPILER_INIT windres)
    ENABLE_LANGUAGE(RC)
endif ()

add_executable(${PROJECT_NAME})
target_compile_options(${PROJECT_NAME} PRIVATE ${warn_flags})
target_link_options(${PROJECT_NAME} PRIVATE ${warn_flags})


################################################################################
# Sub-projects
################################################################################
add_subdirectory(src)
