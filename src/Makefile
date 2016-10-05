SRC = irmc.c cwprotocol.c beep.c util.c
OBJ = ${SRC:.c=.o}
LDFLAGS = -lportaudio -lpthread -lm 
CFLAGS = -Wall -Wno-format-zero-length -Wno-parentheses
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

raspi:	
	@${CC} -c -DRASPI ${CFLAGS} ${SRC}
	@${CC} -o irmc ${OBJ} ${LDFLAGS} -lwiringPi

java:
	java -jar test/MorseKOB.jar 

clean:
	@echo cleaning
	@rm -f irmc irmc.core ${OBJ} 
install: irmc
	cp irmc ${INSTALLDIR}
