#include <iostream>
#include <mutex>
#include <thread>
#include <stack>

std::mutex mtx1;
int shared_data = 100;


void use_lock() {

	while (true) {
		mtx1.lock();
		shared_data++;
		std::cout << "current thread is " << std::this_thread::get_id() << std::endl;
		std::cout << "share data is" << shared_data << std::endl;

		mtx1.unlock();
		std::this_thread::sleep_for(std::chrono::microseconds(10));
	}
}

void tese_lock() {

	std::thread t1(use_lock);
	std::thread t2([]()
	{
		while (true) {

			{
				std::lock_guard<std::mutex> lk_guard(mtx1);
				shared_data--;
				std::cout << "current thread is " << std::this_thread::get_id() << std::endl;
				std::cout << "shared_data is" << shared_data << std::endl;
				
			}
			std::this_thread::sleep_for(std::chrono::microseconds(10));
		}
	});

	t1.join();
	t2.join();

}

template<typename T>
class threadsafe_stack1 {

private:
	std::stack<T> data;
	mutable std::mutex m;

public:
	threadsafe_stack1() {}
	threadsafe_stack1(const threadsafe_stack1& other){
		std::lock_guard<std::mutex> lock(other.m);
		data = other.data;
	}

	threadsafe_stack1& operator=(const threadsafe_stack1&) = delete;
	
	void push(T new_value) {
		std::lock_guard<std::mutex> lock(m);
		data.push(std::move(new_value));
	}

	//�������
	T pop() {
		std::lock_guard<std::mutex> lock(m);
		auto element = data.top();
		data.pop();
		return element;
	}

	//Σ��
	bool empty() const{
		std::lock_guard<std::mutex> lock(m);
		return data.empty();
	}
};



struct empty_stack :std::exception {
	const char* what() const throw();
};

template<typename T>
class threadsafe_stack {
private:
	std::stack<T> data;
	mutable std::mutex m;

public:
	threadsafe_stack() {}
	threadsafe_stack(const threadsafe_stack& other) {
		std::lock_guard<std::mutex> lock(other.m);
		data = other.data;
	}

	threadsafe_stack& operator=(const threadsafe_stack&) = delete;

	void push(T new_value) {
		std::lock_guard<std::mutex> lock(m);
		data.push(std::move(new_value));
	}

	//����취1 ����ָ��
	std::shared_ptr<T> pop() {
		std::lock_guard<std::mutex> lock(m);
		if (data.empty()) {
			return nullptr;
		}
		std::shared_ptr<T> const res(std::make_shared<T>(data.top()));
		data.pop();
		return res;
	}

	void pop(T& value) {
		std::lock_guard<std::mutex> lock(m);
		if (data.empty()) {
			throw empty_stack();
		}
		value = data.top();
		data.pop();
	}

	//Σ��
	bool empty() const {
		std::lock_guard<std::mutex> lock(m);
		return data.empty();
	}
};

void test_threadsafe_stack1() {

	threadsafe_stack1<int> safe_stack;
	safe_stack.push(1);
	//safe_stack.push(2);

	std::thread t1([&safe_stack]() {
		if (!safe_stack.empty()) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			
			safe_stack.pop();
		}
		});

	std::thread t2([&safe_stack]() {
		if (!safe_stack.empty()) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			safe_stack.pop();
		}
		});

	t1.join();
	t2.join();

}


std::mutex t_lock_1;
std::mutex t_lock_2;

int m_1 = 0;
int m_2 = 1;

//����
void dead_lock1(){

	while (true) {
		std::cout << "dead_lock1" << std::endl;
		t_lock_1.lock();
		m_1 = 1024;
		t_lock_2.lock();
		m_2 = 2048;
		t_lock_2.unlock();
		t_lock_1.unlock();
		std::cout << "dead_lock2 end" << std::endl;
	}
}

void dead_lock2() {

	while (true) {
		std::cout << "dead_lock1" << std::endl;
		t_lock_2.lock();
		m_1 = 1024;
		t_lock_1.lock();
		m_2 = 2048;
		t_lock_1.unlock();
		t_lock_2.unlock();
		std::cout << "dead_lock2 end" << std::endl;
	}
}

