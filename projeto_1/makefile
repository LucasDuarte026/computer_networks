all: server client
		
clean:
		rm -f client server

server: 
		g++ -o server server.cpp

client: 
		g++ -o client client.cpp

debug:
	g++  -g3 -o server server.cpp -pthread
	g++  -g3 -o client client.cpp -pthread

# fullDebug :
# 	g++  -Wall -g3 ./*.cpp -o server
# run:
# 	./server