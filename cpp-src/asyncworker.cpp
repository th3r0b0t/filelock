#include "./asyncworker.hpp"

asyncworker::asyncworker(Napi::Env caller_env, int fd, std::string const operation): Napi::AsyncWorker(caller_env), deferred_promise(caller_env), lockFD(fd), operation(operation)
{}


void asyncworker::Execute()
{
    errno = 0;
    if(operation == "lock_ex") { if( flock(lockFD, LOCK_EX) != 0 ) { SetError("Couldn't lock! " + std::string(strerror(errno)) ); } }
    else if(operation == "lock_sh") { if( flock(lockFD, LOCK_SH) != 0 ) { SetError("Couldn't lock! " + std::string(strerror(errno)) ); } }
    else if(operation == "lock_un") { if( flock(lockFD, LOCK_UN) != 0 && errno != EBADF ) { SetError("Couldn't unlock! " + std::string(strerror(errno)) ); } }
}

/*void asyncworker::Execute()
{
    std::cerr << "Inside Execute: Attempting to " << operation << " on FD " << lockFD << std::endl;

    int operationFlag = 0;
    if (operation == "lock_ex") {
        operationFlag = LOCK_EX;
    } else if (operation == "lock_sh") {
        operationFlag = LOCK_SH;
    } else if (operation == "lock_un") {
        operationFlag = LOCK_UN;
    } else {
        SetError("Invalid lock operation!");
        return;
    }

    if (flock(lockFD, operationFlag) == 0) {
        std::cerr << "Inside Execute: Successfully " << operation << " on FD " << lockFD << std::endl;
    } else {
        std::cerr << "Inside Execute: Failed to " << operation << " on FD " << lockFD << " with error: " << strerror(errno) << std::endl;
        SetError("Couldn't lock!");
    }
}*/

void asyncworker::OnOK()
{
    deferred_promise.Resolve(Napi::AsyncWorker::Env().Undefined());
}

void asyncworker::OnError(const Napi::Error& err)
{
    deferred_promise.Reject(err.Value());
}