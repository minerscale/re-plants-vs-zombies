################################################################################
# Project definition
################################################################################


target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_sources(${PROJECT_NAME} PRIVATE
        "Common.cpp"
        "SexyApp.cpp"
        "SexyAppBase.cpp"
)

target_precompile_headers(${PROJECT_NAME} PRIVATE
        "$<$<COMPILE_LANGUAGE:CXX>:Common.h>"
)

add_subdirectory(graphics)
add_subdirectory(sound)
add_subdirectory(widget)
add_subdirectory(misc)
add_subdirectory(paklib)
add_subdirectory(imagelib)


################################################################################
# External projects
################################################################################

set(FETCHCONTENT_QUIET FALSE)
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)

include(FetchContent)

macro(FetchGit_Declare link)
    option(is_header "Is header" OFF)
    if (${link} MATCHES "https://([^/]+)/([^/]+)/([^/]+)/tree/([^/]+)")
        set(name ${CMAKE_MATCH_3})
        set(tag ${CMAKE_MATCH_4})
        set(url "https://${CMAKE_MATCH_1}/${CMAKE_MATCH_2}/${name}.git")
    elseif (${link} MATCHES "https://([^/]+)/([^/]+)/([^/]+)")
        set(name ${CMAKE_MATCH_3})
        set(url "https://${CMAKE_MATCH_1}/${CMAKE_MATCH_2}/${name}.git")
        set(tag OFF)
    else ()
        message(FATAL_ERROR "Invalid link")
    endif ()

    FetchContent_Declare(
            ${name}
            GIT_REPOSITORY ${url}
            ${tag} GIT_TAG ${tag}
            OVERRIDE_FIND_PACKAGE
            GIT_PROGRESS TRUE
            GIT_SHALLOW TRUE
            FETCHCONTENT_UPDATES_DISCONNECTED TRUE
            SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/${name}
            ${is_header} CONFIGURE_COMMAND ""
            ${is_header} BUILD_COMMAND ""
            ${is_header} SYSTEM
    )
endmacro()

FetchGit_Declare(https://github.com/madler/zlib/tree/v1.3.1)
FetchContent_MakeAvailable(zlib)

FetchGit_Declare(https://github.com/simdutf/simdutf/tree/0df857246627616ef912a784c21d2fbf73820446)
FetchContent_MakeAvailable(simdutf)

set(HAVE_SDL_LOADSO TRUE)
set(HAVE_SDL_THREADS TRUE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_LIBC ON CACHE BOOL "" FORCE)
FetchGit_Declare(https://github.com/libsdl-org/SDL/tree/release-2.30.x)
FetchContent_MakeAvailable(SDL)

include_directories(${SDL_SOURCE_DIR}/include)

FetchGit_Declare(https://github.com/libsdl-org/SDL_image/tree/release-2.8.x)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL_image)

FetchGit_Declare(https://github.com/libsdl-org/SDL_ttf/tree/release-2.22.x)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(SDL2TTF_HARFBUZZ ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL_ttf)

#FetchContent_Declare(https://github.com/libsdl-org/SDL_mixer/tree/release-2.6.3)
#set(SDL2MIXER_MOD ON CACHE BOOL "" FORCE)
#set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
#set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
#set(BUILD_DOCS OFF CACHE BOOL "" FORCE)
#set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
#set(BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
#FetchContent_MakeAvailable(SDL2_mixer)

FetchGit_Declare(https://github.com/g-truc/glm/tree/1.0.0 is_header)
FetchContent_MakeAvailable(glm)

FetchGit_Declare(https://github.com/avaneev/avir is_header)

FetchContent_GetProperties(avir)
if (NOT avir_POPULATED)
    FetchContent_Populate(avir)
endif ()

add_library(avir INTERFACE)

target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        SDL2::SDL2-static
        SDL2_image::SDL2_image-static
        SDL2_ttf::SDL2_ttf-static
        #SDL2_mixer::SDL2_mixer-static
        zlibstatic
        simdutf::simdutf
        glm::glm
        avir
)
# Adds include directories
target_include_directories(
        ${PROJECT_NAME}
        SYSTEM
        PRIVATE
        ${SDL_SOURCE_DIR}/include
        ${SDL_image_SOURCE_DIR}/include
        ${SDL_ttf_SOURCE_DIR}/include
        #${SDL_mixer_SOURCE_DIR}/include
        ${zlib_SOURCE_DIR}
        ${zlib_BINARY_DIR}
        ${avir_SOURCE_DIR}
        ${simdutf_SOURCE_DIR}/include
)
