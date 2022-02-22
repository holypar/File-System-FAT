# Target library
lib := libfs.a

all: $(lib)
CC = gcc
FLAGS = -Wall -Wextra -Werror
OBJS = disk.o fs.o

deps := $(patsubst %.o, %.d, $(OBJS))
-include $(deps)

libfs.a: $(OBJS)
	ar rcs $@ $^

disk.o: disk.c disk.h
	$(CC) $(FLAGS) -c -o $@ $<

fs.o: fs.c fs.h disk.h
	$(CC) $(FLAGS) -c -o $@ $<

clean:
	rm -f $(lib) $(OBJS)


## TODO: Phase 1
## To reflect advanced Makefile from Discussion 4 slides