#include "utilCallJs.h"

void utilCallJs::ExecuteWork(napi_env env, void *data)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "StartThread %{public}s %{public}zu\n",__func__,std::this_thread::get_id() );
    CallbackData *callbackData = reinterpret_cast<CallbackData *>(data);
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "StartThreadxxxxxxx %{public}s %{public}zu %{public}u\n",__func__,std::this_thread::get_id() ,callbackData);

    std::promise<std::string>& promise = std::ref(callbackData->obj->prom);
    napi_call_threadsafe_function(callbackData->tsfn, &promise, napi_tsfn_nonblocking);
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
    CallbackData *callbackData = reinterpret_cast<CallbackData *>(context);
    napi_value undefined = nullptr;
    napi_value promise = nullptr;
    napi_value callbackArg[1] = {nullptr};
    
    napi_create_string_utf8(env, callbackData->parm.c_str(),NAPI_AUTO_LENGTH, &callbackArg[0]);
    napi_get_undefined(env, &undefined);
    napi_call_function(env, undefined, jsCb, 1, callbackArg, &promise);
//     napi_call_function(env, undefined, callback, 1, callbackArg, &result);
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

std::future<std::string> utilCallJs::executeJs(napi_env env, bool isMainThread,std::string& parm)
{
    this->callbackData->parm = parm;
    if (isMainThread) {
        // 在主线程中，创建异步队列
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "%{public}s::在线程%{public}u中，创建异步队列\n",__func__ ,std::this_thread::get_id());
        napi_queue_async_work(env, this->callbackData->work);
    }
    else {
        // 非主线程，直接执行
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "%{public}s::在线程%{public}u中，直接调用\n",__func__ ,std::this_thread::get_id());
        utilCallJs::ExecuteWork(env,this->callbackData);
    }
    return this->prom.get_future();
}

void utilCallJs::executeJs(napi_env env, bool isMainThread, getStr* cb)
{
        // 在主线程中，创建异步队列
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "%{public}s::在线程%{public}u中，创建异步队列\n",__func__ ,std::this_thread::get_id());
        napi_queue_async_work(env, this->callbackData->work);
}