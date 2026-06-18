################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ancit_fota/fota_uart.c \
../ancit_fota/fota_uart_config.c \
../ancit_fota/fota_utils.c 

OBJS += \
./ancit_fota/fota_uart.o \
./ancit_fota/fota_uart_config.o \
./ancit_fota/fota_utils.o 

C_DEPS += \
./ancit_fota/fota_uart.d \
./ancit_fota/fota_uart_config.d \
./ancit_fota/fota_utils.d 


# Each subdirectory must supply rules for building sources it contributes
ancit_fota/%.o: ../ancit_fota/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@ancit_fota/fota_uart.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


