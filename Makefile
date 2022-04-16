CROSS_PREFIX=	m68k-atari-mint-

CROSS_CC=	$(CROSS_PREFIX)gcc
CROSS_STRIP=	$(CROSS_PREFIX)strip

UXNASM=		uxnasm.tos
UXNCLI=		uxncli.tos
UXNEMU=		uxnemu.app

UXNASM_SRCS=	# defined
UXNASM_SRCS+=	src/uxnasm.c

UXNCLI_SRCS=	# defined
UXNCLI_SRCS+=	src/devices/datetime.c
UXNCLI_SRCS+=	src/devices/file.c
UXNCLI_SRCS+=	src/devices/system.c
UXNCLI_SRCS+=	src/uxncli.c
UXNCLI_SRCS+=	src/uxn.c

UXNEMU_SRCS=	# defined
UXNEMU_SRCS+=	src/uxn.c
UXNEMU_SRCS+=	src/uxnemu.c
UXNEMU_SRCS+=	src/devices/audio.c
UXNEMU_SRCS+=	src/devices/controller.c
UXNEMU_SRCS+=	src/devices/datetime.c
UXNEMU_SRCS+=	src/devices/file.c
UXNEMU_SRCS+=	src/devices/mouse.c
UXNEMU_SRCS+=	src/devices/screen.c
UXNEMU_SRCS+=	src/devices/system.c

.PHONY: all
all: $(UXNASM) $(UXNCLI) $(UXNEMU)

.PHONY: clean
clean:
	rm -f $(UXNASM) $(UXNCLI) $(UXNEMU)

$(UXNASM): $(UXNASM_SRCS)
	$(CROSS_CC) -o $@ $^
	$(CROSS_STRIP) $@

$(UXNCLI): $(UXNCLI_SRCS)
	$(CROSS_CC) -o $@ $^
	$(CROSS_STRIP) $@

$(UXNEMU): $(UXNEMU_SRCS)
	$(CROSS_CC) -o $@ $^
	$(CROSS_STRIP) $@
