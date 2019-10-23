#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <queue>

typedef long long ll;
typedef uint8_t byte;

using namespace std;

class ParallelArray {
public:
    const int NumTasks = 1024 * 1024;
    byte* byteArray;
    mutex m;
    
    ParallelArray() {};
    
    void print() {
        for (int i = 0; i < NumTasks; i++)
            cout << static_cast<int>(byteArray[i]) << " ";
    }
    
    void initArray() {
        byteArray = new byte[NumTasks];
        for (int i = 0; i < NumTasks; i++)
            byteArray[i] = 0;
    }
    
    void check() {
        for (int i = 0; i < NumTasks; i++)
            assert(byteArray[i] == 1);
    }
    
    virtual void init() = 0;
    virtual void increment() = 0;
    
    void task(int numThreads) {
        init();
        thread threads[numThreads];
        for (int i = 0; i < numThreads; i ++)
            threads[i] = thread([&]() { increment(); });
        for(int i = 0; i < numThreads; i ++)
            if (threads[i].joinable())
                threads[i].join();
        check();
    }
    
    void measureTime(int numThreads) {
        auto begin = chrono::steady_clock::now();
        task(numThreads);
        auto end = chrono::steady_clock::now();
        auto time = chrono::duration_cast<chrono::milliseconds>(end - begin);
        cout << "Время работы при NumThreads = " << numThreads << ": "
        << (double)time.count() / 1000 << endl;
    }

};

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
            auto prev = Index.fetch_add(1, memory_order_relaxed);
            if (prev >= NumTasks)
                return;
            byteArray[prev]++;
            this_thread::sleep_for(chrono::nanoseconds(10));
        }
    }
};


int main(int argc, const char * argv[]) {
    
    MutexParallelArray mutexArray;
    mutexArray.measureTime(4);
    mutexArray.measureTime(8);
    mutexArray.measureTime(16);
    mutexArray.measureTime(32);
    
    cout << endl;
    
    AtomicParallelArray atomicArray;
    atomicArray.measureTime(4);
    atomicArray.measureTime(8);
    atomicArray.measureTime(16);
    atomicArray.measureTime(32);
    
    return 0;
}
