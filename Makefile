default: slowloris

slowloris: slowloris.c
	gcc -std=c99 -Wall -pthread slowloris.c -o slowloris

clean:
	rm -rf slowloris