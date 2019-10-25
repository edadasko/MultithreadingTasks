#include "MutexParallelArray.cpp"
#include "AtomicParallelArray.cpp"

void task(ParallelArray &a, int numThreads) {
    a.init();
    vector<thread> threads;
    for (int i = 0; i < numThreads; i ++)
        threads.push_back(thread([&]() { a.increment(); }));
    for (int i = 0; i < numThreads; i ++)
        threads[i].join();
    a.check();
}

void measureTime(ParallelArray &a, int numThreads) {
    auto begin = chrono::steady_clock::now();
    task(a, numThreads);
    auto end = chrono::steady_clock::now();
    auto time = chrono::duration_cast<chrono::milliseconds>(end - begin);
    cout << "Время работы при NumThreads = " << numThreads << ": "
    << (double)time.count() / 1000 << endl;
}

int main(int argc, const char * argv[]) {
    MutexParallelArray mutexArray;
    vector<int> threadNums = {4, 8, 16, 32};
    
    for (auto num: threadNums)
        measureTime(mutexArray, num);
    
    cout << endl;
    
    AtomicParallelArray atomicArray;
    for (auto num: threadNums)
         measureTime(atomicArray, num);
    
    return 0;
}
