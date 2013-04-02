#
#       !!!! Do NOT edit this makefile with an editor which replace tabs by spaces !!!!    
#
##############################################################################################
# 
# On command line:
#
# make all = Create project
#
# make clean = Clean project files.
#
# To rebuild project do "make clean" and "make all".
#

##############################################################################################
# Start of tools section
#

TRGT = arm-none-eabi-
CC   = $(TRGT)gcc
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp
BIN  = $(CP) -O ihex 

MCU  = cortex-m4

#
# End of tools section
##############################################################################################

##############################################################################################
# Start of user section
#

# 
# Define project name and Ram/Flash mode here
PROJECT        = test
RUN_FROM_FLASH = 1
USE_HARD_FPU   = 1
HEAP_SIZE      = 8192
STACK_SIZE     = 2048

#
# Define linker script file here
#
ifeq ($(RUN_FROM_FLASH), 0)
LDSCRIPT = ./prj/stm32f4xx_ram.ld
FULL_PRJ = $(PROJECT)_ram
else
LDSCRIPT = ./prj/stm32f4xxxg_flash.ld
FULL_PRJ = $(PROJECT)_rom
endif

#
# Define FPU settings here
#
ifeq ($(USE_HARD_FPU), 0)
FPU =
else
FPU = -mfloat-abi=hard -mfpu=fpv4-sp-d16 -D__FPU_USED=1
endif


# List all user C define here, like -D_DEBUG=1
UDEFS = -DUSE_STDPERIPH_DRIVER -DUSE_USB_OTG_FS

# Define ASM defines here
UADEFS = 

# List C source files here
SRC  = ./cmsis/device/system_stm32f4xx.c \
       ./STM32F4xx_StdPeriph_Driver/src/misc.c \
       ./STM32F4xx_StdPeriph_Driver/src/stm32f4xx_exti.c \
       ./STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c \
       ./STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c \
       ./STM32F4xx_StdPeriph_Driver/src/stm32f4xx_syscfg.c \
       ./STM32_USB_Device_Library/Core/src/usbd_core.c \
       ./STM32_USB_Device_Library/Core/src/usbd_ioreq.c \
       ./STM32_USB_Device_Library/Core/src/usbd_req.c \
       ./STM32_USB_Device_Library/Class/hid/src/usbd_hid_core.c \
       ./STM32_USB_OTG_Driver/src/usb_core.c \
       ./STM32_USB_OTG_Driver/src/usb_dcd.c \
       ./STM32_USB_OTG_Driver/src/usb_dcd_int.c \
       ./src/stm32f4_discovery.c \
       ./src/stm32f4xx_it.c \
       ./src/syscalls.c \
       ./src/usb_bsp.c \
       ./src/usbd_desc.c \
       ./src/usbd_usr.c \
       ./src/main.c

       #./STM32_USB_OTG_Driver/src/usb_hcd.c \
       #./STM32_USB_OTG_Driver/src/usb_otg.c \

# List ASM source files here
ASRC = ./cmsis/device/startup_stm32f4xx.s

# List all user directories here
UINCDIR = ./inc \
          ./cmsis/core \
          ./cmsis/device \
          ./STM32F4xx_StdPeriph_Driver/inc/\
          ./STM32_USB_Device_Library/Core/inc \
          ./STM32_USB_Device_Library/Class/hid/inc \
          ./STM32_USB_OTG_Driver/inc

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS = 

# Define optimisation level here
OPT = -O0 
#OPT = -O2 -falign-functions=16 -fno-inline -fomit-frame-pointer

#
# Define openocd flash commands here
#
OPENOCD_FLASH_CMDS=-c init -c halt -c 'sleep 10' -c 'flash write_image erase unlock $(FULL_PRJ).elf' -c shutdown

#
# End of user defines
##############################################################################################


INCDIR  = $(patsubst %,-I%,$(UINCDIR))
LIBDIR  = $(patsubst %,-L%,$(ULIBDIR))

ifeq ($(RUN_FROM_FLASH), 0)
DEFS    = $(UDEFS) -DRUN_FROM_FLASH=0 -DVECT_TAB_SRAM
else
DEFS    = $(UDEFS) -DRUN_FROM_FLASH=1
endif

ADEFS   = $(UADEFS) -D__HEAP_SIZE=$(HEAP_SIZE) -D__STACK_SIZE=$(STACK_SIZE)
OBJS    = $(ASRC:.s=.o) $(SRC:.c=.o)
LIBS    = $(ULIBS)
MCFLAGS = -mthumb -mcpu=$(MCU) $(FPU)

ASFLAGS  = $(MCFLAGS) $(OPT) -g -gdwarf-2 -Wa,-amhls=$(<:.s=.lst) $(ADEFS)

CPFLAGS  = $(MCFLAGS) $(OPT) -gdwarf-2 -Wall -Wstrict-prototypes -fverbose-asm 
CPFLAGS += -ffunction-sections -fdata-sections -Wa,-ahlms=$(<:.c=.lst) $(DEFS)

LDFLAGS  = $(MCFLAGS) -nostartfiles -T$(LDSCRIPT) -Wl,-Map=$(FULL_PRJ).map,--cref,--gc-sections,--no-warn-mismatch $(LIBDIR)

# Generate dependency information
CPFLAGS += -MD -MP -MF .dep/$(@F).d

#
# makefile rules
#

all: $(OBJS) $(FULL_PRJ).elf $(FULL_PRJ).hex

flash:
	openocd -f openocd.cfg $(OPENOCD_FLASH_CMDS)

debug:
	openocd -f openocd.cfg

%.o : %.c
	$(CC) -c $(CPFLAGS) -I . $(INCDIR) $< -o $@

%.o : %.s
	$(AS) -c $(ASFLAGS) $< -o $@

%elf: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $(LIBS) -o $@
  
%hex: %elf
	$(BIN) $< $@

clean:
	-rm -f $(OBJS)
	-rm -f $(FULL_PRJ).elf
	-rm -f $(FULL_PRJ).map
	-rm -f $(FULL_PRJ).hex
	-rm -f $(SRC:.c=.c.bak)
	-rm -f $(SRC:.c=.lst)
	-rm -f $(ASRC:.s=.s.bak)
	-rm -f $(ASRC:.s=.lst)
	-rm -fR .dep

# 
# Include the dependency files, should be the last of the makefile
#
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

# *** EOF ***
