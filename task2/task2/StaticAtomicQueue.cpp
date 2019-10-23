#include <stdio.h>
#include "Queue.cpp"

class StaticAtomicQueue : public Queue {
private:
    byte* q;
    mutex m;
    int size;
    atomic<int> pushIndex, popIndex;
    
public:
    
    StaticAtomicQueue(int s) {
        pushIndex = 0;
        popIndex = 0;
        size = s;
        q = new byte[size];
        for (int i = 0; i < size; i ++)
            q[i] = 0;
    }
    
    void push(byte val) override {
        while (true) {
            int currentPushIndex = pushIndex.load();
            while (!pushIndex.compare_exchange_weak(currentPushIndex,
                                                    currentPushIndex + 1));
            if (currentPushIndex  == popIndex.load() + size) {
                continue;
            }
            q[currentPushIndex % size] = val;
            return;
        }
    }
    
    bool pop(byte& val) override {
        int currentPopIndex = popIndex.load();
        if (currentPopIndex  == pushIndex.load())
            return false;
        if (popIndex.compare_exchange_weak(currentPopIndex,
                                           currentPopIndex + 1)) {
            val = q[currentPopIndex % size];
            return true;
            
        }
        return false;
    }
};

class StaticAtomicQueueTask {
    static void startTask(int producerNum, int consumerNum, int taskNum, int size) {
        StaticAtomicQueue q(size);
        atomic<int> sum(0);
        
        auto producer = [&]() {
            for (int i = 0; i < taskNum; i ++) {
                q.push(1);
            }
            return 0;
        };
        
        auto consumer = [&]() {
            for (int i = 0; i < taskNum * producerNum / consumerNum; i ++) {
                byte poppedValue = 0;
                while(!q.pop(poppedValue));
                sum += poppedValue;
            }
        };
        
        vector<thread> threads;;
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
