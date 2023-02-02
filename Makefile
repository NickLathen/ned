WARNALL=-Wall -Wextra -Wpedantic
DEBUG=-g
STD=--std=c++17
LDLIBS=-lncurses
CXXFLAGS=$(STD) $(WARNALL) $(DEBUG)

ned : src/bufferoperation.o src/bufferposition.o src/buffercursor.o src/editbuffer.o src/pane.o src/ned.o
	$(CXX) $^ -o $@ $(LDLIBS)


clean:
	rm -f ./src/*.o
	rm -f ./ned
  