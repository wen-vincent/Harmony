#include "napi/native_api.h"
#include "hilog/log.h"

static napi_value Add(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};

    napi_get_cb_info(env, info, &argc, args , nullptr, nullptr);

    napi_valuetype valuetype0;
    napi_typeof(env, args[0], &valuetype0);

    napi_valuetype valuetype1;
    napi_typeof(env, args[1], &valuetype1);

    double value0;
    napi_get_value_double(env, args[0], &value0);

    double value1;
    napi_get_value_double(env, args[1], &value1);

    napi_value sum;
    napi_create_double(env, value0 + value1, &sum);

    return sum;

}

struct CallbackData {
  napi_async_work asyncWork = nullptr;
  napi_ref callbackRef = nullptr;
  double args[2] = {0};
  double result = 0;
};

static void CompleteCB(napi_env env, napi_status status, void *data) 
{
    
    CallbackData *callbackData = reinterpret_cast<CallbackData *>(data);
    napi_value callbackArg[1] = {nullptr};
    napi_create_double(env, callbackData->result, &callbackArg[0]);
    napi_value callback = nullptr;
    napi_get_reference_value(env, callbackData->callbackRef, &callback);
    // 执行回调函数
    napi_value result;
    napi_value undefined;
    napi_get_undefined(env, &undefined);
    napi_value argv;
    napi_call_function(env, undefined, callback, 1, callbackArg, &result);

    int num;
    napi_get_value_int32(env, result, &num);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "GetMediasoupDevice %{public}d\n",num);
    // 删除napi_ref对象以及异步任务
    napi_delete_reference(env, callbackData->callbackRef);
    napi_delete_async_work(env, callbackData->asyncWork);
    delete callbackData;
}

static void ExecuteCB(napi_env env, void *data) 
{
    CallbackData *callbackData = reinterpret_cast<CallbackData *>(data);
    callbackData->result = callbackData->args[0] + callbackData->args[1];
}

napi_value AsyncWork(napi_env env, napi_callback_info info) 
{
    size_t argc = 3;
    napi_value args[3];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    auto asyncContext = new CallbackData();
    // 将接收到的参数保存到callbackData
    napi_get_value_double(env, args[0], &asyncContext->args[0]);
    napi_get_value_double(env, args[1], &asyncContext->args[1]);
    // 将传入的callback转换为napi_ref延长其生命周期，防止被GC掉
    napi_create_reference(env, args[2], 1, &asyncContext->callbackRef);
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, "asyncWorkCallback", NAPI_AUTO_LENGTH, &resourceName);
    // 创建异步任务
    napi_create_async_work(env, nullptr, resourceName, ExecuteCB, CompleteCB, 
                           asyncContext, &asyncContext->asyncWork); 
    // 将异步任务加入队列
    napi_queue_async_work(env, asyncContext->asyncWork);
    return nullptr;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        { "add", nullptr, Add, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "asyncWork", nullptr, AsyncWork, nullptr, nullptr, nullptr, napi_default, nullptr }
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END

static napi_module demoModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "entry",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterEntryModule(void)
{
    napi_module_register(&demoModule);
}
