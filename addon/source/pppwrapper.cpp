// myobject.cc
#include "pppwrapper.h"
#include <string>
#include <node_buffer.h>
#include <node.h>
#include <uv.h>

using namespace addon;

using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::Persistent;
using v8::String;
using v8::Value;

std::string toStdString(v8::Local<v8::Value> arg)
{
    v8::String::Utf8Value value(arg->ToString());
    return std::string(*value);
}

struct WorkItemBase
{
    std::string error;
    std::string imageKey;
};

struct SetImageWorkItem : WorkItemBase
{
    uv_work_t  request;
    Persistent<Function> callback;

    // User data
    char *imageDataPtr;
    int imageDataLength;
    std::shared_ptr<PublicPppEngine> pppEngine;
};

struct DetectLandMarksWorkItem : WorkItemBase
{
    uv_work_t  request;
    Persistent<Function> callback;

    std::shared_ptr<PublicPppEngine> pppEngine;
    std::string landmarks;
};

struct CreateTilePrintWorkItem : WorkItemBase
{
    uv_work_t  request;
    Persistent<Function> callback;

    std::shared_ptr<PublicPppEngine> pppEngine;
    std::string printOptions;
    std::vector<byte> printPhoto;
};

Persistent<Function> PppWrapper::constructor;

PppWrapper::PppWrapper()
    : m_enginePtr(std::make_shared<PublicPppEngine>())
{
}

PppWrapper::~PppWrapper()
{
}


void PppWrapper::Init(Local<Object> exports)
{
    Isolate* isolate = exports->GetIsolate();

    // Prepare constructor template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "PppWrapper"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Prototype
    NODE_SET_PROTOTYPE_METHOD(tpl, "configure", Configure);
    NODE_SET_PROTOTYPE_METHOD(tpl, "setImage", SetImage);
    NODE_SET_PROTOTYPE_METHOD(tpl, "detectLandmarks", DetectLandMarks);
    NODE_SET_PROTOTYPE_METHOD(tpl, "createTilePrint", CreateTiledPrint);

    constructor.Reset(isolate, tpl->GetFunction());
    exports->Set(String::NewFromUtf8(isolate, "PppWrapper"),
        tpl->GetFunction());
}

void PppWrapper::New(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();

    if (args.IsConstructCall()) {
        // Invoked as constructor: `new PppWrapper(...)`
        auto obj = new PppWrapper();
        obj->Wrap(args.This());
        args.GetReturnValue().Set(args.This());
    }
    else
    {
        // Invoked as plain function `PppWrapper(...)`, turn into construct call.
        const auto argc = 1;
        Local<Value> argv[argc] = { args[0] };
        auto cons = Local<Function>::New(isolate, constructor);
        args.GetReturnValue().Set(cons->NewInstance(argc, argv));
    }
}

void PppWrapper::Configure(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    auto isolate = args.GetIsolate();
    if (args.Length() != 1 || !(args[0]->IsString() || args[0]->IsObject()))
    {
        v8::Exception::TypeError(String::NewFromUtf8(isolate, "configure() expects a JSON string or configuration object"));
    }
    // Get the configuration as a C++ std::string
    v8::String::Utf8Value jsstr(args[0]->ToString());
    std::string configurationJson(*jsstr);

    auto pppWrapper = ObjectWrap::Unwrap<PppWrapper>(args.This());
    pppWrapper->m_enginePtr->configure(configurationJson);
}

void PppWrapper::SetImage(const FunctionCallbackInfo<Value>& args)
{
    auto isolate = args.GetIsolate();
    if(args.Length() != 2
        || !args[0]->IsArrayBuffer()
        || !args[1]->IsFunction())
    {
        v8::Exception::TypeError(String::NewFromUtf8(isolate, "setImage() takes an array buffer (image data) and a callback function"));
    }
    auto pppWrapper = ObjectWrap::Unwrap<PppWrapper>(args.This());
    auto imageDataPtr = node::Buffer::Data(args[0]);
    auto imageDataLen = node::Buffer::Length(args[0]);
;
    Local<Function> callback = Local<Function>::Cast(args[1]);

    auto work = new SetImageWorkItem();
    work->request.data = work;
    work->imageDataPtr = imageDataPtr;
    work->imageDataLength = imageDataLen;
    work->pppEngine = pppWrapper->m_enginePtr;
    work->callback.Reset(isolate, callback);

    // Kick off the worker thread
    uv_queue_work(uv_default_loop(), &work->request,
        SetImageWorkAsync, SetImageWorkAsyncComplete);

    args.GetReturnValue().Set(Undefined(isolate));
}

void PppWrapper::DetectLandMarks(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    auto isolate = args.GetIsolate();
    if (args.Length() != 2 || !args[0]->IsString() || !args[1]->IsFunction())
    {
        v8::Exception::TypeError(String::NewFromUtf8(isolate,
            "detectLandMarks() takes 2 parameter: an image key of type string and a callback function"));
    }

    auto pppWrapper = ObjectWrap::Unwrap<PppWrapper>(args.This());
    auto work = new DetectLandMarksWorkItem();
    work->request.data = work;
    work->imageKey = toStdString(args[0]);
    work->pppEngine = pppWrapper->m_enginePtr;
    work->callback.Reset(isolate, Local<Function>::Cast(args[1]));

    // Kick off the worker thread
    uv_queue_work(uv_default_loop(), &work->request,
        DetectLandMarksWorkAsync, DetectLandMarksWorkAsyncComplete);

    args.GetReturnValue().Set(Undefined(isolate));
}

