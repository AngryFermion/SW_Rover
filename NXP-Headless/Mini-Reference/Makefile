# ==== Toolchain path ====

S32DS_ROOT_DIR := C:/NXP/S32DS.3.5/S32DS/build_tools/gcc_b1620
NXP_K144_RTM := C:/NXP/S32DS.3.5/S32DS/software/S32SDK_S32K1XX_RTM_4.0.1
PROJECT_NAME := SubSys_Mini_HeadLight
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
    -Wl,--start-group -lgcc -lc -lnosys -Wl,--end-group



# ==== Include paths ====
INCLUDES := \
  -I./board \
  -I./src \
  -I./ancit_device_drivers \
  -I./ancit_lib/Analog \
  -I./ancit_lib/CAN \
  -I./ancit_lib/Digital \
  -I./ancit_lib/DWIN \
  -I./ancit_lib/Others \
  -I./ancit_lib/Timers \
  -I./ancit_lib/UART \
  -I./ancit_lib \
  -I./ancit_fota \
  -I./ancit_boot/BootMgr \
  -I./ancit_boot/NvmMgr \
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
  -I./SDK/platform/drivers/src/interrupt \
  -I./SDK/platform/drivers/src/lpuart \
  -I./SDK/platform/drivers/src/lptmr \
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
C_SOURCES :=

ASM_SOURCES := \
    Project_Settings/Startup_Code/startup_S32K144.S

# ==== Object files ====
C_OBJS := $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
ASM_OBJS := $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.S=.o)))
OBJS := $(C_OBJS) $(ASM_OBJS)

# ==== Source-to-object mapping ====
SRC_TO_OBJ := \
  $(foreach src,$(C_SOURCES),$(BUILD_DIR)/$(notdir $(src:.c=.o)):$(src)) \
  $(foreach src,$(ASM_SOURCES),$(BUILD_DIR)/$(notdir $(src:.S=.o)):$(src))

# ==== Default target ====
all: $(TARGET)

# ==== Link rule (group libs so __aeabi_* etc. resolve) ====
$(TARGET): $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS)  -o $@
	$(SIZE) $@


# ==== Unified build rule for both .c and .S ====
$(BUILD_DIR)/%.o:
	@mkdir -p $(BUILD_DIR)
	$(eval SRC := $(word 2,$(subst :, ,$(filter $@:%,$(SRC_TO_OBJ)))))
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