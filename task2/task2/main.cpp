#include "DynamicMutexQueue.cpp"
#include "StaticMutexQueue.cpp"
#include "StaticAtomicQueue.cpp"


int main(int argc, const char * argv[]) {
    
    DynamicMutexQueueTask a;
    a.task();
    
    StaticMutexQueueTask b;
    b.task();
    
    StaticAtomicQueueTask c;
    c.task();
    
    return 0;
}
