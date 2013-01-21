program_NAME := asm
program_SRCS := $(wildcard *.c)
program_OBJS := ${program_SRCS:.c=.o}

.PHONY: all clean distclean

all: $(program_NAME)

$(program_NAME): $(program_OBJS)
	gcc -ansi -pedantic -Wall $(program_OBJS) -o $(program_NAME)

clean:
	@- $(RM) $(program_NAME)
	@- $(RM) $(program_OBJS)

distclean: clean
