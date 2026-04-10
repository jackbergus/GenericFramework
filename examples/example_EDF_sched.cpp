#include <iostream>

#include <cstdint>
#include <functional>
#include <thread>

#include <jackbergus/concurrency/HoareMonitor.h>

using fd_operation_result = int64_t;
using buffer_type = char*;
using buffer_size = uint64_t;
constexpr static inline bool isFDOperationSuccessful(int64_t val) {
    return val >= 0;
}
#define GENERIC_FD_OPERATION_ERROR  ((fd_operation_result)-1)
static_assert(!isFDOperationSuccessful(GENERIC_FD_OPERATION_ERROR));
using is_read_enabled_t             = bool;
#define READ_ENABLED                ((is_read_enabled_t)true)
#define READ_DISABLED               ((is_read_enabled_t)false)
using is_write_enabled_t            = bool;
#define WRITE_ENABLED               ((is_write_enabled_t)true)
#define WRITE_DISABLED              ((is_write_enabled_t)false)
using isUpdateOperationSuccessful_t = bool;
#define SUCCESSFUL_UPDATE           ((isUpdateOperationSuccessful_t)true)
#define UNSUCCESSFUL_UPDATE         ((isUpdateOperationSuccessful_t)false)

using ReadFDOperation = std::function<fd_operation_result(buffer_type, buffer_size)>;
using WriteFDOperation = std::function<fd_operation_result(const buffer_type, buffer_size)>;
using DataChannelOp = std::function<isUpdateOperationSuccessful_t(is_read_enabled_t, ReadFDOperation,
                                                                  is_write_enabled_t, WriteFDOperation)>;


enum class ThreadPoolEvents {
    HasSomeTaskOccurrence
};

#include <jackbergus/data_structures/IntervalTree.h>
#include <atomic>
typedef uint64_t(*task_ptr)(const char*, uint64_t);

uint64_t some_task(const char* data_buffer, uint64_t data_size) {
    uint64_t result;
    if (data_buffer == nullptr) {
        result = 0;
    } else {
        if (data_size == sizeof(uint64_t)) {
            std::cout << *(uint64_t*)data_buffer << std::endl;
            result = *(uint64_t*)data_buffer+1;
        } else {
            result = 0;
        }
    }
    std::cout << "Task Done: " << result << std::endl;
    return result;
}

uint64_t some_taskP(const char* data_buffer, uint64_t data_size) {
    uint64_t result;
    if (data_buffer == nullptr) {
        result = 0;
    } else {
        if (data_size == sizeof(uint64_t)) {
            std::cout << *(uint64_t*)data_buffer << std::endl;
            result = *(uint64_t*)data_buffer+1;
        } else {
            result = 0;
        }
    }
    std::cout << "Task Done: " << result << std::endl;
    return result;
}

#include <chrono>

template<typename time_granularity = std::milli>
uint32_t getTimestamp() {
    return static_cast<uint32_t>(std::chrono::duration<double, time_granularity>(std::chrono::high_resolution_clock::now().time_since_epoch()).count());
}

struct Task {
    // Cannot use real numbers for defining lexicographical orderings [Debreu (1954)], and then I am forced to use integers.
    // Otherwise, I would have need to use hyperreal numbers.
    // Debreu 1954: "Representation of a Preference ordering by a numerical function". _Decision processes_, 3, 159-165
    uint64_t      computed_priority     = 0;
    uint32_t      provided_time         = 0;
    uint32_t      estimated_duration    = 0;
    uint32_t      deadline              = 0;
    task_ptr    task_to_run             = nullptr;
    const char* buffer_data             = nullptr;
    uint64_t    buffer_size             = 0;
    // Node<uint32_t>* isIntervalOfTree  = nullptr;

