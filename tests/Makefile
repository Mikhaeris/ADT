CC       ?= clang
CPPFLAGS ?=
CFLAGS   ?= -std=c11 -Wall -Wextra -Werror -g3
LDFLAGS  ?=
LDLIBS   ?=

SANITIZE ?= -fsanitize=address,undefined -fno-sanitize-recover=all \
            -fno-omit-frame-pointer

BUILDDIR := build

SRCS  := $(wildcard *.c)
BINS  := $(SRCS:%.c=$(BUILDDIR)/%)
DEPS  := $(SRCS:%.c=$(BUILDDIR)/%.d)
NAMES := $(basename $(SRCS))

.DEFAULT_GOAL := test
.PHONY: all test check clean $(NAMES)

all: $(BINS)

test check: $(BINS)
	@for bin in $(BINS); do \
		echo "==> $$bin"; \
		./$$bin || exit $$?; \
	done

$(NAMES): %: $(BUILDDIR)/%
	./$<

$(BUILDDIR)/%: %.c | $(BUILDDIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SANITIZE) -MMD -MP -o $@ $< $(LDFLAGS) $(LDLIBS)

$(BUILDDIR):
	@mkdir -p $@

clean:
	$(RM) -r $(BUILDDIR)

-include $(DEPS)
