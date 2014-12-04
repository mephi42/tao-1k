all : tao-1k

tao-1k : Tao1k.cpp
	g++ -o tao-1k -std=c++11 -pthread `pkg-config --cflags TAO_CosNaming` -Wall -Wextra -Werror Tao1k.cpp `pkg-config --libs TAO_CosNaming`

check-style : Tao1k.cpp
	cpplint.py --filter=-whitespace/tab,-build/c++11,-legal/copyright,-readability/streams,-build/include_order,-whitespace/indent Tao1k.cpp

test : tao-1k
	./tao-1k -ORBInitRef NameService=corbaloc:iiop:127.0.0.1:2809

clean :
	rm -f tao-1k
