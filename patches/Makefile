VERSION ?= us
ifneq ($(words $(VERSION)),1)
$(error VERSION must be us or jp)
endif
ifeq ($(filter $(VERSION),us jp),)
$(error VERSION must be us or jp)
endif
BUILD_DIR := $(if $(filter jp,$(VERSION)),build/jp,.)
TARGET = $(BUILD_DIR)/patches.elf

CC      ?= clang
LD      ?= ld.lld

CFLAGS   := -target mips -mips2 -mabi=32 -O2 -G0 -mno-abicalls -mno-odd-spreg -mno-check-zero-division \
			-fomit-frame-pointer -ffast-math -fno-unsafe-math-optimizations -fno-builtin-memset \
			-Wall -Wextra -Wno-incompatible-library-redeclaration -Wno-unused-parameter -Wno-unknown-pragmas -Wno-unused-variable -Wno-unused-but-set-variable -Wno-missing-braces -Wno-unsupported-floating-point-opt -Wno-switch
CPPFLAGS := -nostdinc -DVERSION_$(if $(filter jp,$(VERSION)),JP,US)=1 -DF3DEX_GBI -D_LANGUAGE_C -DMIPS -DM2CTX -I dummy_headers -I ../lib/sk1-decomp/include -I ../lib/sk1-decomp/include/PR -I ../lib/rt64/include
CPPFLAGS += $(EXTRA_CPPFLAGS)
LDFLAGS  := -nostdlib -T patches.ld -T syms.ld -Map $(BUILD_DIR)/patches.map --unresolved-symbols=ignore-all --emit-relocs

C_SRCS := $(wildcard *.c)
C_OBJS := $(addprefix $(BUILD_DIR)/,$(C_SRCS:.c=.o))
C_DEPS := $(C_OBJS:.o=.d)

$(TARGET): $(C_OBJS) patches.ld syms.ld
	$(LD) $(C_OBJS) $(LDFLAGS) -o $@

$(C_OBJS): $(BUILD_DIR)/%.o : %.c Makefile
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) $< -MMD -MF $(@:.o=.d) -c -o $@

clean:
	rm -f $(C_OBJS) $(C_DEPS) $(TARGET)

-include $(C_DEPS)

.PHONY: clean
