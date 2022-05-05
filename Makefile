VPATH = /

DEVA2560 = ~/dev/bbedit-workspace-a2560
FOENIX = module/Calypsi-m68k-Foenix
TARGET = $(DEVA2560)/_target_foenix

# Common source files
LIB_SRCS = lib_sys.c theme.c control_template.c font.c window.c control.c general.c bitmap.c text.c list.c event.c mouse.c
TEST_SRCS = bitmap_test.c font_test.c lib_sys_test.c text_test.c window_test.c general_test.c 
DEMO_SRCS = bitmap_demo.c font_demo.c lib_sys_demo.c text_demo.c window_demo.c
TUTORIAL_SRCS = blackjack.c
TEXT_DEMO_SRCS = lib_sys.c theme.c control_template.c font.c window.c control.c general.c bitmap.c text.c list.c event.c mouse.c text_demo.c
SYS_DEMO_SRCS = lib_sys.c theme.c control_template.c font.c window.c control.c general.c bitmap.c text.c list.c event.c mouse.c lib_sys_demo.c

MODEL = --code-model=large --data-model=large
LIB_MODEL = lc-ld

FOENIX_LIB = $(FOENIX)/Foenix-$(LIB_MODEL).a
A2560U_RULES = $(FOENIX)/linker-files/a2560u-simplified.scm
#A2560K_RULES = $(FOENIX)/linker-files/a2560k-simplified.scm
A2560K_RULES = a2560k-osf.scm

# Object files
OBJS = $(C_SRCS:%.c=build_calypsi/obj/%.o)
OBJS_DEBUG = $(C_SRCS:%.c=build_calypsi/obj/%-debug.o)
LIB_OBJS = $(LIB_SRCS:%.c=build_calypsi/obj/%.o)
TEST_OBJS = $(TEST_SRCS:%.c=build_calypsi/obj/%.o)
DEMO_OBJS = $(DEMO_SRCS:%.c=build_calypsi/obj/%.o)
TUTORIAL_OBJS = $(TUTORIAL_SRCS:%.c=build_calypsi/obj/%.o)
TEXT_DEMO_OBJS = $(TEXT_DEMO_SRCS:%.c=build_calypsi/obj/%.o)
SYS_DEMO_OBJS = $(SYS_DEMO_SRCS:%.c=build_calypsi/obj/%.o)

build_calypsi/obj/%.o: %.c
	cc68k -D_A2560K_ -D_f68_ --core=68000 $(MODEL) --debug -I$(TARGET)/include/ --list-file=$(@:%.o=%.lst) -o $@ $<
#	cc68k --core=68000 $(MODEL) --target=Foenix --debug -I$(TARGET)/include/ --list-file=$(@:%.o=%.lst) -o $@ $<

build_calypsi/obj/%-debug.o: %.c
	cc68k -D_A2560K_ -D_f68_ --core=68000 $(MODEL) --debug -I$(TARGET)/include/ --list-file=$(@:%.o=%.lst) -o $@ $<

# make SYS as static lib
#vc +/opt/vbcc/config/a2560-4lib-micah -o a2560_sys.lib lib_sys.c theme.c control_template.c font.c window.c control.c general.c bitmap.c text.c -lm

all: headers lib tests demos

headers:
	@echo "Copying headers to target..."
	cp a2560_platform.h $(TARGET)/include/mb/
	cp bitmap.h $(TARGET)/include/mb/
	cp control_template.h $(TARGET)/include/mb/
	cp control.h $(TARGET)/include/mb/
	cp event.h $(TARGET)/include/mb/
	cp font.h $(TARGET)/include/mb/
	cp general.h $(TARGET)/include/mb/
	cp lib_sys.h $(TARGET)/include/mb/
	cp list.h $(TARGET)/include/mb/
	cp mouse.h $(TARGET)/include/mb/
	cp text.h $(TARGET)/include/mb/
	cp theme.h $(TARGET)/include/mb/
	cp window.h $(TARGET)/include/mb/

lib:	$(LIB_OBJS) $(FOENIX_LIB)
	@echo "Building library..."
#	ln68k -o $@ $^ $(A2560K_RULES) clib-68000-$(LIB_MODEL)-Foenix.a --output-format=raw --list-file=a2560_sys.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user
	nlib build_calypsi/a2560_sys.a $(LIB_OBJS)

