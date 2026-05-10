################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Boot_Manager/Boot_mangr.c \
../Boot_Manager/Boot_mangr_Cfg.c 

OBJS += \
./Boot_Manager/Boot_mangr.o \
./Boot_Manager/Boot_mangr_Cfg.o 

C_DEPS += \
./Boot_Manager/Boot_mangr.d \
./Boot_Manager/Boot_mangr_Cfg.d 


# Each subdirectory must supply rules for building sources it contributes
Boot_Manager/%.o: ../Boot_Manager/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@Boot_Manager/Boot_mangr.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


