#include "Pipeline.h"
#define BOOST_TEST_MODULE Pipeline Tests
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_CATCH_SYSTEM_ERRORS yes
#include <boost/test/unit_test.hpp>
// this runs it 'g++ test.cpp  -lboost_unit_test_framework'

/*
 * Just tests to make sure boose and header imports work as I expect
 */
BOOST_AUTO_TEST_SUITE( TestBoost )

BOOST_AUTO_TEST_CASE( test_header )
{
  pipeline::PipelinePhase* p = new pipeline::InstructionFetch("IF");
}

BOOST_AUTO_TEST_SUITE_END()
