#ifndef EVENT_CLASS_HPP
#define EVENT_CLASS_HPP

#include <functional>
#include <string>
#include <vector>
#include <condition_variable>
#include <thread>

using std::string, std::vector, std::function, std::condition_variable, std::mutex, std::thread;

template <typename... InputTypes>
class Event {
    public:
        string Name;
        
        Event(string Name) : Name(Name) {}
        Event() : Name("Event") {}
        
        void Fire(InputTypes... Input) {
            std::lock_guard<mutex> Lock(Mutex);
            Fired = true;
            ConditionVariable.notify_all();
            
            if (Callbacks.empty() && OnceCallbacks.empty()) return;
            for (auto& Callback : ParallelCallbacks) {
                thread(Callback, Input...).detach();
            }
            
            for (auto& Callback : ParallelOnceCallbacks) {
                thread(Callback, Input...).detach();
            }
            
            for (auto& Callback : Callbacks) {
                Callback(Input...);
            }
            
            for (auto& Callback : OnceCallbacks) {
                Callback(Input...);
            }
            
            ParallelOnceCallbacks.clear();
            OnceCallbacks.clear();
        }
        
        void Connect(function<void(InputTypes...)> Callback) {Callbacks.push_back(Callback);}
        void Once(function<void(InputTypes...)> Callback) {OnceCallbacks.push_back(Callback);}
        void ConnectParallel(function<void(InputTypes...)> Callback) {ParallelCallbacks.push_back(Callback);}
        void OnceParallel(function<void(InputTypes...)> Callback) {ParallelOnceCallbacks.push_back(Callback);}
        
        void Wait() {
            std::unique_lock<mutex> Lock(Mutex);
            ConditionVariable.wait(Lock, [this](){ return Fired; });
        }
        
    private:
        std::mutex Mutex;
        std::condition_variable ConditionVariable;
        bool Fired = false;
        
        // Standard Callbacks
        vector<function<void(InputTypes...)>> Callbacks;
        vector<function<void(InputTypes...)>> OnceCallbacks;
        
        // Threaded (Parallel) Callbacks
        vector<function<void(InputTypes...)>> ParallelCallbacks;
        vector<function<void(InputTypes...)>> ParallelOnceCallbacks;
};

#endif