#include <stdio.h>
#include "Queue.cpp"

class StaticMutexQueue : public Queue {
private:
    byte* q;
    mutex m;
    int size;
    condition_variable conditionPush, conditionPop;
    atomic<int> popIndex, pushIndex, elementsCount;
    
public:
    
    StaticMutexQueue(int s) {
        size = s;
        q = new byte[size];
        for (int i = 0; i < size; i ++)
            q[i] = 0;
        elementsCount = 0;
        popIndex = 0;
        pushIndex = 0;
    }
    
    void push(byte val) override {
        unique_lock<mutex> lock(m);
        while (elementsCount == size)
            conditionPush.wait(lock);
        q[pushIndex % size] = val;
        pushIndex ++;
        elementsCount ++;
        conditionPop.notify_one();
    }
    
    bool pop(byte& val) override {
        unique_lock<mutex> lock(m);
        while (elementsCount == 0 )
            conditionPop.wait(lock);
        val = q[popIndex % size];
        q[popIndex % size] = 0;
        popIndex ++;
        elementsCount --;
        conditionPush.notify_one();
        return true;
    }
};

class StaticMutexQueueTask {
    static void startTask(int producerNum, int consumerNum, int taskNum, int size) {
        StaticMutexQueue q(size);
        
        atomic<int> sum(0);
        
        auto producer = [&]() {
            for (int i = 0; i < taskNum; i ++) {
                q.push(1);
            }
            return 0;
        };
        auto consumer = [&]() {
            for (int i = 0; i < taskNum * producerNum / consumerNum; i ++) {
                byte poppedValue;
                q.pop(poppedValue);
                sum += poppedValue;
            }
        };
        
        vector<thread> threads;
        for (int i = 0; i < producerNum; i++)
            threads.push_back(thread(producer));
        for (int i = 0; i < consumerNum; i++)
            threads.push_back(thread(consumer));
        for (int i = 0; i < consumerNum + producerNum; i++)
            threads[i].join();
        
        cout << "consumerNum: " << consumerNum << " producerNum: " << producerNum << endl;
        assert(sum == taskNum * producerNum);
    }
public:
    static void task() {
        vector<int> consumerNums = {1, 2, 4};
        vector<int> produserNums = {1, 2, 4};
        vector<int> sizes = {1, 4, 16};
        int taskNum = 4 * 1024 * 1024;
        
        for (auto consumer: consumerNums) {
            for (auto producer: produserNums) {
                for (auto size: sizes) {
                    cout << "size: " << size << endl;
                    auto start = chrono::high_resolution_clock::now();
                    startTask(producer, consumer, taskNum, size);
                    auto end = chrono::high_resolution_clock::now();
                    auto time = chrono::duration_cast<chrono::milliseconds>(end - start);
                    cout << "Time: " << (double)time.count() / 1000 << "\n\n";
                }
            }
        }
    }
};
