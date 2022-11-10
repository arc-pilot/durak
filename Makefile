LDFLAGS = -O2
all:
	$(CC) $(LDFLAGS) -o durak `find . -name *.c`

clean:
	rm durak
