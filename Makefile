VERBOSE=yes
ifneq (yes,$(VERBOSE))
    Q=@
else
    Q=
endif

CROSS=Y

CROSSBINDIR_IS_Y=m68k-atari-mint-
CROSSBINDIR_IS_N=

CROSSBINDIR=$(CROSSBINDIR_IS_$(CROSS))

UNAME := $(shell uname)
ifeq ($(CROSS), Y)
ifeq ($(UNAME),Linux)
PREFIX=m68k-atari-mint
HATARI=hatari
else
PREFIX=m68k-atari-mint
HATARI=/usr/bin/hatari
endif
else
PREFIX=/usr
endif

DEPEND=depend

LIBCMINI=../libcmini
LIBCMINI_LIB=$(LIBCMINI)/build

#INCLUDE=
#LIBS=-lgem -lm -lcmini -lstdc++ -lcmini -lgcc
LIBS=-lgem -lm
CC=$(CROSSBINDIR)gcc
CPP=$(CROSSBINDIR)g++
STRIP=$(CROSSBINDIR)strip
STACK=$(CROSSBINDIR)stack
NATIVECC=gcc

APP=osview.app
TEST_APP=$(APP)

# CHARSET_FLAGS= -finput-charset=ISO-8859-1 \
#               -fexec-charset=ATARIST

CFLAGS= \
	-O2 \
	-g \
	-DFORCE_GEMLIB_UDEF \
	-Wl,-Map,mapfile \
	-Wall \
	$(CHARSET_FLAGS)

SRCDIR=sources
INCDIR=include
INCLUDE+=-I/usr/m68k-atari-mint/include/c++/4.6.4/m68k-atari-mint/ -I$(INCDIR)

CCSRCS=\
	$(SRCDIR)/osview.cc \
	$(SRCDIR)/global.cc \
	$(SRCDIR)/window.cc \
	$(SRCDIR)/statwindow.cc \
	$(SRCDIR)/util.cc \
	$(SRCDIR)/meter.cc \
	$(SRCDIR)/fieldmeter.cc \
	$(SRCDIR)/fieldmeterdecay.cc \
	$(SRCDIR)/os_stat.cc

CSRCS = \
	# $(SRCDIR)/natfeats.c


ASRCS=\
	#$(SRCDIR)/nkcc-gc.S \
	#$(SRCDIR)/nf_asm.S

CCOBJS=$(patsubst $(SRCDIR)/%.o,%.o,$(patsubst %.cc,%.o,$(CCSRCS)))
COBJS=$(patsubst $(SRCDIR)/%.o,%.o,$(patsubst %.c,%.o,$(CSRCS)))
AOBJS=$(patsubst $(SRCDIR)/%.o,%.o,$(patsubst %.S,%.o,$(ASRCS)))
OBJS=$(CCOBJS) $(COBJS) $(AOBJS)

TRGTDIRS=. ./m68020-60 ./m5475
OBJDIRS=$(patsubst %,%/objs,$(TRGTDIRS))

#
# multilib flags. These must match m68k-atari-mint-gcc -print-multi-lib output
#
m68020-60/$(APP):CFLAGS += -m68020-60
m5475/$(APP):CFLAGS += -mcpu=5475

all: $(patsubst %,%/$(APP),$(TRGTDIRS))

$(DEPEND): $(ASRCS) $(CSRCS) $(CCSRCS)
	-rm -f $(DEPEND)
	$(Q)for d in $(TRGTDIRS);\
		do $(CC) $(CFLAGS) $(INCLUDE) -M $(ASRCS) $(CCSRCS) | sed -e "s#^\(.*\).o:#$$d/objs/\1.o:#" >> $(DEPEND); \
	done

#
# generate pattern rules for multilib object files.
#
define CC_TEMPLATE
$(1)/objs/%.o:$(SRCDIR)/%.cc
	$(Q)echo "CPP $$<"
	$(Q)$(CPP) $$(CFLAGS) $(INCLUDE) -c $$< -o $$@

$(1)/objs/%.o:$(SRCDIR)/%.c
	$(Q)echo "CC $$<"
	$(Q)$(CC) $$(CFLAGS) $(INCLUDE) -c $$< -o $$@

$(1)/objs/%.o:$(SRCDIR)/%.S
	$(Q)echo "CC $$<"
	$(Q)$(CC) $$(CFLAGS) $(INCLUDE) -c $$< -o $$@

$(1)_OBJS=$(patsubst %,$(1)/objs/%,$(OBJS))
$(1)/$(APP): $$($(1)_OBJS)
	$(Q)echo "CPP $$<"
	$(Q)$(CPP) $$(CFLAGS) -o $$@ $$($(1)_OBJS) $(LIBS)
	#$(Q)$(STRIP) $$@
endef
$(foreach DIR,$(TRGTDIRS),$(eval $(call CC_TEMPLATE,$(DIR))))
# $(Q)$(CPP) -o $$@ -nostdlib $(LIBCMINI_LIB)/crt0.o -L$$(LIBCMINI_LIB)/$(1) $$($(1)_OBJS) $(LIBS)

clean:
	@rm -f $(patsubst %,%/objs/*.o,$(TRGTDIRS)) $(patsubst %,%/$(APP),$(TRGTDIRS))
	@rm -f $(DEPEND) mapfile include/patterns.h

.PHONY: printvars
printvars:
	@$(foreach V,$(.VARIABLES), $(if $(filter-out environment% default automatic, $(origin $V)),$(warning $V=$($V))))

.phony: $(DEPEND)

ifneq (clean,$(MAKECMDGOALS))
-include $(DEPEND)
endif
