################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/BSP/as5048a.c \
../Core/Src/BSP/current_sense.c \
../Core/Src/BSP/delay_us.c 

OBJS += \
./Core/Src/BSP/as5048a.o \
./Core/Src/BSP/current_sense.o \
./Core/Src/BSP/delay_us.o 

C_DEPS += \
./Core/Src/BSP/as5048a.d \
./Core/Src/BSP/current_sense.d \
./Core/Src/BSP/delay_us.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/BSP/%.o Core/Src/BSP/%.su Core/Src/BSP/%.cyclo: ../Core/Src/BSP/%.c Core/Src/BSP/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G474xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-BSP

clean-Core-2f-Src-2f-BSP:
	-$(RM) ./Core/Src/BSP/as5048a.cyclo ./Core/Src/BSP/as5048a.d ./Core/Src/BSP/as5048a.o ./Core/Src/BSP/as5048a.su ./Core/Src/BSP/current_sense.cyclo ./Core/Src/BSP/current_sense.d ./Core/Src/BSP/current_sense.o ./Core/Src/BSP/current_sense.su ./Core/Src/BSP/delay_us.cyclo ./Core/Src/BSP/delay_us.d ./Core/Src/BSP/delay_us.o ./Core/Src/BSP/delay_us.su

.PHONY: clean-Core-2f-Src-2f-BSP

