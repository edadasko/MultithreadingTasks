#include "MutexParallelArray.cpp"
#include "AtomicParallelArray.cpp"

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
