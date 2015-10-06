a.out: child parent.c
	gcc -Wall parent.c
child: child.c
	gcc -Wall child.c -o child
clean:
	rm a.out child