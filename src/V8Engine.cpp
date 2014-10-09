#include "V8Engine.h"

extern void log_notice(string message);
extern void fatal(string message);

// Initialize the system-wide plaform
//Platform* V8Engine::platform=platform::CreateDefaultPlatform();

void V8Engine::initialize_v8(){

	// Initialize
	V8::InitializeICU();
	V8::InitializePlatform(platform::CreateDefaultPlatform());//V8Engine::platform);
	V8::Initialize();


}


V8Engine::V8Engine() {


	// Set up isolate
	_isolate=Isolate::New();
	Isolate::Scope isolate_scope(_isolate);

	// Create a global object
	HandleScope handle_scope(_isolate);
	Handle<ObjectTemplate> global = ObjectTemplate::New(_isolate);
	global->Set(String::NewFromUtf8(_isolate, "log_notice"), FunctionTemplate::New(_isolate, JSlog_notice));

	// Set up a permanent context with our global
	Handle<Context> context = Context::New(_isolate, NULL, global);
	_context.Reset(_isolate,context);
//	Context::Scope context_scope(cx);

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
	Handle<Script> script=Script::Compile(String::NewFromUtf8(_isolate, "log_notice('Hey there!!!');")); //str.c_str()));
	TryCatch tc;
	Local<Value> result=script->Run();
	if(result.IsEmpty()) {
		Local<Value> exception =tc.Exception();
		String::Utf8Value str(exception);
		fatal(*str);
	}

	// String::Utf8Value s(result);
	// printf("%s\n",*s);


}

void V8Engine::JSlog_notice(const FunctionCallbackInfo<Value>& args) {

	if(args.Length() <1 ) {

		return;
	}

//	Isolate::Scope isolate_scope(args.GetIsolate());
	HandleScope scope(args.GetIsolate());
	Handle<Value> arg=args[0];
	String::Utf8Value value(arg);
	log_notice(*value);
}

