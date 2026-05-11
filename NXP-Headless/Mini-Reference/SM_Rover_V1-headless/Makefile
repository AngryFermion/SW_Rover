# ==== Toolchain path ====

S32DS_ROOT_DIR := C:/NXP/S32DS.3.5/S32DS/build_tools/gcc_b1620
NXP_K144_RTM := C:/NXP/S32DS.3.5/S32DS/software/S32SDK_S32K1XX_RTM_4.0.1
PROJECT_NAME := SM_Rover_V1
TOOLCHAIN_PATH := $(S32DS_ROOT_DIR)/gcc-6.3-arm32-eabi/bin



# Toolchain roots (no spaces/backslashes)
TOOLCHAIN_ROOT := $(S32DS_ROOT_DIR)/gcc-6.3-arm32-eabi
SYSROOT := $(TOOLCHAIN_ROOT)/arm-none-eabi/newlib

LIBRARIES_SUPPORT := $(S32DS_ROOT_DIR)/gcc-6.3-arm32-eabi/arm-none-eabi/newlib/lib/thumb/v7e-m/fpv4-sp/hard/nosys.specs
# Library dirs
LIBDIR_GCC := $(TOOLCHAIN_ROOT)/lib/gcc/arm-none-eabi/6.3.1/thumb/v7e-m/fpv4-sp/hard
LIBDIR_NEWLIB := $(SYSROOT)/lib/thumb/v7e-m/fpv4-sp/hard
SYSROOT_NEW := $(S32DS_ROOT_DIR)/gcc-6.3-arm32-eabi/arm-none-eabi/newlib/lib/thumb/v7e-m/fpv4-sp/hard
# ==== Tool commands ====
CC := $(TOOLCHAIN_PATH)/arm-none-eabi-gcc
LD := $(TOOLCHAIN_PATH)/arm-none-eabi-gcc
OBJCOPY := $(TOOLCHAIN_PATH)/arm-none-eabi-objcopy
SIZE := $(TOOLCHAIN_PATH)/arm-none-eabi-size

# ==== Project and output ====
BUILD_DIR := build
TARGET := $(BUILD_DIR)/$(PROJECT_NAME).elf
MAP_FILE := $(BUILD_DIR)/$(PROJECT_NAME).map
LD_SCRIPT := Project_Settings/Linker_Files/S32K144_64_flash.ld

# ==== Flags ====
COMMON_ARCH := -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -specs=$(LIBRARIES_SUPPORT)
CFLAGS := $(COMMON_ARCH) \
    -std=c99 \
    -DCPU_S32K144HFT0VLLT \
    -DS32K144 \
    -DSDK_OS_BAREMETAL \
    -DDEV_ERROR_DETECT \
    -O1 -g3 \
    -pedantic -Wall -Wextra \
    -Wstrict-prototypes -Wsign-compare -Wunused \
    -ffunction-sections -fdata-sections -fno-common \
    -funsigned-bitfields -fshort-enums -fno-jump-tables -funsigned-char \
    -fmessage-length=0 \
    --sysroot="$(SYSROOT_NEW)"

LDFLAGS := $(COMMON_ARCH) \
    -T"$(LD_SCRIPT)" \
    -nostartfiles \
    -Wl,-Map,"$(BUILD_DIR)/$(PROJECT_NAME).map" \
    -Wl,--gc-sections \
    -L"$(LIBDIR_GCC)" -L"$(LIBDIR_NEWLIB)" \
    -L"$(SYSROOT_NEW)" \
    -Wl,--start-group -lgcc -lc -lm -lnosys -Wl,--end-group



# ==== Include paths ====
INCLUDES := \
  -I./board \
  -I./src \
  -I./ancit_device_drivers \
  -I./ancit_lib/Digital \
  -I./ancit_lib/Others \
  -I./ancit_lib/Timers \
  -I./ancit_lib/UART \
  -I./ancit_lib/Ultrasonic \
  -I./ancit_lib/i2c/i2c_manager \
  -I./ancit_lib/i2c/oled \
  -I./MATLAB/extern/include \
  -I./MATLAB/rtw/c/src \
  -I./MATLAB/simulink/include \
  -I./SDK/platform/devices \
  -I./SDK/platform/devices/common \
  -I./SDK/platform/devices/S32K144/include \
  -I./SDK/platform/devices/S32K144/startup \
  -I./SDK/platform/drivers/inc \
  -I./SDK/platform/drivers/src/pins \
  -I./SDK/platform/drivers/src/flexcan \
  -I./SDK/platform/drivers/src/edma \
  -I./SDK/platform/drivers/src/interrupt \
  -I./SDK/platform/drivers/src/adc \
  -I./SDK/platform/drivers/src/ftm \
  -I./SDK/platform/drivers/src/lpuart \
  -I./SDK/platform/drivers/src/lptmr \
  -I./SDK/platform/drivers/src/lpi2c \
  -I./SDK/platform/drivers/src/clock/S32K1xx \
  -I./SDK/rtos/osif \
  -I $(NXP_K144_RTM)/platform \
  -I $(NXP_K144_RTM)/platform/devices \
  -I $(NXP_K144_RTM)/platform/devices/common \
  -I $(NXP_K144_RTM)/platform/devices/S32K144/include \
  -I $(NXP_K144_RTM)/platform/devices/S32K144/startup \
  -I $(NXP_K144_RTM)/platform/drivers/inc \
  -I $(NXP_K144_RTM)/platform/drivers/src/lptmr \
  -I $(NXP_K144_RTM)/platform/drivers/src/clock

