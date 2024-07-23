all:
	gcc 6502.c -o 6502 -Wall -Wextra -pedantic

clean:
	rm -rf 6502 && clear

run:
	./6502
