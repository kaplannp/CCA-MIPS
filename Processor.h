#ifndef PROCESSOR_H_INSCLUDED
#define PROCESSOR_H_INSCLUDED
#include "Pipeline.h"
#include "Instruction.h"
#include "Mem.h"
#include<array>

using namespace std;
using namespace pipeline;
using namespace instruction;
using namespace mem;

//TODO how am I to handle memory. Would love to not deal with 4GB of RAM
//and I need a bit for my instructions
class Processor5S{
  private:
    array<PipelinePhase*, 5> pipe;
    PC pc;
    MemoryUnit& rf;
    MemoryUnit& mainMem;
    data64 acc;
    string name;
    unsigned int currentCycle;
    ofstream log;
    
  public:
    /*
     * memSize is the size of MainMemory
     * rfSize is the size of the RegisterFile
     */
    Processor5S(string name, MemoryUnit& mainMem, MemoryUnit& rf, 
        data32 instrStart, string logFilename);

    /*
     * The method to advance time for the processor
     * returns true if this cycle caused a quit condition. Otherwise false
     */
    bool updateCycle(int timeToAdvance);
    
    /*
     * Starts the processor going at location i in main memory.
     * It'll only stop when it executes a syscall
     */
    void start(int startI);

    /*
     * close the log
     */
    ~Processor5S();
};

/*
 * An Array with a size. Just a handy way to ship data. You're responsible
 * for deleting the dynamically allocated array.
 */
template<typename T>
struct SizedArr{
  T* arr;
  size_t size;
};

class MachineCodeFileReader{
  public:
    SizedArr<data32> loadFile(string filename);
};

class ProgramLoader{
  private:
    MachineCodeFileReader exeReader;
    Processor5S p; //will be overwritten by constructor
    MemoryUnit* mainMem;
    MemoryUnit* rf;
  public:
    ProgramLoader(MemoryUnit* mainMem, MemoryUnit* rf);
    void loadProgram(string filename);
    void run();
    ~ProgramLoader();
};
#endif
