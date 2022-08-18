VPATH = ../

FOENIX = ../module/Calypsi-m68k-Foenix
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
#FOENIX_LINKER_RULES = $(FOENIX)/linker-files/a2560u-simplified.scm
#FOENIX_LINKER_RULES = $(FOENIX)/linker-files/a2560k-simplified.scm
FOENIX_LINKER_RULES = $(TARGET)/a2560k-osf.scm

# Object files
OBJS = $(C_SRCS:%.c=obj/%.o)
OBJS_DEBUG = $(C_SRCS:%.c=obj/%-debug.o)
LIB_OBJS = $(LIB_SRCS:%.c=obj/%.o)
TEST_OBJS = $(TEST_SRCS:%.c=obj/%.o)
DEMO_OBJS = $(DEMO_SRCS:%.c=obj/%.o)
TUTORIAL_OBJS = $(TUTORIAL_SRCS:%.c=obj/%.o)
TEXT_DEMO_OBJS = $(TEXT_DEMO_SRCS:%.c=obj/%.o)
SYS_DEMO_OBJS = $(SYS_DEMO_SRCS:%.c=obj/%.o)
BITMAP_DEMO_OBJS = $(BITMAP_DEMO_SRCS:%.c=obj/%.o)

obj/%.o: %.c
	cc68k -D_A2560K_ -D_f68_ --core=68000 $(MODEL) --debug -I$(TARGET)/include/ --list-file=$(@:%.o=%.lst) -o $@ $<
#	cc68k --core=68000 $(MODEL) --target=Foenix --debug -I$(TARGET)/include/ --list-file=$(@:%.o=%.lst) -o $@ $<

obj/%-debug.o: %.c
	cc68k -D_A2560K_ -D_f68_ --core=68000 $(MODEL) --debug -I$(TARGET)/include/ --list-file=$(@:%.o=%.lst) -o $@ $<

# make SYS as static lib
#vc +/opt/vbcc/config/a2560-4lib-micah -o a2560_sys.lib lib_sys.c theme.c control_template.c font.c window.c control.c general.c bitmap.c text.c -lm

all: headers lib tests demos

headers:
	@echo "Copying headers to target..."
	cp ../a2560_platform.h $(TARGET)/include/mb/
	cp ../bitmap.h $(TARGET)/include/mb/
	cp ../control_template.h $(TARGET)/include/mb/
	cp ../control.h $(TARGET)/include/mb/
	cp ../event.h $(TARGET)/include/mb/
	cp ../font.h $(TARGET)/include/mb/
	cp ../general.h $(TARGET)/include/mb/
	cp ../lib_sys.h $(TARGET)/include/mb/
	cp ../list.h $(TARGET)/include/mb/
	cp ../menu.h $(TARGET)/include/mb/
	cp ../mouse.h $(TARGET)/include/mb/
	cp ../text.h $(TARGET)/include/mb/
	cp ../theme.h $(TARGET)/include/mb/
	cp ../window.h $(TARGET)/include/mb/

lib:	$(LIB_OBJS) $(FOENIX_LIB)
	@echo "Building library..."
#	ln68k -o $@ $^ $(FOENIX_LINKER_RULES) clib-68000-$(LIB_MODEL)-foenix.a --output-format=raw --list-file=a2560_sys.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user
	nlib $(TARGET)/lib/a2560_sys.a $(LIB_OBJS)

