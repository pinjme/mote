// The skeleton for this file was from https://code.google.com/p/googletest/wiki/Primer



#include "gtest/gtest.h"
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "Logger.h"
#include "V8Engine.h"
#include <boost/shared_ptr.hpp>

using namespace std;
using boost::shared_ptr;


namespace {

class V8EngineTest : public ::testing::Test {
 protected:

	V8EngineTest() {

		V8Engine::initialize_v8();
		logger=shared_ptr<Logger>(new Logger(""));
		logger->open_buffer();
		V8Engine::logger=logger;

	}

  virtual ~V8EngineTest() {
	// You can do clean-up work that doesn't throw exceptions here.
  }


  virtual void SetUp() {

  	e=new V8Engine();

  }

  virtual void TearDown() {
	// Code here will be called immediately after each test (right
	// before the destructor).
  }


	shared_ptr<Logger> logger;
	V8Engine* e;

};

TEST_F(V8EngineTest, ConsoleLogging) {

	// Check console.log
	e->run("console.log(\"this text was logged from the console\");");

	stringstream s;
	s << " this text was logged from the console" << endl;
	string expected_text=s.str();
	ASSERT_STREQ(logger->get_buffer().c_str(), expected_text.c_str());

}


  // // Read JS
  // ifstream f(__NAME__ ".js");
  // string js((istreambuf_iterator<char>(f)),(istreambuf_iterator<char>()));
  // f.close();
  // l->notice(__NAME__);

// // Tests that the Foo::Bar() method does Abc.
// TEST_F(FooTest, MethodBarDoesAbc) {
//   const string input_filepath = "this/package/testdata/myinputfile.dat";
//   const string output_filepath = "this/package/testdata/myoutputfile.dat";
//   Foo f;
//   EXPECT_EQ(0, f.Bar(input_filepath, output_filepath));
// }

// // Tests that Foo does Xyz.
// TEST_F(FooTest, DoesXyz) {
//   // Exercises the Xyz feature of Foo.
// }
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}





