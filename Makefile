# Some useful Makefile abbreviations
# $@ = target file
# $< = first dependency
# $^ = all dependencies

# First rule is the one executed when no parameters are fed to the Makefile
all: run

# Notice how dependencies are built as needed
kernel.bin: kernel_entry.o kernel.o
	ld -m elf_i386 -s -o $@ -Ttext 0x1000 $^ --oformat binary

kernel_entry.o: kernel_entry.asm
	nasm $< -f elf -o $@
# -Os
kernel.o: kernel.c
	gcc -fno-pic -m32 -ffreestanding -Os -Wl,--gc-sections  -c $< -o $@

# Rule to disassemble the kernel - may be useful to debug
kernel.dis: kernel.bin
	ndisasm -b 32 $< > $@

bootsect.bin: bootsect.asm
	nasm $< -f bin -o $@

boothello.bin: boothello.asm
	nasm $< -f bin -o $@

os-image.bin: bootsect.bin kernel.bin
	cat $^ > $@

run: os-image.bin
	qemu-system-i386 -curses -fda  $< -m 500M

clean:
	rm *.bin *.o *.dis