# ==== Source files ====
C_SOURCES := \
	$(NXP_K144_RTM)/platform/devices/S32K144/startup/system_S32K144.c \
	$(NXP_K144_RTM)/platform/devices/startup.c \
	$(NXP_K144_RTM)/platform/drivers/src/clock/S32K1xx/clock_S32K1xx.c \
	$(NXP_K144_RTM)/platform/drivers/src/interrupt/interrupt_manager.c \
	$(NXP_K144_RTM)/platform/drivers/src/pins/pins_driver.c \
	$(NXP_K144_RTM)/platform/drivers/src/pins/pins_port_hw_access.c \
	SDK/platform/drivers/src/adc/adc_driver.c \
	SDK/platform/drivers/src/edma/edma_driver.c \
	SDK/platform/drivers/src/edma/edma_hw_access.c \
	SDK/platform/drivers/src/edma/edma_irq.c \
	SDK/platform/drivers/src/flash/flash_driver.c \
	SDK/platform/drivers/src/flexcan/flexcan_driver.c \
	SDK/platform/drivers/src/flexcan/flexcan_hw_access.c \
	SDK/platform/drivers/src/flexcan/flexcan_irq.c \
	SDK/platform/drivers/src/ftm/ftm_common.c \
	SDK/platform/drivers/src/ftm/ftm_hw_access.c \
	SDK/platform/drivers/src/ftm/ftm_ic_driver.c \
	SDK/platform/drivers/src/ftm/ftm_pwm_driver.c \
	SDK/platform/drivers/src/lpi2c/lpi2c_driver.c \
	SDK/platform/drivers/src/lpi2c/lpi2c_hw_access.c \
	SDK/platform/drivers/src/lpi2c/lpi2c_irq.c \
	SDK/platform/drivers/src/lptmr/lptmr_driver.c \
	SDK/platform/drivers/src/lptmr/lptmr_hw_access.c \
	SDK/platform/drivers/src/lpuart/lpuart_driver.c \
	SDK/platform/drivers/src/lpuart/lpuart_hw_access.c \
	SDK/platform/drivers/src/lpuart/lpuart_irq.c \
	SDK/rtos/osif/osif_baremetal.c \
	ancit_device_drivers/ancit_driver_digital_io.c \
	ancit_device_drivers/ancit_driver_timer.c \
	ancit_device_drivers/ancit_driver_uart.c \
	ancit_lib/Digital/ancit_di.c \
	ancit_lib/Digital/ancit_pwm.c \
	ancit_lib/Others/ancit_common.c \
	ancit_lib/Timers/ancit_scheduler.c \
	ancit_lib/Timers/ancit_timer.c \
	ancit_lib/UART/ancit_uart_rte.c \
	ancit_lib/Ultrasonic/ancit_ultrasonic.c \
	ancit_lib/i2c/i2c_manager/ancit_i2c_manager.c \
	ancit_lib/i2c/oled/ancit_oled_driver.c \
	ancit_lib/i2c/oled/ancit_oled_handler.c \
	ancit_lib/i2c/oled/ssd1306_fonts.c \
	board/clock_config.c \
	board/peripherals_MyADC_0.c \
	board/peripherals_MyADC_1.c \
	board/peripherals_edma_config_1.c \
	board/peripherals_flash_1.c \
	board/peripherals_flexTimer_ic_echo.c \
	board/peripherals_flexTimer_pwm_0.c \
	board/peripherals_flexcan_config_0.c \
	board/peripherals_lpi2c_config_1.c \
	board/peripherals_lptmr_1.c \
	board/peripherals_lpuart_1.c \
	board/peripherals_osif_1.c \
	board/pin_mux.c \
	src/ACC.c \
	src/ACC_data.c \
	src/SK_DC.c \
	src/genx_common.c \
	src/genx_i2c_manager.c \
	src/genx_oled.c \
	src/genx_pwm.c \
	src/genx_scheduler.c \
	src/genx_simulink_bridge.c \
	src/genx_uart_rte.c \
	src/main.c \
	src/rt_logging.c \
	src/rt_nonfinite.c

ASM_SOURCES := \
    $(NXP_K144_RTM)/platform/devices/S32K144/startup/gcc/startup_S32K144.S

# ==== Object files ====
C_OBJS := $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
ASM_OBJS := $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.S=.o)))
OBJS := $(C_OBJS) $(ASM_OBJS)

# ==== Source-to-object mapping ====
SRC_TO_OBJ := \
  $(foreach src,$(C_SOURCES),$(BUILD_DIR)/$(notdir $(src:.c=.o))|$(src)) \
  $(foreach src,$(ASM_SOURCES),$(BUILD_DIR)/$(notdir $(src:.S=.o))|$(src))

# ==== Default target ====
all: $(TARGET)

# ==== Link rule (group libs so __aeabi_* etc. resolve) ====
$(TARGET): $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS)  -o $@
	$(SIZE) $@


# ==== Unified build rule for both .c and .S ====
$(BUILD_DIR)/%.o:
	@mkdir -p $(BUILD_DIR)
	$(eval SRC := $(word 2,$(subst |, ,$(filter $@|%,$(SRC_TO_OBJ)))))
	$(CC) $(CFLAGS) $(INCLUDES) -c $(SRC) -o $@

# ==== Clean ====
clean:
	rm -rf $(BUILD_DIR)

# ==== Optional HEX output ====
hex: $(TARGET)
	$(OBJCOPY) -O ihex $< $(BUILD_DIR)/$(PROJECT_NAME).hex

# ======= SREC Output =============
srec: $(TARGET)
	$(OBJCOPY) -O srec $< $(BUILD_DIR)/$(PROJECT_NAME).srec --srec-forceS3