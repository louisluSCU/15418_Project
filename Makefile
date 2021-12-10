APP_NAME = correctness_test performance_test performance_test_ye handin.tar
CORRECTNESS_OBJS = fine_grain/fine_grain.o coarse_grain/coarse_grain.o lock_free/lockfree_bst.o test/correctness_test.o 
PERFORMANCE_OBJS = fine_grain/fine_grain.o coarse_grain/coarse_grain.o lock_free/lockfree_bst.o test/performance_test.o
PERFORMANCE_YE_OBJS = fine_grain/fine_grain.o coarse_grain/coarse_grain.o lock_free/lockfree_bst.o test/performance_test_ye.o
CXX = g++ -m64 -std=c++11
CXXFLAGS = -I. -O3 -Wall -pthread
FILES = project_final_report.pdf project_milestone_report.pdf project_proposal.pdf \
	lock_free/lockfree_bst.h lock_free/lockfree_bst.cc fine_grain/fine_grain.h fine_grain/fine_grain.cpp \
	coarse_grain/coarse_grain.h coarse_grain/coarse_grain.cpp test/correctness_test.cpp test/performance_test_ye.cpp \
	test/performance_test.cpp README.md Makefile 

default: $(APP_NAME)

correctness_test: $(CORRECTNESS_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(CORRECTNESS_OBJS)

performance_test: $(PERFORMANCE_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(PERFORMANCE_OBJS)

performance_test_ye: $(PERFORMANCE_YE_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(PERFORMANCE_YE_OBJS)

fine_grain/%.o: fine_grain/%.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@

coarse_grain/%.o: coarse_grain/%.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@

lock_free/%.o: lock_free/%.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@

handin.tar: $(FILES)
	tar cvf handin.tar $(FILES)

clean:
	/bin/rm -rf fine_grain/*.o coarse_grain/*.o lock_free/*.o test/*.o $(APP_NAME)
	rm -f *~ handin.tar