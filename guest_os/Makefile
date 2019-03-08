# configure
ADDNAME = llvm-
TARGET = sampleOS

# toolchain
CC = clang  -target aarch64-elf 
CPP = clang++ -target aarch64-elf 
AS = $(ADDNAME)as  -target aarch64-elf 
LD = ld.lld
STRIP = $(ADDNAME)strip
READELF = $(ADDNAME)readelf
OBJDUMP = $(ADDNAME)objdump
OBJCOPY = $(ADDNAME)objcopy-6.0

AARCH64-QEMU = /home/envzhu/programing/tools/arm/bin/qemu-system-aarch64
AARCH64-GDB  = /home/envzhu/programing/tools/arm/bin/aarch64-elf-gdb

RM = rm
# souces
OBJS = startup.o main.o lib.o

LDSCRIPT=memory.ld

# FLAGS
CFLAGS =  -mcpu=cortex-a53 -mfloat-abi=soft -mlittle-endian -fno-builtin -nostdlib
CFLAGS += -I.
CFLAGS += -g
LFLAGS = -static -nostdlib

.SUFFIXES: .elf .img

all: $(TARGET).img

$(TARGET).elf: $(LDSCRIPT) $(OBJS) Makefile
	$(LD) $(LFLAGS) -T $(LDSCRIPT) $(OBJS)  -o $@ 
	cp $(TARGET).elf $(TARGET)
	$(OBJCOPY) $(TARGET) -strip-all
	$(OBJDUMP) -d -r $(TARGET).elf > $(TARGET).asm

.elf.img:
	$(OBJCOPY) -O binary $< $@
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
.S.o:
	$(CC) $(CFLAGS) -c $< -o $@

disas:
	$(OBJDUMP) -d $(TARGET).elf > $(TARGET).asm

image :		$(TARGET).img

img2obj: 	$(TARGET).img
	aarch64-linux-gnu-objcopy -I binary -B aarch64 -O elf64-littleaarch64 $(TARGET).bin $(TARGET)-img.o

line:
	find ./ -name "*.[chsS]" | xargs wc -l

run: $(TARGET).elf
	$(AARCH64-QEMU) \
	-m 512 -M raspi3 -kernel $(TARGET).elf \
	-serial null -serial stdio \
	-accel tcg,thread=single -smp 4

qemu: $(TARGET).elf
	$(AARCH64-QEMU) \
	-m 512 -M raspi3 -kernel $(TARGET).elf \
	-serial null -serial stdio \
	-gdb tcp::12345 -S \
	-accel tcg,thread=single -smp 1

gdb: $(TARGET).elf
	$(AARCH64-GDB) $(TARGET).elf -x gdb.src

serial:$(TARGET).elf
	minicom -b 115200 -D /dev/ttyUSB1

clean:
	$(RM) -f $(OBJS) $(TARGET) $(TARGET).elf $(TARGET).img
