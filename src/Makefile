SRC = irmc.c
OBJ = ${SRC:.c=.o}
LDFLAGS = -L/usr/local/lib -L/opt/local/lib  -lm -lmorse
CFLAGS = -I/usr/local/include -I/opt/local/include -Wall 
INSTALLDIR = ${HOME}/bin

all: options irmc 

options:
	@echo irmc build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"
	@echo "INSTALLDIR = ${INSTALLDIR}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

irmc:   ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

hex: 	${OBJ}
	@echo avr-gcc -o $@
	avr-gcc -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f irmc irmc.core ${OBJ} 
install: irmc
	cp irmc ${INSTALLDIR}
