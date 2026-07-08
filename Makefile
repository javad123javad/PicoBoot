BOOTLOADER_NAME=bootloader-m3
APP=app-m3
####
CROSS_COMPILE=arm-none-eabi
CC=$(CROSS_COMPILE)-gcc
LD=$(CROSS_COMPILE)-ld
OBJCOPY=$(CROSS_COMPILE)-objcopy

CFLAGS=-mcpu=cortex-m3 -mthumb -g -ggdb -Wall -Wno-main
LDFLAGS= -gc-sections -nostdlib 
all: $(BOOTLOADER_NAME).bin $(APP).bin image.bin size
image.bin:
	cat $(BOOTLOADER_NAME).bin $(APP).bin > image.bin
$(BOOTLOADER_NAME).bin: $(BOOTLOADER_NAME).elf
	$(OBJCOPY) -O binary --pad-to=4096 --gap-fill=0xFF $^ $@
$(BOOTLOADER_NAME).elf: $(BOOTLOADER_NAME).o bootloader.ld
	$(LD) -T bootloader.ld $(LDFLAGS) -Map=bootloader.map $(BOOTLOADER_NAME).o -o $@
$(BOOTLOADER_NAME).o: $(BOOTLOADER_NAME).c
	$(CC) $(CFLAGS) -c -o $(BOOTLOADER_NAME).o $(BOOTLOADER_NAME).c 


# Build App
$(APP).bin: $(APP).elf
	$(OBJCOPY) -O binary $^ $@
$(APP).elf: $(APP).o app.ld
	$(LD) -T app.ld $(LDFLAGS) -Map=app.map  startup.o $(APP).o -o $@
$(APP).o: startup.o app.c
	$(CC) $(CFLAGS) -c -o $(APP).o  app.c
startup.o: startup.c
size:
	$(CROSS_COMPILE)-size -A -x $(APP).elf
clean:
	rm -rf $(BOOTLOADER_NAME).bin $(BOOTLOADER_NAME).elf *.o $(BOOTLOADER_NAME).map
	rm -rf $(APP).bin $(APP).elf $(APP).map 
	rm -rf image.bin
