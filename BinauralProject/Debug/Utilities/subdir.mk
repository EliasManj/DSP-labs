################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Utilities/stm32f4_discovery.c \
../Utilities/stm32f4_discovery_lis302dl.c 

OBJS += \
./Utilities/stm32f4_discovery.o \
./Utilities/stm32f4_discovery_lis302dl.o 

C_DEPS += \
./Utilities/stm32f4_discovery.d \
./Utilities/stm32f4_discovery_lis302dl.d 


# Each subdirectory must supply rules for building sources it contributes
Utilities/%.o: ../Utilities/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -DSTM32F407VGTx -DARM_MATH_CM4 -D__FPU_PRESENT -D__FPU_USED -DSTM32F4 -DSTM32F4DISCOVERY -DSTM32 -DDEBUG -DUSE_STDPERIPH_DRIVER -DSTM32F40XX -DSTM32F40_41xxx -I"C:/Users/Fernando/Downloads/courses/ARMLibFloatFir/ARMLibFloatFir/inc" -I"C:/Users/Fernando/Downloads/courses/ARMLibFloatFir/ARMLibFloatFir/CMSIS/core" -I"C:/Users/Fernando/Downloads/courses/ARMLibFloatFir/ARMLibFloatFir/CMSIS/device" -I"C:/Users/Fernando/Downloads/courses/ARMLibFloatFir/ARMLibFloatFir/StdPeriph_Driver/inc" -I"C:/Users/Fernando/Downloads/courses/ARMLibFloatFir/ARMLibFloatFir/Utilities" -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


