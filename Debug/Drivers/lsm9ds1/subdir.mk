################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/lsm9ds1/lsm9ds1_reg.c 

OBJS += \
./Drivers/lsm9ds1/lsm9ds1_reg.o 

C_DEPS += \
./Drivers/lsm9ds1/lsm9ds1_reg.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/lsm9ds1/%.o Drivers/lsm9ds1/%.su: ../Drivers/lsm9ds1/%.c Drivers/lsm9ds1/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F427xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-lsm9ds1

clean-Drivers-2f-lsm9ds1:
	-$(RM) ./Drivers/lsm9ds1/lsm9ds1_reg.d ./Drivers/lsm9ds1/lsm9ds1_reg.o ./Drivers/lsm9ds1/lsm9ds1_reg.su

.PHONY: clean-Drivers-2f-lsm9ds1

