#include "V8Engine.h"
#include <cassert>

// Initialize the system-wide plaform
//Platform* V8Engine::platform=platform::CreateDefaultPlatform();
shared_ptr<Logger> V8Engine::logger=shared_ptr<Logger>();
int V8Engine::initialization_complete=0;

void V8Engine::initialize_v8(){


	// Initialize
	V8::InitializeICU();
	V8::InitializePlatform(platform::CreateDefaultPlatform());//V8Engine::platform);
	V8::Initialize();

	V8Engine::initialization_complete=1;
}


V8Engine::V8Engine() {

	// If you try to create instances of this class before calling V8Engine::initialize_v8, you will fail!
	assert(V8Engine::initialization_complete);

	// Set up isolate
	_isolate=Isolate::New();
	Isolate::Scope isolate_scope(_isolate);

	// Create a global object
	HandleScope handle_scope(_isolate);
	Handle<ObjectTemplate> global = ObjectTemplate::New(_isolate);

	// console.log
	Handle<ObjectTemplate> console = ObjectTemplate::New(_isolate);
	console->Set(String::NewFromUtf8(_isolate, "log"), FunctionTemplate::New(_isolate, JS_clog));
	global->Set(String::NewFromUtf8(_isolate, "console"), console);


	// Set up a permanent context with our global
	Handle<Context> context = Context::New(_isolate, NULL, global);
	_context.Reset(_isolate,context);

}

V8Engine::~V8Engine() {

	_context.Reset();
	_isolate->Dispose();

}

void V8Engine::run(string str){


	Isolate::Scope isolate_scope(_isolate);		// We get segfault if we don't do this

	// Create scope
	HandleScope handle_scope(_isolate);

	// Create a new handle bound to the permenent context
	Local<Context> context=Local<Context>::New(_isolate, _context);
	Context::Scope context_scope(context);

	// Create and run the script
	Handle<Script> script=Script::Compile(String::NewFromUtf8(_isolate, str.c_str()));
	TryCatch tc;
	Local<Value> result=script->Run();
	if(result.IsEmpty()) {
		Local<Value> exception =tc.Exception();
		String::Utf8Value str(exception);
		logger->err(*str);
		return;
	}

	// String::Utf8Value s(result);
	// printf("%s\n",*s);


}

void V8Engine::JS_clog(const FunctionCallbackInfo<Value>& args) {

	if(args.Length() <1 ) {

		return;
	}

	if(logger==NULL) {

		return;
	}

//	Isolate::Scope isolate_scope(args.GetIsolate());
	HandleScope scope(args.GetIsolate());
	Handle<Value> arg=args[0];
	String::Utf8Value value(arg);
	logger->notice(*value);
}

