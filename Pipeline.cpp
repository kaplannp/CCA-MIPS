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

  bool PipelinePhase::canUpdateArgs(){
    if(!isBusy()){
      return true;
    } else { 
      BOOST_LOG_TRIVIAL(fatal) << "<<" + getName() + ">> " <<
        "tried to execute, but Pipeline stage was busy. ";
      return false;
    }
  }

  void PipelinePhase::updateCycle(int cycleChange){
    // this function includes nice logging and error checking
    setCyclesRemaining(cyclesRemaining - cycleChange);
  }

  //TODO If I change the brackets to () I don't get compiler error, I get
  //linker obscure error?
  InstructionFetch::InstructionFetch(std::string name, mem::MemoryUnit& mem):
    PipelinePhase(name),
    mem{ mem }{
    cyclesRemaining = 0;
  }

  void InstructionFetch::execute(void* args){
    assert(canUpdateArgs());
    this->args = *( (PCOut*) args );
    setCyclesRemaining(1);
    BOOST_LOG_TRIVIAL(debug) << "<<" + getName() + ">> " << "executing args"
      " with address " << this->args.addr << std::endl;
  }

  void* InstructionFetch::getOut(){
    try{
      assert(!isBusy());
    } catch (std::exception& e){
      BOOST_LOG_TRIVIAL(fatal) << "<<" + getName() + ">> " <<
        "tried to getOut, but Pipeline stage was busy with " << 
        cyclesRemaining << ". The instruction you're trying to get output for"
        " may not be finished executing. " << e.what();
      throw e;
    }
    unsigned int addr = args.addr;
    mem::data32 instrInt = mem.ld(addr);
    instruction::Instruction instr = instruction::Instruction(instrInt);
    IFOut* out = new IFOut(instr);
    out->instr = instr; 
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
    assert(canUpdateArgs());
    this->cyclesRemaining = 1;
    this->args = *((IFOut*) args);
    BOOST_LOG_TRIVIAL(debug) << "<<" + getName() + ">> " << "executing args"
      " with instruction " << this->args.instr.toString() << std::endl;
  }

  void* InstructionDecode::getOut(){
    try{
      assert(!isBusy());
    } catch (std::exception& e){
      BOOST_LOG_TRIVIAL(fatal) << "<<" + getName() + ">> " <<
        "tried to getOut, but Pipeline stage was busy with " << 
        cyclesRemaining << ". The instruction you're trying to get output for"
        " may not be finished executing. " << e.what();
      throw e;
    }
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
    } else if(args.instr.getType().find("I-Type") != std::string::npos){
      //Is I-Type
      regVals = std::vector<mem::data32>(2);
      std::bitset<6>* rs = args.instr.getSlice(21,26);
      //depending on the type, this next register may be rt or Rd. It doesn't
      //matter at this stage
      std::bitset<6>* rtOrRd = args.instr.getSlice(16,21);
      regVals[0] = loadReg(*rs);
      regVals[1] = loadReg(*rtOrRd);
    } else {
      //Is J-Type
      regVals = std::vector<mem::data32>(0);
    }
    IDOut* out = new IDOut(args.instr, regVals);
    return out;
  }

  Execute::Execute(std::string name) : PipelinePhase(name){
    cyclesRemaining = 1;
  }

  void Execute::execute(void* args){
    assert(canUpdateArgs());
    this->args = *( (IDOut*) args );
    setCyclesRemaining(1);
    BOOST_LOG_TRIVIAL(debug) << "<<" + getName() + ">> " << "executing args"
      " with instruction " << this->args.instr.toString() << std::endl;
  }

  void* Execute::getOut(){
    mem::data64 comp;
    std::string instrType = args.instr.getType();
    std::cout << instrType << std::endl;
    if(instrType == "R-Type"){
      //Is R-Type
      //initialize some helpful vars for the func type
      std::string func = args.instr.getFuncType();
      mem::data32 rs = args.regVals[0];
      mem::data32 rt = args.regVals[1];
      mem::data32 rd = args.regVals[2];
      std::bitset<6>* shamtBits = args.instr.getSlice(6,11);
      mem::data32 shamt = shamtBits->to_ulong();
      delete shamtBits;
      if(func == "subu"){
        comp = (mem::data32) (rt-rs);
      } else if(func == "sub"){
        //no trapping so does same thing
        comp = (mem::data32) (rt-rs);
      } else if(func == "addu"){
        comp = (mem::data32) (rt+rs);
      } else if(func == "add"){
        //no trapping so does same thing
        comp = (mem::data32) (rt+rs);
      } else if(func == "sll"){ //shifts
        comp = (mem::data32) (rs << shamt);
      } else if(func == "sllv"){ 
        comp = (mem::data32) (rs << (rt & 0b11111)); //The & gets lower order 5 bits
      } else if(func == "srl"){ 
        comp = (mem::data32) (rs >> shamt);
      } else if(func == "srlv"){ 
        comp = (mem::data32) (rs >> (rt & 0b11111));
      } else if(func == "jr"){ 
        comp = (mem::data32) (rs);
      } else if(func == "and"){ //Logical
        comp = (mem::data32) (rs & rt);
      } else if(func == "or"){ 
        comp = (mem::data32) (rs | rt);
      } else if(func == "xor"){ 
        comp = (mem::data32) (rs ^ rt);
      } else if(func == "nor"){ 
        comp = (mem::data32) (~(rs | rt));
      } else if(func == "slt"){ //comparison
        comp = ((mem::signedData32) rs < (mem::signedData32) rt) ? 1 : 0;
      } else if(func == "sltu"){ 
        comp = (rs < rt) ? 1 : 0;
      } else if(func == "mult"){ 
        //Yes, this casting is pretty wild, let me speak it to you.
        //ya take in two unsigned 32 bit values, but you want to treat
        //them as signed (first cast). Then ya gotta make sure if you overflow
        //that is captured as long, (second cast)
        comp = (mem::signedData64)(mem::signedData32)rs * 
          (mem::signedData64)(mem::signedData32)rt;
      } else if(func == "multu"){ 
        comp = (mem::data64)rs * (mem::data64)rt;
      } else if(func == "div"){ 
        mem::data64 quotient = (mem::signedData64)rs / (mem::signedData64)rt;
        mem::data64 rem = (mem::signedData64)rs % (mem::signedData64)rt;
        comp = (rem << 32) + quotient;
      } else if(func == "divu"){ 
        mem::data64 quotient = (mem::data64)rs / (mem::data64)rt;
        mem::data64 rem = (mem::data64)rs % (mem::data64)rt;
        comp = (rem << 32) + quotient;
      }
      else {
        //Couldn't find the function specified, so unimplemented, so dying
        BOOST_LOG_TRIVIAL(fatal) << "<<" + getName() + ">> encountered"
          " unimplemented function type for R-Type instruction, " + func << "." 
          << std::endl;
        //TODO come up with a nicer exception
        throw std::exception();
      }
    }
    //Now we're in I instr land
    else if (instrType == "I-Type:beq"){
      comp = 9;
    }
    else {
      //Whatever the type of this instruction, it hasn't been implemented
      BOOST_LOG_TRIVIAL(fatal) << "<<" + getName() + ">> encountered" 
        " unimplemented instruction type, " + instrType + "." << std::endl;
      throw std::exception();
    }
    //You've done the heavy lifting at this point. Now you just assemble the
    //struct
    EXOut* exOut = new EXOut(args.instr, args.regVals, comp);
    return exOut;
  }

}
