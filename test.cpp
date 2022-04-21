#include "Pipeline.h"
#include "Mem.h"
#include "Instruction.h"
#include "Debug.h"

#define BOOST_TEST_MODULE Pipeline Tests
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_CATCH_SYSTEM_ERRORS yes

#include <boost/test/unit_test.hpp>
#include <exception>
#include <string>
#include <math.h>

using namespace mem;
using namespace pipeline;
using namespace std;
using namespace instruction;
// this runs it 'g++ test.cpp  -lboost_unit_test_framework'

BOOST_AUTO_TEST_SUITE( TestInstruction )

  BOOST_AUTO_TEST_CASE( TestRInstruction ){
    #define SIZE 13
    std::string types[SIZE] = {"R-Type", "J-Type:j", "J-Type:jal", "I-Type:beq", 
      "I-Type:bne", 
      "I-Type:addi", "I-Type:addiu", "I-Type:slti", "I-Type:sltiu", 
      "I-Type:andi", 
      "I-Type:ori", "I-Type:xori", "I-Type:lui"
    };
    int codes[SIZE] = {
      0x0, 0x2, 0x3, 0x4, 0x5, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf
    };
    for(int i = 0; i < SIZE; i++){
      instruction::Instruction instr = instruction::Instruction(
          codes[i] << 26
          );
      BOOST_CHECK(instr.toString().find(types[i]) != std::string::npos);
    }
    #undef SIZE
  }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( TestMemory )

  BOOST_AUTO_TEST_CASE( TestDRAM ){
    size_t s1 = 1000;
    mem::MemoryUnit* m1 = new mem::DRAM(s1, "m1");

    //test some simple store
    for(int i = 0; i < 10; i++)
      m1->sw(i, i);
    for(int i = 0; i < 10; i++)
      BOOST_CHECK_EQUAL(m1->ld(i), i);

    //test random stores
    m1->sw(459, 8);
    BOOST_CHECK_EQUAL(m1->ld(459), 8);
    m1->sw(999, 23); // edge of mem
    BOOST_CHECK_EQUAL(m1->ld(999), 23);

    size_t s2 = 6;
    mem::data32* mo2 = new mem::data32[s2];
    mo2[0] = 4;
    mo2[1] = 5;
    mo2[2] = 2;
    mo2[3] = 4;
    mo2[4] = 50;
    mo2[5] = 29;
    mem::MemoryUnit* m2 = new mem::DRAM(s2, mo2, "m2");
    for(int i = 0; i < s2; i++)
      BOOST_CHECK_EQUAL(m2->ld(i), mo2[i]);
    delete m1;
    delete m2;
  }

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( TestPipelinePhases )

  BOOST_AUTO_TEST_CASE( TestIF )
  {
    //TODO would love to have m1 be a reference instead of a pointer
    mem::MemoryUnit* m1 = new mem::DRAM(100, "m1");
    //store a couple words in the DRAM
    m1->sw(0,5);
    m1->sw(5,10);
    pipeline::PipelinePhase* p = new pipeline::InstructionFetch("IF", *m1);
    BOOST_CHECK_EQUAL(p->getName(), "IF");
    pipeline::StageOut* args = new pipeline::PCOut(0);
    p->execute(&args);
    BOOST_CHECK(p->isBusy());
    p->updateCycle(1);
    BOOST_CHECK( !(p->isBusy()) );
    
    //Now, see if you can get the instructions right
    // expected instructions to be fetched
    instruction::Instruction instr1 = instruction::Instruction(5);
    instruction::Instruction instr2 = instruction::Instruction(10);
    pipeline::IFOut o1 = {instr1};
    pipeline::IFOut o2 = {instr2};
    pipeline::IFOut* trueOut = (pipeline::IFOut*) (p->getOut());
    BOOST_CHECK_EQUAL(trueOut->instr.getInstr(), o1.instr.getInstr());
    pipeline::StageOut* op2 = new pipeline::PCOut(5); //addr 5
    delete trueOut;
    p->execute(&op2);
    p->updateCycle(1);
    trueOut = (pipeline::IFOut*) (p->getOut());
    BOOST_CHECK_EQUAL(trueOut->instr.getInstr(), o2.instr.getInstr());
    delete trueOut;
    delete p;
    delete m1;
  }

  BOOST_AUTO_TEST_CASE( TestID ){
    mem::MemoryUnit* rf1 = new mem::DRAM(100, "rf1");
    pipeline::PipelinePhase* id = new pipeline::InstructionDecode("ID",*rf1);
    id->updateCycle(1); //burn the bubble

    //Check an R-Type Instruction
    int rs = 1;
    int rt = 2;
    int rd = 4;
    int shamt = 0;
    int func = 0;
    int regVals[3] = {1,2,4};
    //write some registers
    rf1->sw(rs, regVals[0]);
    rf1->sw(rt, regVals[1]);
    rf1->sw(rd, regVals[2]);
    //create instruction and execute it
    unsigned int instructionVal = constructRInstr(rs, rt, rd, shamt, func);
    instruction::Instruction instr = instruction::Instruction(instructionVal);
    BOOST_CHECK(instr.toString().find("R-Type") != std::string::npos);
    pipeline::StageOut* ifOut = new pipeline::IFOut(instr);
    std::cout << "is busy is " << id->isBusy() << std::endl;
    id->execute(&ifOut);
    id->updateCycle(1); //pass one timestep
    //check for correct output
    pipeline::IDOut* idOut = (pipeline::IDOut*) id->getOut();
    for(int i = 0; i < 3; i++){
      BOOST_CHECK_EQUAL(idOut->regVals[i], regVals[i]);
    }
    delete idOut;
    //Check an I-Type Instruction
    int opcode2 = 4;
    int rs2 = 15;
    int rtOrRD = 30;
    int instrVal = 0;
    int regVals2[2] = {7,20};
    //write some registers
    rf1->sw(rs2, regVals2[0]);
    rf1->sw(rtOrRD, regVals2[1]);
    //create instruction and execute it
    unsigned int instructionVal2 = constructIInstr(opcode2, rs2, rtOrRD, 
        instrVal);
    instruction::Instruction instr2 = instruction::Instruction(instructionVal2);
    BOOST_CHECK(instr2.toString().find("I-Type") != std::string::npos);
    pipeline::StageOut* ifOut2 = new pipeline::IFOut(instr2);
    id->execute(&ifOut2);
    id->updateCycle(1); //pass one timestep
    //check for correct output
    pipeline::IDOut* idOut2 = (pipeline::IDOut*) id->getOut();
    for(int i = 0; i < idOut2->regVals.size(); i++){
      BOOST_CHECK_EQUAL(idOut2->regVals[i], regVals2[i]);
    }
    delete idOut2;
    //Check a J-Type Instruction
    int opcode3 = 2;
    int instrVal3 = 32;
    unsigned int instructionVal3 = constructJInstr(opcode3, instrVal3);
    instruction::Instruction instr3 = instruction::Instruction(instructionVal3);
    BOOST_CHECK(instr3.getType().find("J-Type") != std::string::npos);
    pipeline::StageOut* ifOut3 = new pipeline::IFOut(instr3);
    id->execute(&ifOut3);
    id->updateCycle(1); //pass one timestep
    //check for correct output
    pipeline::IDOut* idOut3 = (pipeline::IDOut*) id->getOut();
    BOOST_CHECK(idOut3->regVals.empty());
    delete idOut3;
    delete id;
    delete rf1;
  }

  bool testIInstr(pipeline::PipelinePhase* ex, mem::data32 opcode, 
      mem::data32 rs, mem::data32 rtOrRD, mem::data32 val, 
      mem::data64 expected){
    //Note the 1, 2 would be addresses, but not necessary cause in this case
    //rs and rt are the actual data
    unsigned int instrVal = constructIInstr(opcode, 1, 2, val);
    instruction::Instruction instr = instruction::Instruction(instrVal);
    std::vector<mem::data32> regVals = {rs, rtOrRD};
    pipeline::StageOut* idOut = new pipeline::IDOut(instr, regVals);
    ex->execute(&idOut);
    ex->updateCycle(1);
    pipeline::EXOut* exOut = (pipeline::EXOut*) ex->getOut();
    bool result =  exOut->comp == expected;
    if(!result){
      std::cout << "expected " << expected << ", but got " << exOut->comp 
        << std::endl;
    }
    delete exOut;
    return result;
  }

  BOOST_AUTO_TEST_CASE( TestExecute ){
    pipeline::PipelinePhase* ex = new pipeline::Execute("EX");
    ex->updateCycle(1); // Burn the bubble 
    typedef struct runArgs {
      mem::data32 rs;
      mem::data32 rt;
      mem::data32 rd;
      unsigned int shamt;
      unsigned int func;
      mem::data64 expected;
      runArgs( mem::data32 rs, mem::data32 rt, mem::data32 rd,
          unsigned int shamt, unsigned int func, mem::data64 expected) : 
          rs{rs}, rt{rt}, rd{rd}, shamt{shamt}, func{func}, expected{expected}{}
    } runArgs;
    //rs,rt,rd,shamt,func,expected
    std::vector<runArgs> runs = {
      //addu
      runArgs(2,4,0,0,0x21,6), //addu 2+4 = 6
      runArgs(-1,1,0,0,0x21,0), //addu -1+1 = 0 *note -1 = 4294967295
      runArgs(-1,1,0,0,0x21,0), //addu -1+1 = 0 *note -1 = 4294967295
      //add
      runArgs(-1,1,0,0,0x20,0), //addu -1+1 = 0 *note -1 = 4294967295
      //subu
      runArgs(2,4,0,0,0x23,2), //subu 4-2 = 2
      runArgs(1,0,0,0,0x23,(mem::data32)-1), //subu 0-1 = -1 *note -1 = 4294967295
      //sub
      runArgs(1,0,0,0,0x22,(mem::data32)-1), //subu 0-1 = -1 *note -1 = 4294967295
      //sll
      runArgs(1,0,0,31,0x0,std::pow(2,31)), //sll 1 << 31 = 2**31
      runArgs(2,0,0,31,0x0,0), //sll 2 << 31 = 0 *overflow
      runArgs(5,0,0,4,0x0,5<<4), //sll 5 << 4 = 5 << 4 
      runArgs(5,0,0,0,0x0,5), //sll 5 << 0 = 5 
      //sllv
      runArgs(2,4,0,0,0x4,2<<4), //2<<4 = 2<<4
      runArgs(1,31,0,0,0x4,(mem::data32)(1<<31)), //1<<31 
      runArgs(1,63,0,0,0x4,(mem::data32)(1<<31)), //rt is 6bit, 1<<31 (low bits)
      runArgs(9,0,0,0,0x4,9), //not shifting anything
      runArgs(2,31,0,0,0x4,0), //overflow, should get 0
      //srl
      runArgs(31,0,0,4,0x2,1), //31 >> 4 = 1
      runArgs(59,0,0,31,0x2,0), //59>>31 = 0 *overflow
      runArgs(8,0,0,3,0x2,1), // 2^3 >> 3 = 1
      runArgs(5,0,0,0,0x2,5), //5 >> 0 = 5 
      //srlv
      runArgs(31,4,0,0,0x6,1), //31 >> 4 = 1
      runArgs(59,31,0,0,0x6,0), //59>>31 = 0 *overflow
      runArgs(8,3,0,0,0x6,1), // 2^3 >> 3 = 1
      runArgs(5,0,0,0,0x6,5), //5 >> 0 = 5 
      runArgs(-1,63,0,0,0x6,1), //low bits of rt, 2**32-1>>31
      //jr
      runArgs(81,0,0,0,0x8,81), 
      runArgs(0,0,0,0,0x8,0), 
      runArgs(-1,0,0,0,0x8,(mem::data32)-1), 
      //and
      runArgs(0,0,0,0,0x24,0), 
      runArgs(-1,0,0,0,0x24,0), 
      runArgs(43,99,0,0,0x24,43&99), 
      //or
      runArgs(0,0,0,0,0x25,0), 
      runArgs(-1,0,0,0,0x25,(mem::data32)-1), 
      runArgs(43,99,0,0,0x25,43|99), 
      //xor
      runArgs(0,0,0,0,0x26,0), 
      runArgs(-1,0,0,0,0x26,(mem::data32)-1), 
      runArgs(43,99,0,0,0x26,43^99), 
      //nor
      runArgs(0,0,0,0,0x27,(mem::data32)~0), 
      runArgs(-1,0,0,0,0x27,~(-1)), 
      runArgs(43,99,0,0,0x27,(mem::data32)~(43|99)), 
      //slt
      runArgs(0,0,0,0,0x2a,0), // 0 < 0
      runArgs(-1,0,0,0,0x2a,1), // -1 < 0 signed
      runArgs(43,99,0,0,0x2a,1), // 43<99
      runArgs(99,43,0,0,0x2a,0), //99<43
      runArgs(-99,-43,0,0,0x2a,1), //-99<-43
      //sltu
      runArgs(0,0,0,0,0x2b,0), // 0 < 0
      runArgs(-1,0,0,0,0x2b,0), // -1 < 0 unsigned
      runArgs(43,99,0,0,0x2b,1), // 43<99
      runArgs(99,43,0,0,0x2b,0), //99<43
      runArgs(-43,-98,0,0,0x2b,0), //-99<-43 unsigned
    //rs,rt,rd,shamt,func,expected
      //mult
      runArgs(-43,-98,0,0,0x18,4214), //-99*-43 double neg
      runArgs(43,98,0,0,0x18,4214), //99*43 no neg
      runArgs(43,-98,0,0,0x18,-4214), //-99*43 one neg
      runArgs(0,-98,0,0,0x18,0), //-99*43 mul 0
      runArgs(2147483647,90,0,0,0x18,193273528230), //overflow 32, still in 64
      //multu
      runArgs(43,98,0,0,0x19,4214), //99*43 no neg
      runArgs(1,-1,0,0,0x19,(1l<<32) - 1), //1 * -1 is treated unsigned
      runArgs(0,-98,0,0,0x18,0), //-99*43 mul 0
      runArgs(2147483647,90,0,0,0x18,193273528230), //overflow 32, still in 64
      //div
      runArgs(100,10,0,0,0x1a,10), //simple div. No rem
      runArgs(100,-10,0,0,0x1a,-10), //neg div. No rem
      runArgs(-100,-10,0,0,0x1a,10), //2neg div. No rem
      runArgs(101,10,0,0,0x1a,(1l<<32) + 10), //div with rem 1
      runArgs(4,-1,0,0,0x1a,-4), //div with rem 1
      //divu
      runArgs(100,10,0,0,0x1b,10), //simple div. No rem
      runArgs(101,10,0,0,0x1a,(1l<<32) + 10), //div with rem 1
    };
    for(runArgs args : runs){
      //Note that rs, rt, rd addrs don't matter, only their values, so that
      //is what is stored here
      mem::data32 rs = args.rs; //val to invert
      mem::data32 rt = args.rt; //val not inverted
      mem::data32 rd = args.rd; //destination
      unsigned int shamt = args.shamt;
      unsigned int func = args.func;
      unsigned int instructionVal = constructRInstr(0, 0, 0, shamt, func);
      instruction::Instruction instr = instruction::Instruction(instructionVal);
      std::vector<mem::data32> regVals = {rs, rt, rd};
      pipeline::StageOut* idOut = new pipeline::IDOut(instr, regVals);
      ex->execute(&idOut);
      ex->updateCycle(1);
      pipeline::EXOut* exOut = (pipeline::EXOut*) ex->getOut();
      BOOST_CHECK_EQUAL(exOut->comp, args.expected);
      delete exOut;
    }
    //ex, opcode, rs, rtOrRD, val, expected
    //beq
    BOOST_CHECK(testIInstr(ex,0x4,1,1,0,1)); //expected is rs==rt
    //bne
    BOOST_CHECK(testIInstr(ex,0x5,1,1,0,0)); //expected is rs==rt
    //addi
    BOOST_CHECK(testIInstr(ex,0x8,1,0,1,2)); //1+1
    BOOST_CHECK(testIInstr(ex,0x8,1,0,-1,0)); //1+1
    BOOST_CHECK(testIInstr(ex,0x8,1,0,-2,-1l)); //1+1
    //addiu
    BOOST_CHECK(testIInstr(ex,0x9,1,0,1,2)); //1+1
    BOOST_CHECK(testIInstr(ex,0x9,1,0,-2,-1l)); //1+1
    //slti
    BOOST_CHECK(testIInstr(ex,0xa,1,0,90,1)); // 1 < 0
    BOOST_CHECK(testIInstr(ex,0xa,1,0,0,0)); // 1 > 0
    BOOST_CHECK(testIInstr(ex,0xa,0,0,0,0)); // 0 le 0, so false
    BOOST_CHECK(testIInstr(ex,0xa,9,0,-10,0)); // 9 < -10 false
    BOOST_CHECK(testIInstr(ex,0xa,9,0,-10,0)); // 9 < -10 false
    BOOST_CHECK(testIInstr(ex,0xa,-9,0,-10,0)); // -9 > -10 false
    //sltiu
    BOOST_CHECK(testIInstr(ex,0xb,1,0,90,1)); // 1 < 90 true
    BOOST_CHECK(testIInstr(ex,0xb,1,0,0,0)); // 1 < 0 false
    BOOST_CHECK(testIInstr(ex,0xb,0,0,-1,1)); // 0 < big num, so true
    BOOST_CHECK(testIInstr(ex,0xb,0,0,0,0)); // 0 !< 0 false
    //andi
    BOOST_CHECK(testIInstr(ex,0xc,5,0,5,5)); // 5&5 = 5
    BOOST_CHECK(testIInstr(ex,0xc,-1,0,5,5)); // True & 5 = 5
    BOOST_CHECK(testIInstr(ex,0xc,0,0,5,0)); // False &5 = 0
    BOOST_CHECK(testIInstr(ex,0xc,0,0,0,0)); // False & False = 0
    //ori
    BOOST_CHECK(testIInstr(ex,0xd,5,0,5,5)); // 5|5 = 5
    BOOST_CHECK(testIInstr(ex,0xd,-1,0,0,(1 << 16) - 1)); // True | False = -1
    BOOST_CHECK(testIInstr(ex,0xd,0,0,5,5)); // False | 5 = 5
    BOOST_CHECK(testIInstr(ex,0xd,0,0,0,0)); // False | False = 0
    //xori
    BOOST_CHECK(testIInstr(ex,0xe,-1,0,0,(1 << 16) - 1)); // True ^ False = -1
    BOOST_CHECK(testIInstr(ex,0xe,0,0,0,0)); // False ^ False = 0
    BOOST_CHECK(testIInstr(ex,0xe,-1,0,-1,0)); // True ^ True = 0
    BOOST_CHECK(testIInstr(ex,0xe,1,0,0,1)); // 1 ^ 0 = 1
    //lui (load upper immediate)
    BOOST_CHECK(testIInstr(ex,0xf,0,0,-1,((short) -1) << 16)); 
    BOOST_CHECK(testIInstr(ex,0xf,0,0,0,0)); // False ^ False = 0
    BOOST_CHECK(testIInstr(ex,0xf,0,0,5,5 << 16)); // True ^ True = 0
    //lw
    BOOST_CHECK(testIInstr(ex,0x23,1,0,-2,-1l)); //1-2
    BOOST_CHECK(testIInstr(ex,0x23,7,0,9,16)); //7+9
    //sw
    BOOST_CHECK(testIInstr(ex,0x2b,0,1,-2,-1l)); //1-2
    BOOST_CHECK(testIInstr(ex,0x2b,0,1,1,2)); //1+1
    //j/jal (always return 0. nothing to do for ALU)
    BOOST_CHECK(testIInstr(ex,0x2,0,0,40,0)); 
    BOOST_CHECK(testIInstr(ex,0x3,0,0,0,0)); 
    //LEAKS! why do I even need this?
    //Objects get destructed when they go out of scope, but default destructor
    //of pointer is let it go away. Need delete to destruct object pointed to
    //use concrete types. No new.
    delete ex;
  }

  /*
   * Tests a memory load 
   */
  bool maTestLoad(MemoryUnit* mem, PipelinePhase* ma, data64 comp,
      data32 rs){
    //Note, RS, RD/RT have been read by now, and val was used during the execute
    //stage, so is not necessary here
    //for here
    data32 expected = mem->ld((data32) comp);
    unsigned int instrVal = constructIInstr(0x23, 1, 2, 3);
    Instruction instr = Instruction(instrVal);
    vector<mem::data32> regVals = {rs, 0};
    StageOut* exOut = new EXOut(instr, regVals, comp);
    ma->execute(&exOut);
    ma->updateCycle(1);
    pipeline::MAOut* maOut = (MAOut*) ma->getOut();
    bool result =  maOut->loaded == expected;
    if(!result){
      std::cout << "expected " << expected << ", but got " << maOut->comp 
        << std::endl;
    }
    delete exOut;
    return result;
  }

  /*
   * Tests a Memory store
   */
  bool maTestStore(MemoryUnit* mem, PipelinePhase* ma, data64 comp,
      data32 rs){
    //Note, RS, RD/RT have been read by now, and val was used during the execute
    //stage, so is not necessary here
    //for here
    data32 expected = mem->ld((data32) comp);
    unsigned int instrVal = constructIInstr(0x2b, 1, 2, 3);
    Instruction instr = Instruction(instrVal);
    vector<mem::data32> regVals = {rs, 0};
    StageOut* exOut = new EXOut(instr, regVals, comp);
    ma->execute(&exOut);
    ma->updateCycle(1);
    pipeline::MAOut* maOut = (MAOut*) ma->getOut();
    data32 mVal = mem->ld((data32) comp);
    bool result =  rs == mVal;
    if(!result){
      std::cout << "expected " << rs << ", but got " << mVal << std::endl;
    }
    delete exOut;
    return result;
  }

  BOOST_AUTO_TEST_CASE( TestMemoryAccess ){
    size_t s1 = 10;
    MemoryUnit* m1 = new DRAM(s1, "MainMem1");
    PipelinePhase* ma = new MemoryAccess("MA", m1);
    ma->updateCycle(1);
    //check loads
    m1->sw(5, 3141);
    BOOST_CHECK(maTestLoad(m1,ma,5,0));
    m1->sw(5, 0);
    BOOST_CHECK(maTestLoad(m1,ma,5,0));
    m1->sw(0, 42);
    BOOST_CHECK(maTestLoad(m1,ma,0,0));
    //check stores
    BOOST_CHECK(maTestStore(m1,ma,4, 9));
    BOOST_CHECK(maTestStore(m1,ma,5, 0));
    BOOST_CHECK(maTestStore(m1,ma,0, 9));
  }

  //BOOST_AUTO_TEST_CASE( TestWriteBack ){
  //  PipelinePhase* wb = new WriteBack("WB");
  //  wb->updateCycle(1);
  //  data8 rd = 4;
  //  unsigned int instrVal = constructRInstr(1/*rs*/,2/*rt*/,rd,4/*shamt*/,func)
  //  Instruction instr = new Instruction(instrVal);
  //  StageOut* exOut = new EXOut(instr, 
  //}

BOOST_AUTO_TEST_SUITE_END()
