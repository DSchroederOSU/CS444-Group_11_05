default: all

recitation1:
	gcc -lpthread -o recitation1 recitation1.c

concurrency1:
	gcc -lpthread -o concurrency1 concurrency1.c

all: recitation1 concurrency1

clean:
	rm -f *.o recitation1 concurrency1
