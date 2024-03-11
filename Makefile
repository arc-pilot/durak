LDFLAGS = -O2
all:
	$(CC) $(LDFLAGS) -o durak durak.c

clean:
	rm durak
