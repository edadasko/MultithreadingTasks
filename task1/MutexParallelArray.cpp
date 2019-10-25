#include "ParallelArray.cpp"

class MutexParallelArray: public ParallelArray {
private:
    int Index = 0;
    mutex m;
    
    void init() override {
        Index = 0;
        initArray();
    }
    
    void increment() override {
        while (true) {
            m.lock();
            int currentIndex = Index;
            Index ++;
            m.unlock();
            if (currentIndex < NumTasks) {
                byteArray[currentIndex] ++;
                this_thread::sleep_for(chrono::nanoseconds(10));
            }
            else {
                return;
            }
        }
    }
};
