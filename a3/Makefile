all : mkwords psort helper

mkwords : mkwords.c helper.h helper.c
	gcc -Wall -g -std=gnu99 -o mkwords mkwords.c -lm

psort : psort.c helper.c helper.h helper.c
	gcc -Wall -g -std=gnu99 -o psort psort.c

helper: helper.c helper.h
	gcc -Wall -g -std=gnu99 -o helper helper.c
clean :
	rm mkwords psort