#include "Pipeline.h"
#include "Processor.h"
#include "Mem.h"

using namespace std;
using namespace pipeline;
using namespace mem;

int main(){
  ProgramLoader loader( new VirtualMem(new DRAM(0x100, "MainMem")),
      new DRAM(0b100000, "rf"));
  loader.loadProgram("out");
  loader.run();
}
