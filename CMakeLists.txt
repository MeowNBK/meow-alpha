cmake_minimum_required(VERSION 3.25)
project(meow-alpha VERSION 0.1.0 LANGUAGES CXX)

# --- C++ Standard Configuration ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Output Directory Configuration ---
# All executables will be placed in the build/<config>/bin directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

# --- Main Executable: meow-alpha ---
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
add_executable(${PROJECT_NAME} ${APP_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/include/meow-alpha"
    "${PROJECT_SOURCE_DIR}/include/meow-alpha/frontend"
    "${PROJECT_SOURCE_DIR}/include/meow-alpha/backend"
)

# --- Precompiled Headers (PCH) ---
# This project can use PCH by creating a "pch.h" file at the path below.
set(PCH_HEADER "${PROJECT_SOURCE_DIR}/include/pch.h")
if (EXISTS "${PCH_HEADER}")
    message(STATUS "PCH: Found ${PCH_HEADER} -> Enabling precompiled headers.")
    target_precompile_headers(${PROJECT_NAME} PRIVATE "${PCH_HEADER}")
else()
    message(STATUS "PCH: ${PCH_HEADER} not found -> Precompiled headers disabled.")
endif()