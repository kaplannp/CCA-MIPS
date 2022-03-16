CC = g++
CFLAGS = -ggdb
LOG_LIBS = -lboost_log -lpthread -fmax-errors=4 -Wno-inline
#How do I find these -l<names> ? 
UNIT_TEST_LIB = -lboost_unit_test_framework

test: Pipeline.cpp test.cpp Mem.cpp Instruction.cpp
	$(CC) $^ -o $@  $(LOG_LIBS) $(CFLAGS) $(UNIT_TEST_LIB)

main: Pipeline.cpp main.cpp Mem.cpp Instruction.cpp
	$(CC) $^ -o $@  $(LOG_LIBS) $(CFLAGS)

clean:
	rm -f test && rm -f main
