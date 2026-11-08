# Ten du an va thu muc OUTPUT 					-----------------------------------------
TARGET 		= app1
BUILD_DIR 	= build

# Toolchain, cac cong cu 						-----------------------------------------
PREFIX 		= arm-none-eabi-
CC     		= $(PREFIX)gcc
AS     		= $(PREFIX)gcc -x assembler-with-cpp
CP     		= $(PREFIX)objcopy
SZ     		= $(PREFIX)size
HEX    		= $(CP) -O ihex
BIN    		= $(CP) -O binary -S

# Thong tin Target, kien truc va tap lenh 		-----------------------------------------
CPU       	= -mcpu=cortex-m3
MCU       	= $(CPU) -mthumb

# File Linker, so do chip 						-----------------------------------------
LDSCRIPT  	= STM32F103C8Tx_FLASH.ld

# Danh sach file .c vaf .s 						-----------------------------------------
C_SOURCES   = $(wildcard Src/*.c) \
			$(wildcard Drivers/STM32F10x_StdPeriph_Driver/src/*.c)
ASM_SOURCES = startup_stm32f103xb.s

C_DEFS 		=  \
-DSTM32F10X_MD \
-DUSE_STDPERIPH_DRIVER

C_INCLUDES 	= \
-IInc \
-IDrivers/STM32F10x_StdPeriph_Driver/inc \
-IDrivers/CMSIS/Device/ST/STM32F1xx/Include \
-IDrivers/CMSIS/Include

# Cac co bien dich 								-----------------------------------------
OPT     	= -Og
CFLAGS  	= $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall \
			-fdata-sections -ffunction-sections \
			-MMD -MP -MF"$(@:%.o=%.d)"

LDFLAGS 	= $(MCU) -specs=nano.specs -T$(LDSCRIPT) \
			-Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref \
			-Wl,--gc-sections

OBJECTS = $(addprefix $(BUILD_DIR)/, $(notdir $(C_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/, $(notdir $(ASM_SOURCES:.s=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

# Cac quy tac build 							-----------------------------------------
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS)
	$(CC) $^ $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	$(BIN) $< $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf
	$(HEX) $< $@

$(BUILD_DIR):
	mkdir -p $@

# Cac lenh make 								-----------------------------------------
.PHONY: all clean flash

all: $(BUILD_DIR)/$(TARGET).elf \
		$(BUILD_DIR)/$(TARGET).bin \
		$(BUILD_DIR)/$(TARGET).hex

flash: $(BUILD_DIR)/$(TARGET).bin
	st-flash write $< 0x08012000

clean:
	rm -rf $(BUILD_DIR)

-include $(wildcard $(BUILD_DIR)/*.d)