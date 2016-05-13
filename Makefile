TARGET 		= xfer

OBJS 		= main.o http.o cpc.o parse.o
INCDIR 		= -I.
LIBS  		= wsock32
CC 			= gcc
CFLAGS 		= -O2  -W
LDFLAGS 		= -s -Wl
STRIP		= strip


all:		$(TARGET)


$(TARGET): ${OBJS}
	${CC} -o "$@" ${OBJS} ${LDFLAGS} -l${LIBS}
	$(STRIP) $(TARGET).exe
	rm -f ${OBJS}

%.o : %.c
	${CC} ${CFLAGS} ${INCDIR} -c $< -o $@

clean:
	rm -f ${OBJS}