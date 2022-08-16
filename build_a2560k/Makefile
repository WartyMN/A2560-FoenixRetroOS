VPATH = /

DEVA2560 = ~/dev/bbedit-workspace-a2560
FOENIX = module/Calypsi-m68k-Foenix
#TARGET = $(DEVA2560)/_target_foenix
TARGET = ../config_a2560k

# Common source files
LIB_SRCS = lib_sys.c theme.c control_template.c font.c window.c control.c general.c bitmap.c text.c list.c event.c mouse.c menu.c
TEST_SRCS = bitmap_test.c font_test.c lib_sys_test.c text_test.c window_test.c general_test.c 
DEMO_SRCS = bitmap_demo.c font_demo.c lib_sys_demo.c text_demo.c window_demo.c
TUTORIAL_SRCS = blackjack.c
TEXT_DEMO_SRCS = lib_sys.c theme.c control_template.c font.c window.c control.c general.c bitmap.c text.c list.c event.c mouse.c menu.c text_demo.c
SYS_DEMO_SRCS = lib_sys.c theme.c control_template.c font.c window.c control.c general.c bitmap.c text.c list.c event.c mouse.c menu.c lib_sys_demo.c
BITMAP_DEMO_SRCS = bitmap_demo.c

MODEL = --code-model=large --data-model=large
LIB_MODEL = lc-ld

FOENIX_LIB = $(FOENIX)/foenix-$(LIB_MODEL).a
A2560U_RULES = $(FOENIX)/linker-files/a2560u-simplified.scm
#A2560K_RULES = $(FOENIX)/linker-files/a2560k-simplified.scm
A2560K_RULES = $(TARGET)/a2560k-osf.scm

# Object files
OBJS = $(C_SRCS:%.c=build_a2560k/obj/%.o)
OBJS_DEBUG = $(C_SRCS:%.c=build_a2560k/obj/%-debug.o)
LIB_OBJS = $(LIB_SRCS:%.c=build_a2560k/obj/%.o)
TEST_OBJS = $(TEST_SRCS:%.c=build_a2560k/obj/%.o)
DEMO_OBJS = $(DEMO_SRCS:%.c=build_a2560k/obj/%.o)
TUTORIAL_OBJS = $(TUTORIAL_SRCS:%.c=build_a2560k/obj/%.o)
TEXT_DEMO_OBJS = $(TEXT_DEMO_SRCS:%.c=build_a2560k/obj/%.o)
SYS_DEMO_OBJS = $(SYS_DEMO_SRCS:%.c=build_a2560k/obj/%.o)
BITMAP_DEMO_OBJS = $(BITMAP_DEMO_SRCS:%.c=build_a2560k/obj/%.o)

build_a2560k/obj/%.o: %.c
	cc68k -D_A2560K_ -D_f68_ --core=68000 $(MODEL) --debug -I$(TARGET)/include/ --list-file=$(@:%.o=%.lst) -o $@ $<
#	cc68k --core=68000 $(MODEL) --target=Foenix --debug -I$(TARGET)/include/ --list-file=$(@:%.o=%.lst) -o $@ $<

build_a2560k/obj/%-debug.o: %.c
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
	cp menu.h $(TARGET)/include/mb/
	cp mouse.h $(TARGET)/include/mb/
	cp text.h $(TARGET)/include/mb/
	cp theme.h $(TARGET)/include/mb/
	cp window.h $(TARGET)/include/mb/

lib:	$(LIB_OBJS) $(FOENIX_LIB)
	@echo "Building library..."
#	ln68k -o $@ $^ $(A2560K_RULES) clib-68000-$(LIB_MODEL)-foenix.a --output-format=raw --list-file=a2560_sys.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user
	nlib config_a2560k/lib/a2560_sys.a $(LIB_OBJS)