    Task() {}
    Task(const Task&) = default;
    Task(Task&&x) : computed_priority{x.computed_priority}, provided_time(x.provided_time), estimated_duration(x.estimated_duration), deadline(x.deadline), task_to_run(x.task_to_run), buffer_data(x.buffer_data), buffer_size(x.buffer_size) {

    }
    Task& operator=(const Task&) = default;
    Task& operator=(Task&& x) {
        computed_priority = x.computed_priority;
        provided_time = x.provided_time;
        estimated_duration = x.estimated_duration;
        deadline = x.deadline;
        task_to_run = x.task_to_run;
        buffer_data = x.buffer_data;
        buffer_size = x.buffer_size;
        return *this;
    }

    /**
     *
     * @tparam time_granularity Granularity for the minimal time sensibility required by the specs
     * @param duration_tg
     * @return The task described as being scheduled from now (even though it might take some more time)
     */
    template<typename time_granularity = std::milli>
    static Task makeTask(uint32_t duration_tg,
                         task_ptr task,
                         const char* buffer_data,
                         uint64_t buffer_size,
                         uint32_t deadline_tg = 0) {
        Task t;
        t.computed_priority = 0;
        t.provided_time = getTimestamp<time_granularity>();
        t.estimated_duration = duration_tg;
        t.deadline = deadline_tg ? deadline_tg : duration_tg + t.provided_time;
        t.task_to_run = task;
        t.buffer_data = buffer_data;
        t.buffer_size = buffer_size;
        return t;
    }

