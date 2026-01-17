# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly

# Resource: Under Minimal Component CMakeLists --> https://docs.espressif.com/projects/esp-idf/en/v3.2.2/api-guides/build-system-cmake.html
#                                              --> https://forum.allaboutcircuits.com/threads/using-esp-idf-with-vscode-adding-custom-component.181145/
cmake_minimum_required(VERSION 3.16)

idf_component_register(SRCS 
                            "src/DW1000Ranging.cpp"
                            "src/DW1000Time.cpp"
                            "src/DW1000Mac.cpp"
                            "src/DW1000Device.cpp"
                            "src/DW1000.cpp"
                       INCLUDE_DIRS "src"
                       REQUIRES 
                            "driver"
                            "esp_timer"
                            "freertos"
                       )

include($ENV{IDF_PATH}/tools/cmake/project.cmake)