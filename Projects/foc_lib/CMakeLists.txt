add_library(foc_lib OBJECT)

target_sources(foc_lib PRIVATE
    # App
    App/commander.c

    # Common
    Common/math/lut_sincos.c

    # Control
    Control/foc/foc_loop.c
    Control/position/position_control.c
    Control/pwm/svpwm.c
    Control/speed/speed_control.c
    Control/speed/speed_estimator.c

    # Drivers
    Drivers/board/board.c
    Drivers/delay_us/delay_us.c
    Drivers/drv8353/drv8353.c
    Drivers/powerstage/powerstage_drv8353.c
    Drivers/powerstage/powerstage_ihm03.c
    Drivers/current_sense/current_sense.c
    Drivers/encoder/as5048a.c
    Drivers/encoder/encoder_hub.c
    Drivers/voltage_sense/voltage_sense.c

    # MotorControl
    MotorControl/motor_control.c
    MotorControl/motor_control_callbacks.c
    MotorControl/motor_alignment.c
)

# target_include_directories(foc_lib PUBLIC
#     App
#     Common/math
#     Common/types
#     Control/ccs_mpc
#     Control/fcs_mpc
#     Control/foc
#     Control/position
#     Control/pwm
#     Control/speed
#     Drivers/board
#     Drivers/delay_us
#     Drivers/drv8353
#     Drivers/powerstage
#     Drivers/current_sense
#     Drivers/encoder
#     Drivers/voltage_sense
#     MotorControl
# )

target_include_directories(foc_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/App
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/math
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/types
    ${CMAKE_CURRENT_SOURCE_DIR}/Control/ccs_mpc
    ${CMAKE_CURRENT_SOURCE_DIR}/Control/fcs_mpc
    ${CMAKE_CURRENT_SOURCE_DIR}/Control/foc
    ${CMAKE_CURRENT_SOURCE_DIR}/Control/position
    ${CMAKE_CURRENT_SOURCE_DIR}/Control/pwm
    ${CMAKE_CURRENT_SOURCE_DIR}/Control/speed
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/board
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/delay_us
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/drv8353
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/powerstage
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/current_sense
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/encoder
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/voltage_sense
    ${CMAKE_CURRENT_SOURCE_DIR}/MotorControl
)

# Definicja mikrokontrolera wymagana przez nagłówki ST:
# target_compile_definitions(foc_lib PUBLIC
#     STM32G474xx
#     USE_HAL_DRIVER
# )

# target_compile_options(foc_lib PRIVATE
#     -O3
# )

target_link_libraries(foc_lib PUBLIC
    stm32cubemx
)