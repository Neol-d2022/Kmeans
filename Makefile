CC=gcc

CFLAGS=-Wall -Wextra -s -Ofast
LFLAGS=

OBJS=main.o
DEPS=
LIBS=-lm

BIN=kmeans

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(LFLAGS) $(LIBS)

clean:
	rm -f $(OBJS) $(BIN)
