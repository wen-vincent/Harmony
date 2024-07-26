//
// Created on 2024-07-26.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef HARMONY_PLUGINMANAGER_H
#define HARMONY_PLUGINMANAGER_H
#include <ace/xcomponent/native_interface_xcomponent.h>

#include <unordered_map>
#include "common/log_common.h"
#include "samples/sample_bitmap.h"
class PluginManager {
public:
    ~PluginManager();

    static PluginManager *GetInstance();

    void SetNativeXComponent(std::string &id, OH_NativeXComponent *nativeXComponent);
    SampleBitMap *GetRender(std::string &id);
    void Export(napi_env env, napi_value exports);
private:

    std::unordered_map<std::string, OH_NativeXComponent *> nativeXComponentMap_;
    std::unordered_map<std::string, SampleBitMap *> pluginRenderMap_;
};

#endif //HARMONY_PLUGINMANAGER_H
