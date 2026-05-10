################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Comm_manger/Comm_mangr.c 

OBJS += \
./Comm_manger/Comm_mangr.o 

C_DEPS += \
./Comm_manger/Comm_mangr.d 


# Each subdirectory must supply rules for building sources it contributes
Comm_manger/%.o: ../Comm_manger/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@Comm_manger/Comm_mangr.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


