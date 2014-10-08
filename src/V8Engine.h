#include <v8.h>
#include <libplatform/libplatform.h>
#include <string>

using namespace v8;
using namespace std;

class V8Engine {

public:
	V8Engine();
	~V8Engine();
	void run(string str);

private:
	v8::Platform* platform;
	Isolate* isolate;
	Persistent<Context> context;
	static void JSlog_notice(const v8::FunctionCallbackInfo<v8::Value>& args);


};