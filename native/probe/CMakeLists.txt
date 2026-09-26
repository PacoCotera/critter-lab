cmake_minimum_required(VERSION 3.28)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(critter_probe LANGUAGES C)
target_sources(app PRIVATE src/main.c)
