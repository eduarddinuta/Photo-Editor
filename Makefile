CFLAGS=-Wall -Wextra -std=c99

build:
	gcc $(CFLAGS) -o image_editor image_editor.c my_functions.h my_functions.c -lm

pack:
	zip -FSr 311CA_DinutaEduardStefan_Tema3.zip README Makefile *.c *.h

clean:
	rm -f image_editor

.PHONY: pack clean