tests:	$(TEST_OBJS) $(FOENIX_LIB)
	@echo "Building tests..."
	#ln68k -o $@ $^ $(FOENIX_LINKER_RULES) clib-68000-$(LIB_MODEL)-foenix.a config_a2560k/lib/a2560_sys.a --output-format=pgz -l --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user
	ln68k -o bitmap_test.elf obj/bitmap_test.o $(FOENIX_LINKER_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/Foenix-lc-ld.a $(TARGET)/lib/a2560_sys.a --output-format=pgz --list-file=obj/bitmap_test.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug
	ln68k -o font_test.elf   obj/font_test.o   $(FOENIX_LINKER_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/foenix-lc-ld.a $(TARGET)/lib/a2560_sys.a --output-format=pgz --list-file=obj/font_test.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug
	ln68k -o general_test.elf obj/general_test.o $(FOENIX_LINKER_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/foenix-lc-ld.a $(TARGET)/lib/a2560_sys.a --output-format=pgz --list-file=obj/general_test.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug
	ln68k -o lib_sys_test.elf obj/lib_sys_test.o $(FOENIX_LINKER_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/foenix-lc-ld.a $(TARGET)/lib/a2560_sys.a --output-format=pgz --list-file=obj/lib_sys_test.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug
	ln68k -o text_test.elf obj/text_test.o $(FOENIX_LINKER_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/foenix-lc-ld.a $(TARGET)/lib/a2560_sys.a --output-format=pgz --list-file=obj/text_test.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug
	ln68k -o window_test.elf obj/window_test.o $(FOENIX_LINKER_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/foenix-lc-ld.a $(TARGET)/lib/a2560_sys.a --output-format=pgz --list-file=obj/window_test.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug
	
demos:	$(DEMO_OBJS) $(FOENIX_LIB)
	@echo "Building demos..."
	ln68k -o bitmap_demo.elf obj/bitmap_demo.o $(FOENIX_LINKER_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/foenix-lc-ld.a $(TARGET)/lib/a2560_sys.a --output-format=pgz --list-file=obj/bitmap_demo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug
	ln68k -o font_demo.elf obj/font_demo.o $(FOENIX_LINKER_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/foenix-lc-ld.a $(TARGET)/lib/a2560_sys.a --output-format=pgz --list-file=obj/font_demo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug
	ln68k -o sys_demo.elf obj/lib_sys_demo.o $(FOENIX_LINKER_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/foenix-lc-ld.a $(TARGET)/lib/a2560_sys.a --hosted --output-format=pgz --list-file=obj/lib_sys_demo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug
	ln68k -o text_demo.elf obj/text_demo.o $(FOENIX_LINKER_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/foenix-lc-ld.a $(TARGET)/lib/a2560_sys.a --output-format=pgz --list-file=obj/text_demo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug
	ln68k -o window_demo.elf obj/window_demo.o $(FOENIX_LINKER_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/foenix-lc-ld.a $(TARGET)/lib/a2560_sys.a --output-format=pgz --list-file=obj/window_demo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug

tutorials:	$(TUTORIAL_OBJS) $(FOENIX_LIB)
	@echo "Building tutorials..."
	ln68k -o blackjack.elf obj/blackjack.o $(FOENIX_LINKER_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/foenix-lc-ld.a $(TARGET)/lib/a2560_sys.a --output-format=pgz --list-file=obj/blackjack.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug

	
textdemo: headers $(TEXT_DEMO_OBJS) $(FOENIX_LIB)
	@echo "Building text demo..."
	ln68k -o text.elf $(TEXT_DEMO_OBJS) $(FOENIX_LINKER_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/foenix-lc-ld.a --output-format=pgz --list-file=obj/textdemo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug


sysdemo: headers $(SYS_DEMO_OBJS) $(FOENIX_LIB)
	@echo "Building sys demo..."
	ln68k -o sysdemo.elf $(SYS_DEMO_OBJS) $(FOENIX_LINKER_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/foenix-lc-ld.a --output-format=pgz --list-file=obj/sysdemo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug
	

bitmapdemo: headers $(BITMAP_DEMO_OBJS) $(FOENIX_LIB)
	@echo "Building bitmap demo..."
	ln68k -o bitmap_demo.elf $(BITMAP_DEMO_OBJS) $(FOENIX_LINKER_RULES) clib-68000-$(LIB_MODEL).a $(FOENIX)/foenix-lc-ld.a --output-format=pgz --list-file=obj/bitmap_demo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user --debug
	

$(FOENIX_LIB):
	(cd $(FOENIX) ; make all)

clean:
	-rm $(SYS_DEMO_OBJS) obj/*.lst obj/*.o
	-rm $(TEXT_DEMO_OBJS) obj/*.lst obj/*.o
	-rm $(OBJS) $(OBJS:%.o=%.lst) $(OBJS_DEBUG) $(OBJS_DEBUG:%.o=%.lst) $(FOENIX_LIB)
	-rm build_a2560k/*.pgz
	-rm build_a2560k/*.elf
	-(cd $(FOENIX) ; make clean)
