all:
	g++ server.cpp -Wall -o s1 `sdl2-config --cflags --libs` -l sqlite3 -l pthread
	g++ client.cpp -Wall -o c1

clean:
	rm -f *~ server client