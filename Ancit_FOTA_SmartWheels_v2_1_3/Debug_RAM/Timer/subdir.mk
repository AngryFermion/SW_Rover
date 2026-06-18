################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Timer/timer.c 

OBJS += \
./Timer/timer.o 

C_DEPS += \
./Timer/timer.d 


# Each subdirectory must supply rules for building sources it contributes
Timer/%.o: ../Timer/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@Timer/timer.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


