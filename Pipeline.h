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
      const std::string name;
      int cyclesRemaining = 0; // The number of cycles left before freed

    public:
      /*
       * return string: the name of this PipelinePhase
       */
      std::string getName() const;

      /*
       * Constructor for initializing name
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
       * immediately set the number of remaining cycles to the current cycle
       * Precondtions:
       *   cycles remaining should be 0
       */
      void setCycle(int cycle);

      /*
       * process an incoming instruction.
       * params:
       *   instr: the instruction to be processed
       * Precondtions:
       *   isBusy should be false
       */
      virtual void processInstr(std::string instr) = 0;
  };



  /*
   * Child of Pipeline, InstructionFetch
   * TODO add destructor
   */
  class InstructionFetch: public PipelinePhase{

    private:
      std::string instr;

    public:

      /*
       * name is initialized as specified,
       * instruction will be a bubble,
       * cyclesRemaining will be 0
       */
      InstructionFetch(std::string name);


      /*
       * processes an incoming instruction.
       * throws:
       *   TODO do I lie here?
       *   AssertionException if processor is busy.
       */
      void processInstr(std::string instr);

  };

}
#endif
