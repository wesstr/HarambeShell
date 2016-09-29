#
#makefile for the harambe shell

target: harambe_shell.c
	gcc -g -Wall -o harambe_shell harambe_shell.c

clean:
	$(RM) harambe_shell
