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
    Drivers/board/delay_us.c
    Drivers/board/drv8353.c
    Drivers/board/powerstage_drv8353.c
    Drivers/board/powerstage_ihm03.c
    Drivers/current_sense/current_sense.c
    Drivers/encoder/as5048a.c
    Drivers/encoder/encoder_hub.c
    Drivers/voltage_sense/voltage_sense.c

    # MotorControl
    MotorControl/motor_control.c
    MotorControl/motor_control_callbacks.c
)

target_include_directories(foc_lib PUBLIC
    App
    Common/math
    Common/types
    Control/ccs_mpc
    Control/fcs_mpc
    Control/foc
    Control/position
    Control/pwm
    Control/speed
    Drivers/board
    Drivers/current_sense
    Drivers/encoder
    Drivers/voltage_sense
    MotorControl
)

target_link_libraries(foc_lib PUBLIC
    stm32cubemx
)