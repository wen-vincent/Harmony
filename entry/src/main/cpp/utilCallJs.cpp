#include "utilCallJs.h"
// static std::promise<std::string> prom;
// static std::future<std::string> fu = prom.get_future();
 void utilCallJs::ExecuteWork(napi_env env, void *data)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "StartThread %{public}s %{public}zu\n",__func__,std::this_thread::get_id() );
    CallbackData *callbackData = reinterpret_cast<CallbackData *>(data);
    std::promise<std::string> promise;
    auto future = promise.get_future();
    napi_call_threadsafe_function(callbackData->tsfn, &promise, napi_tsfn_nonblocking);
            try {

        std::string result = future.get();
        callbackData->obj->prom.set_value(result);

                  char buf[32] = "asdasd";
        const char* cstr = result.c_str();
        std::copy(cstr, cstr + (result.length()+1), buf);
//         const char* info = result.c_str();
//         const char info[32] = "asdasd";
        const  char *buf2 = buf;
//         OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest StartThread %{public}s\n",buf2);
    } catch (const std::exception &e) {
//         OH_LOG_Print(LOG_APP, LOG_INFO,LOG_DOMAIN,"mytest, Result from JS %{public}s", e.what());
    }
}

napi_value utilCallJs::ResolvedCallback(napi_env env, napi_callback_info info)
{
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "StartThread %{public}s %{public}zu\n",__func__ ,std::this_thread::get_id());
    void *data = nullptr;
    size_t argc = 1;
    napi_value argv[1];
    if (napi_get_cb_info(env, info, &argc, argv, nullptr, &data) != napi_ok) {
        return nullptr;
    }
    size_t result = 0;
    char buf[32] = {0};
    napi_get_value_string_utf8(env, argv[0], buf, 32, &result);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "StartThread %{public}s %{public}zu %{public}s\n",__func__ ,result,buf);
    reinterpret_cast<std::promise<std::string> *>(data)->set_value(std::string(buf));
    return nullptr;
}

napi_value utilCallJs::RejectedCallback(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "StartThread %{public}s\n",__func__ );
    void *data = nullptr;
    if (napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &data) != napi_ok) {
        return nullptr;
    }
    reinterpret_cast<std::promise<std::string> *>(data)->set_exception(
        std::make_exception_ptr(std::runtime_error("Error in jsCallback")));
    return nullptr;
}

 void utilCallJs::CallJs(napi_env env, napi_value jsCb, void *context, void *data)
{
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "StartThread %{public}s %{public}zu\n",__func__,std::this_thread::get_id() );

    if (env == nullptr) {
        return;    
    }
    napi_value undefined = nullptr;
    napi_value promise = nullptr;
    napi_get_undefined(env, &undefined);
    napi_call_function(env, undefined, jsCb, 0, nullptr, &promise);
    napi_value thenFunc = nullptr;
    if (napi_get_named_property(env, promise, "then", &thenFunc) != napi_ok) {
        return;
    }
    napi_value resolvedCallback;
    napi_value rejectedCallback;
    napi_create_function(env, "resolvedCallback", NAPI_AUTO_LENGTH, ResolvedCallback, data,
                         &resolvedCallback);
    napi_create_function(env, "rejectedCallback", NAPI_AUTO_LENGTH, RejectedCallback, data,
                         &rejectedCallback);
    napi_value argv[2] = {resolvedCallback, rejectedCallback};
    napi_call_function(env, promise, thenFunc, 2, argv, nullptr);
    
}

 void utilCallJs::WorkComplete(napi_env env, napi_status status, void *data)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "StartThread %{public}s %{public}zu\n",__func__,std::this_thread::get_id() );
    CallbackData *callbackData = reinterpret_cast<CallbackData *>(data);
    napi_release_threadsafe_function(callbackData->tsfn, napi_tsfn_release);
    napi_delete_async_work(env, callbackData->work);
    callbackData->tsfn = nullptr;
    callbackData->work = nullptr;
    
}

napi_value utilCallJs::loadJs(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "StartThread %{public}s %{public}zu\n",__func__ ,std::this_thread::get_id());
    size_t argc = 1;
    napi_value jsCb = nullptr;
    CallbackData *callbackData = nullptr;
    napi_get_cb_info(env, info, &argc, &jsCb, nullptr, reinterpret_cast<void **>(&callbackData));

    // 创建一个线程安全函数
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, "Thread-safe Function Demo", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_threadsafe_function(env, jsCb, nullptr, resourceName, 0, 1, callbackData, nullptr, 
        callbackData, CallJs, &callbackData->tsfn);

    // 创建一个异步任务
    napi_create_async_work(env, nullptr, resourceName, ExecuteWork, WorkComplete, callbackData,&callbackData->work);
    
    callbackData->obj=this;
    this->callbackData = callbackData;
    return nullptr;
}

std::future<std::string> utilCallJs::executeJs(napi_env env)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "StartThread %{public}s %{public}zu\n",__func__ ,std::this_thread::get_id());

    // 将异步任务加入到异步队列中
    napi_queue_async_work(env, this->callbackData->work);
    
//     return nullptr;
//     return std::ref(fu);
    return this->prom.get_future();
}