tests:	$(TEST_OBJS) $(FOENIX_LIB)
	@echo "Building tests..."
	#ln68k -o build_calypsi/$@ $^ $(A2560K_RULES) clib-68000-$(LIB_MODEL)-Foenix.a build_calypsi/a2560_sys.a --output-format=pgz -l --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user
	ln68k -o build_calypsi/bitmap_test.pgz build_calypsi/obj/bitmap_test.o $(A2560K_RULES) clib-68000-$(LIB_MODEL)-Foenix.a $(FOENIX)/foenix-lc-sd.a build_calypsi/a2560_sys.a --output-format=pgz --list-file=build_calypsi/bitmap_test.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user
	ln68k -o build_calypsi/font_test.pgz   build_calypsi/obj/font_test.o   $(A2560K_RULES) clib-68000-$(LIB_MODEL)-Foenix.a $(FOENIX)/foenix-lc-sd.a build_calypsi/a2560_sys.a --output-format=pgz --list-file=build_calypsi/font_test.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user
	ln68k -o build_calypsi/general_test.pgz build_calypsi/obj/general_test.o $(A2560K_RULES) clib-68000-$(LIB_MODEL)-Foenix.a $(FOENIX)/foenix-lc-sd.a build_calypsi/a2560_sys.a --output-format=pgz --list-file=build_calypsi/general_test.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user
	ln68k -o build_calypsi/lib_sys_test.pgz build_calypsi/obj/lib_sys_test.o $(A2560K_RULES) clib-68000-$(LIB_MODEL)-Foenix.a $(FOENIX)/foenix-lc-sd.a build_calypsi/a2560_sys.a --output-format=pgz --list-file=build_calypsi/lib_sys_test.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user
	ln68k -o build_calypsi/text_test.pgz build_calypsi/obj/text_test.o $(A2560K_RULES) clib-68000-$(LIB_MODEL)-Foenix.a $(FOENIX)/foenix-lc-sd.a build_calypsi/a2560_sys.a --output-format=pgz --list-file=build_calypsi/text_test.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user
	ln68k -o build_calypsi/window_test.pgz build_calypsi/obj/window_test.o $(A2560K_RULES) clib-68000-$(LIB_MODEL)-Foenix.a $(FOENIX)/foenix-lc-sd.a build_calypsi/a2560_sys.a --output-format=pgz --list-file=build_calypsi/window_test.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user
	
demos:	$(DEMO_OBJS) $(FOENIX_LIB)
	@echo "Building demos..."
	ln68k -o build_calypsi/bitmap_demo.pgz build_calypsi/obj/bitmap_demo.o $(A2560K_RULES) clib-68000-$(LIB_MODEL)-Foenix.a $(FOENIX)/foenix-lc-sd.a build_calypsi/a2560_sys.a --output-format=pgz --list-file=build_calypsi/bitmap_demo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user
	ln68k -o build_calypsi/font_demo.pgz build_calypsi/obj/font_demo.o $(A2560K_RULES) clib-68000-$(LIB_MODEL)-Foenix.a $(FOENIX)/foenix-lc-sd.a build_calypsi/a2560_sys.a --output-format=pgz --list-file=build_calypsi/font_demo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user
	ln68k -o build_calypsi/lib_sys_demo.pgz build_calypsi/obj/lib_sys_demo.o $(A2560K_RULES) clib-68000-$(LIB_MODEL)-Foenix.a $(FOENIX)/foenix-lc-sd.a build_calypsi/a2560_sys.a --hosted --output-format=pgz --list-file=build_calypsi/lib_sys_demo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user
	ln68k -o build_calypsi/text_demo.pgz build_calypsi/obj/text_demo.o $(A2560K_RULES) clib-68000-$(LIB_MODEL)-Foenix.a $(FOENIX)/foenix-lc-sd.a build_calypsi/a2560_sys.a --output-format=pgz --list-file=build_calypsi/text_demo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user
	ln68k -o build_calypsi/window_demo.pgz build_calypsi/obj/window_demo.o $(A2560K_RULES) clib-68000-$(LIB_MODEL)-Foenix.a $(FOENIX)/foenix-lc-sd.a build_calypsi/a2560_sys.a --output-format=pgz --list-file=build_calypsi/window_demo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user

tutorials:	$(TUTORIAL_OBJS) $(FOENIX_LIB)
	@echo "Building tutorials..."
	ln68k -o build_calypsi/blackjack.pgz build_calypsi/obj/blackjack.o $(A2560K_RULES) clib-68000-$(LIB_MODEL)-Foenix.a $(FOENIX)/foenix-lc-sd.a build_calypsi/a2560_sys.a --output-format=pgz --list-file=build_calypsi/blackjack.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user

	#ln68k -o  build_calypsi/$@ $^ $(A2560K_RULES) clib-68000-$(LIB_MODEL)-Foenix.a build_calypsi/a2560_sys.a --output-format=pgz -l --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user
	
textdemo: headers $(TEXT_DEMO_OBJS) $(FOENIX_LIB)
	@echo "Building text demo..."
#	echo $@
	ln68k -o build_calypsi/text.pgz $(TEXT_DEMO_OBJS) $(A2560K_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/Foenix-lc-ld.a --output-format=pgz --list-file=build_calypsi/textdemo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user

sysdemo: headers $(SYS_DEMO_OBJS) $(FOENIX_LIB)
	@echo "Building sys demo..."
#	echo $@
#	ln68k -o build_calypsi/sysdemo.elf $(SYS_DEMO_OBJS) $(A2560K_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/Foenix-lc-ld.a --list-file=build_calypsi/sysdemo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug
	ln68k -o build_calypsi/sysdemo.pgz $(SYS_DEMO_OBJS) $(A2560K_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/Foenix-lc-ld.a --output-format=pgz --list-file=build_calypsi/sysdemo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user
	

$(FOENIX_LIB):
	(cd $(FOENIX) ; make all)

clean:
	-rm $(SYS_DEMO_OBJS) obj/*.lst sysdemo.pgz sysdemo.elf
	-rm $(TEXT_DEMO_OBJS) obj/*.lst text.pgz text.elf
	-rm $(OBJS) $(OBJS:%.o=%.lst) $(OBJS_DEBUG) $(OBJS_DEBUG:%.o=%.lst) $(FOENIX_LIB)
	-rm build_calypsi/*
	-rm build_calypsi/obj/*
	-(cd $(FOENIX) ; make clean)
