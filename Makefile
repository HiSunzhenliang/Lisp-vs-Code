CC = gcc
CFLAGS = -std=c99 -Wall -g
LFLAGS = 
FILES = parsing s_expressions
PLATFORM = $(shell uname)

ifeq ($(findstring Linux,$(PLATFORM)),Linux)
	LFLAGS += -ledit -lm
	FILES += prompt_unix
endif

ifeq ($(findstring Darwin,$(PLATFORM)),Darwin)
	LFLAGS += -ledit -lm
	FILES += prompt_unix
endif

ifeq ($(findstring MINGW,$(PLATFORM)),MINGW)
	FILES += prompt_windows
endif

all: $(FILES)

%: %.c mpc.c
	$(CC) $(CFLAGS) $^ $(LFLAGS) -o $@
  
