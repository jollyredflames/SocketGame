all : mkwords readbin psort

mkwords : mkwords.c helper.h
	gcc -Wall -g -std=gnu99 -o mkwords mkwords.c -lm

psort : psort.c helper.c helper.h helper.c
	gcc -Wall -std=gnu99 -o psort psort.c

clean :
	rm readbin mkwords psort