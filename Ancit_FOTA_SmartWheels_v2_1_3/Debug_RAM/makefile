################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include board/subdir.mk
-include UART/subdir.mk
-include Timer/subdir.mk
-include SDK/rtos/osif/subdir.mk
-include SDK/platform/drivers/src/pins/subdir.mk
-include SDK/platform/drivers/src/lpuart/subdir.mk
-include SDK/platform/drivers/src/lpit/subdir.mk
-include SDK/platform/drivers/src/interrupt/subdir.mk
-include SDK/platform/drivers/src/flash/subdir.mk
-include SDK/platform/drivers/src/edma/subdir.mk
-include SDK/platform/drivers/src/clock/S32K1xx/subdir.mk
-include SDK/platform/devices/S32K144/startup/subdir.mk
-include SDK/platform/devices/subdir.mk
-include Project_Settings/Startup_Code/subdir.mk
-include Project_Settings/Linker_Files/subdir.mk
-include Fls_handler/subdir.mk
-include Comm_manger/subdir.mk
-include Boot_Manager/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
UART_Bootloader_1_2_1.srec \

SECONDARY_SIZE += \
UART_Bootloader_1_2_1.siz \


# All Target
all: UART_Bootloader_1_2_1.elf secondary-outputs

# Tool invocations
UART_Bootloader_1_2_1.elf: $(OBJS) C:\NXP\S32DS.3.5\S32DS\software\S32SDK_S32K1XX_RTM_4.0.1\platform\devices\S32K144\linker\gcc\S32K144_64_ram.ld $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Standard S32DS C Linker'
	arm-none-eabi-gcc -o "UART_Bootloader_1_2_1.elf" "@UART_Bootloader_1_2_1.args"  $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

UART_Bootloader_1_2_1.srec: UART_Bootloader_1_2_1.elf
	@echo 'Invoking: Standard S32DS Create Flash Image'
	arm-none-eabi-objcopy -O srec UART_Bootloader_1_2_1.elf  "UART_Bootloader_1_2_1.srec" --srec-forceS3
	@echo 'Finished building: $@'
	@echo ' '

UART_Bootloader_1_2_1.siz: UART_Bootloader_1_2_1.elf
	@echo 'Invoking: Standard S32DS Print Size'
	arm-none-eabi-size --format=berkeley UART_Bootloader_1_2_1.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) ./*
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets
