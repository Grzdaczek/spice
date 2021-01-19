#=-- General ------------------------------------------------------------------

V	= 0
OPT	= -O2

CFILES	= main.c image.c err.c action.c
BINFILE	= ppmsteg

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

i:
	@printf "$(SRCS)\n"
	@printf "$(OBJS)\n"
	@printf "$(DEPS)\n"

#=-- Tests --------------------------------------------------------------------
IMG		= Droste.ppm
TAIL	= -resize 680 512 340 0 -compose $(IMG)
HEAD	= $(Q)$(BINDIR)/$(BINFILE) $(IMG) $(TESTDIR)/$@.ppm

OPTIONLESS = invert dither sharpen
OPTIONFULL = blur contrast scale

test: testdir $(BINDIR)/$(BINFILE) $(OPTIONLESS) $(OPTIONFULL)

testdir:
	@rm -rf $(TESTDIR)
	@mkdir -p $(TESTDIR)

$(OPTIONLESS):
	@printf "  TEST\t$@\n"
	$(HEAD) -$@ $(TAIL)

blur:
	@printf "  TEST\t$@\n"
	$(HEAD) -$@ 10 $(TAIL)

scale:
	@printf "  TEST\t$@\n"
	$(HEAD) -$@ 680 1024 $(TAIL)

contrast:
	@printf "  TEST\t$@\n"
	$(HEAD) -$@ 1.5 $(TAIL)


.PHONY: testdir all clean test