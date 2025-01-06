#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <memory>
//#include <condition_variable>

int num = 1;
std::mutex mt;
std::condition_variable va1;
std::condition_variable va2;

void PoorImplemam(){

	std::thread t1([]() {
		for (;;) {
			{
				std::lock_guard<std::mutex> lock(mt);
				if (num == 1) {

					std::cout << "thread A print 1....." << std::endl;
					num++;
					continue;
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
		});

	std::thread t2([]() {
		for (;;) {
			{
				std::lock_guard<std::mutex> lock(mt);
				if (num == 2) {

					std::cout << "thread B print 2....." << std::endl;
					num--;
					continue;
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
		});
	t1.join();
	t2.join();
}

void ResonableImplemention() {

	std::thread t1([]() {

		for (;;) {
			std::unique_lock<std::mutex> lock(mt);
			va1.wait(lock, []() {
				return 1 == num;
				});
			num++;
			std::cout << "thread A print 1....." << std::endl;
			va2.notify_one();
		}
		});


	std::thread t2([]() {

		for (;;) {
			std::unique_lock<std::mutex> lock(mt);
			va2.wait(lock, []() {
				return 2 == num;
				});
			num--;
			std::cout << "thread B print 2....." << std::endl;
			va1.notify_one();
		}
		});

	t1.join();
	t2.join();
		
}

template<typename T>
class threadsafe_queue {

private:
	mutable std::mutex mt;
	std::queue<T> data_queue;
	std::condition_variable data_cond;
public:
	threadsafe_queue() {}
	threadsafe_queue(const threadsafe_queue& other) {
		
		std::lock_guard<std::mutex> lock(mt);
		data_queue = other.data_queue;
	}
	void push(T new_value) {
		std::lock_guard<std::mutex> lock(mt);
		data_queue.push(new_value);
	}

	void wait_and_pop(T& value) {
		std::unique_lock <std::mutex> lock(mt);
		data_cond.wait(lock, [this]() {
			return !data_queue.empty();
			});
		value = data_queue.front();
		data_queue.pop();
	}

	std::shared_ptr<T> wait_and_pop() {
		std::unique_lock < std::mutex > lock(mt);
		data_cond.wait(lock, [this]() {
			return !data_queue.empty();
			});
		std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
		data_queue.pop();
		return res;
	}

	bool try_pop(T& value) {
		
		std::lock_guard<std::mutex> lock(mt);
		if (data_queue.empty()) {
			return false;
		}
		value = data_queue.front();
		data_queue.pop();
		return true;
	}

	std::shared_ptr<T> try_pop() {

		std::lock_guard<std::mutex> lock(mt);
		if (data_queue.empty()) {
			return std::shared_ptr<T>();
		}
		std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
		data_queue.pop();
		return res;
	}

	bool empty() const {

		std::lock_guard<std::mutex> lock(mt);
		return data_queue.empty();
	}
};

void test_safe_que() {
	threadsafe_queue<int>  safe_que;
	std::mutex  mtx_print;
	std::thread producer(
		[&]() {
			for (int i = 0; ;i++) {
				safe_que.push(i);
				{
					std::lock_guard<std::mutex> printlk(mtx_print);
					std::cout << "producer push data is " << i << std::endl;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}
		}
	);
	std::thread consumer1(
		[&]() {
			for (;;) {
				auto data = safe_que.wait_and_pop();
				{
					std::lock_guard<std::mutex> printlk(mtx_print);
					std::cout << "consumer1 wait and pop data is " << *data << std::endl;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
			}
		}
	);
	std::thread consumer2(
		[&]() {
			for (;;) {
				auto data = safe_que.try_pop();
				if (data != nullptr) {
					{
						std::lock_guard<std::mutex> printlk(mtx_print);
						std::cout << "consumer2 try_pop data is " << *data << std::endl;
					}
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
			}
		}
	);
	producer.join();
	consumer1.join();
	consumer2.join();
}

int main() {

	//PoorImplemam();
	//ResonableImplemention();
	test_safe_que();
	
	return 0;
}