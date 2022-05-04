#ifndef PIPELINE_H_INCLUDED
#define PIPELINE_H_INCLUDED
#include <string>
#include <assert.h>
#include <boost/log/trivial.hpp>
#include <vector>
#include <string>
#include <fstream>

#include "Mem.h"
#include "Instruction.h"

#define BOOST_LOG_DYN_LINK

using namespace mem;
using namespace instruction;
using namespace std;
/*
 * The core class of this module is the PipelinePhase. This is a single stage
 * in a pipeline. It must implement the included methods.
 */
namespace pipeline{

  /*
   * This class looks simple, and it is. It's purpose is to provide a class
   * of data carrying classes for output of pipeline stages
   */
  class StageOut{
    public:
      StageOut(data32 addr);
      StageOut();
      const data32 addr;
  };

  class IFOut : public StageOut { 
    public:
      const instruction::Instruction instr;
      IFOut(data32 addr, const instruction::Instruction instr);
      IFOut();
  };
  /*
   * This represents the output of an Instruction Decode Phase. The first
   * element is the instruction that was being processed. The second element is
   * the register values that were loaded. This second element merits an
   * explanation.
   *
   * std::vector<mem::data32> regVals is a vector of loaded data values. It has
   * variable length because the number of registers loaded is dependent
   * upon the instruction. The order of the registers is the same as in the
   * machine encoding of the instruction. For example, an R-Type instruction
   * uses three registers, rs, rt, rd, so the vector will be {ld(rs), ld(rt),
   * ld(rd)}
   */
  class IDOut : public StageOut {
    public:
      const instruction::Instruction instr;
      const std::vector<mem::data32> regVals;
      IDOut(data32 addr, instruction::Instruction instr, 
          std::vector<mem::data32> regVals);
      IDOut();
  };

  /*
   * Ouput of an Execute pipeline phase.
   * Builds upon the IDOut by adding a comp field for the result of the
   * computation.
   */
  class EXOut : public StageOut {
    public:
      const instruction::Instruction instr;
      const std::vector<mem::data32> regVals;
      const mem::data64 comp;
      EXOut(data32 addr, Instruction instr, vector<data32> regVals,
          data64 comp);
      EXOut();
  };


  /*
   * Class to represent the output of a memory unit
   * This is copy and past nightmare. Exactly the smae out as a EX, but
   * sacrificing design at this point for development time
   */
  class MAOut : public StageOut {
    public:
      const instruction::Instruction instr;
      const std::vector<mem::data32> regVals;
      const mem::data64 comp;
      const mem::data32 loaded;
      MAOut(data32 addr, Instruction instr, vector<data32> regVals,
          data64 comp, data32 loaded);
      MAOut();
  };

  /*
   * Note that the address for WBOut is meaningless because it is never used.
   * here only for compatability
   */
  class WBOut : public StageOut {
    public:
      const bool quit;
      WBOut(data32 addr, bool quit);
      WBOut();
  };

  class PC {
    private:
      data32 index;
      string name;
      void logCurrentIndex();

    protected:
      string getName();
    public:
      PC(string name, data32 startIndex);

      /*
       * @returns struct with address at the current index
       * NOTE does not add to the index
       */
      StageOut* getOut() const;
      
      /*
       * sets the program counter to a new index
       * @param the index to set the program counter to
       */
      void set(data32 index);

      /*
       * sets the lower n bits to the passed value
       */
      void setLowBits(data32 index, unsigned char nBits);

      /*
       * increments the program counter by the given increment (in words)
       * @param the amount to increment
       */
      void inc(data32 increment);
  };

  /*
   * This is the core abstract class for a pipeline phase
   * TODO add destructor
   */
  class PipelinePhase{

    private:
      /*
       * return bool: True if all invariants are correct
       */
      bool checkInvariants() const;

      /*
       * checks to ensure that the the cyclesRemaining is >= 0
       * return bool: True if condition is true
       */
      bool checkCyclesRemaining() const;

      long nCyclesPassed;

    protected:
      /* This is used by logging to log the current address. You always
       * need to update this when you call execute (for bad design reasons)*/
      data32 currentAddr;
      /* name of this PipelinePhase */
      const std::string name;
      /* The number of cycles left before freed */
      int cyclesRemaining; 
      /*A logger to write the current update stage*/
      ofstream& log;

      /*
       * immediately set the number of remaining cycles to the current cycle
       * has some nice error checking and logging that makes this method
       * preferable to manual assignment in most circumstances
       * params:
       *   cycles: the number to set cyclesRemaining to.
       */
      void setCyclesRemaining(int cycles);

      /*
       * Initializes name to name. 
       * one clock cycle
       */
      PipelinePhase(std::string name, ofstream& log);

      /*
       * This is useful for code reuse. Most (all?) children will use this
       * function before updating arguments in execute
       * returns: true if safe to change the arguments, false if not
       */
      bool canUpdateArgs();

    public:

      /*
       * return string: the name of this PipelinePhase
       */
      virtual std::string getName() const;

      /*
       * return bool: True if this pipeline is processing it's current
       *   instruction
       */
      virtual bool isBusy() const;

      /*
       * This function is used to update the current cycle. This probably
       * involves decrementing a timer on the current operation
       * params:
       *   cycleChange: the number of clock cycles that have passed
       * Preconditions:
       *   cycle should never run below 0
       */
      virtual void updateCycle(int cycleChange);

      /*
       * This function does two things.
       * 1. It stores the arguments needed for this instruction
       * 3. It updates the cyclesRemaining
       * params: 
       *   args: the arguments from last stage. Should be a pointer to the
       *     pointer to the StageOut. The double pointer is necessary just
       *     so that you can make the original pointer NULL because you don't
       *     want the user to ever use args again after it's been passed to
       *     execute
       * returns:
       *   the instruction that was being worked on
       */
      virtual void execute(StageOut** args) = 0;