    friend bool operator<(const Task &lhs, const Task &rhs) {
        return lhs.computed_priority < rhs.computed_priority;
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


#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue> // also providing the priority queue

enum PriorityComputationType {
    EDF_DeadlinePriority = 0,
    EDF_DeadlineWithActualDeadlineAndOriginalOne = 1,
};

struct EDF  {

    EDF(uint64_t jobid = 0) : jobid_(jobid) { }

    ~EDF() {
    }

    void start() {
        if ((!have_all_terminated) && (!terminate_cmd) && (!isStarted)) {
            isStarted = true;
            threads_in_pool = std::thread([this]() {
                        while (true) {
                            Task top;
                            {
                                ThreadPoolMonitor::CS lock(std::move(hlm.lock()));
                                // std::unique_lock<std::mutex> lock{monitor};
                                if (task_queue.empty()) {
                                    if ((!have_all_terminated) && (!terminate_cmd))
                                        lock.waitCond(ThreadPoolEvents::HasSomeTaskOccurrence, -1);
                                    else
                                        break;
                                }
                                if (terminate_cmd || have_all_terminated) {
                                    break;
                                }
                                auto currentsize = task_queue.size();
                                top = task_queue.top();
                                task_queue.pop();
                                std::cout << "#" << jobid_ << ": from " << currentsize << " to " << task_queue.size() << std::endl;
                            }
                            top.task_to_run(top.buffer_data, top.buffer_size);
                        }
                        {
                            // std::unique_lock<std::mutex> lock{monitor};
                            if (terminate_cmd) {
                                // If there are no more tasks and I am asking the threads to terminate, then I am sending the singal to all threads to terminate
                                //if (!have_all_termi nated)
                                have_all_terminated = true;
                                terminate_cmd = true;
                            }
                        }

                    });
        }
    }

    bool submitJob(const Task& task,
                   bool forceInsertion = false,
                   PriorityComputationType priority = EDF_DeadlinePriority)  {
        // std::unique_lock<std::mutex> lock{monitor};
        // lock.lock();
        bool returnValue = false;
        {
            ThreadPoolMonitor::CS lock(std::move(hlm.lock()));
            if (have_all_terminated || terminate_cmd || (!isStarted)) {
                // If the thread is either terminated or about to terminate, then return false, as the task is not going to be submitted anyway
                returnValue = false;
            } else {
                auto current_timestamp = getTimestamp<>();
                Interval<uint32_t> i{current_timestamp, std::max(task.deadline, task.estimated_duration+current_timestamp)};
                if (forceInsertion || (!it.lookup(i))) {
                    // Inserting the task only if the algorithm is not going to make me miss a deadline
                    if (!it.insertInterval(i)) {
                        returnValue = false;
                    } else {
                        auto t = task;
                        t.provided_time = current_timestamp;
                        // For the simple algorithm, there is no other priority than the deadline that needs to be met.
                        switch (priority) {
                            case EDF_DeadlinePriority:
                                t.computed_priority = t.deadline;
                                break;
                            case EDF_DeadlineWithActualDeadlineAndOriginalOne:
                                t.computed_priority =  static_cast<uint64_t>(std::max(std::numeric_limits<uint32_t>::max(), t.provided_time+t.estimated_duration)) << 32;
                                t.computed_priority += t.deadline;
                                break;
                        }
                        task_queue.push(t);
                        returnValue = true;
                    }
                } else {
                    returnValue = false;
                }
            }
            if (returnValue)
                lock.signalCond(ThreadPoolEvents::HasSomeTaskOccurrence, -1);
        }
        return returnValue;
    }

    bool terminate()  {
        bool retVal = false;
        // std::unique_lock<std::mutex> lock{monitor};
        // lock.lock();
        {
            ThreadPoolMonitor::CS lock(std::move(hlm.lock()));
            if (!isStarted) {
                retVal =  false;
            } else {
                if ((!have_all_terminated) && (!terminate_cmd)) {
                    while (!task_queue.empty()) {
                        // noop, polling and waiting
                    }
                    terminate_cmd = true;
                    if (task_queue.empty()) {
                        lock.signalCond(ThreadPoolEvents::HasSomeTaskOccurrence, -1);
                        lock.unlock();
                        threads_in_pool.join();
                    }
                }
                /*if (!have_all_terminated)
                    thread.join();*/
                retVal =  true;
            }
        }
        return retVal;
    }

    uint64_t jobid_;

    using ThreadPoolMonitor = jackbergus::concurrency::HighLevelHoareMonitor<ThreadPoolEvents>;
private:
    IntervalTree<uint32_t> it;
    bool isStarted = false;
    bool terminate_cmd = false;
    bool have_all_terminated = false;
    std::thread threads_in_pool;
    // std::condition_variable wake_up_waiting_thread;
    std::priority_queue<Task> task_queue;
    ThreadPoolMonitor hlm;
    //std::mutex monitor;
    //std::condition_variable has_some_tasks;
    // std::thread thread;
};

template<uint64_t N, uint64_t LateTaskThreads>
struct ThreadPool {
    static_assert(LateTaskThreads<N);
    static constexpr uint64_t normal_threads = N-LateTaskThreads;
    EDF                         t[N];
    // std::atomic<bool> thread_started[N];
    // std::atomic<bool>       has_work[N];
    // std::atomic<bool>           exit[N];

    ThreadPool() {
        for (uint64_t i = 0; i < N; i++) {
            t[i].jobid_ = i;
        }
    }

    void start() {
        for (uint64_t i = 0; i < N; i++) {
            t[i].start();
        }
    }

    bool submitJob(uint64_t idx, const Task& task) {
        bool result;
        if (idx >= normal_threads) {
            result = false;
        } else {
            if (! t[idx].submitJob(task, false, EDF_DeadlinePriority)) {
                result = forceComputationThatWillBeDelayedNevertheless(task);
            } else {
                result = true;
            }
        }
        return result;
    }

    bool submitJob(const Task& task) {
        uint64_t orig_value = round_robin_normies;
        bool result = false;
        for (uint64_t i = 0; i < normal_threads; i++) {
            auto curr_idx = (i + round_robin_normies) % normal_threads;
            if (t[orig_value].submitJob(task, false, EDF_DeadlinePriority)) {
                round_robin_normies = (curr_idx + 1) % normal_threads;
                result = true;
                break;
            }
        }
        if (!result) {
            forceComputationThatWillBeDelayedNevertheless(task);
        } else {
            // noop: already computed
        }
        return result;
    }

    void terminate() {
        for (uint64_t i = 0; i < N; i++) {
            t[i].terminate();
        }
    }

private:

    bool forceComputationThatWillBeDelayedNevertheless(const Task& task) {
        uint64_t orig_value = round_robin_lates;
        bool result = false;
        for (uint64_t i = 0; i < LateTaskThreads; i++) {
            auto curr_idx = (i + round_robin_lates) % LateTaskThreads;
            if (t[orig_value+normal_threads].submitJob(task, false, EDF_DeadlineWithActualDeadlineAndOriginalOne)) {
                round_robin_lates = (curr_idx + 1) % LateTaskThreads;
                result = true;
                break;
            }
        }
        if (!result) {
            round_robin_lates = (round_robin_lates+1) % LateTaskThreads;
            //None of the available threads was ready. Thus, using the round robin to determine the current
            //thread that will be forced to digest the task
            result = t[orig_value+normal_threads].submitJob(task, true, EDF_DeadlineWithActualDeadlineAndOriginalOne);
        } else {
            // noop: already computed
        }
        return result;
    }

    uint64_t round_robin_lates = 0;
    uint64_t round_robin_normies = 0;
};

#if 0
struct JackBergusDataChannel {
    ReadFDOperation     read_;
    WriteFDOperation    write;
    DataChannelOp       op___;

    JackBergusDataChannel(const is_read_enabled_t has_read = READ_DISABLED,
                          const ReadFDOperation&  read = [](buffer_type, buffer_size) { return GENERIC_FD_OPERATION_ERROR; },
                          const is_write_enabled_t has_write = WRITE_DISABLED,
                          const WriteFDOperation& write = [](const buffer_type, buffer_size) { return GENERIC_FD_OPERATION_ERROR; },
                          const DataChannelOp& op = [](is_read_enabled_t, ReadFDOperation,
                                                                  is_write_enabled_t, WriteFDOperation) { return UNSUCCESSFUL_UPDATE; }
        ) : read_enabled{has_read},
            read_{read},
            write_enabled{has_write},
            write{write},
            op___{op} {}

    const ReadFDOperation& getReadOperation() const {
        return read_;
    }

    JackBergusDataChannel& enableRead(const ReadFDOperation& op) {
        read_ = op;
        read_enabled = READ_ENABLED;
        return *this;
    }

    JackBergusDataChannel& disableRead() {
        read_ = [](buffer_type, buffer_size) { return GENERIC_FD_OPERATION_ERROR; };
        read_enabled = READ_DISABLED;
        return *this;
    }

    JackBergusDataChannel& enableWrite(const WriteFDOperation& op) {
        write = op;
        write_enabled = WRITE_ENABLED;
        return *this;
    }

    JackBergusDataChannel& disableWrite() {
        write = [](const buffer_type, buffer_size) { return GENERIC_FD_OPERATION_ERROR; };
        write_enabled = false;
        return *this;
    }

private:
    is_read_enabled_t   read_enabled;
    is_write_enabled_t  write_enabled;
};
#endif

void intervalTreeTest() {
    IntervalTree<double> it;
    it.insertInterval({-1.0, 5.0});
    it.insertInterval({2.0, 3.0});
    it.insertInterval({4.0, 6.0});
    it.insertInterval({5.0, 16.0});
    it.insertInterval({20.0, 19.0});
    it.insertInterval({200.0, 409.0});
    it.insertInterval({-200.0, -49.0});

    std::cout << *it.lookup({7.0, 11.0}) << std::endl;
}

// using time_granularity = std::milli;



// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
int main() {
    auto start = getTimestamp<>();
    ThreadPool<8, 2> tp;

    uint32_t estimated_duration = 0;
    for (uint64_t i = 0; i < 1000; i++) {
        auto begin = getTimestamp<>();
        some_task((const char*)&i, sizeof(i));
        auto end = getTimestamp<>();
        estimated_duration = std::max(estimated_duration, end-begin);
    }

    std::cout << "Running time: " << estimated_duration << std::endl;

    tp.start();
    uint64_t data_array[500];
    for (uint64_t i = 0; i < 500; i++) {
        data_array[i] = i+1;
        auto t = Task::makeTask(estimated_duration, some_taskP, (char*)&data_array[i], sizeof(data_array[i]));
        tp.submitJob(t);
    }

    tp.terminate();

    // TIP See CLion help at <a href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a>. Also, you can try interactive lessons for CLion by selecting 'Help | Learn IDE Features' from the main menu.
}