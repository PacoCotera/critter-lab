cmake_minimum_required(VERSION 3.28)
project(critter_lab_native VERSION 0.1.0 LANGUAGES C)

add_executable(critter_lab main.c)
set_target_properties(critter_lab PROPERTIES C_STANDARD 17 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
target_compile_options(critter_lab PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_compile_definitions(critter_lab PRIVATE CRITTER_LAB_VERSION="${PROJECT_VERSION}")
target_link_options(critter_lab PRIVATE "-Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/critter_lab.map")
