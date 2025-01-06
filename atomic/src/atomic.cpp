#include <atomic>
#include <iostream>
#include <vector>
#include <thread>
#include <cassert>

class SpinLock {

public:
	void lock() {

		while (flag.test_and_set(std::memory_order_acquire));	//×ÔÐýµÈ´ý
		
	}

	void unlock() {
		flag.clear(std::memory_order_release); //ÊÍ·ÅËø;
	}


private:
	std::atomic_flag flag = ATOMIC_FLAG_INIT;
};

void TestSpinLock() {
    SpinLock spinlock;
    std::thread t1([&spinlock]() {
        spinlock.lock();
        for (int i = 0; i < 3; i++) {
            std::cout << "*";
        }
        std::cout << std::endl;
        spinlock.unlock();
        });
    std::thread t2([&spinlock]() {
        spinlock.lock();
        for (int i = 0; i < 3; i++) {
            std::cout << "?";
        }
        std::cout << std::endl;
        spinlock.unlock();
        });
    t1.join();
    t2.join();
}

std::atomic<bool> x, y;
std::atomic<int> z;
void write_x_then_y() {
    x.store(true, std::memory_order_relaxed);  // 1
    y.store(true, std::memory_order_relaxed);  // 2
}
void read_y_then_x() {
    while (!y.load(std::memory_order_relaxed)) { // 3
        std::cout << "y load false" << std::endl;
    }
    if (x.load(std::memory_order_relaxed)) { //4
        ++z;
    }
}
void TestOrderRelaxed() {
    std::thread t1(write_x_then_y);
    std::thread t2(read_y_then_x);
    t1.join();
    t2.join();
    assert(z.load() != 0); // 5
}
void write_x_then_y1() {
    x.store(true, std::memory_order_seq_cst);  // 1
    y.store(true, std::memory_order_seq_cst);  // 2
}
void read_y_then_x1() {
    while (!y.load(std::memory_order_seq_cst)) { // 3
        std::cout << "y load false" << std::endl;
    }
    if (x.load(std::memory_order_seq_cst)) { //4
        ++z;
    }
}
void TestOrderSeqCst() {
    std::thread t1(write_x_then_y1);
    std::thread t2(read_y_then_x1);
    t1.join();
    t2.join();
    assert(z.load() != 0); // 5
}

int main() {
    //TestSpinLock();
    //TestOrderRelaxed();
    TestOrderSeqCst();
}