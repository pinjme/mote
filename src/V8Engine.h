// V8Engine
//
// Wraps the Google V8 engine in a way that makes it easy to multithread java code segents
//
// Each instance of this class contains one V8 instance and context.
//
// To use this class:
//
// 1. You will probably want to subclass V8Engine in order to get access to expected Javascript objects without some kind of lookup table
// 2. Before you create any instances of this class, MAKE SURE YOU CALL V8Engine::initialize_v8()!!!! 




#include <v8.h>
#include <libplatform/libplatform.h>
#include <string>
#include <boost/shared_ptr.hpp>
#include "Logger.h"

using boost::shared_ptr;
using namespace v8;
using namespace std;



class V8Engine {

public:
	static void initialize_v8();
	V8Engine();
	~V8Engine();
	void run(string str);
	static shared_ptr<Logger> logger;
	//static v8::Platform* platform;

private:
	static int initialization_complete;
	Isolate* _isolate;
	Persistent<Context> _context;
	static void JS_clog(const v8::FunctionCallbackInfo<v8::Value>& args);


};