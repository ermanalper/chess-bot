cmake_minimum_required(VERSION 3.20)  # or whatever your CMake supports
project(chess_bot)

set(CMAKE_CXX_STANDARD 20)

# Make our custom FindSDL2.cmake visible
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules")

# Correct SDL2 root (use forward slashes on Windows)
set(SDL2_PATH "${CMAKE_SOURCE_DIR}/SDL2-2.32.10/x86_64-w64-mingw32")

find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIR})

# Build a console-friendly SDL2 link list for this target
set(SDL2_LIB_CORE ${SDL2_LIBRARY})

# Do NOT link SDL2main.a when we provide our own main()
if (SDL2MAIN_LIBRARY)
    list(REMOVE_ITEM SDL2_LIB_CORE "${SDL2MAIN_LIBRARY}")
endif()

# On MinGW, FindSDL2 adds 'mingw32' and '-mwindows' which expect WinMain.
# Remove them so we get a normal console entry point.
if (MINGW)
    list(REMOVE_ITEM SDL2_LIB_CORE "mingw32" "-mwindows")
endif()

add_executable(chess_bot
        main.cpp
        functions.c
        hash.c
)

target_link_libraries(chess_bot ${SDL2_LIB_CORE})