all: server fcgi

server: serwerHTTP.cpp ConfigFile.cpp FCGIManager.cpp HTTPManager.cpp Parser.cpp ConnectionManager.cpp Record.cpp StreamRecord.cpp BeginRecord.cpp
	g++ serwerHTTP.cpp ConfigFile.cpp FCGIManager.cpp HTTPManager.cpp Parser.cpp ConnectionManager.cpp Record.cpp StreamRecord.cpp BeginRecord.cpp -std=c++11 -lconfig++ -Wall -pthread -o bin/serverHTTP

fcgi: fcgi.cpp
	g++ fcgi.cpp -lfcgi++ -lfcgi -o bin/fcgi
