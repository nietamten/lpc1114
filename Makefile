# Sources and targets
TARGET			= main

#SOURCE			= startup.c memcpy.c 
#SOURCE			= startup.c test_I2C.c I2C.c
#SOURCE			= startup.c test_SysTick.c SysTick.c 
#SOURCE			= startup.c test_lcd.c SysTick.c I2C.c lcd.c
#SOURCE			= startup.c test_SYSPLL.c UART.c SysTick.c lcd.c I2C.c SYSPLL.c
#SOURCE			= startup.c IR.c  UART.c SysTick.c 
#SOURCE			= startup.c test_Stepper.c UART.c SysTick.c lcd.c I2C.c 
SOURCE			= startup.c test_UART_TX.c UART.c SysTick.c  I2C.c INA219.c 
SOURCE_ASM = libaeabi-cortexm0/crt.S libaeabi-cortexm0/idivmod.S \
	libaeabi-cortexm0/ldivmod.S libaeabi-cortexm0/llsr.S \
	libaeabi-cortexm0/memset.S libaeabi-cortexm0/modsi3.S libaeabi-cortexm0/ulcmp.S \
	libaeabi-cortexm0/umodsi3.S libaeabi-cortexm0/uread8.S libaeabi-cortexm0/uwrite8.S \
	libaeabi-cortexm0/divdi3.S libaeabi-cortexm0/idiv.S libaeabi-cortexm0/lcmp.S \
	libaeabi-cortexm0/llsl.S libaeabi-cortexm0/lmul.S  libaeabi-cortexm0/memmove.S \
	libaeabi-cortexm0/moddi3.S libaeabi-cortexm0/umoddi3.S libaeabi-cortexm0/uread4.S \
	libaeabi-cortexm0/uwrite4.S
#libaeabi-cortexm0/lasr.S 
	
#SOURCE			= startup.c main.c lcd.c SysTick.c I2C.c 

# Processor/Microcontroller family
MCU				= cortex-m0

# Tools
CC 				= arm-none-eabi-gcc
OBJCOPY 		= arm-none-eabi-objcopy
OBJDUMP			= arm-none-eabi-objdump
SIZE			= arm-none-eabi-size
PGM				= openocd-x64-0.8.0.exe
RM 				= rm
ECHO			= echo

# C Compiler Optons
CFLAGS=-mthumb # 	           		  Using the Thumb Instruction Set
CFLAGS+= -mcpu=$(MCU) #			      The MCU Family
CFLAGS+= -O2 # 						  Compile with Size Optimizations
#CFLAGS+= -ggdb #					  Generate debugging info

CFLAGS+= -ffunction-sections # 		  Create a separate function section
CFLAGS+= -fdata-sections # 			  Create a separate data section
#CFLAGS+= -std=c99 # 				  Comply with C99
CFLAGS+= -Wall # 					  Enable All Warnings 
CFLAGS+= -fno-common #				  Disable COMMON sections
CFLAGS+= -fno-builtin #
CFLAGS+= -Wa,-adhlns=$(<:%.c=%.lst) # Generate assembly files

#CFLAGS+= -x c++ 
#CFLAGS+= -I /usr/arm-none-eabi/include/c++/8.2.0/ -I /usr/arm-none-eabi/include/c++/8.2.0/arm-none-eabi/thumb/nofp/

# CFLAGS+= -fdump-tree-all #          Dump GIMPLE/SSA passes
# CFLAGS+= -fdump-rtl-all -da #       Dump RTL passes

# Linker Options
LDFLAGS=-Wl,--gc-sections # 			Linker to ignore sections that aren't used.
LDFLAGS+= -Wl,-Map,$(TARGET).map #		Generate memory map file
LDFLAGS+= -Wl,-T,"$(TARGET).ld" # 	Path to Linker Script
#LDFLAGS+= -Wl,--build-id=none #			Do NOT require a .note.gnu.build-id section (as of GCC 4.9)
LDFLAGS+= -nostdlib
LDFLAGS+= -nostartfiles
LDFLAGS+= -g
LDFLAGS+= -mcpu=cortex-m0 
LDFLAGS+= -mthumb 

CFLAGS+= -Llibaeabi-cortexm0

#LDFLAGS+= -L.
#LDFLAGS+= -laeabi-cortexm0

#LDFLAGS+= -fPIC
#CFLAGS+=  -fpic 
#LDFLAGS+= -Wl,-Bstatic


# Define object and assembly list files
OBJ = $(SOURCE:%.c=%.o)
OBJ_ASM = $(SOURCE_ASM:%.S=%.o)
LST = $(SOURCE:%.c=%.lst)


#TST = $(SOURCE_ASM:%.S=%.o)
#$(warning TST is $(TST)) 

.PHONY: all clean bin disasm program

all: $(TARGET).elf

$(TARGET).elf: $(OBJ) $(OBJ_ASM)
	$(CC) $(LDFLAGS) -o $@ $(OBJ)  $(OBJ_ASM)
	$(SIZE) -x -A $(TARGET).elf | grep -E -i "section|vectors|text|bss|data"

%.o: %.c 
	$(CC) $(CFLAGS) -c -o $@ $<
#	$(MAKE) -C libaeabi-cortexm0
	
clean:
#	$(MAKE) -C libaeabi-cortexm0 clean
	$(RM) $(OBJ)
	$(RM) $(OBJ_ASM)
	$(RM) $(TARGET).bin $(TARGET).hex $(TARGET).elf $(TARGET).map $(TARGET).lss $(LST)

	
bin: $(TARGET).bin

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

disasm: $(TARGET).lss

$(TARGET).lss: $(TARGET).elf
	$(OBJDUMP) -h -S $< > $@

program: $(TARGET).bin
	lpc21isp -control -bin main.bin /dev/ttyUSB0 115200 14746
#	$(PGM) -f interface/stlink-v1.cfg -f lpc1114.cfg -c "program $(TARGET).elf verify reset"



#	********** NOTES ************

#	[How to extract HEX from ELF]
#	arm-none-eabi-objcopy -O ihex hello_world.elf hello_world.hex

#	[How to cross-compile for ARM Cortex-M (*.c to *.o) without linking "-c"]
#	arm-none-eabi-gcc -Os -ffunction-sections -fdata-sections -Wall -c -mcpu=cortex-m0 -mthumb -o hello_world.o hello_world.c

#	[How to link *.o files]
#	arm-none-eabi-gcc -Wl,--gc-sections -Wl,-Map,hello_world.map -Wl,-T,"..\startup.ld" -o hello_world.elf hello_world.o startup.o

#   [How to view a particular section]
# 	arm-none-eabi-objdump -x .vectors hello_world.elf

#	[How to view the symbol table]
#	arm-none-eabi-objdump -t hello_world.elf
#	arm-none-eabi-nm -n hello_world.elf

