#include <atomic>

class tas_lock {
    std::atomic<bool> lock_ = {false};

  public:
    inline void lock() {
        while (lock_.exchange(true))
            ;
    }

    inline void unlock() { lock_.store(false); }
};

class tas_lock_nofence {
    std::atomic<bool> lock_ = {false};

  public:
    inline void lock() {
        while (lock_.exchange(true, std::memory_order_acquire))
            ;
    }

    inline void unlock() { lock_.store(false, std::memory_order_release); }
};

// I have removed the PAUSE instruction which reduces performance when not using hyperthreading
class ttas_lock {
    std::atomic<bool> lock_ = {false};

  public:
    inline void lock() {
        for (;;) {
            if (!lock_.exchange(true, std::memory_order_acquire)) {
                break;
            }
            while (lock_.load(std::memory_order_relaxed))
                ;
        }
    }
    inline void unlock() { lock_.store(false, std::memory_order_release); }
};

// I have removed the PAUSE instruction which reduces performance when not using hyperthreading
class amd_lock {
    std::atomic<bool> lock_ = {false};
public:

    void lock() {
        for (;;) {
            bool was_locked = lock_.load(std::memory_order_relaxed);
            if (!was_locked && lock_.compare_exchange_weak(was_locked, true, std::memory_order_acquire))
                break;
        }
    }
    void unlock() { lock_.store(false, std::memory_order_release); }
};