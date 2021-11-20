APP_NAME=fine_grain

OBJS=fine_grain.o

CXX = g++-10 -m64 -std=c++11
CXXFLAGS = -I. -O3 -Wall

default: $(APP_NAME)

$(APP_NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@

clean:
	/bin/rm -rf *~ *.o $(APP_NAME) *.class