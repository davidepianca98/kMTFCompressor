
#ifndef MTF_MONITOR_H
#define MTF_MONITOR_H

#include <mutex>
#include <condition_variable>

class Monitor {
public:
    enum thread_type {
        READER,
        CODER,
        WRITER
    };

    inline void wait(enum thread_type type) {
        std::unique_lock<std::mutex> lock(mutex);

        if (type == READER) {
            if (state != STATE1) {
                io.wait(lock);
            }
            reading = true;
        } else if (type == WRITER) {
            io.wait(lock);
            writing = true;
        } else if (type == CODER) {
            coders.wait(lock);
            encoding++;
        }
    }

    inline void notify(enum thread_type type) {
        std::unique_lock<std::mutex> lock(mutex);
        if (state == STATE1) {
            if (type == READER) {
                reading = false;
                state = STATE2;
                coders.notify_all();
            }
        } else if (state == STATE2) {
            if (type == CODER) {
                encoding--;
                if (encoding == 0) {
                    state = STATE3;
                    writing = true;
                    io.notify_all();
                }
            }
        } else if (state == STATE3) {
            if (type == READER) {
                reading = false;
                if (!reading && !writing) {
                    state = STATE2;
                    coders.notify_all();
                }
            } else if (type == WRITER) {
                writing = false;
                if (!reading && !writing) {
                    state = STATE2;
                    coders.notify_all();
                }
            }
        }
    }

private:

    enum monitor_state {
        STATE1,
        STATE2,
        STATE3
    };

    std::mutex mutex;
    std::condition_variable coders;
    std::condition_variable io;

    enum monitor_state state = STATE1;
    int encoding = 0;
    bool reading = true;
    bool first_read = false;
    bool writing = false;
};

#endif //MTF_MONITOR_H
