#=-- General ------------------------------------------------------------------

V	= 0
OPT	= -O2

CFILES	= main.c image.c err.c action.c
BINFILE	= spice

SRCDIR	:= src
OBJDIR	:= obj
BINDIR	:= bin
DEPDIR 	:= deps
TESTDIR	:= test

SRCS	:= $(CFILES:%=$(SRCDIR)/%)
OBJS	:= $(CFILES:%.c=$(OBJDIR)/%.o)
DEPS	:= $(CFILES:%.c=$(DEPDIR)/%.d)
BUILDDIRS = $(OBJDIR) $(BINDIR) $(DEPDIR) $(TESTDIR)

V	?= 0
ifeq ($(V),0)
Q	:= @
endif

#=-- Compiler & Linker config -------------------------------------------------
CC	= gcc
LD	= gcc
CFLAGS	+= -ansi
CFLAGS	+= $(OPT)
LDFLAGS	+= #...
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

#=-- Build Rules --------------------------------------------------------------
all: $(BINDIR)/$(BINFILE)

$(BINDIR)/$(BINFILE): $(OBJS) $(DEPS)
	@printf "  LD\t$^\n"
	@mkdir -p $(dir $@)
	$(Q)$(LD) $(LDLAGS) $(OBJS) -o $@

$(OBJDIR)/%.o $(DEPDIR)/%.d: $(SRCDIR)/%.c
	@printf "  CC\t$<\n"
	@mkdir -p $(OBJDIR)
	@mkdir -p $(DEPDIR)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ -c $<
	
clean:
	@printf "  RM\t$(BUILDDIRS)\n"
	$(Q)rm -rf $(BUILDDIRS)

%.d: ;

#=-- Tests --------------------------------------------------------------------
IMG		= Parrot.ppm
TAIL	= --resize 1000 600 500 0 --compose $(IMG)
HEAD	= $(Q)$(BINDIR)/$(BINFILE) $(IMG) $(TESTDIR)/$@.ppm

OPTIONLESS = invert sharpen grayscale
OPTIONFULL = blur contrast scale dither

test: testdir $(BINDIR)/$(BINFILE) $(OPTIONLESS) $(OPTIONFULL)

testdir:
	@rm -rf $(TESTDIR)
	@mkdir -p $(TESTDIR)

$(OPTIONLESS):
	@printf "  TEST\t$@\n"
	$(HEAD) --$@ $(TAIL)

blur:
	@printf "  TEST\t$@\n"
	$(HEAD) --$@ 10 $(TAIL)

scale:
	@printf "  TEST\t$@\n"
	$(HEAD) --$@ 1000 1200 $(TAIL)

contrast:
	@printf "  TEST\t$@\n"
	$(HEAD) --$@ 1.5 $(TAIL)

dither:
	@printf "  TEST\t$@\n"
	$(HEAD) --$@ 2 $(TAIL)

.PHONY: all clean test testdir