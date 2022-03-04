#ifndef PIPELINE_H
#define PIPELINE_H
#include <string>
#include <assert.h>
#include <boost/log/trivial.hpp>

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
      /* current instruction being executed (Bubble indicates a bubble */
      std::string instr; 
      /* name of this PipelinePhase */
      const std::string name;
      /* The number of cycles left before freed */
      int cyclesRemaining; 
      /*
       * immediately set the number of remaining cycles to the current cycle
       * Precondtions:
       *   cycles remaining should be 0
       */
      void setCycle(int cycle);
      /*
       * process an incoming instruction by modifying current state, so now
       * you are working on the new instruction
       * params:
       *   instr: the instruction to be processed
       * Precondtions:
       *   isBusy should be false
       */
      virtual void updateInstr(std::string instr) = 0;

    public:

      /*
       * return string: the name of this PipelinePhase
       */
      std::string getName() const;

      /*
       * Initializes name to name. Current instruction is set to Bubble with
       * one clock cycle
       */
      PipelinePhase(std::string name);

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
       * 1. It returns the instruction that was being worked on (and should be
       *   finished. If that instruction is not finished, a call to this method
       *   will raise an Exception
       * 2. It updates it's current instruction to be the instruction that you
       *   are passing
       * params: 
       *   instr: the instruction to be processed
       * returns:
       *   the instruction that was being worked on
       */
      std::string processInstr(std::string instr);

      //TODO This line of code breaks it all why?
      //virtual ~PipelinePhase() = 0;
  };



  /*
   * Child of Pipeline, InstructionFetch
   * TODO add destructor
   */
  class InstructionFetch: public PipelinePhase{

    public:

      /*
       * name is initialized as specified,
       * instruction will be a bubble,
       * cyclesRemaining will be 0
       */
      InstructionFetch(std::string name);


      /*
       * updates the current instruction and sets time
       * throws:
       *   TODO do I lie here?
       *   AssertionException if processor is busy.
       */
      void updateInstr(std::string instr);

  };

}
#endif
