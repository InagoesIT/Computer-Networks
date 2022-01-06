all:
	g++ server.cpp -Wall -o server `sdl2-config --cflags --libs` -l sqlite3 -l pthread
	g++ client.cpp -Wall -o client

clean:
	rm -f *~ server client