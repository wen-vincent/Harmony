#ifndef HARMONY_UTILCALLJS_H
#define HARMONY_UTILCALLJS_H
#include "napi/native_api.h"
#include "hilog/log.h"
#include "future"
extern class utilCallJs;

struct CallbackData {
    napi_threadsafe_function tsfn;
    napi_async_work work;
    utilCallJs* obj;
    std::string parm;
};
typedef void(*getStr)(std::string);
class utilCallJs {
    public:
        utilCallJs(){}
        ~utilCallJs(){}
    public:
        napi_value loadJs(napi_env env, napi_callback_info info);
        std::future<std::string> executeJs(napi_env env,bool isMainThread,std::string& parm);
        void  executeJs(napi_env env, bool isMainThread,  getStr* cb);

    private:
        static void WorkComplete(napi_env env, napi_status status, void *data);
        static void CallJs(napi_env env, napi_value jsCb, void *context, void *data);
        static napi_value RejectedCallback(napi_env env, napi_callback_info info);
        static napi_value ResolvedCallback(napi_env env, napi_callback_info info);
        static void ExecuteWork(napi_env env, void *data);
    private:
        CallbackData * callbackData = nullptr;
        std::promise<std::string> prom;
        std::string parm;
};

#endif //HARMONY_UTILCALLJS_H
