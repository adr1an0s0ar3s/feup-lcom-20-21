PROG=proj

SRCS = proj.c keyboard.c mouse.c util.c graph.c game.c timer.c menu.c serial.c rtc.c

# __LCOM_OPTIMIZED__ disables most of the LCF's overhead (and functionality!)
CPPFLAGS += -pedantic -D __LCOM_OPTIMIZED__

DPADD += ${LIBLCF}
LDADD += -llcf

.include <minix.lcom.mk>
