################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include board/subdir.mk
-include ancit_fota/subdir.mk
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
Ancit_FOTA_SmartWheels_v2_1_3.srec \

SECONDARY_SIZE += \
Ancit_FOTA_SmartWheels_v2_1_3.siz \


# All Target
all: Ancit_FOTA_SmartWheels_v2_1_3.elf secondary-outputs

# Tool invocations
Ancit_FOTA_SmartWheels_v2_1_3.elf: $(OBJS) C:\NXP\S32DS.3.5\S32DS\software\S32SDK_S32K1XX_RTM_4.0.1\platform\devices\S32K144\linker\gcc\S32K144_64_flash.ld $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Standard S32DS C Linker'
	arm-none-eabi-gcc -o "Ancit_FOTA_SmartWheels_v2_1_3.elf" "@Ancit_FOTA_SmartWheels_v2_1_3.args"  $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

Ancit_FOTA_SmartWheels_v2_1_3.srec: Ancit_FOTA_SmartWheels_v2_1_3.elf
	@echo 'Invoking: Standard S32DS Create Flash Image'
	arm-none-eabi-objcopy -O srec Ancit_FOTA_SmartWheels_v2_1_3.elf  "Ancit_FOTA_SmartWheels_v2_1_3.srec" --srec-forceS3
	@echo 'Finished building: $@'
	@echo ' '

Ancit_FOTA_SmartWheels_v2_1_3.siz: Ancit_FOTA_SmartWheels_v2_1_3.elf
	@echo 'Invoking: Standard S32DS Print Size'
	arm-none-eabi-size --format=berkeley Ancit_FOTA_SmartWheels_v2_1_3.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) ./*
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets
