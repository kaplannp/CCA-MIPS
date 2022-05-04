#define BOOST_LOG_DYN_LINK
#include <string>
#include <boost/log/trivial.hpp>
#include <iostream>
#include <exception>
#include "Pipeline.h"
#include "Mem.h"

using namespace std;
using namespace instruction;
using namespace mem;

namespace pipeline{

  //Out classes 
  StageOut::StageOut(data32 addr) : addr{addr}{}
  StageOut::StageOut() : addr{(data32)-1}{}

  IFOut::IFOut(data32 addr, const instruction::Instruction instr) : StageOut{addr},
    instr{instruction::Instruction(instr)} {};
  IFOut::IFOut() : instr{instruction::Instruction(0)}{};

  IDOut::IDOut(data32 addr,
      instruction::Instruction instr, std::vector<mem::data32> regVals)
    : StageOut{addr}, instr{instr}, regVals{regVals}{};
  IDOut::IDOut(): instr{instruction::Instruction(0)}, regVals(0){};

  EXOut::EXOut(data32 addr,
      instruction::Instruction instr, std::vector<mem::data32> regVals,
      mem::data64 comp) : StageOut{addr}, instr{instr}, regVals{regVals}, 
      comp{comp}{};
  EXOut::EXOut() : instr{instruction::Instruction(0)}, regVals(0), comp{0}{};

  MAOut::MAOut(data32 addr,
      instruction::Instruction instr, std::vector<mem::data32> regVals,
    mem::data64 comp, mem::data32 loaded) : StageOut{addr}, instr{instr}, 
    regVals{regVals}, comp{comp}, loaded{loaded}{};
  MAOut::MAOut() : instr{instruction::Instruction(0)}, regVals(0), comp{0},
    loaded{0}{};
  
  WBOut::WBOut(data32 addr, bool quit) : StageOut{addr}, quit{quit}{}
  WBOut::WBOut() : quit{false}{}

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

  PipelinePhase::PipelinePhase(std::string name, ofstream& log) : name(name),
      log{log}{
    nCyclesPassed = 0;
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
    int cyclesToSet = cyclesRemaining - cycleChange;
    if(cyclesToSet < 0){
      cyclesToSet = 0;
    }
    setCyclesRemaining(cyclesToSet);
    nCyclesPassed += cycleChange;
    log << "cycle:" << nCyclesPassed << "\tname:" << getName() << 
      "\taddr:" << currentAddr << endl;
  }


  //TODO If I change the brackets to () I don't get compiler error, I get
  //linker obscure error?
  InstructionFetch::InstructionFetch(std::string name, mem::MemoryUnit& mem,
      ofstream& log):
    PipelinePhase(name, log),
    mem{ mem }{
    cyclesRemaining = 0;
    args = nullptr;
  }

  void InstructionFetch::execute(StageOut** args){
    assert(canUpdateArgs());
    //Delete the old arguments
    delete this->args;
    //save the pointer to the arguments
    this->args = (*args);
    //Nullify the old pointer so user can't use it anymore
    *args = nullptr;
    setCyclesRemaining(1);
    BOOST_LOG_TRIVIAL(debug) << "<<" + getName() + ">> " << "executing args"
      " with address " << this->args->addr << std::endl;
    currentAddr = (this->args == nullptr ? (data32) -1 : this->args->addr);
  }

  StageOut* InstructionFetch::getOut(){
    IFOut* out = nullptr;
    if(args != nullptr){
      if(isBusy()){
        out = new IFOut();
      } else {
        unsigned int addr = args->addr;
        mem::data32 instrInt = mem.ld(addr);
        instruction::Instruction instr = instruction::Instruction(instrInt);
        out = new IFOut(addr, instr);
      }
    }
    return out;
  }

  InstructionFetch::~InstructionFetch(){
    delete args;
  }

  InstructionDecode::InstructionDecode(std::string name, mem::MemoryUnit& rf,
      ofstream& log) :
    PipelinePhase(name, log),
    rf{rf}
  {
    cyclesRemaining = 1;
    args=nullptr;
  }

  mem::data32 InstructionDecode::loadReg(const std::bitset<5>& addr) const{
    unsigned long addr_int = addr.to_ulong();
    return rf.ld(addr_int);
  }

  void InstructionDecode::execute(StageOut** args){
    assert(canUpdateArgs());
    //Delete the old arguments
    delete this->args;
    this->cyclesRemaining = 1;
    //save the pointer to the Out
    this->args = (IFOut*) *args;
    //nullptrIFY the users pointer
    *args = nullptr;
    BOOST_LOG_TRIVIAL(debug) << "<<" + getName() + ">> " << "executing args"
      " with instruction " << 
      (this->args == nullptr ? "null" : this->args->instr.toString())
      << std::endl;
    currentAddr = (this->args == nullptr ? (data32) -1 : this->args->addr);
  }

  StageOut* InstructionDecode::getOut(){
    IDOut* out = nullptr;
    if(args!=nullptr){
      if(isBusy()){
        //bubble
        out = new IDOut();
      } else {
        std::vector<mem::data32> regVals;
        if(args->instr.getType().find("R-Type") != std::string::npos){
          //Is R-Type
          regVals = std::vector<mem::data32>(3);
          std::bitset<5> rs = args->instr.getSlice<21,26>();
          std::bitset<5> rt = args->instr.getSlice<16,21>();
          std::bitset<5> rd = args->instr.getSlice<11,16>();
          regVals[0] = loadReg(rs);
          regVals[1] = loadReg(rt);
          regVals[2] = loadReg(rd);
        } else if(args->instr.getType().find("I-Type") != std::string::npos){
          //Is I-Type
          regVals = std::vector<mem::data32>(2);
          std::bitset<5> rs = args->instr.getSlice<21,26>();
          //depending on the type, this next register may be rt or Rd. It doesn't
          //matter at this stage
          std::bitset<5> rtOrRd = args->instr.getSlice<16,21>();
          regVals[0] = loadReg(rs);
          regVals[1] = loadReg(rtOrRd);
        } else {
          //Is J-Type
          regVals = std::vector<mem::data32>(0);
        }
        out = new IDOut(args->addr, args->instr, regVals);
      }
    }
    return out;
  }

  InstructionDecode::~InstructionDecode(){
    delete args;
  }

  Execute::Execute(std::string name, PC& pc, ofstream& log) : PipelinePhase(name, log), pc{pc}{
    cyclesRemaining = 1;
    args = nullptr;
  }

  void Execute::execute(StageOut** args){
    assert(canUpdateArgs());
    //Delete the old arguments
    delete this->args;
    //save the ptr to the struct
    this->args = (IDOut*) (*args);
    //Nullify the ptr for user cause they should never use again
    *args = nullptr;
    setCyclesRemaining(1);
    BOOST_LOG_TRIVIAL(debug) << "<<" + getName() + ">> " << "executing args"
      " with instruction " << 
      (this->args == nullptr ? "null" : this->args->instr.toString())
      << std::endl;
    currentAddr = (this->args == nullptr ? (data32) -1 : this->args->addr);
  }

  StageOut* Execute::getOut(){
    EXOut* out = nullptr;
    if(args!=nullptr){
      if(isBusy()){
        //bubble
        out = new EXOut();
      } else {
        mem::data64 comp;
        std::string instrType = args->instr.getType();
        if(instrType == "R-Type"){
          //Is R-Type
          //initialize some helpful vars for the func type
          std::string func = args->instr.getFuncType();
          mem::data32 rs = args->regVals[0];
          mem::data32 rt = args->regVals[1];
          mem::data32 rd = args->regVals[2];
          std::bitset<5> shamtBits = args->instr.getSlice<6,11>();
          mem::data32 shamt = shamtBits.to_ulong();
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
            comp = (mem::data32) (rs << (rt & 0b11111)); //&get low order 5 bits
          } else if(func == "srl"){ 
            comp = (mem::data32) (rs >> shamt);
          } else if(func == "srlv"){ 
            comp = (mem::data32) (rs >> (rt & 0b11111));
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
            //them as signed (first cast). Then ya gotta make sure if you
            //overflow
            //that is captured as long, (second cast)
            comp = (mem::signedData64)(mem::signedData32)rs * 
              (mem::signedData64)(mem::signedData32)rt;
          } else if(func == "multu"){ 
            comp = (mem::data64)rs * (mem::data64)rt;
          } else if(func == "div"){ 
            mem::data64 quotient = (mem::signedData32)rs/(mem::signedData32)rt;
            mem::data64 rem = (mem::signedData32)rs % (mem::signedData32)rt;
            comp = (rem << 32) + quotient;
          } else if(func == "divu"){ 
            mem::data64 quotient = rs / rt;
            mem::data64 rem = rs % rt;
            comp = (rem << 32) + quotient;
          } else if(func == "sra"){ 
            comp = (mem::data32) (((signedData32) rs) >> shamt);
          } else if(func == "srav"){ 
            comp = (mem::data32) (((signedData32) rs) >> (rt & 0b11111));
          } else if(func == "jalr"){ 
            StageOut* out = pc.getOut();
            data32 pcAddr = out->addr;
            delete out;
            comp = pcAddr + 2;
          } else {
            //If it is R-Type, but not those signatures, there is nothing to do
            comp = 0; 
          }
        }
        //Now we're in I instr land
        else if (instrType.find("I-Type") != std::string::npos){
          mem::data32 rs = args->regVals[0];
          mem::data32 rt = args->regVals[1];
          mem::data32 immediate = args->instr.getSlice<0,16>().to_ulong();
          if (instrType == "I-Type:beq"){
            comp = rs==rt;
          } else if (instrType == "I-Type:bne"){
            comp = rs != rt;
          } else if (instrType == "I-Type:addi"){
            comp = (mem::signedData32) rs + (short)immediate;
          } else if (instrType == "I-Type:addiu"){
            comp = (mem::signedData32) rs + (short)immediate;
          } else if (instrType == "I-Type:slti"){
            comp =  (mem::signedData32) rs < (short) immediate;
          } else if (instrType == "I-Type:sltiu"){
            comp = rs < (unsigned short)immediate;
          } else if (instrType == "I-Type:andi"){
            comp = (unsigned short) rs & (unsigned short) immediate;
          } else if (instrType == "I-Type:ori"){
            comp = (unsigned short) rs | (unsigned short) immediate;
          } else if (instrType == "I-Type:xori"){
            comp = (unsigned short) rs ^ (unsigned short) immediate;
          } else if (instrType == "I-Type:lui"){
            comp = ((unsigned short) immediate) << 16;
            //lb to lwu do same thing just an add
          } else if (instrType == "I-Type:lb"){
            comp = (mem::signedData32) rs + (short)immediate;
          } else if (instrType == "I-Type:lh"){
            comp = (mem::signedData32) rs + (short)immediate;
          } else if (instrType == "I-Type:lw"){
            comp = (mem::signedData32) rs + (short)immediate;
          } else if (instrType == "I-Type:lbu"){
            comp = (mem::signedData32) rs + (short)immediate;
          } else if (instrType == "I-Type:lhu"){
            comp = (mem::signedData32) rs + (short)immediate;
          } else if (instrType == "I-Type:lwu"){
            comp = (mem::signedData32) rs + (short)immediate;
          } else if (instrType == "I-Type:sb"){
            comp = (mem::signedData32) rt + (short)immediate;
          } else if (instrType == "I-Type:sh"){
            comp = (mem::signedData32) rt + (short)immediate;
          } else if (instrType == "I-Type:sw"){
            comp = (mem::signedData32) rt + (short)immediate;
          } else if (instrType == "I-Type:bltz"){
            comp = (mem::signedData32) rs < 0;
          }
        }
        else if (instrType.find("J-Type") != std::string::npos){
          //get current pc
          StageOut* out = pc.getOut();
          data32 pcAddr = out->addr;
          delete out;
          if (instrType == "J-Type:jal"){
            comp = pcAddr + 2;
          } 
          else {
            comp = 0;
          }
        }
        else {
          //Whatever the type of this instruction, it hasn't been implemented
          BOOST_LOG_TRIVIAL(fatal) << "<<" + getName() + ">> encountered" 
            " unimplemented instruction type, " + instrType + "." << std::endl;
          throw std::exception();
        }
        //You've done the heavy lifting at this point. Now you just assemble the
        //struct
         out = new EXOut(args->addr, args->instr, args->regVals, comp);
      }
    }
    return out;
  }

  Execute::~Execute(){
    delete args;
  }

  MemoryAccess::MemoryAccess(std::string name, mem::MemoryUnit& mem,
      ofstream& log) : PipelinePhase(name, log), mem{mem}{
      cyclesRemaining = 1;
      args = nullptr;
    }

  void MemoryAccess::execute(StageOut** args){
    assert(canUpdateArgs());
    //Delete the old arguments
    delete this->args;
    //save the ptr to the struct
    this->args = (EXOut*) *args;
    //Nullify the ptr for user cause they should never use again
    *args = nullptr;
    setCyclesRemaining(1);
    BOOST_LOG_TRIVIAL(debug) << "<<" + getName() + ">> " << "executing args"
      " with instruction " << 
      (this->args == nullptr ? "null" : this->args->instr.toString())
      << std::endl;
    currentAddr = (this->args == nullptr ? (data32) -1 : this->args->addr);
  }

  StageOut* MemoryAccess::getOut(){
    MAOut* out = nullptr;
    if(args!=nullptr){
      if(isBusy()){
        //bubble
        out = new MAOut();
      } else {
        mem::data32 loaded = 0;
        std::string instrType = args->instr.getType();
        if (instrType == "I-Type:sw"){
          mem::data32 rs = args->regVals[0];
          mem.sw(args->comp, rs);
        } else if (instrType == "I-Type:lw"){
          loaded = mem.ld((mem::data32) args->comp);
        }
        out = new MAOut(args->addr, args->instr, args->regVals, args->comp,
            loaded);
      }
    }
    return out;
  }

  MemoryAccess::~MemoryAccess(){
    delete args;
  }

  WriteBack::WriteBack(string name, MemoryUnit& rf, data64& acc, PC& pc,
      ofstream& log) : PipelinePhase(name, log), rf(rf), acc{acc}, pc{pc}
  {
      cyclesRemaining = 1;
      args = nullptr;
  }

  void WriteBack::execute(StageOut** args){
    assert(canUpdateArgs());
    //Delete the old arguments
    delete this->args;
    //save the ptr to the struct
    this->args = (MAOut*) *args;
    //Nullify the ptr for user cause they should never use again
    *args = nullptr;
    setCyclesRemaining(1);
    BOOST_LOG_TRIVIAL(debug) << "<<" + getName() + ">> " << "executing args"
      " with instruction " << 
      (this->args == nullptr ? "null" : this->args->instr.toString())
      << std::endl;
    currentAddr = (this->args == nullptr ? (data32) -1 : this->args->addr);
  }

  //TODO this is terribly named, you don't really want to get out here.
  //There is no out
  StageOut* WriteBack::getOut(){
    StageOut* out = new WBOut((data32) -1, false); // assume not quiting
    static const vector<string> simpleRInstrs = {
      "sub", "subu", "addu", "add", "sll", "sllv", "srl", "srlv", "and", "or",
      "xor", "nor", "srav","sra"
    };
    static const vector<string> simpleIInstrs = {
      "I-Type:addi", "I-Type:addiu", "I-Type:slti", "I-Type:sltiu", 
      "I-Type:andi", "I-Type:ori", "I-Type:xori", "I-Type:lui"
    };

    static const vector<string> loadIInstrs = {
      "I-Type:lb", "I-Type:lh", "I-Type:lw", 
      "I-Type:lbu", "I-Type:lhu", "I-Type:lwu"
    };

    if(args != nullptr){
      string instrType = args->instr.getType();
      data32 comp = args->comp;
      if(instrType == "R-Type"){
        //Is R-Type
        //initialize some helpful vars for the func type
        string func = args->instr.getFuncType();
        data32 rdAddr = args->instr.getSlice<11,16>().to_ulong();
        data32 rs = args->regVals[0];

        //check if instruction is simple
        bool isSimple = false;
        for(string s : simpleRInstrs){
          if(func == s){
            isSimple = true;
            break;
          }
        }

        if(func == "syscall"){
          //quiting 
          delete out;
          out = new WBOut((data32) -1, true);
        } else if(isSimple){
          rf.sw(rdAddr, comp);
        } else if(func == "jr"){ 
          pc.set(rs);
        } else if(func == "slt"){ //comparison
          data32 res = ((bool) comp) ? 1 : 0;
          rf.sw(rdAddr, res);
        } else if(func == "sltu"){ 
          data32 res = ((bool) comp) ? 1 : 0;
          rf.sw(rdAddr, res);
        } else if(func == "mult"){ 
          acc = comp;
        } else if(func == "multu"){ 
          acc = comp;
        } else if(func == "div"){ 
          acc = comp;
        } else if(func == "divu"){ 
          acc = comp;
        } else if(func == "mfhi"){ 
          rf.sw(rdAddr, acc >> 32); //sluff off the upper stuff
        } else if(func == "mflo"){ 
          rf.sw(rdAddr, (data32) acc); //truncate the higher order things
        } else if(func == "move"){
          rf.sw(rdAddr, rs);
        } else if(func == "jalr"){
          //load the return addr into segment
          rf.sw(rdAddr, (data32) comp);
          //set the program counter
          pc.set(rs);
        }
        //Otherwise, unseen r instr or r instr that we don't do anything for
      } else if(instrType.find("I-Type") != std::string::npos){
    //    //Now in I-Instr land
        data32 rdAddr = args->instr.getSlice<16,20>().to_ulong();
        data16 immediate = args->instr.getSlice<0,16>().to_ulong();
        data16 loaded = args->loaded;
        //check if instruction is simple
        bool isSimple = false;
        for(string s : simpleIInstrs){
          if(instrType == s){
            isSimple = true;
            break;
          }
        }

        //if it's not simple, then check if it is a load
        bool isLoad = false;
        if(!isSimple){
          for(string s : loadIInstrs){
            if(instrType == s){
              isLoad = true;
              break;
            }
          }
        }
        //TODO loads are not simple, dude
        if(isSimple){
          rf.sw(rdAddr, comp);
        } else if ((instrType == "I-Type:bne") || (instrType == "I-Type:beq") ||
            (instrType == "I-Type:bltz")){
          //branches
          if(comp){
            pc.set(immediate);
          }
        } else if (isLoad){
          rf.sw(rdAddr, loaded);
        } else if ((instrType == "I-Type:sb") || (instrType == "I-Type:sh") || 
            (instrType == "I-Type:sw")){
          //stores do nada
       }
      } else if(instrType.find("J-Type") != std::string::npos){
        data32 immediate = args->instr.getSlice<0,26>().to_ulong();
        if(instrType == "J-Type:j"){
          pc.setLowBits(immediate, 28);
        } else if(instrType == "J-Type:jal"){
          //load the return addr into Ra ($31)
          rf.sw(31, (data32) comp);
          //set the program counter
          pc.setLowBits(immediate, 28);
        }
      }
    }
    return out;
  }

  WriteBack::~WriteBack(){
    delete args;
  }

  void PC::logCurrentIndex(){
    BOOST_LOG_TRIVIAL(debug) << "<<" + getName() + ">> " << "current index "
      "is " << this->index << endl;
  }

  string PC::getName(){ return name; }

  PC::PC(string name, data32 startIndex) : name{ name }, index{startIndex} {}
  
  StageOut* PC::getOut() const {
    return new StageOut(index);
  }

  void PC::set(data32 index) {
    this->index = index;
    logCurrentIndex();
  }

  void PC::setLowBits(data32 index, unsigned char nBits){
    char sluffBits = 32 - nBits;
    data32 sluffedAdd = ((index << sluffBits) >> sluffBits);
    data32 sluffedIndex = ((this->index >> nBits) << nBits);
    data32 newIndex = sluffedIndex + sluffedAdd;
    set(newIndex);
  }
   
  void PC::inc(data32 increment) {
    this->index += increment;
    logCurrentIndex();
  }

}
