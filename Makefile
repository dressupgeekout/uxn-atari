CROSS_PREFIX=	m68k-atari-mint-

CROSS_CC=	$(CROSS_PREFIX)gcc
CROSS_STRIP=	$(CROSS_PREFIX)strip

UXNASM=		uxnasm.tos
UXNCLI=		uxncli.tos

UXNASM_SRCS=	# defined
UXNASM_SRCS+=	src/uxnasm.c

UXNCLI_SRCS=	# defined
UXNCLI_SRCS+=	src/devices/datetime.c
UXNCLI_SRCS+=	src/devices/file.c
UXNCLI_SRCS+=	src/devices/system.c
UXNCLI_SRCS+=	src/uxncli.c
UXNCLI_SRCS+=	src/uxn.c

.PHONY: all
all: $(UXNASM) $(UXNCLI)

.PHONY: clean
clean:
	rm -f $(UXNASM) $(UXNCLI)

$(UXNASM): $(UXNASM_SRCS)
	$(CROSS_CC) -o $@ $^
	$(CROSS_STRIP) $@

$(UXNCLI): $(UXNCLI_SRCS)
	$(CROSS_CC) -o $@ $^
	$(CROSS_STRIP) $@
