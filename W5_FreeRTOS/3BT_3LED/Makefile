TARGET = firmware

CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

FREERTOS = ../../FreeRTOS-Kernel
STD_PERIPH = ../../STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/STM32F10x_StdPeriph_Driver
CMSIS = ../../CMSIS
SPL_TEMPLATE = ../../STM32F10x_StdPeriph_Lib_V3.5.0/Project/STM32F10x_StdPeriph_Template

CFLAGS = -mcpu=cortex-m3 -mthumb -Wall -O0 \
         -ffreestanding -nostdlib -fno-builtin \
         -I$(CMSIS) \
         -I$(STD_PERIPH)/inc \
         -I$(SPL_TEMPLATE) \
         -IDrivers \
         -I$(FREERTOS)/include \
         -I$(FREERTOS)/portable/GCC/ARM_CM3 \
         -DSTM32F10X_MD \
         -DUSE_STDPERIPH_DRIVER \
         '-Dassert_param(expr)=((void)0)'

LDFLAGS = -T STM32F103C8Tx_FLASH.ld -nostdlib -lgcc

SRCS = main.c \
       startup.c \
       Drivers/led.c \
       Drivers/button.c \
       Drivers/libc.c \
       $(STD_PERIPH)/src/stm32f10x_gpio.c \
       $(STD_PERIPH)/src/stm32f10x_rcc.c \
       $(STD_PERIPH)/src/stm32f10x_tim.c \
       $(STD_PERIPH)/src/misc.c \
       $(FREERTOS)/tasks.c \
       $(FREERTOS)/list.c \
       $(FREERTOS)/queue.c \
       $(FREERTOS)/portable/GCC/ARM_CM3/port.c \
       $(FREERTOS)/portable/MemMang/heap_4.c

all: $(TARGET).bin

$(TARGET).elf: $(SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

flash: $(TARGET).bin
	st-flash write $(TARGET).bin 0x08000000

clean:
	rm -f *.elf *.bin

.PHONY: all flash clean
