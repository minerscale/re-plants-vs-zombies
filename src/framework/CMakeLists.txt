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
# SDL external projects
################################################################################

set(FETCHCONTENT_QUIET FALSE)
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)

include(FetchContent)
set(FETCHCONTENT_SOURCE_DIR_ZLIB ${CMAKE_SOURCE_DIR}/external/zlib)
FetchContent_Declare(
        zlib
        GIT_REPOSITORY https://github.com/madler/zlib.git
        GIT_TAG v1.3.1
        OVERRIDE_FIND_PACKAGE
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(zlib)

set(HAVE_SDL_LOADSO TRUE)
set(HAVE_SDL_THREADS TRUE)
set(FETCHCONTENT_SOURCE_DIR_SDL2 ${CMAKE_SOURCE_DIR}/external/SDL2)
FetchContent_Declare(
        SDL2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-2.30.x
        OVERRIDE_FIND_PACKAGE
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL2)

include_directories(${SDL2_SOURCE_DIR}/include)

set(FETCHCONTENT_SOURCE_DIR_SDL2_IMAGE ${CMAKE_SOURCE_DIR}/external/SDL2_image)
FetchContent_Declare(
        SDL2_image
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
        GIT_TAG release-2.8.x
        OVERRIDE_FIND_PACKAGE
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL2_image)

#FetchContent_Declare(
#    SDL2_mixer
#    GIT_REPOSITORY          https://github.com/libsdl-org/SDL_mixer.git
#    GIT_TAG                 release-2.6.3
#    OVERRIDE_FIND_PACKAGE
#    GIT_PROGRESS            TRUE
#    GIT_SHALLOW             TRUE
#    SOURCE_DIR              ${CMAKE_SOURCE_DIR}/external/SDL2_mixer
#)
#set(SDL2MIXER_MOD ON CACHE BOOL "" FORCE)
#set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
#set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
#set(BUILD_DOCS OFF CACHE BOOL "" FORCE)
#set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
#set(BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
#FetchContent_MakeAvailable(SDL2_mixer)

set(FETCHCONTENT_SOURCE_DIR_GLM ${CMAKE_SOURCE_DIR}/external/glm)
FetchContent_Declare(glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG "1.0.0"
        OVERRIDE_FIND_PACKAGE
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
        SYSTEM
)

FetchContent_MakeAvailable(glm)

set(FETCHCONTENT_SOURCE_DIR_AVIR ${CMAKE_SOURCE_DIR}/external/avir)
FetchContent_Declare(avir
        GIT_REPOSITORY https://github.com/avaneev/avir.git
        OVERRIDE_FIND_PACKAGE
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        SYSTEM
)

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
        #SDL2_mixer::SDL2_mixer-static
        zlibstatic
        glm::glm
        avir
)
# Adds include directories
target_include_directories(
        ${PROJECT_NAME}
        SYSTEM
        PRIVATE
        ${SDL2_SOURCE_DIR}/include
        ${SDL2_image_SOURCE_DIR}/include
        #${SDL2_mixer_SOURCE_DIR}/include
        ${zlib_SOURCE_DIR}
        ${zlib_BINARY_DIR}
        ${avir_SOURCE_DIR}
)
