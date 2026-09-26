cmake_minimum_required(VERSION 3.28)
# Build the scaffold and its required SDK components; log is a common dependency.
set(COMPONENTS main)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(critter_companion VERSION 0.1.0)
