################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
E:/AGH/INTEGRA/Balans/TSP-Balans/Projects/BLDC-FOC-EOC/CAN_Profile/can_profile/Src/can_odrive.c \
E:/AGH/INTEGRA/Balans/TSP-Balans/Projects/BLDC-FOC-EOC/CAN_Profile/can_profile/Src/can_wrapper.c \
E:/AGH/INTEGRA/Balans/TSP-Balans/Projects/BLDC-FOC-EOC/CAN_Profile/can_profile/Src/commander.c \
E:/AGH/INTEGRA/Balans/TSP-Balans/Projects/BLDC-FOC-EOC/CAN_Profile/can_profile/Src/slave_driver.c 

OBJS += \
./can_profile/Src/can_odrive.o \
./can_profile/Src/can_wrapper.o \
./can_profile/Src/commander.o \
./can_profile/Src/slave_driver.o 

C_DEPS += \
./can_profile/Src/can_odrive.d \
./can_profile/Src/can_wrapper.d \
./can_profile/Src/commander.d \
./can_profile/Src/slave_driver.d 


# Each subdirectory must supply rules for building sources it contributes
can_profile/Src/can_odrive.o: E:/AGH/INTEGRA/Balans/TSP-Balans/Projects/BLDC-FOC-EOC/CAN_Profile/can_profile/Src/can_odrive.c can_profile/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DDEVICE_IS_MASTER -DUSE_HAL_DRIVER -DSTM32G431xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I"E:/AGH/INTEGRA/Balans/TSP-Balans/Projects/BLDC-FOC-EOC/CAN_Profile/can_profile/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
can_profile/Src/can_wrapper.o: E:/AGH/INTEGRA/Balans/TSP-Balans/Projects/BLDC-FOC-EOC/CAN_Profile/can_profile/Src/can_wrapper.c can_profile/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DDEVICE_IS_MASTER -DUSE_HAL_DRIVER -DSTM32G431xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I"E:/AGH/INTEGRA/Balans/TSP-Balans/Projects/BLDC-FOC-EOC/CAN_Profile/can_profile/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
can_profile/Src/commander.o: E:/AGH/INTEGRA/Balans/TSP-Balans/Projects/BLDC-FOC-EOC/CAN_Profile/can_profile/Src/commander.c can_profile/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DDEVICE_IS_MASTER -DUSE_HAL_DRIVER -DSTM32G431xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I"E:/AGH/INTEGRA/Balans/TSP-Balans/Projects/BLDC-FOC-EOC/CAN_Profile/can_profile/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
can_profile/Src/slave_driver.o: E:/AGH/INTEGRA/Balans/TSP-Balans/Projects/BLDC-FOC-EOC/CAN_Profile/can_profile/Src/slave_driver.c can_profile/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DDEVICE_IS_MASTER -DUSE_HAL_DRIVER -DSTM32G431xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I"E:/AGH/INTEGRA/Balans/TSP-Balans/Projects/BLDC-FOC-EOC/CAN_Profile/can_profile/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-can_profile-2f-Src

clean-can_profile-2f-Src:
	-$(RM) ./can_profile/Src/can_odrive.cyclo ./can_profile/Src/can_odrive.d ./can_profile/Src/can_odrive.o ./can_profile/Src/can_odrive.su ./can_profile/Src/can_wrapper.cyclo ./can_profile/Src/can_wrapper.d ./can_profile/Src/can_wrapper.o ./can_profile/Src/can_wrapper.su ./can_profile/Src/commander.cyclo ./can_profile/Src/commander.d ./can_profile/Src/commander.o ./can_profile/Src/commander.su ./can_profile/Src/slave_driver.cyclo ./can_profile/Src/slave_driver.d ./can_profile/Src/slave_driver.o ./can_profile/Src/slave_driver.su

.PHONY: clean-can_profile-2f-Src

