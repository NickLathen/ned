CC=g++
CPP=g++
CXX=g++
WARNALL=-Wall -Wextra -Wpedantic
DEBUG=-g
STD=--std=c++17
LDLIBS=-lstdc++ -lncurses
CXXFLAGS=$(STD) $(WARNALL) $(DEBUG)

ned : src/bufferposition.o src/buffercursor.o src/editbuffer.o src/pane.o src/ned.o
	$(CXX) $^ -o $@ $(LDLIBS)

clean:
	rm -f ./src/*.o
	rm -f ./ned
  