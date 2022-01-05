all:
	g++ server.cpp -Wall -o server `sdl2-config --cflags --libs` -l sqlite3
	g++ client.cpp -Wall -o client
clean:
	rm -f *~ server client