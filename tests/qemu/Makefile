# QEMU tests for mm_halow_sched.c on a Cortex-M55 (mps3-an547 == the N6's core).
#
#   make        build + run under QEMU
#
# Cortex-M55 needs arm-none-eabi-gcc 14 or newer (Ubuntu 24.04 ships 13.2,
# which rejects -mcpu=cortex-m55 with hard float).  Override CC to point at a
# suitable toolchain if the default is too old.
CC       ?= arm-none-eabi-gcc
HALOW    := ../../src
# mps3.ld is a copy of MicroPython's ports/qemu/mcu/arm/mps3.ld (MIT).
LDSCRIPT := mps3.ld

# Matches ports/qemu/boards/MPS3_AN547/mpconfigboard.mk, which in turn matches
# how the N6 is built: hard float, so the vpush {d8-d15} path is live.
ARCH   := -mthumb -mcpu=cortex-m55 -mfloat-abi=hard -mfpu=fpv5-d16
CFLAGS := $(ARCH) -std=c11 -Wall -Wextra -Werror -Wno-unused-parameter -Og -g \
          -ffreestanding -fno-common \
          -Istub -I$(HALOW) -I../../lib/mm-iot-sdk/framework/morselib/include
LDFLAGS := $(ARCH) -T$(LDSCRIPT) -nostartfiles -Wl,--gc-sections \
           -Wl,-Map=test_sched.map -specs=nosys.specs \
           -Wl,--no-warn-rwx-segments

SRC := startup.c test_sched.c $(HALOW)/mm_halow_sched.c

all: run

test_sched.elf: $(SRC) $(LDSCRIPT)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRC)

run: test_sched.elf
	@qemu-system-arm -M mps3-an547 -cpu cortex-m55 -nographic \
	    -semihosting-config enable=on,target=native \
	    -kernel test_sched.elf

clean:
	rm -f test_sched.elf test_sched.map

.PHONY: all run clean
