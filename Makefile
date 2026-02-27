tetris: tetris.c
	gcc tetris.c -o tetris

run: tetris
	./tetris

clean:
	rm -f tetris