tests:	$(TEST_OBJS) $(FOENIX_LIB)
	@echo "Building tests..."
	#ln68k -o build_a2560k/$@ $^ $(A2560K_RULES) clib-68000-$(LIB_MODEL)-foenix.a config_a2560k/lib/a2560_sys.a --output-format=pgz -l --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user
	ln68k -o build_a2560k/bitmap_test.elf build_a2560k/obj/bitmap_test.o $(A2560K_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/Foenix-lc-ld.a config_a2560k/lib/a2560_sys.a --output-format=pgz --list-file=build_a2560k/obj/bitmap_test.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug
	ln68k -o build_a2560k/font_test.elf   build_a2560k/obj/font_test.o   $(A2560K_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/foenix-lc-ld.a config_a2560k/lib/a2560_sys.a --output-format=pgz --list-file=build_a2560k/obj/font_test.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug
	ln68k -o build_a2560k/general_test.elf build_a2560k/obj/general_test.o $(A2560K_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/foenix-lc-ld.a config_a2560k/lib/a2560_sys.a --output-format=pgz --list-file=build_a2560k/obj/general_test.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug
	ln68k -o build_a2560k/lib_sys_test.elf build_a2560k/obj/lib_sys_test.o $(A2560K_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/foenix-lc-ld.a config_a2560k/lib/a2560_sys.a --output-format=pgz --list-file=build_a2560k/obj/lib_sys_test.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug
	ln68k -o build_a2560k/text_test.elf build_a2560k/obj/text_test.o $(A2560K_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/foenix-lc-ld.a config_a2560k/lib/a2560_sys.a --output-format=pgz --list-file=build_a2560k/obj/text_test.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug
	ln68k -o build_a2560k/window_test.elf build_a2560k/obj/window_test.o $(A2560K_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/foenix-lc-ld.a config_a2560k/lib/a2560_sys.a --output-format=pgz --list-file=build_a2560k/obj/window_test.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug
	
demos:	$(DEMO_OBJS) $(FOENIX_LIB)
	@echo "Building demos..."
	ln68k -o build_a2560k/bitmap_demo.elf build_a2560k/obj/bitmap_demo.o $(A2560K_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/foenix-lc-ld.a config_a2560k/lib/a2560_sys.a --output-format=pgz --list-file=build_a2560k/obj/bitmap_demo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug
	ln68k -o build_a2560k/font_demo.elf build_a2560k/obj/font_demo.o $(A2560K_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/foenix-lc-ld.a config_a2560k/lib/a2560_sys.a --output-format=pgz --list-file=build_a2560k/obj/font_demo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug
	ln68k -o build_a2560k/sys_demo.elf build_a2560k/obj/lib_sys_demo.o $(A2560K_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/foenix-lc-ld.a config_a2560k/lib/a2560_sys.a --hosted --output-format=pgz --list-file=build_a2560k/obj/lib_sys_demo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug
	ln68k -o build_a2560k/text_demo.elf build_a2560k/obj/text_demo.o $(A2560K_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/foenix-lc-ld.a config_a2560k/lib/a2560_sys.a --output-format=pgz --list-file=build_a2560k/obj/text_demo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug
	ln68k -o build_a2560k/window_demo.elf build_a2560k/obj/window_demo.o $(A2560K_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/foenix-lc-ld.a config_a2560k/lib/a2560_sys.a --output-format=pgz --list-file=build_a2560k/obj/window_demo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug

tutorials:	$(TUTORIAL_OBJS) $(FOENIX_LIB)
	@echo "Building tutorials..."
	ln68k -o build_a2560k/blackjack.elf build_a2560k/obj/blackjack.o $(A2560K_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/foenix-lc-ld.a config_a2560k/lib/a2560_sys.a --output-format=pgz --list-file=build_a2560k/obj/blackjack.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug

	
textdemo: headers $(TEXT_DEMO_OBJS) $(FOENIX_LIB)
	@echo "Building text demo..."
	ln68k -o build_a2560k/text.elf $(TEXT_DEMO_OBJS) $(A2560K_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/foenix-lc-ld.a --output-format=pgz --list-file=build_a2560k/obj/textdemo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug


sysdemo: headers $(SYS_DEMO_OBJS) $(FOENIX_LIB)
	@echo "Building sys demo..."
	ln68k -o build_a2560k/sysdemo.elf $(SYS_DEMO_OBJS) $(A2560K_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/foenix-lc-ld.a --output-format=pgz --list-file=build_a2560k/obj/sysdemo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug
	

bitmapdemo: headers $(BITMAP_DEMO_OBJS) $(FOENIX_LIB)
	@echo "Building bitmap demo..."
	ln68k -o build_a2560k/bitmap_demo.elf $(BITMAP_DEMO_OBJS) $(A2560K_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/foenix-lc-ld.a --output-format=pgz --list-file=build_a2560k/obj/bitmap_demo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug
	

$(FOENIX_LIB):
	(cd $(FOENIX) ; make all)

clean:
	-rm $(SYS_DEMO_OBJS) build_a2560k/obj/*.lst build_a2560k/obj/*.o
	-rm $(TEXT_DEMO_OBJS) build_a2560k/obj/*.lst build_a2560k/obj/*.o
	-rm $(OBJS) $(OBJS:%.o=%.lst) $(OBJS_DEBUG) $(OBJS_DEBUG:%.o=%.lst) $(FOENIX_LIB)
	-rm build_a2560k/*.pgz
	-rm build_a2560k/*.elf
	-(cd $(FOENIX) ; make clean)
