
APP_NAME = correctness_test performance_test
CORRECTNESS_OBJS = fine_grain.o correctness_test.o
PERFORMANCE_OBJS = fine_grain.o performance_test.o
CXX = g++-10 -m64 -std=c++11
CXXFLAGS = -I. -O3 -Wall -pthread

default: $(APP_NAME)

# $(APP_NAME): $(OBJS)
# 	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

correctness_test: $(CORRECTNESS_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(CORRECTNESS_OBJS)

performance_test: $(PERFORMANCE_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(PERFORMANCE_OBJS)

%.o: %.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@

clean:
	/bin/rm -rf *~ *.o $(APP_NAME) *.class