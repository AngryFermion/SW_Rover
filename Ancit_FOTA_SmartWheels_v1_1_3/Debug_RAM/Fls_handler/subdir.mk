################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Fls_handler/Fls_handler.c 

OBJS += \
./Fls_handler/Fls_handler.o 

C_DEPS += \
./Fls_handler/Fls_handler.d 


# Each subdirectory must supply rules for building sources it contributes
Fls_handler/%.o: ../Fls_handler/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@Fls_handler/Fls_handler.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


