//
// Created by gyankos on 03/04/26.
//

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <ostream>
#include <thread>
#include <queue>

#include "jackbergus/concurrency/HoareMonitor.h"

struct Task {
    double priority;

    friend bool operator<(const Task &lhs, const Task &rhs) {
        return lhs.priority < rhs.priority;
    }

    friend bool operator<=(const Task &lhs, const Task &rhs) {
        return !(rhs < lhs);
    }

    friend bool operator>(const Task &lhs, const Task &rhs) {
        return rhs < lhs;
    }

    friend bool operator>=(const Task &lhs, const Task &rhs) {
        return !(lhs < rhs);
    }
};

enum class SchedulerCond {
    QueueNotEmpty
};

class SchedulerEDF {
    std::thread t;
    jackbergus::concurrency::HighLevelHoareMonitor<SchedulerCond> gm;
    // std::mutex lock;
    std::priority_queue<Task> q;
    // std::condition_variable cv;
    bool running = false;
    bool terminated = false;

public:

    ~SchedulerEDF() {
        terminate();
    }

    bool start() {
        if (!running) {
            running = true;
            t = std::thread([&]() {
                while (true) {
                    auto lock = std::move(gm.lock());
                    // gm.mutex_in(-1);
                    // std::unique_lock mutex{lock};
                    if (q.empty()) {
                        if (terminated)
                            break;
                        else
                            lock.waitCond(SchedulerCond::QueueNotEmpty, -1);
                    }
                    Task task = q.top();
                    q.pop();
                    std::cout << task.priority << std::endl;
                }
            });
            return true;
        } else {
            return false;
        }
    }

    bool push(double test) {
        if (!running) {
            return false;
        } else {
            {
                auto lock =std::move(gm.lock());
                // std::unique_lock<std::mutex> mutex{lock};
                q.push(Task{test});
            }
            return true;
        }
    }

    bool terminate() {
         if (!running) {
             return false;
         }  else if (!terminated) {
             {
                 jackbergus::concurrency::CriticalSection<SchedulerCond> lock(std::move(gm.lock()));
                 // std::unique_lock mutex{lock};
                 terminated = true;
                 lock.signalCond(SchedulerCond::QueueNotEmpty, -1);
                 //lock.unlock();
             }
             // cv.notify_one(); // The semantics in C++ is different from java: the notification shall occur outside the
             // mutex, as this does not behave like Hoare's monitor, and just sends a signal to the thread, but does
             // not make itself to pause and let the other thread continue, as the signalling one is not a thread, but
             // a main process
             t.join();
             return true;
         } else {
             return false;
         }
    }

};

int main(void) {
    SchedulerEDF bogus;
    bogus.start();
    bogus.push(1.0);
    bogus.push(2.0);
    bogus.push(3.0);
    bogus.push(4.0);
    bogus.terminate();
}