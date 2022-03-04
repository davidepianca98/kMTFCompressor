
#ifndef MTF_SEMAPHORE_H
#define MTF_SEMAPHORE_H

#include <mutex>
#include <condition_variable>

class Semaphore {
public:
    explicit Semaphore(int count = 0) : count(count) {}

    inline void notify() {
        std::unique_lock<std::mutex> lock(mutex);
        count++;
        cv.notify_one();
    }

    inline void notify_all() {
        std::unique_lock<std::mutex> lock(mutex);
        count = 0;
        cv.notify_all();
    }

    inline void wait() {
        std::unique_lock<std::mutex> lock(mutex);

        while (count == 0) {
            cv.wait(lock);
        }
        count--;
    }

    inline int waiting() {
        std::unique_lock<std::mutex> lock(mutex);
        return count;
    }

private:
    std::mutex mutex;
    std::condition_variable cv;
    int count;
};

#endif //MTF_SEMAPHORE_H
