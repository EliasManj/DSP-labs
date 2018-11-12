################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
../startup/startup_stm32f4xx.S 

OBJS += \
./startup/startup_stm32f4xx.o 

S_UPPER_DEPS += \
./startup/startup_stm32f4xx.d 


# Each subdirectory must supply rules for building sources it contributes
startup/%.o: ../startup/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 '-D__FPU_USED = 1' '-D__FPU_PRESENT =1' -DSTM32F407VGTx -DARM_MATH_CM4 -DSTM32F4 -DSTM32F4DISCOVERY -DSTM32 -DUSE_STDPERIPH_DRIVER -DSTM32F40XX -DSTM32F40_41xxx -I"C:/Users/Fernandp/workspaceSTM32/ARMLibTest/inc" -I"C:/Users/Fernandp/workspaceSTM32/ARMLibTest/CMSIS/core" -I"C:/Users/Fernandp/workspaceSTM32/ARMLibTest/CMSIS/device" -I"C:/Users/Fernandp/workspaceSTM32/ARMLibTest/StdPeriph_Driver/inc" -I"C:/Users/Fernandp/workspaceSTM32/ARMLibTest/Utilities" -O3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


