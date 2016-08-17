// addon.cc
#include <node.h>
#include "pppwrapper.h"

using v8::Local;
using v8::Object; 

using namespace addon;

void InitAll(Local<Object> exports) {
  PppWrapper::Init(exports);
}

NODE_MODULE(addon, InitAll)

