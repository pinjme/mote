#include <v8.h>
#include <libplatform/libplatform.h>
#include <string>

using namespace v8;
using namespace std;

class V8Engine {

public:
	static void initialize_v8();
	V8Engine();
	~V8Engine();
	void run(string str);
	//static v8::Platform* platform;

private:
	Isolate* _isolate;
	Persistent<Context> _context;
	static void JSlog_notice(const v8::FunctionCallbackInfo<v8::Value>& args);


};