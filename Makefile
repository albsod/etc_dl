CFLAGS  =-Wall -Wextra -pedantic -O3
FILES   = src/platform.c src/date.c src/curly.c src/clopt.c
LDFLAGS =-lcurl

build: $(FILES)	
	cc -o etc-dl src/etc_dl.c $(CFLAGS) $(LDFLAGS) $(FILES)

clean:
	rm -f etc-dl

