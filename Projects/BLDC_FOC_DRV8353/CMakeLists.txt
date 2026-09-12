# cmake_minimum_required(VERSION 3.22)

# # Setup compiler settings
# set(CMAKE_C_STANDARD 11)
# set(CMAKE_C_STANDARD_REQUIRED ON)
# set(CMAKE_C_EXTENSIONS ON)

# # Define the build type
# if(NOT CMAKE_BUILD_TYPE)
#     set(CMAKE_BUILD_TYPE "Debug")
# endif()

# # Set the project name
# set(CMAKE_PROJECT_NAME BLDC_FOC_DRV8353)

# # Enable compile command to ease indexing with e.g. clangd
# set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# # Core project settings
# project(${CMAKE_PROJECT_NAME})
# message("Build type: " ${CMAKE_BUILD_TYPE})

# # Enable CMake support for ASM and C languages
# enable_language(C ASM)

# # Create an executable object type
# add_executable(${CMAKE_PROJECT_NAME})

# # Add STM32CubeMX generated sources
# add_subdirectory(cmake/stm32cubemx)

# # Link directories setup
# target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
#     # Add user defined library search paths
# )

# # Add sources to executable
# target_sources(${CMAKE_PROJECT_NAME} PRIVATE
#     # Add user sources here
# )

# # Add include paths
# target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
#     Core/Inc
# )

# # Add project symbols (macros)
# target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
#     # Add user defined symbols
# )

# # Remove wrong libob.a library dependency when using cpp files
# list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# add_subdirectory(
#     ../foc_lib
#     ${CMAKE_BINARY_DIR}/foc_lib
# )

# # Kluczowe dla clangd i kompilacji: przekazanie nagłówków projektu i HAL do foc_lib
# target_include_directories(foc_lib PUBLIC
#     ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
# )

# target_link_libraries(foc_lib PUBLIC
#     stm32cubemx
# )

# # add_subdirectory(Thirdparty/stm32_can_lib)
# # target_compile_definitions(stm32_can_lib PUBLIC
# #     DEVICE_IS_SLAVE
# #     SLAVE_NODE_ID=0x02
# # )

# # Add linked libraries
# target_link_libraries(${CMAKE_PROJECT_NAME}
#     stm32cubemx
#     foc_lib
#     # stm32_can_lib
#     m
# )

# target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
#     -u _printf_float
# )

# # target_compile_options(${PROJECT_NAME} PRIVATE
# #     -O3
# # )

# # Automatyczne kopiowanie bazy kompilacji dla clangd do głównego katalogu
# add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different
#         ${CMAKE_BINARY_DIR}/compile_commands.json
#         ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
#     COMMENT "Kopiowanie compile_commands.json do korzenia projektu"
# )

# nowe

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

# Dołączenie foc_lib
add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/../foc_lib"
    "${CMAKE_CURRENT_BINARY_DIR}/foc_lib"
)

# Przekazanie nagłówków Core/Inc do foc_lib (jeśli foc_lib ich potrzebuje)
target_include_directories(foc_lib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc"
)


add_subdirectory(Thirdparty/stm32_can_lib)
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

# Kopiowanie bazy dla clangd do korzenia BLDC_FOC_DRV8353
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
    COMMENT "Kopiowanie compile_commands.json do korzenia projektu"
)