#include "V8Engine.h"

extern void log_notice(string message);
extern void fatal(string message);


V8Engine::V8Engine() {

	v8::V8::InitializeICU();
	platform=v8::platform::CreateDefaultPlatform();
	v8::V8::InitializePlatform(platform);
	v8::V8::Initialize();

	isolate=Isolate::New();
	Isolate::Scope isolate_scope(isolate);

	HandleScope handle_scope(isolate);

	// Create a global object
	// Handle<ObjectTemplate> global = ObjectTemplate::New(isolate);
	// global->Set(String::NewFromUtf8(isolate, "log_notice"), FunctionTemplate::New(isolate, JSlog_notice));

	// Set up a context with this global
//	Handle<Context> cx = Context::New(isolate, NULL, global);
	Handle<Context> cx = Context::New(isolate);
	context.Reset(isolate,cx);
	Context::Scope context_scope(cx);


}

V8Engine::~V8Engine() {

	context.Reset();
	isolate->Dispose();

}

void V8Engine::run(string str){

	//Isolate::Scope isolate_scope(isolate);

	HandleScope handle_scope(isolate);
	Context::Scope context_scope(context);

	Handle<Script> script=Script::Compile(String::NewFromUtf8(isolate, "\"whzsssup\"")); //str.c_str()));
	TryCatch tc;
	Local<Value> result=script->Run();
	if(result.IsEmpty()) {
		Local<Value> exception =tc.Exception();
		String::Utf8Value str(exception);
		fatal(*str);
	}

	String::Utf8Value s(result);
	printf("%s\n",*s);


}

void V8Engine::JSlog_notice(const v8::FunctionCallbackInfo<v8::Value>& args) {

	if(args.Length() <1 ) {

		return;
	}

//	Isolate::Scope isolate_scope(args.GetIsolate());
	HandleScope scope(args.GetIsolate());
	Handle<Value> arg=args[0];
	String::Utf8Value value(arg);
	log_notice(*value);
}

