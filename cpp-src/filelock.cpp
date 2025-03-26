/*
* @Ref <br>
* 1- https://github.com/nodejs/node-addon-api/blob/main/doc/object_wrap.md <br>
* 2- https://github.com/nodejs/node-addon-api/blob/main/doc/object.md
* 3- https://github.com/nodejs/node-addon-api/blob/main/doc/class_property_descriptor.md
*/

#include <iostream>
#include <stdio.h>
#include <unistd.h>         /* For unlink(2)/close(2) */
#include <fcntl.h>          /* For O_* constants */
#include <sys/file.h>       /* For flock(2) */
#include <string>           /* Pretty obvious */
#include <errno.h>
#include <napi.h>


//-----------------------CLASS----------------------//
class filelock : public Napi::ObjectWrap<filelock>, public Napi::AsyncWorker
{
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);  //This function is required by Node (actually a helper function for actual Init)!
    void Finalize(Napi::Env env) override;                     //This must be public to be recognized by Napi
    filelock(const Napi::CallbackInfo& info);
    void Execute() override;
    void OnOK() override;
    void OnError(const Napi::Error& err) override;
    Napi::Promise::Deferred deferred_promise;
    
  private:
    //-----------members
    const std::string lockfiles_dir;
    int lockFD;
    std::string requested_operation;
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
    return open( ("/run/lock/" + lockfiles_dir + lockfileName).c_str(), O_RDWR | O_CREAT, 00700);
}

void filelock::Finalize(Napi::Env env)
{
    flock(lockFD, LOCK_UN);
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
filelock::filelock(const Napi::CallbackInfo& info) : Napi::ObjectWrap<filelock>(info), Napi::AsyncWorker(info.Env()), deferred_promise(info.Env()), lockfiles_dir("IDL/")
{
    Napi::Env env = info.Env();
    Napi::Object jsFileHandle = info[0].As<Napi::Object>();
    lockFD = jsFileHandle.Get("fd").ToNumber().Int32Value();

    return;
}

Napi::Value filelock::acquireReadLock(const Napi::CallbackInfo& info)
{
    requested_operation = "lock_sh";
    this->Queue();
    return deferred_promise.Promise();
    
    /*Napi::Env env = info.Env();
    
    if( flock(lockFD, LOCK_SH) == 0 )
    { return Napi::Boolean::New(env, true); }
    else
    { return Napi::Boolean::New(env, false); }*/
}

Napi::Value filelock::acquireWriteLock(const Napi::CallbackInfo& info)
{
    requested_operation = "lock_ex";
    this->Queue();
    return deferred_promise.Promise();

    /*Napi::Env env = info.Env();
    
    if( flock(lockFD, LOCK_EX) == 0 )
    { return Napi::Boolean::New(env, true); }
    else
    { return Napi::Boolean::New(env, false); }*/
}

Napi::Value filelock::removeLock(const Napi::CallbackInfo& info)
{
    requested_operation = "lock_un";
    this->Queue();
    return deferred_promise.Promise();
    
    /*Napi::Env env = info.Env();
    
    if( flock(lockFD, LOCK_UN) == 0 )
    { return Napi::Boolean::New(env, true); }
    else
    { return Napi::Boolean::New(env, false); }*/
}

//===========================================================================================================================================================================================
void filelock::Execute()
{
    if(requested_operation == "lock_ex") { if( flock(lockFD, LOCK_EX) != 0 ) { SetError("Couldn't lock!"); } }
    else if(requested_operation == "lock_sh") { if( flock(lockFD, LOCK_SH) != 0 ) { SetError("Couldn't lock!"); } }
    else if(requested_operation == "lock_un") { if( flock(lockFD, LOCK_UN) != 0 ) { SetError("Couldn't unlock!"); } }
}

void filelock::OnOK()
{
    deferred_promise.Resolve(Napi::AsyncWorker::Env().Undefined());
}

void filelock::OnError(const Napi::Error& err)
{
    deferred_promise.Reject(err.Value());
}