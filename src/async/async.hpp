/**
MIT License

Copyright (c) 2021 Philo Kaulkin

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <vector>
#include <algorithm>
#include <thread>
#include <assert.h>
#include <memory>
#include <atomic>
#include <functional>
#include <chrono>
#include <optional>
#include <iostream>
#include <variant>
#include <array>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <iterator>
#include <concepts>
#include <unordered_map>

namespace async{

    class exception : public std::exception {
        public:
        exception(const char* msg) : msg(msg) {}
        virtual const char* what() const noexcept 
            { return msg; }
        protected:
        const char* msg;
    };

    class DanglingPromise : public exception {
        public:
        DanglingPromise() : exception(
"All promises must be awaited, and/or stored correctly until they are satisfied. Do not allow Promise objects to become temporaries unawaited."
            ) {}
    };

    class ThreadTerminator : public exception {
        public:
        ThreadTerminator() : exception(
"This exception is used to terminate a thread from within."
        ) {}
        static void terminate() {
            throw ThreadTerminator();
        } 
    };


    /**
     * The Promise class designates a future location for the return value of a coroutine.
     * A Promise will designate one object of type T.
     * If a Promise is expecting to be fullfilled, do NOT allow it to be a temporary or it will become
     * a dangling promise and an error will be thrown.
     */
    template<typename T>
    class _Promise{

        public:
        /**
         * maybeT represents an object that might contain T, or not.
         */
        typedef std::optional<T> maybeT;
        
        /**
         * The type a Promise is meant to hold.
         */
        typedef T type; 
        _Promise() = default; 
        _Promise(const _Promise<T>& other) : element(other.element) { set = (bool)other.set; };
        ~_Promise() = default;
        /**
         * await() will block the thread until this promise is fulfilled.
         * @returns the value assigned to this promise;
         */
        T& await()
            { while(!set){} return **element; }
        /**
         * block_then will block the thread until the promise is fulfilled,
         * then call your given callback with the result.
         * @param fn The function to call when finished
         * @param args other arguments to pass into your function
         * @returns the value returned by your callback;
         */
        template<typename F, typename ...Args>
        maybeT block_then( F fn, Args&& ... args ) 
            {  return fn(await(), args...); }
        
        /**
         * wait_for will await this promise for a given number of milliseconds.
         * @param ms number of milliseconds to wait.
         * @returns Either the result of the promise, or std::nullopt if it times out
         */
        maybeT wait_for(std::chrono::milliseconds ms)
        { 
            auto now = std::chrono::system_clock::now();
            while(!set and std::chrono::system_clock::now()-now<ms){}
            if(set) return **element ;
            else return std::nullopt;
        }
        
        /**
         * (Internal use)
         */
        void assign(maybeT value)
        { 
            if(element.get() == nullptr){
                element = std::make_shared<maybeT>(value);
            }else{
                *element =  (value); 
            }
            set = true; 
        }
        /**
         * Check if this promise is fulfilled.
         * @returns true if this promise is fulfilled, else flase
         */
        bool done() const noexcept {
            return set;
        }

        


        private:
        std::shared_ptr<maybeT> element ;
        std::atomic<bool> set = false;
    };

    /**
     * The Promise class designates a future location for the return value of a coroutine.
     * A Promise will designate one object of type T.
     * If a Promise is expecting to be fullfilled, do NOT allow it to be a temporary or it will become
     * a dangling promise and an error will be thrown.
     */
    template<typename T>
    using Promise = std::shared_ptr<_Promise<T > >; 


    
    /**
     * The Queue class is a wrapper for the std::queue container
     * that provides atomic thread safety. 
     */
    template<typename T> 
    class Queue{
        public:
        typedef T element_t; 
        Queue() = default;
        Queue(const Queue& q) : base(q.base) {};
        Queue(size_t size) : base(size) {}
        /**
         * Add x to the queue
         * @param x a value to add
         */
        void enqueue(const T& x) 
        { 
            std::lock_guard<std::mutex> lock(mut);
            base.emplace(x); 
            fulltex.notify_one();

        }
        /**
         * Get the number queue elements.
         * @return the number of queue elements
         */
        size_t size(){
            std::lock_guard<std::mutex> lock(mut);
            return base.size();
        }
        /**
         * Determine if the queue is empty.
         * @return if the queue is empty
         */
        bool empty()
        {
            std::lock_guard<std::mutex> lock(mut);
            bool out = base.empty();
            return out;
        }
        /**
         * Remove the last element from the queue
         * @return the element removed
         */
        auto dequeue(){
            std::lock_guard<std::mutex> lock(mut);
            auto out = base.front();
            base.pop();
            if (base.empty()){
                fulllock.try_lock();
            }
            return out;
        }
        /**
         * Safely get the last element of the queue,
         * or an std::nullopt if none is available.
         * @return the last element, or std::nullopt
         */
        std::optional<T> safeget(){
            std::lock_guard<std::mutex> lock(mut);
            if(!base.empty()){
                auto out =  base.front();
                base.pop();
                return out;
            }else{
                fulllock.try_lock();
            }

            return std::nullopt;
        }

        std::optional<T> waitget(){
            std::unique_lock<std::mutex> lock(fulllock);
            fulltex.wait_for( lock, std::chrono::milliseconds(10) ,[this](){ return !this->empty(); } );
            return safeget();
        }

        private:
        std::queue<T> base;
        mutable std::mutex mut;
        mutable std::mutex fulllock;
        std::condition_variable fulltex;
    };

    /**
     * A Coroutine object is able to store a 'function call'
     * with any parameters, of any type.
     * A Coroutine object cannot store a return type, so for situations
     * that require a returned value, run your functions through the EventLoop class'
     * templated execute() functions.
     */
    class Coroutine{
        public:
        std::function<void()> fn = nullptr;
        
        /**
         * Apply some function with some arguments to this coroutine
         */
        template<typename FunctionType, typename ...Args>
        void setfn(FunctionType&& newfn, Args ... args){
            fn = [=]() -> void { newfn(args...); };
        }
        Coroutine() = default;
        Coroutine(const Coroutine& other) = default;
        Coroutine(Coroutine&& other) = default;
        Coroutine(std::function<void()> f) 
            : fn(f) {}
        
        void operator()()
            { fn(); }

    };
    


    /**
     * The EventLoop class is responsible for managing a threadpool, and any given
     * tasks and promises.
     * 
     * An EventLoop can be initialized with a certain number of workers to start, but the
     * EventLoop will add more if the workload begins to exceed the ability of existing workers.
     * To reduce the number of workers in an EventLoop, there is a EventLoop::resize(size_t) function
     * which will allow you to safely reset the number of workers.
     * In order to safely wait for the EventLoop to finish all of its tasks, use EventLoop::join().
     * For EventLoops created for temporary use, EventLoop::join is called by the destructor so there is no need
     * to call it.
     */
    class EventLoop{

        public:

        
        typedef std::shared_ptr< Queue< std::function<void (void)> > > task_handle_t;
        typedef task_handle_t::weak_type weak_task_t;
        typedef Queue<task_handle_t::element_type::element_t> queue_t;
        typedef size_t threadid;

        Queue<threadid> threadDeleter;
        std::atomic<size_t> activeThreads;
        std::atomic<bool> isActive = true;
        size_t max_threads = std::thread::hardware_concurrency();
        /**
         * Create an event loop with a certain number of initial workers.
         * @param workers the number of threads to create initially.
         */
        EventLoop(size_t workers = 2, size_t max_workers = std::thread::hardware_concurrency()) 
            : max_threads(max_workers)
        {   
            growThreads(workers);
        }

        EventLoop(const EventLoop& other) = delete;
        ~EventLoop(){
            join();
        }

        /**
         * Safely close all workers and await their completion.
         * Once an EventLoop is 'joined' it cannot be used again. 
         */
        void join(){
            isActive = false;
            for(auto& t: threadpool){
                t.second.join();
            }
            threadpool.clear();
        }

        /**
         * Safely reset the number of threads. If the number given is
         * less than the current number of threads, threads will be safely
         * closed until the given number remains. If the number given is greater than the
         * current number of threads, more will be added.
         * @param newsize the number of threads that should be running in this EventLoop
         */
        void resize(size_t newsize){
            if (newsize == threadpool.size()) return;
            if (newsize < threadpool.size()) {
                // shrink
                size_t difference = threadpool.size()-newsize;
                size_t deletables = difference;
                for (;difference!=-1ULL;difference--){
                    launch(ThreadTerminator::terminate);
                }
                
                while (threadDeleter.size() < deletables){

                }
                while(threadDeleter.size()){
                    auto id = threadDeleter.safeget();
                    if (id){
                        auto iter = threadpool.find(*id);
                        iter->second.join();
                        threadpool.erase(iter);
                    }
                }

                


            }else{
                if (newsize > max_threads){
                    max_threads = newsize;
                }
                // grow
                growThreads(newsize-threadpool.size());
            }

        }

        void checkup(){
            if (activeThreads == threadpool.size()) // if all threads are occupied
            {
                if (threadpool.size() < max_threads) // if more threads are allowed by the user
                    growThreads(1);
            }
        }

        /**
         * Execute a coroutine through this EventLoop.
         * Note: The given coroutine will be ran in a different thread.
         * @param coroutine a coroutine to be ran asynchronously
         * @returns A Promise set the return value of your coroutine.
         * Note: Do NOT allow this promise to go out of scope before the completion
         * of your coroutine. If the promise does not exist when the coroutine
         * finishes, it will result in undefined behavior. To prevent this,
         * make sure you use some blocking function on your promises before they go out of scope.
         * For example, await(), or block_then()
         */
        template<typename T>
        [[nodiscard]] Promise<T> execute( std::function<T ()> coroutine) 
            {
                assert(isActive);
                _Promise<T> out;
                Promise<T> ptr = std::make_shared<_Promise<T> >(out);
                std::weak_ptr<_Promise<T> > weak = ptr;
                checkup();
                task_handle->enqueue( 
                    [coroutine, weak](){ 

                        auto v = coroutine();
                        // Check to ensure that the promise is still waiting on the other end
                        if (weak.expired()){
                            // Throw if not
                            throw DanglingPromise();
                        }
                        // Assign the value now that the promise is confirmed to exist
                        weak.lock()->assign(v);
                    }
                );
                return ptr;
            }
        

        /**
         * Execute a coroutine through this EventLoop.
         * Note: The given coroutine will be ran in a different thread.
         * @param fn a coroutine to be ran asynchronously
         * @returns A Promise that will satisfy when your coroutine finishes.
         * Note: Do NOT allow this promise to go out of scope before the completion
         * of your coroutine. If the promise does not exist when the coroutine
         * finishes, it will result in undefined behavior. To prevent this,
         * make sure you use some blocking function on your promises before they go out of scope.
         * For example, await(), or block_then()
         */
        [[nodiscard]] Promise<bool> execute ( std::function<void()> fn )
            { return execute(Coroutine(fn)); }

        /**
         * Execute a coroutine through this EventLoop.
         * Note: The given coroutine will be ran in a different thread.
         * @param coroutine a coroutine to be ran asynchronously
         * @param args more arguments to given to your coroutine
         * @returns A Promise set the return value of your coroutine.
         * Note: Do NOT allow this promise to go out of scope before the completion
         * of your coroutine. If the promise does not exist when the coroutine
         * finishes, it will result in undefined behavior. To prevent this,
         * make sure you use some blocking function on your promises before they go out of scope.
         * For example, await(), or block_then()
         */
        template<typename FunctionType, typename Arg1, typename ...Args>
        [[nodiscard]] Promise<bool> execute( FunctionType&& coroutine, Arg1 arg1, Args&& ...args )
            { return execute<bool>( [=](){ std::invoke( coroutine, arg1, args... ); return true;} ); }

        /**
         * Execute a coroutine through this EventLoop.
         * Note: The given coroutine will be ran in a different thread.
         * @param coroutine a coroutine to be ran asynchronously
         * @returns A Promise set the return value of your coroutine.
         * Note: Do NOT allow this promise to go out of scope before the completion
         * of your coroutine. If the promise does not exist when the coroutine
         * finishes, it will result in undefined behavior. To prevent this,
         * make sure you use some blocking function on your promises before they go out of scope.
         * For example, await(), or block_then()
         */
        [[nodiscard]] Promise<bool> execute( Coroutine c )
            { return execute<bool>( [=](){ if (c.fn) c.fn(); return true;} ); }

        /**
         * Execute a series of Coroutines through this EventLoop.
         * Note: Each of these coroutines will be ran in a different 
         * thread than the main thread.
         * @param cs a vector of coroutines to be ran asynchronously
         * @returns A promise that will satisfy once ALL of the given coroutines
         * have finished.
         * Note: Do NOT allow this promise to go out of scope before the completion
         * of your coroutine. If the promise does not exist when the coroutine
         * finishes, it will result in undefined behavior. To prevent this,
         * make sure you use some blocking function on your promises before they go out of scope.
         * For example, await(), or block_then()
         */
        [[nodiscard]] Promise<bool> executes( std::vector<Coroutine> cs )
        {
            std::vector<Promise<bool> > promises;
            promises.reserve(cs.size());
            for (auto& c : cs){
                promises.push_back(execute(c));
            }
            return execute( [promises](){ for (auto& p : promises) p->await(); } );
        }

        /**
         * Launch a detached coroutine.
         * You will have no built-in way of knowing when this coroutine will finish,
         * or what it did. However, you do not have to await any promises.
         * @param c a coroutine to detach and launch
         */ 
        void launch( Coroutine c )
            { checkup(); task_handle->enqueue(c.fn); }

        /**
         * Launch a detached coroutine.
         * You will have no built-in way of knowing when this coroutine will finish,
         * or what it did. However, you do not have to await any promises.
         * @param coroutine a coroutine to detach and launch
         * @param args extra args to pass to coroutine
         */ 
        template<typename FunctionType, typename ...Args>
        void launch( FunctionType&& coroutine, Args&& ...args )
            { checkup(); return task_handle->enqueue( [=](){ std::invoke( coroutine, args... ); } ); }


        private:
        threadid threadIdCounter = 0;
        size_t worker_count = 0;
        std::unordered_map<threadid, std::thread> threadpool;
        task_handle_t task_handle = std::make_shared< queue_t >( queue_t() );


        

        void growThreads(size_t workers);

       

    };

    static void worker_mainloop(std::weak_ptr< Queue< std::function<void (void)> > > activeSignal, EventLoop* rawParent, EventLoop::threadid id){
        while(!activeSignal.expired()){
            if (rawParent->isActive == false){
                return; // exit thread;
            }
            std::optional<std::function<void ()>> maybeTask;
            async::EventLoop::task_handle_t tasks;
            {
                tasks = activeSignal.lock();
            }
            maybeTask = tasks->waitget();
            if (maybeTask) {
                rawParent->activeThreads++;
                
                try{
                    (*maybeTask)();
                } catch ( const ThreadTerminator& t ){
                    rawParent->activeThreads--;
                    rawParent->threadDeleter.enqueue(id);
                    return; // Terminated thread
                }
                
                
                rawParent->activeThreads--;
            }

            std::this_thread::yield();
        }
    }

    void EventLoop::growThreads(size_t workers){
        worker_count += workers;
        for(size_t i = 0; i < workers; i++){
            threadpool[threadIdCounter] = std::thread(worker_mainloop, task_handle, this, threadIdCounter);
            //threadpool.push_back(  );
            threadIdCounter++;
        }
    }

    template<typename T>
    T& await(Promise<T> p) 
        { return p->await(); }
    

    namespace algorithm{
        
        template<class InputIterator, typename Function> 
            requires std::input_iterator<InputIterator>
        [[nodiscard]] Promise<bool> to_each(EventLoop& e, InputIterator begin, InputIterator end, Function fn){
            
            std::vector<Promise<bool> > promises;
            promises.reserve(end-begin);

            std::for_each(begin, end, [&e,fn, begin, end, &promises](auto& i){
                promises.push_back(
                    (e.execute(
                        [&](){
                            fn(i);
                        }
                    ))
                );
            
            });

            return e.execute( [=](){ 
                for (auto& p : promises ) {
                    p->await();
                    }
                } 
            );

        }


    }






};
