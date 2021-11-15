CC = g++
CFLAGS = -std=c++11 -Wall
LFDIR = lock_free

lockfree_bst:
	$(CC) $(CFLAGS) -o $(LFDIR)/lockfree_bst $(LFDIR)/lockfree_bst.cc
