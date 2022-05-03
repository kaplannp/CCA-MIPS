CC = g++
CFLAGS = -ggdb -fmax-errors=4
LOG_LIBS = -lboost_log -lpthread 
#How do I find these -l<names> ? 

UNIT_TEST_LIB = -lboost_unit_test_framework

main: Pipeline.o main.o Mem.o Instruction.o Processor.o
	$(CC) $^ -o $@ $(LOG_LIBS) $(CFLAGS)

test: Pipeline.o test.o Mem.o Instruction.o Processor.o
	$(CC) $^ -o $@ $(LOG_LIBS) $(CFLAGS) $(UNIT_TEST_LIB) 

Processor.o: Processor.cpp Processor.h
	$(CC) Processor.cpp -c $(LOG_LIBS) $(CFLAGS)

Pipeline.o: Pipeline.cpp Pipeline.h
	$(CC) Pipeline.cpp -c $(CFLAGS)

Instruction.o: Instruction.cpp Instruction.h
	$(CC) Instruction.cpp -c $(CFLAGS)

Mem.o: Mem.cpp Mem.h
	$(CC) Mem.cpp -c $(CFLAGS)

test.o: test.cpp 
	$(CC) test.cpp -c $(CFLAGS)

clean:
	rm -f test && rm -f main && rm -f *.o
