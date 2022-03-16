#ifndef PIPELINE_H_INCLUDED
#define PIPELINE_H_INCLUDED
#include <string>
#include <assert.h>
#include <boost/log/trivial.hpp>
#include <vector>

#include "Mem.h"
#include "Instruction.h"

#define BOOST_LOG_DYN_LINK

/*
 * The core class of this module is the PipelinePhase. This is a single stage
 * in a pipeline. It must implement the included methods.
 */
namespace pipeline{


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

    protected:
      /* name of this PipelinePhase */
      const std::string name;
      /* The number of cycles left before freed */
      int cyclesRemaining; 

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
      PipelinePhase(std::string name);

    public:

      /*
       * return string: the name of this PipelinePhase
       */
      std::string getName() const;

      /*
       * return bool: True if this pipeline is processing it's current
       *   instruction
       */
      bool isBusy() const;

      /*
       * This function is used to update the current cycle. This probably
       * involves decrementing a timer on the current operation
       * params:
       *   cycleChange: the number of clock cycles that have passed
       * Preconditions:
       *   cycle should never run below 0
       */
      void updateCycle(int cycleChange);

      /*
       * This function does two things.
       * 1. It stores the arguments needed for this instruction
       * 3. It updates the cyclesRemaining
       * params: 
       *   args: the arguments from last stage
       * returns:
       *   the instruction that was being worked on
       */
      virtual void execute(void* args) = 0;

      virtual void* getOut() = 0;
      //TODO This line of code breaks it all why?
      //virtual ~PipelinePhase() = 0;
  };



  typedef struct PCOut{
    unsigned int addr;
  } PCOut;

  //TODO gotta figure out how to destroy this now
  typedef struct IFOut{
    instruction::Instruction instr;
    IFOut(const instruction::Instruction& instr): instr{
      instruction::Instruction(instr)}{};
    //This is necessary for initialization of Decode
    IFOut(): instr{instruction::Instruction(0)}{};
  } IFOut;
  /*
   * Child of Pipeline, InstructionFetch
   * TODO add destructor
   */
  class InstructionFetch: public PipelinePhase{

    private:
      /* Note that this object does not have responisbility to clean mem*/
      mem::MemoryUnit& mem;
      /* arguments for the current instruction (output from previous stage) */
      PCOut args;

    public:

      /*
       * name is initialized as specified,
       * instruction will be a bubble,
       * cyclesRemaining will be 0
       * params:
       *   name: the name of this InstructionFetch
       *   mem: the memory unit that this instruction fetch has access to
       */
      InstructionFetch(std::string name, mem::MemoryUnit& mem);


      /*
       * updates the args and sets time
       * params:
       *   args: should be of type PCOut containing the address to be looked up
       * throws:
       *   TODO do I lie here?
       *   AssertionException if processor is busy.
       */
      void updateArgs(void* args);

      /*
       * This function does two things.
       * 1. It stores the arguments needed for this instruction
       * 2. It updates the cyclesRemaining
       * params: 
       *   args: of type PCOut containing the address to be looked up
       * returns:
       *   the instruction that was being worked on
       */
      //TODO why do I even have this? it just calls updateARgs. refactor
      void execute(void* args);

      void* getOut();

  };


  //TODO Not sure who has responsibility of destroying this, but someone better
  //do it
  typedef struct IDOut {
    instruction::Instruction instr;
    std::vector<mem::data32> regVals;
  } IDOut;

  /*
   * Much of the actual decoding of instructions is delegated to the Instruction
   * class, but this still leaves some things, most importantly register fetch,
   * which will have some effects on runtime. The primary purpose of this class
   * is to fetch the values from the register file and return them
   */
  class InstructionDecode: public PipelinePhase {
    private:
      mem::MemoryUnit& rf;
      IFOut args;
    
    public:

      InstructionDecode(std::string name, mem::MemoryUnit& rf);

      /*
       * This function does two things.
       * 1. It stores the arguments needed for this instruction
       * 2. It updates the cyclesRemaining
       * params: 
       *   args: of type PCOut containing the address to be looked up
       * returns:
       *   the instruction that was being worked on
       */
      void execute(void* args);

      void* getOut();

      /*
       * loads a register give nthe required address
       * params:
       *   addr: the bits corresponding to the requested address
       * returns: the value in the register file at that address
       */
      mem::data32 loadReg(const std::bitset<6>& addr) const;

  };
  

}
#endif
