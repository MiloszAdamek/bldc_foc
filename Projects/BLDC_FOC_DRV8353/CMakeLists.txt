cmake_minimum_required(VERSION 3.22)

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CMAKE_PROJECT_NAME BLDC_FOC_DRV8353)

# Generuj compile_commands.json dla clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

enable_language(C ASM)

add_executable(${CMAKE_PROJECT_NAME})

# CubeMX HAL / CMSIS
add_subdirectory(cmake/stm32cubemx)

# Include głównego projektu
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    Core/Inc
)

list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

add_compile_definitions(USE_CAN_INTERFACE)

# Dołączenie foc_lib
add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/../Libs/foc_lib"
    "${CMAKE_CURRENT_BINARY_DIR}/foc_lib"
)

# Przekazanie nagłówków Core/Inc do foc_lib (jeśli foc_lib ich potrzebuje)
target_include_directories(foc_lib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc"
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC
    # Add user defined symbols
)

add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/../Libs/stm32_can_lib"
    "${CMAKE_CURRENT_BINARY_DIR}/stm32_can_lib"
)

target_compile_definitions(stm32_can_lib PUBLIC
    DEVICE_IS_SLAVE
    SLAVE_NODE_ID=0x02
)

# Linkowanie do binarnego pliku ELF
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    foc_lib
    stm32_can_lib
    m
)

target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    -u _printf_float
)

# target_compile_options(foc_lib PRIVATE
#     -O3
# )

# target_compile_options(stm32_can_lib PRIVATE
#     -O3
# )

# target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
#     -O3
# )