std::string PppWrapper::toJson(v8::Local<v8::Object> object)
{
    auto JSON = Local<Object>::Cast(object->CreationContext()->Global()
        ->Get(String::NewFromUtf8(object->GetIsolate(), "JSON")));
    auto stringify = Local<Function>::Cast(JSON->Get(String::NewFromUtf8(object->GetIsolate(), "stringify")));

    // Stringify the object
    // Same as using 'global.JSON.stringify.apply(global.JSON, [ obj ])
    Local<Value> args[] = { object };
    Local<String> result = Local<String>::Cast(stringify->Call(JSON, 1, args));
    return std::string(*v8::String::Utf8Value(result));
}

void PppWrapper::CreateTiledPrint(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    auto isolate = args.GetIsolate();
    if (args.Length() != 3 || !args[0]->IsString() || !args[1]->IsObject() || !args[2]->IsFunction())
    {
        v8::Exception::TypeError(String::NewFromUtf8(isolate,
            "detectLandMarks() takes 2 parameter: an image key of type string and a callback function"));
    }
    auto pppWrapper = ObjectWrap::Unwrap<PppWrapper>(args.This());
    auto work = new CreateTilePrintWorkItem();
    work->request.data = work;
    work->imageKey = toStdString(args[0]);
    work->printOptions =  toJson(Local<Object>::Cast(args[1]));
    work->pppEngine = pppWrapper->m_enginePtr;
    work->callback.Reset(isolate, Local<Function>::Cast(args[2]));

    uv_queue_work(uv_default_loop(), &work->request,
        CreateTilePrintWorkAsync, CreateTilePrintWorkAsyncComplete);

    args.GetReturnValue().Set(Undefined(isolate));
}



#pragma region Async Workers
v8::Local<v8::Primitive> GetWorkItemError(WorkItemBase * work)
{
    auto isolate = Isolate::GetCurrent();
    return work->error.empty()
        ? v8::Null(isolate)
        : static_cast<v8::Local<v8::Primitive>>(v8::String::NewFromUtf8(isolate, work->error.c_str()));
}

void PppWrapper::SetImageWorkAsync(uv_work_t* req)
{
    auto work = static_cast<SetImageWorkItem *>(req->data);
    try
    {
        work->imageKey = work->pppEngine->setImage(work->imageDataPtr, work->imageDataLength);
    }
    catch (const std::exception& ex)
    {
        work->error = ex.what();
    }
}

void PppWrapper::SetImageWorkAsyncComplete(uv_work_t* req, int status)
{
    auto isolate = Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate); // Required for Node 4.x

    auto work = static_cast<SetImageWorkItem *>(req->data);

    // Set up return arguments
    v8::Handle<Value> argv[] = { GetWorkItemError(work),  v8::String::NewFromUtf8(isolate, work->imageKey.c_str()) };

    // execute the callback
    Local<Function>::New(isolate, work->callback)->
        Call(isolate->GetCurrentContext()->Global(), 2, argv);

    // Free up the persistent function callback
    work->callback.Reset();
    delete work;
}

void PppWrapper::DetectLandMarksWorkAsync(uv_work_t* req)
{
    auto work = static_cast<DetectLandMarksWorkItem *>(req->data);
    try
    {
        work->landmarks = work->pppEngine->detectLandmarks(work->imageKey);
    }
    catch (const std::exception& ex)
    {
        work->error = ex.what();
    }
}

void PppWrapper::DetectLandMarksWorkAsyncComplete(uv_work_t* req, int status)
{
    auto isolate = Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate); // Required for Node 4.x

    auto work = static_cast<DetectLandMarksWorkItem *>(req->data);

    // set up return arguments
    auto landmarksStr = v8::String::NewFromUtf8(isolate, work->landmarks.data());
    auto landmarks = v8::JSON::Parse(landmarksStr);

    v8::Handle<Value> argv[] = { GetWorkItemError(work), landmarks};

    // execute the callback
    Local<Function>::New(isolate, work->callback)->
        Call(isolate->GetCurrentContext()->Global(), 2, argv);

    // Free up the persistent function callback
    work->callback.Reset();
    delete work;
}

void PppWrapper::CreateTilePrintWorkAsync(uv_work_t* req)
{
    auto work = static_cast<CreateTilePrintWorkItem *>(req->data);
    try
    {
        work->pppEngine->createTiledPrint(work->imageKey, work->printOptions, work->printPhoto);
    }
    catch (const std::exception& ex)
    {
        work->error = ex.what();
    }
}

void PppWrapper::CreateTilePrintWorkAsyncComplete(uv_work_t* req, int status)
{
    auto isolate = Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate); // Required for Node 4.x

    auto work = static_cast<CreateTilePrintWorkItem *>(req->data);

    // set up return arguments
    auto printPhoto = node::Buffer::Copy(isolate, reinterpret_cast<char *>(&work->printPhoto[0]), work->printPhoto.size());

    v8::Handle<Value> argv[] = { GetWorkItemError(work), printPhoto.ToLocalChecked()};

    // execute the callback
    Local<Function>::New(isolate, work->callback)->
        Call(isolate->GetCurrentContext()->Global(), 2, argv);

    // Free up the persistent function callback
    work->callback.Reset();
    delete work;
}
#pragma endregion
