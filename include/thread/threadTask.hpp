#ifndef STAGDEER_THREAD_TASK
#define STAGDEER_THREAD_TASK

#include <atomic>
#include <chrono>
#include <condition_variable>
#include "../client/util/type_util.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <deque>
#include <exception>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <sys/types.h>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>


#define SLEEP(M_sleep_time_microse__)\
    std::this_thread::sleep_for(std::chrono::microseconds(M_sleep_time_microse__))

namespace stagdeer {
    class THREAD;
    class ThreadManager {
    friend class stagdeer::THREAD;
    private:
        struct threadMeta {
            std::thread M_thread;
            std::atomic<bool> M_idle {true};
            uint64_t M_threadProcessed{0};
            threadMeta() = default;
            threadMeta(threadMeta&& M_other) noexcept
            :M_thread(std::move(M_other.M_thread)),
                M_idle(M_other.M_idle.load()),
                M_threadProcessed(M_other.M_threadProcessed) {}
                threadMeta& operator=(threadMeta&& M_other_operator) 
                    noexcept {
                        if (this != &M_other_operator) {
                            M_thread = std::move(M_other_operator.M_thread);
                            M_idle.store(std::move(M_other_operator.M_idle.load()));
                            M_threadProcessed = M_other_operator.M_threadProcessed;
                        }
                        return *this;
                    }

            threadMeta(const threadMeta&) = delete;
            threadMeta& operator=(const threadMeta&) = delete;
        };

        std::vector<std::unique_ptr<struct threadMeta>> M_workThreadMetas;
        std::deque<std::function<void()>> M_tasks;
            std::atomic<int> M_thread_rurinng_count {0};
        std::unique_ptr<std::mutex> M_countMutex_ = std::make_unique<std::mutex>();
        std::unique_ptr<std::mutex> M_tasksMutex_ = std::make_unique<std::mutex>();
        std::unique_ptr<std::condition_variable> M_cv = std::make_unique<std::condition_variable>();
        std::atomic<bool> M_stop{true};
        
        public:
        ThreadManager(ThreadManager&& M_other) 
            noexcept:
                M_workThreadMetas(std::move(M_other.M_workThreadMetas)),
                M_cv(std::move(M_other.M_cv)),
                M_stop(std::move(M_other.M_stop.load())),
                M_tasks(std::move(M_other.M_tasks)),
                M_tasksMutex_(std::move(M_other.M_tasksMutex_)) {};
                    ThreadManager& operator=(ThreadManager&& M_other) 
                        noexcept {
                            if (this != &M_other) {
                                M_workThreadMetas = std::move(M_other.M_workThreadMetas);
                                M_cv = std::move(M_other.M_cv);
                                M_stop = std::move(M_other.M_stop.load());
                                M_tasks = std::move(M_other.M_tasks);
                                M_tasksMutex_ = std::move(M_other.M_tasksMutex_);
                            }
                            return *this;
                        }
        ThreadManager(const ThreadManager&) = delete;
        ThreadManager& operator=(const ThreadManager&) = delete;
        explicit ThreadManager(size_t threadNum = std::thread::hardware_concurrency())
            : M_stop(false) {
                for (size_t index = 0; index < threadNum; ++index) {
                    std::unique_ptr<struct threadMeta> newthreadMetas = std::make_unique<struct threadMeta>();
                    newthreadMetas->M_thread = std::thread([this](){
                        while (true) {
                            std::function<void()> _task;
                            {
                                std::unique_lock<std::mutex> _tasksLock(*M_tasksMutex_);
                              
                                M_cv->wait(_tasksLock , [this](){
                                    return !M_tasks.empty() || M_stop.load();
                                });

                                if (M_tasks.empty() && M_stop.load()) {
                                    break;
                                }

                                if (M_tasks.empty()) {
                                    continue;
                                }

                                _task = std::forward<std::function<void()>>(M_tasks.front());
                                M_tasks.pop_front();

                            }
                            if (_task) {
                                _task();
                                {
                                    std::unique_lock<std::mutex> _countLock(*M_countMutex_);
                                    if (M_thread_rurinng_count > 0) {
                                        M_thread_rurinng_count --;
                                    }
                                    if (M_thread_rurinng_count.load() == 0 && M_tasks.empty()) {
                                        M_cv->notify_all();
                                    }
                                }
                            }
                        }
                    });
                    newthreadMetas->M_idle.store(true);
                    newthreadMetas->M_threadProcessed = 1;
                    M_workThreadMetas.push_back(std::move(newthreadMetas));
                }
            }
        public:
            template<typename Tp , typename ... Args>
            stagdeer::util::lamdba_trais::constraint<
                util::lamdba_trais::M_is_retTp<
                  typename util::lamdba_trais::M_get_lamdba_ret_Tp
                    <Tp , Args ...>::__M_ret_lmdba, void>::__is_M_ret_Tp
            >::type
            asyncTaskvoid(
                Tp&& taskfunc , 
                Args&& ... token
                 ) 
                noexcept {
                    using taskRetTp = std::invoke_result_t<Tp , Args ...>;
                    if constexpr (std::is_same_v<taskRetTp , void>) {
                        std::function<void()> M_task = 
                            [_taskfunc = std::forward<Tp>(taskfunc) , 
                                ...args = std::forward<Args>(token)]() mutable {
                              _taskfunc(std::move(args) ...);
                            };
                        {
                            std::unique_lock<std::mutex> _tasksLock(*M_countMutex_);
                          
                            M_tasks.push_back(std::move(M_task));  
                            M_thread_rurinng_count++;
                            
                        }
                        M_cv->notify_one();
                    }
                    return 0;
                }
                
