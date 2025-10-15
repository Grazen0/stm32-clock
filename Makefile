TARGET := clock.elf
BOARD := STM32F103xB

BUILD_DIR := ./build
SRC_DIRS := ./src

SRCS := $(shell find $(SRC_DIRS) -name '*.c' -or -name '*.s') \
		./external/CMSIS/Device/ST/STM32F1/Source/Templates/system_stm32f1xx.c
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
LINKER := ./data/linker_script.ld

INC_DIRS := ./src \
			./external/CMSIS/Device/ST/STM32F1/Include \
			./external/CMSIS/CMSIS/Core/Include
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CC := arm-none-eabi-gcc

CFLAGS := $(INC_FLAGS) -nostdlib -mcpu=cortex-m3 -mthumb -D$(BOARD) -g -std=c23

OPENOCD := openocd
OPENOCD_FLAGS := -f interface/stlink.cfg -f target/stm32f1x.cfg

GDB := arm-none-eabi-gdb

BEAR := bear
CDB := compile_commands.json

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJS) $(LINKER)
	$(CC) $(CFLAGS) -T $(LINKER) -o $@ $(OBJS) $(LDFLAGS)

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.s.o: %.s
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

flash: $(BUILD_DIR)/$(TARGET)
	$(OPENOCD) $(OPENOCD_FLAGS) -c "program $< verify reset"

debug: $(BUILD_DIR)/$(TARGET)
	$(GDB) $<

compdb:
	mkdir -p $(BUILD_DIR)
	$(BEAR) --output $(BUILD_DIR)/$(CDB) -- make -B

.PHONY: clean all flash compdb
