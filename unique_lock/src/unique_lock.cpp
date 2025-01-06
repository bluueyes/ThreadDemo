#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <map>

//unique_lock 基本用法
std::mutex mtx;
int shared_data = 0;
void use_unique() {
    //lock可自动解锁，也可手动解锁
    std::unique_lock<std::mutex> lock(mtx);
    std::cout << "lock success" << std::endl;
    shared_data++;
    lock.unlock();
}

//通过lock.owns_lock()判断unique_lock是否有锁
void owns_lock() {
    //lock可自动解锁，也可手动解锁
    std::unique_lock<std::mutex> lock(mtx);
    shared_data++;
    if (lock.owns_lock()) {
        std::cout << "owns lock" << std::endl;
    }
    else {
        std::cout << "doesn't own lock" << std::endl;
    }
    lock.unlock();
    if (lock.owns_lock()) {
        std::cout << "owns lock" << std::endl;
    }
    else {
        std::cout << "doesn't own lock" << std::endl;
    }
}

//同时使用owns和defer
void use_own_defer() {
    std::unique_lock<std::mutex>  lock(mtx);
    // 判断是否拥有锁
    if (lock.owns_lock())
    {
        std::cout << "Main thread has the lock." << std::endl;
    }
    else
    {
        std::cout << "Main thread does not have the lock." << std::endl;
    }
    std::thread t([]() {
        std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
        // 判断是否拥有锁
        if (lock.owns_lock())
        {
            std::cout << "Thread has the lock." << std::endl;
        }
        else
        {
            std::cout << "Thread does not have the lock." << std::endl;
        }
        // 加锁
        lock.lock();
        // 判断是否拥有锁
        if (lock.owns_lock())
        {
            std::cout << "Thread has the lock." << std::endl;
        }
        else
        {
            std::cout << "Thread does not have the lock." << std::endl;
        }
        // 解锁
        lock.unlock();
        });
    t.join();
}

//同样支持领养操作
void use_own_adopt() {
    mtx.lock();
    std::unique_lock<std::mutex> lock(mtx, std::adopt_lock);
    if (lock.owns_lock()) {
        std::cout << "owns lock" << std::endl;
    }
    else {
        std::cout << "does not have the lock" << std::endl;
    }
    lock.unlock();
}

int a = 10;
int b = 99;
std::mutex  mtx1;
std::mutex  mtx2;
//交给unique_lock之后的锁不能在自己解锁
void safe_swap() {
    std::lock(mtx1, mtx2);
    std::unique_lock<std::mutex> lock1(mtx1, std::adopt_lock);
    std::unique_lock<std::mutex> lock2(mtx2, std::adopt_lock);
    std::swap(a, b);
    //错误用法
    //mtx1.unlock();
    //mtx2.unlock();
}

void safe_swap2() {
    std::unique_lock<std::mutex> lock1(mtx1, std::defer_lock);
    std::unique_lock<std::mutex> lock2(mtx2, std::defer_lock);
    //需用lock1,lock2加锁
    std::lock(lock1, lock2);
    //错误用法
    //std::lock(mtx1, mtx2);
    std::swap(a, b);
}

//转移互斥量所有权
//互斥量本身不支持move操作，但是unique_lock支持
// 这里返回没有拷贝构造所以用移动构造返回
//这里unique_lock返回一个临时变量被当作右值给lock移动构造了
std::unique_lock <std::mutex>  get_lock() {
    std::unique_lock<std::mutex>  lock(mtx);
    shared_data++;
    return lock;
}
void use_return() {
    std::unique_lock<std::mutex> lock(get_lock());
    shared_data++;
}

void precision_lock() {
    std::unique_lock<std::mutex> lock(mtx);
    shared_data++;
    lock.unlock();
    //不设计共享数据的耗时操作不要放在锁内执行
    std::this_thread::sleep_for(std::chrono::seconds(1));
    lock.lock();
    shared_data++;
} 

class DNService {
public:
    DNService() {}
    //读操作采用共享锁
    std::string QueryDNS(std::string dnsname) {
        std::shared_lock<std::shared_mutex> shared_locks(_shared_mtx);
        auto iter = _dns_info.find(dnsname);
        if (iter != _dns_info.end()) {
            return iter->second;
        }
        return "";
    }
    //写操作采用独占锁
    void AddDNSInfo(std::string dnsname, std::string dnsentry) {
        std::lock_guard<std::shared_mutex>  guard_locks(_shared_mtx);
        _dns_info.insert(std::make_pair(dnsname, dnsentry));
    }
private:
    std::map<std::string, std::string> _dns_info;
    mutable std::shared_mutex  _shared_mtx;
};

class RecursiveDemo {
public:
    RecursiveDemo() {}
    bool QueryStudent(std::string name) {
        std::lock_guard<std::recursive_mutex>  recursive_lock(_recursive_mtx);
        auto iter_find = _students_info.find(name);
        if (iter_find == _students_info.end()) {
            return false;
        }
        return true;
    }
    void AddScore(std::string name, int score) {
        std::lock_guard<std::recursive_mutex>  recursive_lock(_recursive_mtx);
        if (!QueryStudent(name)) {
            _students_info.insert(std::make_pair(name, score));
            return;
        }
        _students_info[name] = _students_info[name] + score;
    }
    //不推荐采用递归锁，使用递归锁说明设计思路并不理想，需优化设计
    //推荐拆分逻辑，将共有逻辑拆分为统一接口
    void AddScoreAtomic(std::string name, int score) {
        std::lock_guard<std::recursive_mutex>  recursive_lock(_recursive_mtx);
        auto iter_find = _students_info.find(name);
        if (iter_find == _students_info.end()) {
            _students_info.insert(std::make_pair(name, score));
            return;
        }
        _students_info[name] = _students_info[name] + score;
        return;
    }
private:
    std::map<std::string, int> _students_info;
    std::recursive_mutex   _recursive_mtx;
};

int main() {

    //use_unique();
    //owns_lock();
    //use_own_defer();
    //use_own_adopt();
    return 0;
}