
CFLAGS=-Wall -std=c99
INCLUDES=-I./inc

MOLTIPLICATORE_SRCS=src/moltiplicatore.c
ESE4_SRCS=src/errExit.c src/main.c

MOLTIPLICATORE_OBJS=$(MOLTIPLICATORE_SRCS:.c=.o)
ESE4_OBJS=$(ESE4_SRCS:.c=.o)

all: moltiplicatore process_ese4

moltiplicatore: $(MOLTIPLICATORE_OBJS)
	@echo "Making executable: "$@
	@$(CC) $^ -o $@

process_ese4: $(ESE4_OBJS)
	@echo "Making executable: "$@
	@$(CC) $^ -o $@

.c.o:
	@echo "Compiling: "$<
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

.PHONY: clean

clean:
	@rm -f src/*.o process_ese4 moltiplicatore
	@echo "Removed object files and executables..."
