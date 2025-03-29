/*
* @Ref <br>
* 1- https://github.com/nodejs/node-addon-api/blob/main/doc/object_wrap.md <br>
* 2- https://github.com/nodejs/node-addon-api/blob/main/doc/object.md
* 3- https://github.com/nodejs/node-addon-api/blob/main/doc/class_property_descriptor.md
*/

//#include <unistd.h>         /* For unlink(2)/close(2) */
#include <fcntl.h>          /* For open(2), O_* constants */
#include <sys/file.h>       /* For flock(2) */
#include <string.h>         /* For strerror(3) */
#include <errno.h>
#include <string>           /* Pretty obvious */
#include <napi.h>
#include "./asyncworker.hpp"


//-----------------------CLASS----------------------//
class filelock : public Napi::ObjectWrap<filelock>
{
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);  //This function is required by Node (actually a helper function for actual Init)!
    void Finalize(Napi::Env env) override;                          //This must be public to be recognized by Napi
    filelock(const Napi::CallbackInfo& info);
    
  private:
    //-----------members
    int lockFD;
    //-----------funcs
    Napi::Value acquireReadLock(const Napi::CallbackInfo& info);
    Napi::Value acquireWriteLock(const Napi::CallbackInfo& info);
    Napi::Value removeLock(const Napi::CallbackInfo& info);
    //-----------Internal-funcs
    int createLockfile(std::string lockfileName);
};
//-----------------------CLASS----------------------//

//-------------------Internal-funcs-------------------//
int filelock::createLockfile(std::string lockfileName)
{
    return open( ("/run/lock/" + lockfileName).c_str(), O_RDWR | O_CREAT, 00600);
}

void filelock::Finalize(Napi::Env env)
{
    errno = 0;
    if( flock(lockFD, LOCK_UN) != 0 && errno != EBADF )
    {
        Napi::Error::New(env, strerror(errno)).ThrowAsJavaScriptException();
    }
    return;
}
//-------------------Internal-funcs-------------------//

//-----------------JS-Requirements-----------------//
Napi::Object filelock::Init(Napi::Env env, Napi::Object exports)
{
    // This method is used to hook the accessor and method callbacks
    Napi::Function functionList = DefineClass(env, "filelock",
    {
        InstanceMethod<&filelock::acquireReadLock>("acquireReadLock", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&filelock::acquireWriteLock>("acquireWriteLock", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&filelock::removeLock>("removeLock", static_cast<napi_property_attributes>(napi_writable | napi_configurable))
    });

    exports.Set("filelock", functionList);
    return exports;
}

// Helps calling multiple class member Init function at the top-level Init and have an addon with multiple classes!
Napi::Object Init (Napi::Env env, Napi::Object exports)
{
    return filelock::Init(env, exports);
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
//-----------------JS-Requirements-----------------//

//===========================================================================================================================================================================================
filelock::filelock(const Napi::CallbackInfo& info) : Napi::ObjectWrap<filelock>(info)
{
    Napi::Object jsFileHandle = info[0].As<Napi::Object>();
    lockFD = jsFileHandle.Get("fd").ToNumber().Int32Value();

    return;
}

Napi::Value filelock::acquireReadLock(const Napi::CallbackInfo& info)
{
    asyncworker* asyncLocker = new asyncworker(info.Env(), lockFD, std::string("lock_sh"));
    asyncLocker->Queue();
    return asyncLocker->deferred_promise.Promise();
}

Napi::Value filelock::acquireWriteLock(const Napi::CallbackInfo& info)
{
    asyncworker* asyncLocker = new asyncworker(info.Env(), lockFD, std::string("lock_ex"));
    asyncLocker->Queue();
    return asyncLocker->deferred_promise.Promise();
}

Napi::Value filelock::removeLock(const Napi::CallbackInfo& info)
{
    asyncworker* asyncLocker = new asyncworker(info.Env(), lockFD, std::string("lock_un"));
    asyncLocker->Queue();
    return asyncLocker->deferred_promise.Promise();
}

//===========================================================================================================================================================================================