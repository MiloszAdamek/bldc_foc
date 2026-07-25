add_library(foc_lib STATIC)

target_sources(foc_lib PRIVATE

    # APP
    Src/App/commander.c
    Src/App/motor_control.c

    # BSP
    Src/BSP/as5048a.c
    Src/BSP/current_sense.c
    Src/BSP/delay_us.c
    Src/BSP/encoder_hub.c
    Src/BSP/drv8353.c
    Src/BSP/powerstage_drv8353.c
    Src/BSP/powerstage_ihm03.c
    Src/BSP/board.c

    # FOC
    Src/FOC/foc_loop.c
    Src/FOC/lut_sin_cos.c
    Src/FOC/position_control.c
    Src/FOC/speed_control.c
    Src/FOC/speed_estimator.c
    Src/FOC/svpwm.c
)

target_include_directories(foc_lib PUBLIC
    Inc
    Inc/App
    Inc/BSP
    Inc/FOC
)

target_link_libraries(foc_lib PUBLIC
    stm32cubemx
)