CC = g++
CFLAGS = -ggdb
LOG_LIBS = -lboost_log -lpthread
#How do I find these -l<names> ? 
UNIT_TEST_LIB = -lboost_unit_test_framework

test: Pipeline.cpp test.cpp
	$(CC) $^ -o $@  $(LOG_LIBS) $(CFLAGS) $(UNIT_TEST_LIB)

main: Pipeline.cpp main.cpp
	$(CC) $^ -o $@  $(LOG_LIBS) $(CFLAGS)

clean:
	rm -f test && rm -f main
