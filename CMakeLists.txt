cmake_minimum_required(VERSION 3.20)
project(Isometrico2D VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Optimizaciones de compilador
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og")

# Release por defecto
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Usar SDL2 de ucrt64 directamente (coincidente con el compilador)
message(STATUS "Usando SDL2 de ucrt64")

set(SDL2_LIBRARIES "C:/msys64/ucrt64/lib/libSDL2.a" "C:/msys64/ucrt64/lib/libSDL2main.a")
set(SDL2_INCLUDE_DIRS "C:/msys64/ucrt64/include" "C:/msys64/ucrt64/include/SDL2")
set(SDL2_LINK_FLAGS "-Wl,--start-group -lmingw32 -Wl,--end-group -mconsole -lkernel32 -luser32 -lsetupapi -lole32 -lgdi32 -lwinmm -limm32 -loleaut32 -lversion -luuid -lcfgmgr32 -ladvapi32 -lshell32")

# Source files
set(SOURCE_FILES
    src/main.cpp
    src/Game.cpp
    src/Renderer.cpp
    src/World.cpp
    src/Chunk.cpp
    src/Camera.cpp
    src/Player.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/libs
    ${SDL2_INCLUDE_DIRS}
)

# STB_IMAGE - definir solo en Renderer.cpp
set_source_files_properties(src/Renderer.cpp PROPERTIES
    COMPILE_DEFINITIONS STB_IMAGE_IMPLEMENTATION
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    -Wl,--start-group
    C:/msys64/ucrt64/lib/libSDL2main.a
    C:/msys64/ucrt64/lib/libSDL2.a
    -lmingw32
    -Wl,--end-group
    -mconsole
    -lkernel32
    -luser32
    -lsetupapi
    -lole32
    -lgdi32
    -lwinmm
    -limm32
    -loleaut32
    -lversion
    -luuid
    -lcfgmgr32
    -ladvapi32
    -lshell32
)

# Copiar assets
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    COMMENT "Copying assets to build directory"
)

# Copiar SDL2.dll si es necesario
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "C:/msys64/ucrt64/bin/SDL2.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
    COMMENT "Copying SDL2.dll"
)
