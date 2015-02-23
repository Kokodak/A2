
a2: a2.cpp a2.hpp
	g++ -std=c++0x -o a2 a2.cpp -lrt -DNDEBUG #-g

clean:
	rm a2
