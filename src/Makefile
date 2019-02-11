CFLAGS  =-Wall -Wextra -pedantic -std=c11
FILES   = platform.c date.c curly.c clopt.c
LDFLAGS =-lcurl

build: $(FILES)	
	cc -o etc-dl etc_dl.c $(CFLAGS) $(LDFLAGS) $(FILES)

clean:
	rm -f etc-dl

