# configure
ADDNAME = llvm-
TARGET = semzhu-visor
LINUX-IMG = ./Image-hvc
LINUX-SYM = ./vmlinux-hvc

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
OBJS = startup.o init.o vector.o asm_func.o interrupt.o uart.o print.o
OBJS += lib.o log.o malloc.o
OBJS += phys_cpu_setting.o guest_vm.o spinlock.o hyp_mmu.o hyp_timer.o pmu.o sd.o smp_mbox.o
OBJS += vcpu.o vm.o hyp_call.o pcpu.o schedule.o fcfs_schedule.o rr_schedule.o no_schedule.o
OBJS += vtimer.o virt_mmio.o virq.o virt_bcm2836_mailbox.o virt_bcm2835_mailbox.o virt_bcm2835_cprman.o virt_gpio.o
OBJS += hyp_security.o hyp_security_fast.o

# guest os
GUEST_OBJS = sampleOS-img.o linux-img.o kozos-img.o bcm2837-rpi-3-b-img.o initrd-img.o


DEPS = $(OBJS:%.o=%.d)

LDSCRIPT = memory.ld

# FLAGS
CFLAGS =  -mcpu=cortex-a53 -mfloat-abi=soft -mlittle-endian -fno-builtin -nostdlib
CFLAGS += -I. -MMD -MP
CFLAGS += -g
LFLAGS = -static -nostdlib

.SUFFIXES: .elf .bin

all: $(TARGET).bin

-include $(DEPS)

$(TARGET).elf: $(LDSCRIPT) $(OBJS) Makefile
	$(LD) $(LFLAGS) -T $(LDSCRIPT) $(OBJS) $(GUEST_OBJS)  -o $@ 
	cp $(TARGET).elf $(TARGET)
	$(OBJCOPY) $(TARGET) -strip-all
	$(OBJDUMP) -d -r $(TARGET).elf > $(TARGET).asm

link:
	$(LD) $(LFLAGS) -T $(LDSCRIPT) $(OBJS) $(IMG_OBJS)  -o $@ 
	cp $(TARGET).elf $(TARGET)
	$(OBJCOPY) $(TARGET) -strip-all
	$(OBJDUMP) -d -r $(TARGET).elf > $(TARGET).asm
	
.elf.bin:
	$(OBJCOPY) -O binary $< $@
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
.S.o:
	$(CC) $(CFLAGS) -c $< -o $@

gusetOS-img:
	(cd ./guest_os; make)

img2obj:
	(cd ./guest_os; make)
	aarch64-linux-gnu-objcopy -I binary -B aarch64 -O elf64-littleaarch64 \
		./guest_os/sampleOS.img sampleOS-img.o

dtb2obj:
	aarch64-linux-gnu-objcopy -I binary -B aarch64 -O elf64-littleaarch64 \
		./bcm2837-rpi-3-b.dtb bcm2837-rpi-3-b-img.o

initrd2obj:
	aarch64-linux-gnu-objcopy -I binary -B aarch64 -O elf64-littleaarch64 \
		./initrd.cpio initrd-img.o

disas:
	$(OBJDUMP) -d $(TARGET).elf > $(TARGET).asm

line:
	find ./ -name "*.[chsS]" | xargs wc -l

run: $(TARGET).bin
	$(AARCH64-QEMU) \
	-m 1024 -M raspi3 -kernel $(TARGET).bin \
	-serial pty -serial stdio \
	-accel tcg,thread=single -smp 4 -icount 1

qemu: $(TARGET).bin
	$(AARCH64-QEMU) \
	-m 1024 -M raspi3 -kernel $(TARGET).bin \
	-serial stdio -serial pty \
	-gdb tcp::12345 -S \
	-icount 1 \
	-accel tcg,thread=single -smp 1

gdb: $(TARGET).elf
	$(AARCH64-GDB) $(TARGET).elf -x gdb.src

gdb-linux: 	$(TARGET).elf
	$(AARCH64-GDB) vmlinux-hvc -x gdb.src

linux-run:
	$(AARCH64-QEMU) \
	 -m 256 -M raspi3 -cpu cortex-a53 -nographic -smp 1 \
	 -dtb bcm2837-rpi-3-b.dtb \
	 -kernel $(LINUX-IMG) \
	 -append "rw earlycon=pl011,0x3f201000 console=ttyAMA0 loglevel=8 " \
	 -accel tcg,thread=single -smp 1

linux-qemu:
	$(AARCH64-QEMU) \
	 -m 256 -M raspi3 -cpu cortex-a53 -nographic -smp 1 \
	 -dtb bcm2837-rpi-3-b.dtb \
	 -kernel $(LINUX-IMG) \
	 -append "rw earlycon=pl011,0x3f201000 console=ttyAMA0 loglevel=8 " \
	 -gdb tcp::12345 -S\
	 -accel tcg,thread=single -smp 1

linux-gdb:
	$(AARCH64-GDB) ./vmlinux-hvc -x linux-gdb.src

linux-img:
	aarch64-linux-gnu-objcopy -I binary -B aarch64 -O elf64-littleaarch64 \
		$(LINUX-IMG) linux-img.o

serial:$(TARGET).elf
	minicom -b 115200 -D /dev/ttyUSB1

clean:
	$(RM) -f $(OBJS) $(DEPS) $(TARGET) $(TARGET).elf $(TARGET).bin
