CC=clang
CPP=clang
CXX=clang
SANITIZE=
WARNALL=-Wall -Wextra -Wpedantic
DEBUG=-g
STD=--std=c++17
LDLIBS=-lstdc++ -lpanel -lncurses
CXXFLAGS=$(STD) $(SANITIZE) $(WARNALL) $(DEBUG)

#implicit rules?
#  %.o : %.cc
# 	$(CXX) $(CPPFLAGS) $< $(CXXFLAGS) -c -o $@
#  % : %.o
# 	$(CXX) $(LDFLAGS) $< $(LOADLIBES) $(LDLIBS) -o $@


ned : src/buffercursor.cc src/editbuffer.cc src/pane.cc src/ned.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDLIBS) $^ -o $@


clean:
	rm -f ./ned