      virtual StageOut* getOut() = 0;

      virtual ~PipelinePhase() = default;
  };

  /*
   * Child of Pipeline, InstructionFetch
   * TODO add destructor
   */
  class InstructionFetch: public PipelinePhase{

    private:
      /* Note that this object does not have responisbility to clean mem*/
      mem::MemoryUnit& mem;
      /* arguments for the current instruction (output from previous stage) */
      StageOut* args;

    public:

      /*
       * name is initialized as specified,
       * instruction will be a bubble,
       * cyclesRemaining will be 0
       * params:
       *   name: the name of this InstructionFetch
       *   mem: the memory unit that this instruction fetch has access to
       */
      InstructionFetch(std::string name, mem::MemoryUnit& mem, ofstream& log);

      /*
       * This function does two things.
       * 1. It stores the arguments needed for this instruction
       * 2. It updates the cyclesRemaining
       * params: 
       *   args: of type PCOut containing the address to be looked up
       * returns:
       *   the instruction that was being worked on
       */
      void execute(StageOut** args);

      StageOut* getOut();

      virtual ~InstructionFetch();
  };


  /*
   * Much of the actual decoding of instructions is delegated to the Instruction
   * class, but this still leaves some things, most importantly register fetch,
   * which will have some effects on runtime. The primary purpose of this class
   * is to fetch the values from the register file and return them
   */
  class InstructionDecode: public PipelinePhase {
    private:
      mem::MemoryUnit& rf;
      IFOut* args;
      /*
       * loads a register give nthe required address
       * params:
       *   addr: the bits corresponding to the requested address
       * returns: the value in the register file at that address
       */
      mem::data32 loadReg(const std::bitset<5>& addr) const;
    
    public:

      InstructionDecode(std::string name, mem::MemoryUnit& rf, ofstream& log);

      /*
       * This function does two things.
       * 1. It stores the arguments needed for this instruction
       * 2. It updates the cyclesRemaining
       * params: 
       *   args: of type PCOut containing the address to be looked up
       * returns:
       *   the instruction that was being worked on
       */
      void execute(StageOut** args);

      /*
       * does necessary computation for whichever arguments are currently
       * stored.
       * returns:
       *   IDOut*, a pointer to struct with instruction and loaded registers.
       *   contents of regVals varies on instr type:
       *     R-Type: size 3, {rs, rt, rd}
       *     I-Type: size 2, {rs, rt/rd}
       *     J-Type: size 0
       */
      StageOut* getOut();

      virtual ~InstructionDecode();
  };
  
  /*
   * This is the class responsible for the execution phase of the pipeline
   * Decides what operation needs to be done and executes it.
   */
  class Execute: public PipelinePhase {
    private:
      IDOut* args;
      PC& pc; //TODO I was able to pass in an entire PC object, and assign
      //by initialization to this reference. How?

    public:
      Execute(std::string name, PC& pc, ofstream& log);

      /*
       * This function does two things.
       * 1. It stores the arguments needed for this instruction
       * 2. It updates the cyclesRemaining
       * params: 
       *   args: of type PCOut containing the address to be looked up
       * returns:
       *   the instruction that was being worked on
       */
      void execute(StageOut** args);

      /*
       * does necessary computation for whichever arguments are currently
       * stored.
       * returns:
       *   IDOut*, a pointer to struct with instruction and loaded registers.
       *   contents of regVals varies on instr type:
       *     R-Type: size 3, {rs, rt, rd}
       *     I-Type: size 2, {rs, rt/rd}
       *     J-Type: size 0
       */
      StageOut* getOut();

      virtual ~Execute();
  };

  /*
   * Memory Access Stage of pipeline is primarily responsible for
   * reading/writing data to main mem, but also does some stuff with Acc
   * and perhaps PC adds
   */
  class MemoryAccess: public PipelinePhase {
    private:
      MemoryUnit& mem;
      EXOut* args;

    public:
      /*
       * Note, this class will modify the mem you give it
       */
      MemoryAccess(std::string name, MemoryUnit& mem, ofstream& log);

      /*
       * This function does two things.
       * 1. It stores the arguments needed for this instruction
       * 2. It updates the cyclesRemaining
       * params: 
       *   args: of type PCOut containing the address to be looked up
       * returns:
       *   the instruction that was being worked on
       */
      void execute(StageOut** args);

      /*
       * does necessary computation for whichever arguments are currently
       * stored.
       * returns:
       *   IDOut*, a pointer to struct with instruction and loaded registers.
       *   contents of regVals varies on instr type:
       *     R-Type: size 3, {rs, rt, rd}
       *     I-Type: size 2, {rs, rt/rd}
       *     J-Type: size 0
       */
      StageOut* getOut();

      virtual ~MemoryAccess();
  };

  /*
   * WriteBack stage to simulate the final stage of 5 stage pipe
   */
  class WriteBack : public PipelinePhase {
    private:
      MAOut* args;
      mem::data64& acc;
      MemoryUnit& rf; // the registerfile
      PC& pc;

    public:
      WriteBack(std::string name, MemoryUnit& rf, data64& acc, PC& pc,
          ofstream& log);

      /*
       * This function does two things.
       * 1. It stores the arguments needed for this instruction
       * 2. It updates the cyclesRemaining
       * params: 
       *   args: of type EXOut
       * returns:
       *   the instruction that was being worked on
       */
      void execute(StageOut** args);

      /*
       * does necessary computation for whichever arguments are currently
       * stored.
       * returns:
       */
      StageOut* getOut();

      virtual ~WriteBack();
  };

}
#endif
