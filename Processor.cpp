#include <fstream>
#include <iostream>
#include <ios>
#include <array>
#define PIPESIZE 5

#include "Pipeline.h"
#include "Instruction.h"
#include "Mem.h"
#include "Processor.h"
using namespace std;
using namespace pipeline;
using namespace instruction;
using namespace mem;

Processor5S::Processor5S(string name, MemoryUnit& mainMem, MemoryUnit& rf,
    data32 instrStart, string logFilename) : 
    mainMem{mainMem}, rf{rf}, acc{0}, currentCycle{0}, name{name}, 
    pc{"PC",instrStart}, log{logFilename}{
  //set rf[0] = 0 cause MIPS hardwired
  rf.sw(0,0);
  //initialize the pipe
  pipe = array<PipelinePhase*, PIPESIZE>();
  pipe[0] = new InstructionFetch("IF", mainMem, log);
  pipe[1] = new InstructionDecode("ID", rf, log);
  pipe[2] = new Execute("EX", pc, log);
  pipe[3] = new MemoryAccess("MA", mainMem, log);
  pipe[4] = new WriteBack("WB", rf, acc, pc, log);
}

bool Processor5S::updateCycle(int cycles){

  //update all the cycles
  for(PipelinePhase* stage : pipe){
    stage->updateCycle(cycles);
  }

  //find first stalling pipe iterating backwards
  char firstStalling = PIPESIZE;
  do {
    firstStalling--;
  } while (firstStalling >= 0 && !(pipe[firstStalling]->isBusy()));
  bool stalling = firstStalling > -1;

  StageOut* out = stalling ? nullptr : pc.getOut();
  for(int i = firstStalling + 1; i < pipe.size(); i++){
    StageOut* tempOut = pipe[i]->getOut();
    pipe[i]->execute(&out); //this deletes out
    out = tempOut;
  }
  return ((WBOut*) out)->quit;
}

void Processor5S::start(int startI){
  pc.set(startI);
  bool quit = false;
  while(!quit){
    quit = updateCycle(1);
    pc.inc(1);
  }
  cout << "Program Terminating" << endl;
}

Processor5S::~Processor5S(){
  log.close(); 
}

SizedArr<data32> MachineCodeFileReader::loadFile(string filename){
  //load file
  ifstream executableFile;
  executableFile.open(filename);
  //count newlines
  size_t nInstrs = count(istreambuf_iterator<char>(executableFile), 
             istreambuf_iterator<char>(), '\n');
  //reset filereader
  executableFile.seekg(0);
  //create array based on number of instrs
  data32* instrs = new data32[nInstrs+1];
  for(int i = 0; i < nInstrs; i++){
    executableFile >> hex >> instrs[i];
  }
  executableFile.close();
  //Write a termination syscall
  instrs[nInstrs] = 0xc;
  SizedArr<data32> sizedArr;
  sizedArr.arr = instrs;
  sizedArr.size = nInstrs+1;
  return sizedArr;
}

//TODO really? this is the best way?
ProgramLoader::ProgramLoader(MemoryUnit* mainMem, MemoryUnit* rf) : 
  exeReader{}, p{"MIPSProcessor", *mainMem, *rf, 0, "pipeline.log"},
  mainMem{mainMem}, rf{rf} {}

void ProgramLoader::loadProgram(string filename){
  SizedArr<data32> exeSized = exeReader.loadFile(filename);
  mainMem->storeBlock(0, exeSized.arr, exeSized.size);
  //set R$31 to return to the exit condition
  rf->sw(31,exeSized.size-1);
}

void ProgramLoader::run(){
  p.start(0);
}

ProgramLoader::~ProgramLoader(){
  delete mainMem;
  delete rf;
}



