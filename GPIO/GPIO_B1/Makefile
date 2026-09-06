TARGET = firmware
CC = arm-none-eabi-gcc
CFLAGS = -mcpu=cortex-m3 -mthumb -Wall -O0 -ffreestanding -nostdlib -fno-builtin \
         -I../CMSIS -DSTM32F10X_MD
LDFLAGS = -T STM32F103C8Tx_FLASH.ld -nostdlib

SRCS = main.c startup.c

all: $(TARGET).bin

$(TARGET).elf: $(SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(TARGET).bin: $(TARGET).elf
	arm-none-eabi-objcopy -O binary $< $@

flash: $(TARGET).bin
	st-flash write $(TARGET).bin 0x8000000

clean:
	rm -f *.elf *.bin
