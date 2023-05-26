RM        := /bin/rm -rf
SIM       := sim
CC        := g++
CFLAGS    := -std=c++23 -lz -lm -W -Wall -Wno-deprecated -Wno-unknown-warning-option -Wno-self-assign -Wno-unused-parameter -Wno-unused-but-set-variable -Wold-style-cast
FASTFLAGS := -ggdb3 -march=native -O3 -frename-registers
DBGFLAGS  := -ggdb3 -O0

FLAGS := ${FASTFLAGS}
ifeq ($(DEBUG),1)
	FLAGS := ${DBGFLAGS}
endif
ifeq ($(GPROF),1)
	FLAGS := -pg ${FLAGS}
endif

SRCFILES:=$(wildcard *.c)
SRCFILES+=$(wildcard *.cpp)

HEADERFILES:=$(wildcard *.h)
HEADERFILES+=$(wildcard *.hpp)

.DEFAULT_GOAL := sim
.PHONY: clean

run: sim
	./${SIM}
	
sim: ${SRCFILES} ${HEADERFILES}
	${CC} ${FLAGS} ${CFLAGS} ${SRCFILES} -o ${SIM}

clean: 
	$(RM) ${SIM} *.o
