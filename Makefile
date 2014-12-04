all : tao-1k

tao-1k : Tao1k.cpp
	g++ -o tao-1k -std=c++11 -pthread `pkg-config --cflags TAO_CosNaming` -Wall -Wextra -Werror Tao1k.cpp `pkg-config --libs TAO_CosNaming`

test : tao-1k
	./tao-1k -ORBInitRef NameService=corbaloc:iiop:127.0.0.1:2809

clean :
	rm -f tao-1k
