VPATH = ../

FOENIX = ../module/Calypsi-65816-Foenix
#TARGET = $(DEVA2560)/_target_foenix
TARGET = ../config_c256

LIB_SRCS = lib_sys.c theme.c control_template.c font.c window.c control.c general.c bitmap.c text.c list.c event.c mouse.c menu.c
TEXT_DEMO_SRCS = lib_sys.c theme.c control_template.c font.c window.c control.c general.c bitmap.c text.c list.c event.c mouse.c menu.c text_demo.c
BITMAP_DEMO_SRCS = bitmap_demo.c
FONT_DEMO_SRCS = font_demo.c

MODEL = --code-model=large --data-model=medium
LIB_MODEL = lc-md

FOENIX_LIB = $(FOENIX)/foenix-$(LIB_MODEL).a
FOENIX_LINKER_RULES = $(TARGET)/c256_osf.scm

LIB_OBJS = $(LIB_SRCS:%.c=obj/%.o)
BITMAP_DEMO_OBJS = $(BITMAP_DEMO_SRCS:%.c=obj/%.o)
FONT_DEMO_OBJS = $(FONT_DEMO_SRCS:%.c=obj/%.o)
TEXT_DEMO_OBJS = $(TEXT_DEMO_SRCS:%.c=obj/%.o)

obj/%.o: %.c
	cc65816 -D_C256_FMX_ --core=65816 $(MODEL) --target=Foenix -I$(TARGET)/include/ --list-file=$(@:%.o=%.lst) -o $@ $<

#lib_sys.c:
#	cc65816 -D_C256_FMX_ --core=65816 $(MODEL) --target=Foenix --debug --list-file=lib_sys.lst -o obj/lib_sys.o ../lib_sys.c

all: headers lib textdemo bitmapdemo fontdemo

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
	nlib c256fmx_sys.a $(LIB_OBJS)
	cp c256fmx_sys.a $(TARGET)/lib/

textdemo: $(TEXT_DEMO_OBJS) $(FOENIX_LIB)
	@echo "Building text demo..."
	ln65816 -o text.hex $(TEXT_DEMO_OBJS) $(FOENIX_LINKER_RULES) clib-$(LIB_MODEL)-foenix.a $(FOENIX)/foenix-lc-md.a --list-file=obj/textdemo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix

bitmapdemo: $(BITMAP_DEMO_OBJS) $(FOENIX_LIB)
	@echo "Building bitmap demo..."
	ln65816 -o bitmap.pgz $(BITMAP_DEMO_OBJS) $(FOENIX_LINKER_RULES) clib-$(LIB_MODEL)-foenix.a $(FOENIX)/foenix-lc-md.a $(TARGET)/lib/c256fmx_sys.a --output-format=pgz --list-file=obj/bitmap_demo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix
	
fontdemo: $(FONT_DEMO_OBJS) $(FOENIX_LIB)
	@echo "Building font demo..."
	ln65816 -o font.pgz $(FONT_DEMO_OBJS) $(FOENIX_LINKER_RULES) clib-$(LIB_MODEL)-foenix.a $(FOENIX)/foenix-lc-md.a $(TARGET)/lib/c256fmx_sys.a --output-format=pgz --list-file=obj/font_demo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix

$(FOENIX_LIB):
	(cd $(FOENIX) ; make all)

clean:
	-rm $(TEXT_DEMO_OBJS) obj/*.lst text.pgz
	-rm $(BITMAP_DEMO_OBJS) obj/*.lst bitmap.pgz
	-rm $(FONT_DEMO_OBJS) obj/*.lst font.pgz
	-rm *.pgz
	-rm *.elf
	
	
	
