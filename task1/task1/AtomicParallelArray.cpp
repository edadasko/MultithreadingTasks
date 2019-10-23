#include "ParallelArray.cpp"

class AtomicParallelArray: public ParallelArray {
private:
    atomic<int> Index;
    atomic_bool isReady;
    
    void init() override {
        Index = 0;
        isReady = true;
        initArray();
    }
    
    void increment() override {
        while (true) {
            auto prev = Index.fetch_add(1);
            if (prev >= NumTasks)
                return;
            byteArray[prev]++;
            this_thread::sleep_for(chrono::nanoseconds(10));
        }
    }
};
