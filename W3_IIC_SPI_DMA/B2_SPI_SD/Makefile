TARGET = firmware

CC = arm-none-eabi-gcc

CFLAGS = -mcpu=cortex-m3 -mthumb -Wall -O0 \
         -ffreestanding -nostdlib -fno-builtin \
         -I../../CMSIS \
         -I../../STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/STM32F10x_StdPeriph_Driver/inc \
         -IDrivers \
         -DSTM32F10X_MD

LDFLAGS = -T STM32F103C8Tx_FLASH.ld -nostdlib

SRCS = main.c \
       startup.c \
       Drivers/spi.c \
       Drivers/sd.c \
       Drivers/uart.c \
       Drivers/fat32.c \
       Drivers/command.c

all: $(TARGET).bin

$(TARGET).elf: $(SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(TARGET).bin: $(TARGET).elf
	arm-none-eabi-objcopy -O binary $< $@

flash: $(TARGET).bin
	st-flash write $(TARGET).bin 0x8000000

clean:
	rm -f *.elf *.bin

.PHONY: all flash clean
