VPATH = /

VBCC = /opt/vbcc
DEVA2560 = ~/dev/bbedit-workspace-a2560
FOENIX = $(DEVA2560)/calypsi-try3/Calypsi-m68k-hello-world/module/Calypsi-m68k-Foenix
TARGET = $(DEVA2560)/_target_foenix

# Common source files
LIB_SRCS = lib_sys.c memory_manager.c theme.c control_template.c font.c window.c control.c general.c bitmap.c text.c list.c event.c
TEST_SRCS = bitmap_test.c font_test.c lib_sys_test.c text_test.c window_test.c general_test.c 
DEMO_SRCS = bitmap_demo.c font_demo.c lib_sys_demo.c text_demo.c window_demo.c

MODEL = --code-model=large --data-model=small
LIB_MODEL = lc-sd

FOENIX_LIB = $(FOENIX)/foenix-$(LIB_MODEL).a
A2560U_RULES = $(FOENIX)/linker-files/a2560u-simplified.scm
A2560K_RULES = $(FOENIX)/linker-files/a2560k-osf.scm

# Object files
OBJS = $(C_SRCS:%.c=obj/%.o)
OBJS_DEBUG = $(C_SRCS:%.c=obj/%-debug.o)
LIB_OBJS = $(LIB_SRCS:%.c=obj/%.o)
TEST_OBJS = $(TEST_SRCS:%.c=obj/%.o)
DEMO_OBJS = $(DEMO_SRCS:%.c=obj/%.o)

obj/%.o: %.c
	cc68k --core=68000 $(MODEL) --target=Foenix --debug -I$(TARGET)/include/ --list-file=$(@:%.o=%.lst) -o $@ $<

obj/%-debug.o: %.c
	cc68k --core=68000 $(MODEL) --debug -I$(TARGET)/include/ --list-file=$(@:%.o=%.lst) -o $@ $<

# make SYS as static lib
#vc +/opt/vbcc/config/a2560-4lib-micah -o a2560_sys.lib lib_sys.c memory_manager.c theme.c control_template.c font.c window.c control.c general.c bitmap.c text.c -lm

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
	cp memory_manager.h $(TARGET)/include/mb/
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
	ln68k -o build_calypsi/bitmap_test.pgz obj/bitmap_test.o $(A2560K_RULES) clib-68000-$(LIB_MODEL)-Foenix.a $(FOENIX)/foenix-lc-sd.a build_calypsi/a2560_sys.a --output-format=pgz --list-file=bitmap_test.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user
	ln68k -o build_calypsi/font_test.pgz   obj/font_test.o   $(A2560K_RULES) clib-68000-$(LIB_MODEL)-Foenix.a $(FOENIX)/foenix-lc-sd.a build_calypsi/a2560_sys.a --output-format=pgz --list-file=font_test.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user
	ln68k -o build_calypsi/general_test.pgz obj/general_test.o $(A2560K_RULES) clib-68000-$(LIB_MODEL)-Foenix.a $(FOENIX)/foenix-lc-sd.a build_calypsi/a2560_sys.a --output-format=pgz --list-file=general_test.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user
	ln68k -o build_calypsi/lib_sys_test.pgz obj/lib_sys_test.o $(A2560K_RULES) clib-68000-$(LIB_MODEL)-Foenix.a $(FOENIX)/foenix-lc-sd.a build_calypsi/a2560_sys.a --output-format=pgz --list-file=lib_sys_test.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user
	ln68k -o build_calypsi/text_test.pgz obj/text_test.o $(A2560K_RULES) clib-68000-$(LIB_MODEL)-Foenix.a $(FOENIX)/foenix-lc-sd.a build_calypsi/a2560_sys.a --output-format=pgz --list-file=text_test.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user
	ln68k -o build_calypsi/window_test.pgz obj/window_test.o $(A2560K_RULES) clib-68000-$(LIB_MODEL)-Foenix.a $(FOENIX)/foenix-lc-sd.a build_calypsi/a2560_sys.a --output-format=pgz --list-file=window_test.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user
	
demos:	$(DEMO_OBJS) $(FOENIX_LIB)
	@echo "Building demos..."
	ln68k -o build_calypsi/bitmap_demo.pgz obj/bitmap_demo.o $(A2560K_RULES) clib-68000-$(LIB_MODEL)-Foenix.a $(FOENIX)/foenix-lc-sd.a build_calypsi/a2560_sys.a --output-format=pgz --list-file=bitmap_demo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user
	ln68k -o build_calypsi/font_demo.pgz obj/font_demo.o $(A2560K_RULES) clib-68000-$(LIB_MODEL)-Foenix.a $(FOENIX)/foenix-lc-sd.a build_calypsi/a2560_sys.a --output-format=pgz --list-file=font_demo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user
	ln68k -o build_calypsi/lib_sys_demo.pgz obj/lib_sys_demo.o $(A2560K_RULES) clib-68000-$(LIB_MODEL)-Foenix.a $(FOENIX)/foenix-lc-sd.a build_calypsi/a2560_sys.a --output-format=pgz --list-file=lib_sys_demo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user
	ln68k -o build_calypsi/text_demo.pgz obj/text_demo.o $(A2560K_RULES) clib-68000-$(LIB_MODEL)-Foenix.a $(FOENIX)/foenix-lc-sd.a build_calypsi/a2560_sys.a --output-format=pgz --list-file=text_demo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user
	ln68k -o build_calypsi/window_demo.pgz obj/window_demo.o $(A2560K_RULES) clib-68000-$(LIB_MODEL)-Foenix.a $(FOENIX)/foenix-lc-sd.a build_calypsi/a2560_sys.a --output-format=pgz --list-file=window_demo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user

	#ln68k -o  build_calypsi/$@ $^ $(A2560K_RULES) clib-68000-$(LIB_MODEL)-Foenix.a build_calypsi/a2560_sys.a --output-format=pgz -l --cross-reference --rtattr printf=float --rtattr cstartup=Foenix_user
	
	
#hello.elf: $(OBJS_DEBUG)
#	ln68k --debug -o $@ $^ $(A2560U_RULES) clib-68000-$(LIB_MODEL).a --list-file=hello-debug.lst --cross-reference --rtattr printf=reduced --semi-hosted --target=Foenix --stack-size=2000 --sstack-size=800

#hello.pgz:  $(OBJS) $(FOENIX_LIB)
#	ln68k -o $@ $^ $(A2560U_RULES) clib-68000-$(LIB_MODEL)-Foenix.a --output-format=pgz --list-file=hello-Foenix.lst --cross-reference --rtattr printf=reduced --rtattr cstartup=Foenix_user

#hello.hex:  $(OBJS) $(FOENIX_LIB)
#	ln68k -o $@ $^ $(A2560K_RULES) clib-68000-$(LIB_MODEL)-Foenix.a --output-format=intel-hex --list-file=hello-Foenix.lst --cross-reference --rtattr printf=reduced --rtattr cstartup=Foenix_morfe --stack-size=2000

$(FOENIX_LIB):
	(cd $(FOENIX) ; make all)

clean:
	-rm $(OBJS) $(OBJS:%.o=%.lst) $(OBJS_DEBUG) $(OBJS_DEBUG:%.o=%.lst) $(FOENIX_LIB)
	-rm -r build_calypsi/*
	-(cd $(FOENIX) ; make clean)
