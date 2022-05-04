#include "Pipeline.h"
#include "Mem.h"
#include "Instruction.h"
#include "Debug.h"
#include "Processor.h"

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

    //Test memory block loading
    size_t s3 = 50;
    MemoryUnit* m3 = new mem::DRAM(s3, "m3");
    data32 arr1[4] = {0,1,2,3};
    m3->storeBlock(0, arr1, 4);
    for(int i = 0; i < 4; i++){
      BOOST_CHECK_EQUAL(i, m3->ld(i));
    }

    data32 arr2[3] = {3,4,5};
    m3->storeBlock(3, arr2, 3);
    for(int i = 0; i < 3+3; i++){
      BOOST_CHECK_EQUAL(i, m3->ld(i));
    }

    delete m1;
    delete m2;
    delete m3;
  }

  BOOST_AUTO_TEST_CASE( TestVirtualMem ){
    size_t s1 = 1000;
    MemoryUnit* m1 = new VirtualMem(new mem::DRAM(s1, "m1"));

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

    //Test memory block loading
    size_t s3 = 50;
    MemoryUnit* m3 = new VirtualMem(new mem::DRAM(s3, "m3"));
    data32 arr1[4] = {0,1,2,3};
    m3->storeBlock(0, arr1, 4);
    for(int i = 0; i < 4; i++){
      BOOST_CHECK_EQUAL(i, m3->ld(i));
    }

    data32 arr2[3] = {3,4,5};
    m3->storeBlock(3, arr2, 3);
    for(int i = 0; i < 3+3; i++){
      BOOST_CHECK_EQUAL(i, m3->ld(i));
    }

    delete m1;
    delete m3;
  }

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( TestPipelinePhases )

  BOOST_AUTO_TEST_CASE( TestIF )
  {
    ofstream log;
    log.open("pipeline.log");
    //TODO would love to have m1 be a reference instead of a pointer
    mem::MemoryUnit* m1 = new mem::DRAM(100, "m1");
    //store a couple words in the DRAM
    m1->sw(0,5);
    m1->sw(5,10);
    pipeline::PipelinePhase* p = new pipeline::InstructionFetch("IF", *m1, log);
    BOOST_CHECK_EQUAL(p->getName(), "IF");
    pipeline::StageOut* args = new pipeline::StageOut(0);
    p->execute(&args);
    BOOST_CHECK(p->isBusy());
    p->updateCycle(1);
    BOOST_CHECK( !(p->isBusy()) );
    
    //Now, see if you can get the instructions right
    // expected instructions to be fetched
    instruction::Instruction instr1 = instruction::Instruction(5);
    instruction::Instruction instr2 = instruction::Instruction(10);
    pipeline::IFOut o1 = {0, instr1};
    pipeline::IFOut o2 = {0, instr2};
    pipeline::IFOut* trueOut = (pipeline::IFOut*) (p->getOut());
    BOOST_CHECK_EQUAL(trueOut->instr.getInstr(), o1.instr.getInstr());
    pipeline::StageOut* op2 = new pipeline::StageOut(5); //addr 5
    delete trueOut;
    p->execute(&op2);
    p->updateCycle(1);
    trueOut = (pipeline::IFOut*) (p->getOut());
    BOOST_CHECK_EQUAL(trueOut->instr.getInstr(), o2.instr.getInstr());
    delete trueOut;
    delete p;
    delete m1;
    log.close();
  }

  BOOST_AUTO_TEST_CASE( TestID ){
    ofstream log;
    log.open("pipeline.log");
    mem::MemoryUnit* rf1 = new mem::DRAM(100, "rf1");
    pipeline::PipelinePhase* id = new pipeline::InstructionDecode("ID",*rf1, 
        log);
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
    pipeline::StageOut* ifOut = new pipeline::IFOut(0, instr);
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
    pipeline::StageOut* ifOut2 = new pipeline::IFOut(0, instr2);
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
    pipeline::StageOut* ifOut3 = new pipeline::IFOut(0,instr3);
    id->execute(&ifOut3);
    id->updateCycle(1); //pass one timestep
    //check for correct output
    pipeline::IDOut* idOut3 = (pipeline::IDOut*) id->getOut();
    BOOST_CHECK(idOut3->regVals.empty());
    delete idOut3;
    delete id;
    delete rf1;
    log.close();
  }

  bool testIInstr(pipeline::PipelinePhase* ex, mem::data32 opcode, 
      mem::data32 rs, mem::data32 rtOrRD, mem::data32 val, 
      mem::data64 expected){
    //Note the 1, 2 would be addresses, but not necessary cause in this case
    //rs and rt are the actual data
    unsigned int instrVal = constructIInstr(opcode, 1, 2, val);
    instruction::Instruction instr = instruction::Instruction(instrVal);
    std::vector<mem::data32> regVals = {rs, rtOrRD};
    pipeline::StageOut* idOut = new pipeline::IDOut(0, instr, regVals);
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
    ofstream log;
    log.open("pipeline.log");
    PC pc = PC("PC", 0);
    pipeline::PipelinePhase* ex = new pipeline::Execute("EX", pc, log);
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
      runArgs(81,0,0,0,0x8,0), 
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
      //sra
      runArgs(-31,0,0,4,0x3,(data32)-2), //31 >> 4 = 1
      runArgs(59,0,0,31,0x3,0), //59>>31 = 0 *overflow
      runArgs(-8,0,0,3,0x3,(data32)-1), // 2^3 >> 3 = 1
      runArgs(5,0,0,0,0x3,5), //5 >> 0 = 5 
      //srav
      runArgs(31,4,0,0,0x7,1), //31 >> 4 = 1
      runArgs(-59,31,0,0,0x7,(data32)-1), //59>>31 = 0 *overflow
      runArgs(8,3,0,0,0x7,1), // 2^3 >> 3 = 1
      runArgs(5,0,0,0,0x7,5), //5 >> 0 = 5 
      //Jalr
      //It is assumed here that no instruction will change pc at execute stage
      //(this should be good assumption)
      runArgs(0,0,0,0,0x9,0+2) //5 >> 0 = 5 
    };
    pc.set(0);
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
      pipeline::StageOut* idOut = new pipeline::IDOut(0, instr, regVals);
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
    //bltz
    BOOST_CHECK(testIInstr(ex,0x1,1,0,0,0)); //expected is false
    BOOST_CHECK(testIInstr(ex,0x1,-1,0,0,1)); //expected is true
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
    StageOut* out = (StageOut*) pc.getOut();
    BOOST_CHECK(testIInstr(ex,0x3,0,0,0, out->addr + 2)); 
    delete out;
    //Objects get destructed when they go out of scope, but default destructor
    //of pointer is let it go away. Need delete to destruct object pointed to
    //use concrete types. No new.
    delete ex;
    log.close();
  }

  /*
   * Tests a memory load 
   */
  bool maTestLoad(MemoryUnit& mem, PipelinePhase* ma, data64 comp,
      data32 rs){
    //Note, RS, RD/RT have been read by now, and val was used during the execute
    //stage, so is not necessary here
    //for here
    data32 expected = mem.ld((data32) comp);
    unsigned int instrVal = constructIInstr(0x23, 1, 2, 3);
    Instruction instr = Instruction(instrVal);
    vector<mem::data32> regVals = {rs, 0};
    StageOut* exOut = new EXOut(0, instr, regVals, comp);
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
  bool maTestStore(MemoryUnit& mem, PipelinePhase* ma, data64 comp,
      data32 rs){
    //Note, RS, RD/RT have been read by now, and val was used during the execute
    //stage, so is not necessary here
    //for here
    data32 expected = mem.ld((data32) comp);
    unsigned int instrVal = constructIInstr(0x2b, 1, 2, 3);
    Instruction instr = Instruction(instrVal);
    vector<mem::data32> regVals = {rs, 0};
    StageOut* exOut = new EXOut(0, instr, regVals, comp);
    ma->execute(&exOut);
    ma->updateCycle(1);
    pipeline::MAOut* maOut = (MAOut*) ma->getOut();
    data32 mVal = mem.ld((data32) comp);
    bool result =  rs == mVal;
    if(!result){
      std::cout << "expected " << rs << ", but got " << mVal << std::endl;
    }
    delete exOut;
    return result;
  }

  BOOST_AUTO_TEST_CASE( TestMemoryAccess ){
    ofstream log;
    log.open("pipeline.log");
    size_t s1 = 10;
    MemoryUnit* m1 = new DRAM(s1, "MainMem1");
    PipelinePhase* ma = new MemoryAccess("MA", *m1, log);
    ma->updateCycle(1);
    //check loads
    m1->sw(5, 3141);
    BOOST_CHECK(maTestLoad(*m1,ma,5,0));
    m1->sw(5, 0);
    BOOST_CHECK(maTestLoad(*m1,ma,5,0));
    m1->sw(0, 42);
    BOOST_CHECK(maTestLoad(*m1,ma,0,0));
    //check stores
    BOOST_CHECK(maTestStore(*m1,ma,4, 9));
    BOOST_CHECK(maTestStore(*m1,ma,5, 0));
    BOOST_CHECK(maTestStore(*m1,ma,0, 9));
    log.close();
  }

  /*
   * Note rd should be passed by address not value
   * Note rs should be passed by value
   */
  WBOut* execRInstrWB(PipelinePhase* wb, 
      data8 rdAddr, data32 rs, data64 comp, data8 func){
    unsigned int instrVal = constructRInstr(1/*rs*/,2/*rt*/,rdAddr,
        4/*shamt*/,func);
    Instruction instr = Instruction(instrVal);
    StageOut* maOut = new MAOut(0, instr, {rs,1,2}, comp, 0);
    wb->execute(&maOut);
    wb->updateCycle(1);
    StageOut* wbOut = wb->getOut();
    return (WBOut*) wbOut;
  }

  bool testSimpleRInstrWB(PipelinePhase* wb, MemoryUnit* rf, 
      data8 rd, data64 comp, data8 func){

    delete execRInstrWB(wb,rd,1,comp,func);
    data32 rdData = rf->ld(rd);
    bool result = rdData == comp;
    if(!result){
      cout << "COMPARISON FAILED " << rdData << " not expected " << comp <<
        endl;
    }
    return result;
  }

  bool testAccRInstrWB(PipelinePhase* wb, data64& acc,
      data64 comp, data8 func){

    delete execRInstrWB(wb,1,1,comp,func);
    bool result = acc == comp;
    if(!result){
      cout << "COMPARISON FAILED " << acc << " not expected " << comp <<
        endl;
    }
    return result;
  }

  bool testSlt(PipelinePhase* wb, MemoryUnit* rf, 
      data8 rd, data64 comp, data8 func){

    delete execRInstrWB(wb,rd,1,comp,func);
    bool result = ((bool) comp) == ((bool) rf->ld(rd));
    if(!result){
      cout << "COMPARISON FAILED " << endl;
    }
    return result;
  }

  bool testAccMf(PipelinePhase* wb, MemoryUnit* rf,
      data8 rd, data64& acc, data8 func, data32 expected){
    
    delete execRInstrWB(wb,rd,1,42,func);
    data32 rdData = rf->ld(rd);
    bool result = rdData == expected;
    if(!result){
      cout << "COMPARISON FAILED " << rdData << " != expected " << expected <<
        "Impropper sluffing" << endl;
    }
    return result;
  }

  /*
   * used to build and execute an I instruction for writeback stage
   */
  WBOut* execIInstrWB(PipelinePhase* wb, data8 opcode,
      data8 rtOrRdAddr, data16 immediate, data64 comp, data32 loaded){
    unsigned int instrVal = constructIInstr(
        opcode,1/*rs*/,rtOrRdAddr,immediate);
    Instruction instr = Instruction(instrVal);
    StageOut* maOut = new MAOut(0, instr, {1,2}, comp, loaded);
    wb->execute(&maOut);
    wb->updateCycle(1);
    StageOut* wbOut = wb->getOut();
    return (WBOut*) wbOut;
  }

  
  bool testLoadIInstrWB(PipelinePhase* wb, MemoryUnit& rf, data8 rtOrRdAddr,
      data32 opcode, data32 loaded){
    delete execIInstrWB(wb, opcode, rtOrRdAddr, 42, 42, loaded);
    data32 result = rf.ld(rtOrRdAddr);
    if(result != loaded){
      cout << "expected " << loaded << ", but got " << result << endl;
    }
    return result == loaded;
  }

  bool testPcIInstrWB(PipelinePhase* wb, const PC& pc, data16 immediate,
      data32 opcode, data64 comp){
    StageOut* firstOut = (StageOut*) pc.getOut();
    delete execIInstrWB(wb, opcode, 42, immediate, comp, 42);
    StageOut* out = (StageOut*) pc.getOut();
    data32 result = out->addr;
    data32 expected = comp ? immediate : firstOut->addr; // If !comp, no change
    if(result != expected){
      cout << "expected " << expected << ", but got " << result << endl;
    }
    delete out;
    delete firstOut;
    return result == expected;
  }

  bool testSimpleIInstrWB(PipelinePhase* wb, MemoryUnit& rf, data8 rtOrRdAddr,
      data32 opcode, data32 comp){
    delete execIInstrWB(wb, opcode, rtOrRdAddr, 0, comp, 0);
    data32 result = rf.ld(rtOrRdAddr);
    if(result != comp){
      cout << "expected " << comp << ", but got " << result << endl;
    }
    return result == comp;
  }

  /*
   * used to build and execute an I instruction for writeback stage
   */
  WBOut* execJInstrWB(PipelinePhase* wb, data8 opcode,
      data32 immediate, data64 comp){
    unsigned int instrVal = constructJInstr(opcode, immediate);
    Instruction instr = Instruction(instrVal);
    StageOut* maOut = new MAOut(0, instr, {}, comp, 42);
    wb->execute(&maOut);
    wb->updateCycle(1);
    StageOut* wbOut = wb->getOut();
    return (WBOut*) wbOut;
  }

  BOOST_AUTO_TEST_CASE( TestWriteBack ){
    ofstream log;
    log.open("pipeline.log");
    PC pc = PC("PC", 0);
    MemoryUnit* rf = new DRAM(32, "RegFile");
    for(int i = 0; i < rf->getSize(); i++)
      rf->sw(i, i);
    data64 acc = 0l;
    PipelinePhase* wb = new WriteBack("WB", *rf, acc, pc, log);
      wb->updateCycle(1);
    //Test some simple instructions
    //sub
    BOOST_CHECK(testSimpleRInstrWB(wb, rf, 4/*rd*/, 1/*comp*/,0x22/*func*/));
    //sll
    BOOST_CHECK(testSimpleRInstrWB(wb, rf, 2/*rd*/, 50/*comp*/,0x0/*func*/));
    //add
    BOOST_CHECK(testSimpleRInstrWB(wb, rf, 4/*rd*/, 0/*comp*/,0x20/*func*/));

    //Test some Acc accessing instructions
    BOOST_CHECK(testAccRInstrWB(wb,acc,100,0x18));
    BOOST_CHECK(testAccRInstrWB(wb,acc,30,0x1b));
    BOOST_CHECK(testAccRInstrWB(wb,acc,4,0x1a));

    //Test slt R instrs
    BOOST_CHECK(testSlt(wb,rf,3,0,0x2a));
    BOOST_CHECK(testSlt(wb,rf,5,1,0x2a));
    BOOST_CHECK(testSlt(wb,rf,0,1,0x2b));
    
    //Test mfhi, mflo
    //mfhi
    acc = 2l<<32;
    BOOST_CHECK(testAccMf(wb,rf,9,acc,0x10, 2));
    acc = 0;
    BOOST_CHECK(testAccMf(wb,rf,3,acc,0x10, 0));
    acc = 89;
    BOOST_CHECK(testAccMf(wb,rf,8,acc,0x10, 0));
    //mflo
    acc = 53l<<32;
    BOOST_CHECK(testAccMf(wb,rf,2,acc,0x12, 0));
    acc = 0;
    BOOST_CHECK(testAccMf(wb,rf,5,acc,0x12, 0));
    acc = 67;
    BOOST_CHECK(testAccMf(wb,rf,9,acc,0x12, 67));
    
    //test move
    //TODO build and test
    delete execRInstrWB(wb, 7, 40, 42, 0x11);
    BOOST_CHECK_EQUAL(rf->ld(7), 40);
    delete execRInstrWB(wb, 2, 0, 42, 0x11);
    BOOST_CHECK_EQUAL(rf->ld(2), 0);


    //test a jump
    int addr = 0;
    StageOut* out = (StageOut*) pc.getOut();
    delete execRInstrWB(wb,0,addr,0,0x8);
    BOOST_CHECK_EQUAL(addr, out->addr);
    delete out;
    //test a jalr
    delete execRInstrWB(wb, 30, 9999, 2, 0x9);
    out = (StageOut*) pc.getOut();
    BOOST_CHECK_EQUAL(9999, out->addr);
    delete out;
    BOOST_CHECK_EQUAL(2, rf->ld(30));



    
    //I instructions
    
    //store has no effects. Just don't crash. And we'll check the pc for giggles
    StageOut* pcOut1 = (StageOut*) pc.getOut();
    WBOut* wbOut = execIInstrWB(wb, 0x29, 42, 42, 42, 42);
    StageOut* pcOut2 = (StageOut*) pc.getOut();
    BOOST_CHECK_EQUAL(pcOut1->addr, pcOut2->addr);
    BOOST_CHECK_EQUAL(wbOut->quit, false);
    delete pcOut2;
    delete pcOut1;
    delete wbOut;
    
    //simple ones
    BOOST_CHECK(testSimpleIInstrWB(wb, *rf, 5, 0x8, 8));
    BOOST_CHECK(testSimpleIInstrWB(wb, *rf, 6, 0xd, 99));
    BOOST_CHECK(testSimpleIInstrWB(wb, *rf, 9, 0xa, 0));
    
    //some simple pc branches
    BOOST_CHECK(testPcIInstrWB(wb, pc, 54, 0x4, 1));
    BOOST_CHECK(testPcIInstrWB(wb, pc, 60, 0x5, 1));
    BOOST_CHECK(testPcIInstrWB(wb, pc, 0, 0x4, 0));
    BOOST_CHECK(testPcIInstrWB(wb, pc, 46, 0x5, 0));

    //a few loads
    BOOST_CHECK(testLoadIInstrWB(wb, *rf, 6, 0x23, 99));
    BOOST_CHECK(testLoadIInstrWB(wb, *rf, 2, 0x23, 0));
    BOOST_CHECK(testLoadIInstrWB(wb, *rf, 7, 0x20, 5));
    BOOST_CHECK(testLoadIInstrWB(wb, *rf, 3, 0x20, 54));

    //Test some jump
    //JAL
    delete execJInstrWB(wb,0x3,32,40);
    StageOut* pcOut = (StageOut*) pc.getOut();
    data32 pcAddr = pcOut->addr;
    delete pcOut;
    BOOST_CHECK_EQUAL(32, pcAddr);
    BOOST_CHECK_EQUAL(40, rf->ld(31));
    //J
    pc.set(1<<31); 
    delete execJInstrWB(wb,0x2,99,42);
    pcOut = (StageOut*) pc.getOut();
    pcAddr = pcOut->addr;
    delete pcOut;
    BOOST_CHECK_EQUAL((1<<31) + 99, pcAddr);


    //check a syscall exit
    wbOut = execRInstrWB(wb, 42, 42, 42, 0xc);
    BOOST_CHECK_EQUAL(wbOut->quit, true);
    delete wbOut;
    
    delete wb;
    delete rf;
    log.close();
  }

  BOOST_AUTO_TEST_CASE( TestPC ){
    PC pc = PC("PC", 0);
    StageOut* out1 = (StageOut*) pc.getOut();
    BOOST_CHECK_EQUAL(out1->addr, 0);
    pc.set(60);
    StageOut* out2 = (StageOut*) pc.getOut();
    BOOST_CHECK_EQUAL(out2->addr, 60);
    pc.inc(3);
    StageOut* out3 = (StageOut*) pc.getOut();
    BOOST_CHECK_EQUAL(out3->addr, 63);
    
    //Test out the setting low bits
    pc.set(-1);
    pc.setLowBits(0, 28);
    StageOut* out4 = (StageOut*) pc.getOut();
    BOOST_CHECK_EQUAL(out4->addr, 15 << 28);
    pc.set(1 << 31);
    pc.setLowBits(-1, 31);
    StageOut* out5 = (StageOut*) pc.getOut();
    BOOST_CHECK_EQUAL(out5->addr, (data32) -1);
    
    delete out1;
    delete out2;
    delete out3;
    delete out4;
  }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( TestProcessor )
  BOOST_AUTO_TEST_CASE( TestInit ){
    MemoryUnit* mem = new DRAM(0x100, "MainMem");
    MemoryUnit* rf = new DRAM(0b100000, "RegisterFile");

    //store some values into registers
    rf->sw(1, 4);
    rf->sw(2, 5);
    data32 val = constructRInstr(
        1, 2, 3, 0, 0x21); //add R[3] = R[1]+R[2] (4+5 = 9)
    mem->sw(0, val);

    Processor5S p("MIPSProcessor", *mem, *rf, 0, "pipeline.log");
    for(int i = 0; i < 6; i++)
      BOOST_CHECK(!p.updateCycle(1));
    BOOST_CHECK_EQUAL(rf->ld(3), 9);

    //try a sequence
    data32 instrs[9] = {
      constructIInstr(0x23, 0, 5, 90), //ld mem[90] into rf[5]
      constructIInstr(0x23, 0, 6, 91), //ld mem[91] into rf[6]
      //stall with nops until the previous instructions have been written
      0, 0, 0, 0, 
      constructRInstr(5,6,0,0,0x18), //perform mult
      constructRInstr(0,0,7,0,0x12), //move low into 7
      constructRInstr(0,0,0,0,0xc) //syscall kill
    };
    //load in the instruction sequence
    mem->storeBlock(0, instrs, 9);
    //load some values into mem for the load
    mem->sw(90, 3);
    mem->sw(91, 4);
    p.start(0);
    //Should have computed mult
    BOOST_CHECK_EQUAL(rf->ld(5), 3);
    BOOST_CHECK_EQUAL(rf->ld(6), 4);
    BOOST_CHECK_EQUAL(rf->ld(7), 12);
    cout << "Congrats! YOU JUST WROTE AND EXECUTED A SUCCESSFUL MIPS PROGRAM!!!"
      << endl;
      

    delete mem;
    delete rf;
  }
  BOOST_AUTO_TEST_CASE( TestMachineCodeReader ){
    //initLog();
    MachineCodeFileReader reader = MachineCodeFileReader();
    reader.loadFile("out");
  }
BOOST_AUTO_TEST_SUITE_END()
