all:
	gcc board.cpp -o board `sdl2-config --cflags --libs`
	gcc db-queries.cpp -o db-queries -l sqlite3
clean:
	rm -f *~ db-queries board