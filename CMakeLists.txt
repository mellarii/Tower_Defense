cmake_minimum_required(VERSION 3.12)
project(CMakeSFMLProject LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(SFML)

# Все cpp из src/
file(GLOB_RECURSE SOURCES
    src/*.cpp
)

add_executable(main ${SOURCES})

# Заголовки в src/ и src/Include (если там лежат .hpp)
target_include_directories(main PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Inlcude
)

target_compile_features(main PRIVATE cxx_std_17)

# Линкуем SFML 3
target_link_libraries(main PRIVATE SFML::Graphics)