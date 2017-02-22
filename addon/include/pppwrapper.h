#pragma once

#include <v8.h>
#include <node.h>
#include <node_object_wrap.h>
#include <memory>

#include <libppp.h>
#include <uv.h>

#ifndef DEBUG
#define DIAGNOSTIC(msg) std::cout << ((msg)) << std::endl;
#else
# define DIAGNOSTIC(msg) ;
#endif

namespace addon
{
    class PppWrapper : public node::ObjectWrap
    {
    private:
        std::shared_ptr<PublicPppEngine> m_enginePtr;

    public:
        static void Init(v8::Local<v8::Object> exports);

    private:
        explicit PppWrapper();
        ~PppWrapper();


    private:
        static v8::Persistent<v8::Function> constructor;

        static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
        static void Configure(const v8::FunctionCallbackInfo<v8::Value>& args);

        static void SetImage(const v8::FunctionCallbackInfo<v8::Value>& args);
        static void DetectLandMarks(const v8::FunctionCallbackInfo<v8::Value>& args);
        static std::string toJson(v8::Local<v8::Object> object);
        static void CreateTiledPrint(const v8::FunctionCallbackInfo<v8::Value>& args);

    private:
        static void SetImageWorkAsync(uv_work_t *req);
        static void SetImageWorkAsyncComplete(uv_work_t *req, int status);

        static void DetectLandMarksWorkAsync(uv_work_t *req);
        static void DetectLandMarksWorkAsyncComplete(uv_work_t *req, int status);

        static void CreateTilePrintWorkAsync(uv_work_t *req);
        static void CreateTilePrintWorkAsyncComplete(uv_work_t *req, int status);
    };



}  // namespace demo
