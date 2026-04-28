//
// Created by gyankos on 31/03/26.
//

#include <atomic>
#include <utility>
#include <functional>

template <typename T>
struct TripleBuffer {


    void update(const std::function<void(T*)>& object) {
        auto ptr = get_for_writer();
        object(ptr);
        publish();
    }

    // Reader API - Reader checks if there's an update
    // on the spare buffer. If there is, it will swap the
    // front and spare buffer.
    // Then it returns the pointer to front buffer
    // for reader to read, and a bool to let reader know
    // if this is 'new' data.
    std::pair<T*, bool> get_for_reader() {
        State curr_spare = spare_.load(std::memory_order_relaxed);
        bool updated = curr_spare.has_update;
        if (curr_spare.has_update) {
            State new_spare {front_idx_, false};
            State prev_spare = spare_.exchange(new_spare,
                                      std::memory_order_acq_rel);
            front_idx_ = prev_spare.idx;
        }
        return {&buffers_[front_idx_], updated};
    }

private:


    // Writer API - returns a pointer to the back buffer
    // for writer to write into.
    T* get_for_writer() {
        return &buffers_[back_idx_];
    }

    // Writer API - Once finished writing, writer calls
    // publish to swap the back and spare buffer.
    void publish() {
        State new_spare {back_idx_, true};
        State prev_spare = spare_.exchange(new_spare,
                                      std::memory_order_acq_rel);
        back_idx_ = prev_spare.idx;
    }

    // On an x86-64 platform,
    // this struct is 8 bytes (inclusive of padding),
    // so a xchg instruction can swap it atomically.
    struct State {
        int idx;
        bool has_update;
    };

    // Implementation is lock-free as long as
    // this static_assert passes
    static_assert(std::atomic<State>::is_always_lock_free);

    T buffers_[3];
    int front_idx_ = 0;
    // spare_idx_ and has_update_
    // are now moved into an atomic struct
    std::atomic<State> spare_ {{1, false}};
    int back_idx_ = 2;
};

#include <iostream>

int main(void) {
    TripleBuffer<double> val;
    val.update([](double* ptr) { *ptr = 2.0; });
    std::cout << *val.get_for_reader().first << std::endl;
    val.update([](double* ptr) { *ptr = 3.0; });
    std::cout << *val.get_for_reader().first << std::endl;
    val.update([](double* ptr) { *ptr = 4.0; });
    std::cout << *val.get_for_reader().first << std::endl;
    val.update([](double* ptr) { *ptr = 2.0; });
    std::cout << *val.get_for_reader().first << std::endl;
    val.update([](double* ptr) { *ptr = 3.0; });
    std::cout << *val.get_for_reader().first << std::endl;
    val.update([](double* ptr) { *ptr = 4.0; });
    std::cout << *val.get_for_reader().first << std::endl;
}