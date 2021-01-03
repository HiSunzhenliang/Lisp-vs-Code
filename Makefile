CC = gcc
CFLAGS = -std=c99 -Wall -g
LFLAGS = 
FILES = parsing 
PLATFORM = $(shell uname)

ifeq ($(findstring Linux,$(PLATFORM)),Linux)
	LFLAGS += -ledit -lm
endif

ifeq ($(findstring Darwin,$(PLATFORM)),Darwin)
	LFLAGS += -ledit -lm
endif

ifeq ($(findstring MINGW,$(PLATFORM)),MINGW)
endif

all: $(FILES)

%: src/%.c src/mpc.c
	$(CC) $(CFLAGS) $^ $(LFLAGS) -o $@
  