void test_dead_lock() {

	std::thread t1(dead_lock1);
	std::thread t2(dead_lock2);

	t1.join();
	t2.join();
}

//�����ͽ�����Ϊԭ�Ӳ�������ϣ�����ֻ�����Լ��Ĺ���
void atomic_lock1() {
	std::cout << "lock1 begin lock" << std::endl;
	t_lock_1.lock();
	m_1 = 1024;
	t_lock_1.unlock();
	std::cout << "lock1 end lock" << std::endl;
}
void atomic_lock2() {
	std::cout << "lock2 begin lock" << std::endl;
	t_lock_2.lock();
	m_2 = 2048;
	t_lock_2.unlock();
	std::cout << "lock2 end lock" << std::endl;
}
void safe_lock1() {
	while (true) {
		atomic_lock1();
		atomic_lock2();
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
}
void safe_lock2() {
	while (true) {
		atomic_lock2();
		atomic_lock1();
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
}
void test_safe_lock() {
	std::thread t1(safe_lock1);
	std::thread t2(safe_lock2);
	t1.join();
	t2.join();
}

//����Ҫʹ������������������ͬʱ����
//�����ͬʱ�����ᵼ������

//��������һ�����ӵ����ݽṹ���������ǲ�����ִ�п�������
class some_big_object {

public:
	some_big_object(int data) :_data(data) {}

	//��������
	some_big_object(const some_big_object& b2) :_data(b2._data) {
	
	}

	//�ƶ�����
	some_big_object(some_big_object&& b2) :_data(std::move(b2._data)) {

	}

	//���ظ�ֵ�����
	some_big_object& operator=(const some_big_object& b2) {
		if (this==&b2) {
			return *this;
		}

		_data = b2._data;
		return *this;
	}

	//���ظ�ֵ�����
	some_big_object& operator=(some_big_object&& b2) {

		_data = std::move(b2._data);
		return *this;
	}


	//������������
	friend std::ostream& operator<<(std::ostream& os, const some_big_object& big_obj) {
		os << big_obj._data;
		return os;
	}

	//��������
	friend void swap(some_big_object& b1,some_big_object& b2) {
		some_big_object temp = std::move(b1);
		b1 = std::move(b2);
		b2 = std::move(temp);
	}

private:
	int _data;
};


//��������Ҫ���ṹ�������˸��ӵĳ�Ա�����һ��������
class big_object_mgr {

public:
	big_object_mgr(int data = 0) :_obj(data) {}
	void printinfo() {
		std::cout << "current obj data is " << _obj << std::endl;
		
	}

	friend void danger_swap(big_object_mgr& b1, big_object_mgr& b2);
	friend void safe_swap(big_object_mgr& b1, big_object_mgr& b2);
	friend void safe_swap_scope(big_object_mgr& b1, big_object_mgr& b2);
private:
	std::mutex _mtx;
	some_big_object _obj;
};

void danger_swap(big_object_mgr& b1, big_object_mgr& b2) {
	std::cout << "thread [ " << std::this_thread::get_id() << " ] begin" << std::endl;
	if (&b1 == &b2) {
		return;
	}

	std::lock_guard<std::mutex> guard1(b1._mtx);
	//�˴�Ϊ�����������֣���˯һ��
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::lock_guard<std::mutex> guard2(b2._mtx);

	swap(b1._obj, b2._obj);
	std::cout << "thread [ " << std::this_thread::get_id() << " ] end" << std::endl;
}

void test_danger_swap() {

	big_object_mgr obj1(5);
	big_object_mgr obj2(100);

	std::thread t1(danger_swap, std::ref(obj1), std::ref(obj2));
	std::thread t2(danger_swap, std::ref(obj2), std::ref(obj1));

	t1.join();
	t2.join();

	obj1.printinfo();
	obj2.printinfo();
}

void safe_swap(big_object_mgr& b1, big_object_mgr& b2) {

	std::cout << "thread [ " << std::this_thread::get_id() << " ] begin" << std::endl;
	if (&b1 == &b2) {
		return;
	}

	std::lock(b1._mtx, b2._mtx);
	//����������������
	std::lock_guard<std::mutex> guard1(b1._mtx, std::adopt_lock);
	//�˴�Ϊ�����������֣���˯һ��
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::lock_guard<std::mutex> guard2(b2._mtx, std::adopt_lock);

	swap(b1._obj, b2._obj);
	std::cout << "thread [ " << std::this_thread::get_id() << " ] end" << std::endl;
}

void test_swap() {

	big_object_mgr obj1(5);
	big_object_mgr obj2(100);

	std::thread t1(safe_swap, std::ref(obj1), std::ref(obj2));
	std::thread t2(safe_swap, std::ref(obj2), std::ref(obj1));

	t1.join();
	t2.join();

	obj1.printinfo();
	obj2.printinfo();
}

void safe_swap_scope(big_object_mgr& b1, big_object_mgr& b2) {
	std::cout << "thread [ " << std::this_thread::get_id() << " ] begin" << std::endl;
	if (&b1 == &b2) {
		return;
	}

	std::scoped_lock guard(b1._mtx, b2._mtx);
	swap(b1._obj, b2._obj);
	std::cout << "thread [ " << std::this_thread::get_id() << " ] end" << std::endl;
}

void test_swap_scope() {

	big_object_mgr obj1(5);
	big_object_mgr obj2(100);

	std::thread t1(safe_swap_scope, std::ref(obj1), std::ref(obj2));
	std::thread t2(safe_swap_scope, std::ref(obj2), std::ref(obj1));

	t1.join();
	t2.join();

	obj1.printinfo();
	obj2.printinfo();
}

//�㼶��
class hierarchical_mutex {
public:
	explicit hierarchical_mutex(unsigned long value) :hierarchy_value(value)
		, _previous_hierarchy_value(0) {
	}

	hierarchical_mutex(const hierarchical_mutex&) = delete;
	hierarchical_mutex& operator=(const hierarchical_mutex&) = delete;

	void lock() {
		check_for_hierarchy_violation();
		_internal_mutex.lock();
		update_hierarchy_violation();
	}

	void unlock() {
		if (_this_thread_hierarchy_value != hierarchy_value) {
			throw std::logic_error("mutex hierary error");
		}
		_this_thread_hierarchy_value = _previous_hierarchy_value;
		_internal_mutex.unlock();
	}

	bool try_lock() {

		check_for_hierarchy_violation();
		if (!_internal_mutex.try_lock()) {
			return false;
		}

		update_hierarchy_violation();
		return true;

	}

private:
	std::mutex _internal_mutex;
	//����ǰ�Ĳ㼶ֵ
	unsigned long const hierarchy_value;
	//��һ���㼶��
	unsigned long _previous_hierarchy_value;
	//���̼߳�¼�Ĳ㼶ֵ
	static thread_local unsigned long _this_thread_hierarchy_value;

	void check_for_hierarchy_violation() {
		if (_this_thread_hierarchy_value <= hierarchy_value) {
			throw std::logic_error("mutex hierarchy violated");
		}
	}

	void update_hierarchy_violation() {
		_previous_hierarchy_value = _this_thread_hierarchy_value;
		_this_thread_hierarchy_value = hierarchy_value;
	}
};

thread_local unsigned long hierarchical_mutex::_this_thread_hierarchy_value(ULONG_MAX);

void test_hierarchy_lock() {

	hierarchical_mutex hmtx1(1000);
	hierarchical_mutex hmtx2(500);

	std::thread t1([&hmtx1, &hmtx2]() {
		hmtx1.lock();
		hmtx2.lock();
		hmtx2.unlock();
		hmtx1.unlock();
		});

	std::thread t2([&hmtx1, &hmtx2]() {
		hmtx2.lock();
		hmtx1.lock();
		hmtx1.unlock();
		hmtx2.unlock();
		});
	t1.join();
	t2.join();
}


int main() {

	//tese_lock();
	//test_threadsafe_stack1();
	//test_dead_lock();
	//test_safe_lock();

	//some_big_object b1(100);
	//some_big_object b2(200);

	//����ûʵ���ƶ���ֵĬ���ߵ��ǿ�����ֵ
	//b1 = std::move(b2);
	
	//test_swap_scope();
	test_hierarchy_lock();

	
	return 0;
}