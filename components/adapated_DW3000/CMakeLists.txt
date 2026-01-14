# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly

# Resource: Under Minimal Component CMakeLists --> https://docs.espressif.com/projects/esp-idf/en/v3.2.2/api-guides/build-system-cmake.html
#                                              --> https://forum.allaboutcircuits.com/threads/using-esp-idf-with-vscode-adding-custom-component.181145/
cmake_minimum_required(VERSION 3.16)

file(GLOB DW3000_SOURCE_FILES "*.c, *.cpp")

idf_component_register(SRCS "${DW3000_SOURCE_FILES}"
                       INCLUDE_DIRS "src"
                       )