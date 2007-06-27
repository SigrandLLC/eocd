CFLAGS := -g

all: $(patsubst %.c,%.o,$(wildcard *.c))

%.o: %.c
	gcc $(CFLAGS) -c $<

test: all
	gcc -o test test.o list.o hash_table.o

clean:
	rm *.o test