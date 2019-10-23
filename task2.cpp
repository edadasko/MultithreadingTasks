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

class ParallelQueue {
public:
    const int taskNum = 4 * 1024 * 1024;
    int consumerNum, producerNum;
    ll sum;
    ParallelQueue(int cn, int pn) {
        consumerNum = cn;
        producerNum = pn;
        sum = 0;
    }
    
    virtual void push (byte val) = 0;
    virtual bool pop (byte& val) = 0;
    
    virtual void consumerTask () = 0;
    virtual void producerTask () = 0;
    
    void startThreads() {
        thread producerThreads[producerNum];
        thread consumerThreads[consumerNum];
        
        for (int i = 0; i < producerNum; i ++) {
            producerThreads[i] = thread([&]() { producerTask(); });
        }
        
        for (int i = 0; i < consumerNum; i ++) {
            consumerThreads[i] = thread([&]() { consumerTask(); });
        }
        
        for (int i = 0; i < producerNum; i ++)
            if (producerThreads[i].joinable())
                producerThreads[i].join();
        
        for (int i = 0; i < consumerNum; i ++)
            if (consumerThreads[i].joinable())
                consumerThreads[i].join();
        
        checkSum();
    }
    
    void measureTime() {
        auto begin = chrono::steady_clock::now();
        startThreads();
        auto end = chrono::steady_clock::now();
        auto time = chrono::duration_cast<chrono::milliseconds>(end - begin);
        cout << "Время работы при consumerNum = "
        << consumerNum <<", producerNum = " << producerNum << ": "
        << (double)time.count() / 1000 << endl;
    }
    
    void checkSum() {
        assert(sum == producerNum * taskNum);
    }
};

class DynamicMutexParallelQueue : public ParallelQueue {
private:
    queue<byte> q;
    mutex commonMutex, pushMutex, popMutex;
    
public:
    DynamicMutexParallelQueue(int cn, int pn) : ParallelQueue(cn, pn) {};
    
    void push(byte val) override {
        pushMutex.lock();
        q.push(val);
        pushMutex.unlock();
    }
    
    bool pop(byte& val) override {
        if (q.empty()){
            commonMutex.unlock();
            this_thread::sleep_for(chrono::milliseconds(1));
            commonMutex.lock();
            if (q.empty())
                return false;
        }
        val = q.front();
        q.pop();
        return true;
    }
    
    void producerTask() override {
        for (int k = 0; k < taskNum; k ++) {
            commonMutex.lock();
            push(1);
            commonMutex.unlock();
        }
    }
    
    void consumerTask() override {
        for (int k = 0; k < taskNum * producerNum / consumerNum; k ++) {
            popMutex.lock();
            commonMutex.lock();
            byte popElement;
            bool isPopped = pop(popElement);
            while (!isPopped)
                pop(popElement);
            sum += popElement;
            commonMutex.unlock();
            popMutex.unlock();
        }
    }
};

class StaticMutexParallelQueue : public ParallelQueue {
private:
    byte* q;
    mutex m;
    int size;
    condition_variable conditionPush, conditionPop;
    atomic<int> popCount, pushCount, elementsCount;
    mutex commonMutex, pushMutex, popMutex;
    int consumerIndex = 0, producerIndex = 0;
    
public:
    StaticMutexParallelQueue(int s, int cn, int pn) : ParallelQueue(cn, pn) {
        popCount = 0;
        pushCount = 0;
        elementsCount = 0;
        size = s;
        q = new byte[size];
        for (int i = 0; i < size; i ++)
            q[i] = 0;
    };
    
    void push(byte val) override {
        while (q[producerIndex % size]) {
            conditionPop.notify_one();
            this_thread::yield();
        }
        q[producerIndex % size] = val;
        elementsCount++;
        producerIndex++;
        producerIndex %= size;
        pushCount++;
        conditionPop.notify_one();
    }
    
    bool pop(byte& val) override {
        if (!q[consumerIndex % size]) {
            conditionPush.notify_one();
            this_thread::sleep_for(chrono::milliseconds(1));
            if (!q[consumerIndex % size])
                return false;
        }
        val = q[consumerIndex % size];
        q[consumerIndex % size] = 0;
        elementsCount--;
        consumerIndex++;
        consumerIndex %= size;
        popCount++;
        conditionPush.notify_one();
        return true;
    }
    
    void producerTask() override {
        while (true) {
            unique_lock<mutex> lock(m);
            
            while (elementsCount.load() >= size) {
                if (pushCount.load() >= producerNum * taskNum) {
                    conditionPush.notify_all();
                    return;
                }
                conditionPush.wait(lock);
            }
            if (pushCount.load() >= producerNum * taskNum) {
                conditionPush.notify_all();
                return;
            }
            push(1);
        }
    }
    
    void consumerTask() override {
        while (true) {
            unique_lock<mutex> lock(m);
            
            while (elementsCount.load() <= 0) {
                if (popCount.load() >= producerNum * taskNum) {
                    conditionPop.notify_all();
                    return;
                }
                conditionPop.wait(lock);
            }
            if (popCount.load() >= producerNum * taskNum) {
                conditionPop.notify_all();
                return;
            }
            byte popElement = 0;
            bool isPopped = pop(popElement);
            while (!isPopped)
                pop(popElement);
            sum += popElement;
        }
    }
};

int main(int argc, const char * argv[]) {
    
    //DynamicMutexParallelQueue dmQueue(2, 1);
    //dmQueue.measureTime();
    
    StaticMutexParallelQueue smQueue(16, 1, 1);
    smQueue.measureTime();
    return 0;
}
