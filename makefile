#最後更改時間
#日期: 2020/10/17
#時間: 02:25

#for ASA板

Path    = D:\AVR\C4MLIB_3.7.0

INC     =  -I "$(Path)"
LIB     = "$(Path)\libc4m.a"

CFLAGS    = -mmcu=atmega128
CFLAGS   += -DF_CPU=11059200UL
CFLAGS   += -Wall -gdwarf-2 -std=gnu99 -Os -lm 
CFLAGS   += -funsigned-char -funsigned-bitfields
CFLAGS   += -fpack-struct -fshort-enums 
CFLAGS   += -lprintf_flt -lscanf_flt
CFLAGS   += -Wl,-u,vfprintf -Wl,-u,vfscanf
CFLAGS   += -Wl,-u,USE_C4MLIB_INTERRUPT
CFLAGS   += -Wl,-u,USE_C4MLIB_STD_ISR

HEXFLAGS  = -R .eeprom -R .fuse -R .lock -R .signature

PORT    = 8

o2elf   = avr-gcc $< $(CFLAGS) $(LIB) -o $@
c2o     = avr-gcc -c $< $(CFLAGS) $(INC) -o  $@

Red     = \e[1;31m
Yellow  = \e[1;33m

Error   = error[^\n]*
Warning = warning[^\n]*

R_Replace   = \\$(Red)&\\\e[0m
Y_Replace   = \\$(Yellow)&\\\e[0m

error_msg = -e 's|$(Error)|$(R_Replace)|gI'
warning_msg = -e 's|$(Warning)|$(Y_Replace)|gI'
delete_msg = -e 's|\[[^\n]*||g'

o_msg   = $$($(o2elf) 2>&1 | sed $(delete_msg) $(error_msg) $(warning_msg))
c_msg   = $$($(c2o) 2>&1 | sed $(delete_msg) $(error_msg) $(warning_msg))

main.elf: main.o 
	@ echo -e "$(o_msg)"
	@ avr-objcopy -O ihex  $(HEXFLAGS) "main.elf" "main.hex"
	@ rm -f *.o  *.elf
	@ echo Complete file!
	@ asaloader prog -p COM$(PORT) -f main.hex
	@ terminal -p $(PORT) -b 38400

main.o:main.c
	@ echo -e "$(c_msg)"

.PHONY: cls load term

cls:
	@ rm -f *.o  *.elf *.hex
	@ echo Complete clean!

load:
	@ asaloader prog -p COM$(PORT) -f main.hex
	
term:
	@ terminal -p $(PORT) -b 38400