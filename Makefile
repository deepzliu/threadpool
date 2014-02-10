

all: clean demo

clean:
	rm -f demo
install:

demo:
	g++ thdpool.cpp demo.cpp -o demo -g -lpthread

