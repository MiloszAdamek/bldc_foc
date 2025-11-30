################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/FOC/foc_loop.c \
../Core/Src/FOC/lut_sin_cos.c \
../Core/Src/FOC/speed_control.c \
../Core/Src/FOC/svpwm.c 

OBJS += \
./Core/Src/FOC/foc_loop.o \
./Core/Src/FOC/lut_sin_cos.o \
./Core/Src/FOC/speed_control.o \
./Core/Src/FOC/svpwm.o 

C_DEPS += \
./Core/Src/FOC/foc_loop.d \
./Core/Src/FOC/lut_sin_cos.d \
./Core/Src/FOC/speed_control.d \
./Core/Src/FOC/svpwm.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/FOC/%.o Core/Src/FOC/%.su Core/Src/FOC/%.cyclo: ../Core/Src/FOC/%.c Core/Src/FOC/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G474xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-FOC

clean-Core-2f-Src-2f-FOC:
	-$(RM) ./Core/Src/FOC/foc_loop.cyclo ./Core/Src/FOC/foc_loop.d ./Core/Src/FOC/foc_loop.o ./Core/Src/FOC/foc_loop.su ./Core/Src/FOC/lut_sin_cos.cyclo ./Core/Src/FOC/lut_sin_cos.d ./Core/Src/FOC/lut_sin_cos.o ./Core/Src/FOC/lut_sin_cos.su ./Core/Src/FOC/speed_control.cyclo ./Core/Src/FOC/speed_control.d ./Core/Src/FOC/speed_control.o ./Core/Src/FOC/speed_control.su ./Core/Src/FOC/svpwm.cyclo ./Core/Src/FOC/svpwm.d ./Core/Src/FOC/svpwm.o ./Core/Src/FOC/svpwm.su

.PHONY: clean-Core-2f-Src-2f-FOC

