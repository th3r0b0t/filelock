#ifndef ASYNCWORKER_HPP
#define ASYNCWORKER_HPP

#include <fcntl.h>          /* For O_* constants */
#include <sys/file.h>       /* For flock(2) */
#include <string.h>         /* For strerror(3) */
#include <string>
#include <errno.h>
#include <napi.h>

class asyncworker : public Napi::AsyncWorker
{
  public:
    asyncworker(Napi::Env caller_env, int fd, std::string const operation);
    void Execute() override;
    void OnOK() override;
    void OnError(const Napi::Error& err) override;
    
    Napi::Promise::Deferred deferred_promise;
    
  private:
    //-----------members
    int lockFD;
    std::string const operation;
};

#endif // ASYNCWORKER_HPP