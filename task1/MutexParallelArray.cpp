#include "ParallelArray.cpp"

class MutexParallelArray: public ParallelArray {
private:
    int Index = 0;
    
    void init() override {
        Index = 0;
        initArray();
    }
    
    void increment() override {
        while (true) {
            m.lock();
            int currentIndex = Index;
            Index ++;
            if (currentIndex < NumTasks) {
                byteArray[currentIndex] ++;
                m.unlock();
                this_thread::sleep_for(chrono::nanoseconds(10));
            }
            else {
                m.unlock();
                return;
            }
        }
    }
};
