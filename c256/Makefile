VPATH = ../

FOENIX = ../module/Calypsi-65816-Foenix
#FOENIX = /Users/micahbly/dev/bbedit-workspace-a2560/frOS/module/Calypsi-65816-Foenix
TARGET = $(DEVA2560)/_target_foenix

TEXT_DEMO_SRCS = lib_sys.c memory_manager.c theme.c control_template.c font.c window.c control.c general.c bitmap.c text.c list.c event.c text_demo.c

MODEL = --code-model=large --data-model=medium
LIB_MODEL = lc-md

FOENIX_LIB = $(FOENIX)/Foenix-$(LIB_MODEL).a
FOENIX_LINKER_RULES = osf_Foenix-FMX.scm

TEXT_DEMO_OBJS = $(TEXT_DEMO_SRCS:%.c=obj/%.o)

obj/%.o: %.c
	cc65816 --core=65816 $(MODEL) --target=Foenix --debug -I$(TARGET)/include/ --list-file=$(@:%.o=%.lst) -o $@ $<

#lib_sys.c:
#	cc65816 --core=65816 $(MODEL) --target=Foenix --debug --list-file=lib_sys.lst -o obj/lib_sys.o ../lib_sys.c

all: headers textdemo

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
	cp ../memory_manager.h $(TARGET)/include/mb/
	cp ../text.h $(TARGET)/include/mb/
	cp ../theme.h $(TARGET)/include/mb/
	cp ../window.h $(TARGET)/include/mb/

textdemo: $(TEXT_DEMO_OBJS) $(FOENIX_LIB)
	@echo "Building text demo..."
#	echo $@
	ln65816 -o text.pgz $(TEXT_DEMO_OBJS) $(FOENIX_LINKER_RULES) clib-$(LIB_MODEL)-Foenix.a $(FOENIX)/Foenix-lc-md.a --output-format=pgz --list-file=textdemo.lst --cross-reference --rtattr printf=float --rtattr cstartup=Foenix

$(FOENIX_LIB):
	(cd $(FOENIX) ; make all)

clean:
	-rm $(TEXT_DEMO_OBJS) obj/*.lst text.pgz
	