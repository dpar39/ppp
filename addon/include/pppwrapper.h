#pragma once

#include <v8.h>
#include <node.h>
#include <node_object_wrap.h>
#include <memory>

#include <libppp.h>
#include <uv.h>


#ifdef LOGGER
#define DIAGLOG(x) ScopeLogger l_##x##_scope(x);
#define __DIAGNOSTIC__ DIAGLOG(__FUNCTION__)
#else
#define DIAGNOSTIC(msg) ;
#define __DIAGNOSTIC__ ;
#endif
#include <iostream>

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


    class ScopeLogger {
    public:
        ScopeLogger(const std::string& msg);

        ~ScopeLogger();
    private:
        std::string m_message;
        static int s_indent;
    };


}  // namespace demo
