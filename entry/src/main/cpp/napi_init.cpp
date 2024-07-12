#include "napi/native_api.h"
#include "hilog/log.h"
#include "future"
#include "utilCallJs.h"

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
static void getValue(std::string str)
{
            OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "StartThread %{public}s %{public}zu -----%{public}s %{public}s\n",__func__ ,std::this_thread::get_id(),"run 结束",str.c_str());

}

static napi_value run(napi_env env, napi_callback_info info,utilCallJs* calljs,bool isMainThread)
{
//     std::this_thread::sleep_for(std::chrono::seconds(2));

//     calljs->executeJs(env);

    std::string str;
    if(!isMainThread) {
        std::future<std::string> fu = calljs->executeJs(env,isMainThread);
        str = fu.get();
            OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "StartThread %{public}s %{public}zu -----%{public}s %{public}s\n",__func__ ,std::this_thread::get_id(),"run 结束",str.c_str());

    }
    else {
//         calljs->executeJs(env,isMainThread,(getStr*)getValue);
        std::future<std::string> fut = calljs->executeJs(env,isMainThread);
        std::chrono::milliseconds span(100);
        while (fut.wait_for(span) == std::future_status::timeout){
            OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "StartThread %{public}s %{public}zu -----%{public}s \n",__func__ ,std::this_thread::get_id(),"run 等待。。。");
        }
        str = fut.get();
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "StartThread %{public}s %{public}zu -----%{public}s %{public}s\n",__func__ ,std::this_thread::get_id(),"run 结束",str.c_str());
    }
    return nullptr;
}

static napi_value StartThread(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "StartThread %{public}s %{public}zu\n",__func__ ,std::this_thread::get_id());
    
    utilCallJs* calljs = new utilCallJs;
    calljs->loadJs(env, info);
    
    std::thread t(run,env,info,calljs,false);
    t.detach();
    
    //     std::async(run,env,info);
//         run(env,info,calljs,true);

    
    return nullptr;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    CallbackData *callbackData = new CallbackData();
    napi_property_descriptor desc[] = {
        { "add", nullptr, Add, nullptr, nullptr, nullptr, napi_default, nullptr },
        {"startThread", nullptr, StartThread, nullptr, nullptr, nullptr, napi_default, callbackData}
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
