cmake_minimum_required(VERSION 3.28)
project(critter_selected_lab VERSION 0.1.0 LANGUAGES C)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED YES)

add_library(selected_lab_core STATIC input.c render.c assets.c overview_assets.c pip_art.c
  ../shared/game_state.c ../shared/game_rules.c ../shared/pip_genetics.c ../shared/save_bytes.c
  ../shared/native_font.c ../shared/lab_font_data.c
  ../shared/lab_heading_font_data.c ../shared/lab_heading_narrow_font_data.c)
target_include_directories(selected_lab_core PUBLIC . ../shared)
target_compile_features(selected_lab_core PUBLIC c_std_17)
target_compile_options(selected_lab_core PRIVATE -Wall -Wextra -Wpedantic -Werror)
set_target_properties(selected_lab_core PROPERTIES C_EXTENSIONS NO)

add_executable(selected_lab main.c)
target_link_libraries(selected_lab PRIVATE selected_lab_core)
target_compile_options(selected_lab PRIVATE -Wall -Wextra -Wpedantic -Werror)
set_target_properties(selected_lab PROPERTIES C_EXTENSIONS NO)

include(CTest)
if(BUILD_TESTING)
 add_executable(game_checks ../tests/test_game.c)
 target_link_libraries(game_checks PRIVATE selected_lab_core)
 target_compile_options(game_checks PRIVATE -Wall -Wextra -Wpedantic -Werror -UNDEBUG)
 add_test(NAME game_domain_checks COMMAND game_checks)
endif()
if(BUILD_TESTING)
  add_executable(selected_lab_checks tests.c)
  target_link_libraries(selected_lab_checks PRIVATE selected_lab_core)
  target_compile_options(selected_lab_checks PRIVATE -Wall -Wextra -Wpedantic -Werror -UNDEBUG)
  set_target_properties(selected_lab_checks PROPERTIES C_EXTENSIONS NO)
  add_test(NAME selected_lab_native_checks COMMAND selected_lab_checks)
endif()
