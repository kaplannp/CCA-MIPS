#define BOOST_LOG_DYN_LINK
#include <string>
#include <boost/log/trivial.hpp>
#include <iostream>
#include <exception>
#include "Pipeline.h"
#include "Mem.h"

namespace pipeline{

  bool PipelinePhase::checkInvariants() const{
    return checkCyclesRemaining();
  }

  void PipelinePhase::setCyclesRemaining(int cycles){
   cyclesRemaining = cycles;
    BOOST_LOG_TRIVIAL(debug) << "<<" + getName() + ">> " << "setting cycle to "
      << cyclesRemaining << "." << std::endl;
    //It is required that cycles Remaining should never fall below 0
    assert(checkInvariants());
  }

  bool PipelinePhase::checkCyclesRemaining() const{
    bool invalidCycles = cyclesRemaining < 0;
    if(invalidCycles){
      BOOST_LOG_TRIVIAL(fatal) << "<<" << getName() << ">> " << 
        "cyclesRemaining has fallen below 0. "
        "You may have missed an event.";
    }
    return !invalidCycles;
  }

  std::string PipelinePhase::getName() const{
    return name;
  }

  bool PipelinePhase::isBusy() const{
    return cyclesRemaining > 0;
  }

  PipelinePhase::PipelinePhase(std::string name) : name(name){
    cyclesRemaining = 1;
  }

  void PipelinePhase::updateCycle(int cycleChange){
    // this function includes nice logging and error checking
    setCyclesRemaining(cyclesRemaining - cycleChange);
  }

  //TODO If I change the brackets to () I don't get compiler error, I get
  //linker obscure error?
  InstructionFetch::InstructionFetch(std::string name, mem::MemoryUnit& mem):
    PipelinePhase{ name },
    mem{ mem }{
    cyclesRemaining = 0;
  }

  void InstructionFetch::updateArgs(void* args){
    try{
      assert(!isBusy());
    } catch (std::exception& e){
      BOOST_LOG_TRIVIAL(fatal) << "<<" + getName() + ">> " <<
        "tried to execute, but Pipeline stage was busy. " << e.what();
      throw e;
    }
    this->args = *( (PCOut*) args );
    setCyclesRemaining(1);
  }


  void InstructionFetch::execute(void* args){
    updateArgs(args);
  }

  void* InstructionFetch::getOut(){
    unsigned int addr = args.addr;
    mem::data32 instrInt = mem.ld(addr);
    instruction::Instruction instr = instruction::Instruction(instrInt);
    IFOut* out = new IFOut(instr);
    out->instr = instr; //TODO gotta copy here
    return out;
  }

  InstructionDecode::InstructionDecode(std::string name, mem::MemoryUnit& rf) :
    PipelinePhase(name),
    rf{rf}
  {
    cyclesRemaining = 1;
  }

  mem::data32 InstructionDecode::loadReg(const std::bitset<6>& addr) const{
    unsigned long addr_int = addr.to_ulong();
    return rf.ld(addr_int);
  }

  void InstructionDecode::execute(void* args){
    this->args = *((IFOut*) args);
    BOOST_LOG_TRIVIAL(debug) << "<<" + getName() + ">> " << "executing args"
      " with instruction " << this->args.instr.toString() << std::endl;
  }

  void* InstructionDecode::getOut(){
    std::vector<mem::data32> regVals;
    if(args.instr.getType().find("R-Type") != std::string::npos){
      //Is R-Type
      regVals = std::vector<mem::data32>(3);
      std::bitset<6>* rs = args.instr.getSlice(21,26);
      std::bitset<6>* rt = args.instr.getSlice(16,21);
      std::bitset<6>* rd = args.instr.getSlice(11,16);
      regVals[0] = loadReg(*rs);
      regVals[1] = loadReg(*rt);
      regVals[2] = loadReg(*rd);
    } else{
      //Is I-Type
      regVals = std::vector<mem::data32>(2);
      std::bitset<6>* rs = args.instr.getSlice(21,26);
      //depending on the type, this next register may be rt or Rd. It doesn't
      //matter at this stage

      std::bitset<6>* rtOrRd = args.instr.getSlice(16,21);
      regVals[0] = loadReg(*rs);
      regVals[1] = loadReg(*rtOrRd);
    }
    IDOut* out = new IDOut(args.instr, regVals);
    return out;
  }

}
