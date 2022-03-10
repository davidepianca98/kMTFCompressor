
#ifndef MTF_PIPELINEBUFFER_H
#define MTF_PIPELINEBUFFER_H

template<typename T>
class PipelineBuffer {
private:
    T *read_buffer;
    T *write_buffer;

public:
    PipelineBuffer(int size) {
        read_buffer = new T[size];
        write_buffer = new T[size];
    }

    ~PipelineBuffer() {
        delete[] read_buffer;
        delete[] write_buffer;
    }

    T operator[](int i) {
        return read_buffer[i];
    }

    void operator[](int i, T value) {

    }

    void swap() {
        T *app = read_buffer;
        read_buffer = write_buffer;
        write_buffer = app;
    }
};

#endif //MTF_PIPELINEBUFFER_H
