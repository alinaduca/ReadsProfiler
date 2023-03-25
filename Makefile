all:
	g++ server.cpp ./files/pugixml.cpp -l pthread -o server
	g++ client.cpp -o client -lcurl
clean:
	rm -f client server
