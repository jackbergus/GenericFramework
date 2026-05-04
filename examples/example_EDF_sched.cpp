#include <iostream>

#include <cstdint>
#include <functional>
#include <thread>

#include <jackbergus/concurrency/HoareMonitor.h>

/*
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
                                                                  is_write_enabled_t, WriteFDOperation)>;*/



#include <jackbergus/data_structures/IntervalTree.h>
#include <atomic>


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


#include <jackbergus/concurrency/thread_pool/Task.h>




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

#include <jackbergus/concurrency/ThreadPool.h>

// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
int main() {
    auto start = getTimestamp<>();
    jackbergus::concurrency::thread_pool::ThreadPool<8, 2> tp;

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
        auto t = jackbergus::concurrency::thread_pool::Task::makeTask(estimated_duration, some_taskP, (char*)&data_array[i], sizeof(data_array[i]));
        tp.submitJob(t);
    }

    tp.terminate();

    // TIP See CLion help at <a href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a>. Also, you can try interactive lessons for CLion by selecting 'Help | Learn IDE Features' from the main menu.
}