            template<typename Tp , typename ... Args>
            inline typename std::enable_if_t<
               !util::lamdba_trais::M_is_retTp<
                typename util::lamdba_trais::M_get_lamdba_ret_Tp
                <Tp, Args...>::__M_ret_lmdba,void>::__is_M_ret_Tp,
                    std::future<typename std::invoke_result_t<Tp,Args...>>
            >
            asyncTaskret (Tp&& taskfunc , Args&& ... token) noexcept {
                    using retTp = std::invoke_result_t<Tp , Args ...>;
                    if constexpr (std::is_same_v<retTp , void>) {
                        static_assert(std::is_same_v<Tp, void>, "Task register faild! [TYPE ERROR]");
                        return {};
                    }
                    std::function<retTp()> bound = std::bind(
                        std::forward<Tp>(taskfunc),
                        std::forward<Args>(token) ...
                    );
                    std::shared_ptr<std::packaged_task<retTp()>> _task = 
                        std::make_shared<std::packaged_task<retTp()>>(bound);
                        std::future<retTp> fut = _task->get_future();
                        {
                            std::lock_guard<std::mutex> _tasksLock(*M_tasksMutex_);
                            M_tasks.emplace_back([_task](){
                                (*_task)();
                            });
                            M_thread_rurinng_count ++;
                        }
                    M_cv->notify_one();
                    return fut;
                }
            
            void waitAllthread() {
                M_stop.store(true);
                M_cv->notify_all();
                {
                    std::chrono::seconds M_threadTimout = std::chrono::seconds(5);
                    std::unique_lock<std::mutex> M_countlock(*M_countMutex_);
                    bool M_completd = M_cv->wait_for(M_countlock , M_threadTimout , 
                        [this](){
                            return M_thread_rurinng_count.load() == 0 && M_tasks.empty();
                        });
                    if (!M_completd) {
                        fprintf(stderr, "Warning: Thread pool did not complete in time\n");
                    }
                }

                for (std::vector<std::unique_ptr<struct threadMeta>>::iterator
                    M_thread_it = M_workThreadMetas.begin(); 
                    M_thread_it != M_workThreadMetas.end();
                    ++ M_thread_it) {
                        if (M_thread_it->get()->M_thread.joinable()) {
                            try {
                                M_thread_it->get()->M_thread.join();
                            } catch (std::exception& M_e) {
                            
                            };
                        }
                    }
            }

            ~ThreadManager() {
                M_stop.store(true);
                if (M_stop.load()) {
                    M_cv->notify_all();
                    for (std::vector<std::unique_ptr<struct threadMeta>>::iterator 
                        it = M_workThreadMetas.begin();
                        it != M_workThreadMetas.end(); ++it) {
                            if (it->get()->M_thread.joinable()) {
                                try {
                                    it->get()->M_thread.join();
                                } catch (const std::exception& M_e) {
                                    
                                }
                            }
                    }
                }
            }
    };

}

#endif