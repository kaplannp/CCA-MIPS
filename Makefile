CC = g++
CFLAGS = -ggdb -fmax-errors=4
LOG_LIBS = -lboost_log -lpthread 
#How do I find these -l<names> ? 
UNIT_TEST_LIB = -lboost_unit_test_framework

test: Pipeline.o test.o Mem.o Instruction.o 
	$(CC) $^ -o $@ $(LOG_LIBS) $(CFLAGS) $(UNIT_TEST_LIB)

main: Pipeline.o main.o Mem.o Instruction.o
	$(CC) $^ -o $@ $(LOG_LIBS) $(CFLAGS)

Pipeline.o: Pipeline.cpp Pipeline.h
	$(CC) Pipeline.cpp -c 

Instruction.o: Instruction.cpp Instruction.h
	$(CC) Instruction.cpp -c 

Mem.o: Mem.cpp Mem.h
	$(CC) Mem.cpp -c 

test.o: test.cpp 
	$(CC) test.cpp -c 

clean:
	rm -f test && rm -f main && rm *.o
