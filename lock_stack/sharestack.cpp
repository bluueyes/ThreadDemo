#include <iostream>
#include <stack>
#include <string>
#include <mutex>
#include <condition_variable>

template<typename T>
class safe_stack {

public:
	safe_stack(){}
	safe_stack(const safe_stack<T>& other) {
		std::lock_guard<std::mutex> lock(other.mt);
		std::cout << "¿½±´" << std::endl;
		data = other.data;
	}
	safe_stack<T>& operator=(const safe_stack<T>& st)= delete;

	safe_stack(safe_stack<T>&& other) {
		
		data=std::move(other.data);
		std::cout << "ÒÆ¶¯" << std::endl;
	}

	void push(T value) {

		std::lock_guard<std::mutex> lock(mt);

		data.push(value);
		vm.notify_one();
	}
	
	std::shared_ptr<T> pop_wait() {

		std::unique_lock<std::mutex> lock(mt);
		vm.wait(lock, [this]() {
			if (data.empty())
				return false;
			return true;
			});

		const std::shared_ptr<T> ret(std::make_shared<T>(std::move(data.top())));
		data.pop();
		return ret;
	}

	void pop_wait(T& value) {
		std::unique_lock<std::mutex> lock(mt);
		vm.wait(mt, [this]() {
			if (data.empty()) return false;
			return true;
			});
		value = std::move(data.top());
		data.pop();
	}

	bool try_pop(T& value) {
		std::lock_guard<std::mutex> lock(mt);
		if (data.empty()) return false;

		value = std::move(data.top());
		data.pop();
		return true;
	}

	std::shared_ptr<T> try_pop() {

		std::lock_guard<std::mutex> lock(mt);
		if (data.empty()) return std::shared_ptr<T>();

		std::shared_ptr<T> ret(std::make_shared<T>(std::move(data.top())));
		data.pop();

		return ret;
	}


	bool empty() const {

		std::lock_guard<std::mutex> lock(mt);
		if (data.empty()) return true;
		return false;
	}



private:
	mutable std::mutex mt;
	std::condition_variable vm;
	std::stack<T> data;
};

class MyClass {

public:
	MyClass(int value):_value(value){}
	MyClass(const MyClass& mc) { _value = mc._value; }
	MyClass(const MyClass&& mc) noexcept{ _value = mc._value; }

	friend std::ostream& operator<<(std::ostream& os, const MyClass& c) {

		std::cout << c._value;
		return os;
	}
private:
	int _value;
};

void test(const safe_stack<std::string>&& s) {

	std::cout << &s << std::endl;
}

std::mutex mt;
void PrintCosumer(const std::string& str,std::shared_ptr<MyClass> mc) {

	std::lock_guard<std::mutex> lock(mt);
	std::cout << str << *mc << std::endl;
}

void testThread() {

	safe_stack<MyClass> st;

	std::thread consumer1([&]() {
		for (;;) {
			std::shared_ptr<MyClass> data = st.pop_wait();
			PrintCosumer("consumer1: ", data);
		}
		});

	std::thread consumer2([&]() {
		for (;;) {
			std::shared_ptr<MyClass> data = st.pop_wait();
			PrintCosumer("consumer2: ", data);
		}
		});

	std::thread producer([&]() {
		for (int i = 0;i < 100;i++) {

			MyClass mc(i);
			st.push(mc);

		}

		});

	consumer1.join();
	consumer2.join();
	producer.join();
}


int main() {

	safe_stack<std::string> s1;

	//safe_stack<std::string> s2=std::move(s1);

	//s1.push("hjj");
	//auto p=s1.pop_wait();

	//test(safe_stack<std::string>());
	//std::cout <<"main" << *p << std::endl;

	testThread();
}