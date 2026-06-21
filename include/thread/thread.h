#ifndef STAGDEER_THREAD
#define STAGDEER_THREAD

#include "threadTask.hpp"
#include <atomic>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>
#include <thread>
#include <chrono>

namespace stagdeer {
    class THREAD {
        public:
            THREAD() = default;
            ~THREAD() {
                if (M_threadmanager) {
                    M_threadmanager->waitAllthread();
                }
            }

            static THREAD& getInstance() {
                static THREAD M_instance_this;
                return M_instance_this;
            }

            static void waitThread() {
                stagdeer::THREAD::getInstance().getThreadManager().waitAllthread();
            }

            void createThreadManager(size_t threadNumber) {
                if (M_is_init_threadmnager.load()) {
                    return;
                }
                M_threadmanager = std::make_unique<stagdeer::ThreadManager>(threadNumber);
                M_is_init_threadmnager.store(true);
                return;
            }

            ThreadManager& getThreadManager() {
                if (!M_is_init_threadmnager.load()) {
                    throw std::runtime_error("threadManager not initialized");
                }
                return *M_threadmanager;
            }

            THREAD(THREAD&& M_other_Thread) = delete;
            THREAD&& operator=(THREAD&& M_other_Operator) = delete;
            THREAD(const THREAD&) = delete;
            THREAD& operator=(const THREAD&) = delete;
            private:
                std::unique_ptr<stagdeer::ThreadManager> M_threadmanager;
                std::atomic<bool> M_is_init_threadmnager{false};
    };
}

#endif