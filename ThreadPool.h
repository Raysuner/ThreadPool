#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <stdexcept>
#include <vector>
#include <future>
#include <functional>
#include <atomic>
#include <type_traits>

class ThreadPool{
    public:
	ThreadPool(size_t thread_nums);
	~ThreadPool();
	template <class F, class... Args>
	    auto addTask(F &&f, Args&&... args);
    private:
	void executeTask(){
	    while(1){
		std::function<void()> current_task;
		{
		    std::unique_lock<std::mutex> locker(mutex_);
		    task_cv_.wait(locker, [this]{return !tasks_.empty() || is_shutdown;});
		    if(is_shutdown || tasks_.empty()){
			return;
		    }
		    current_task = std::move(tasks_.front());
		    tasks_.pop();
		}
		current_task();
	    }
	}

	std::mutex mutex_;
	std::condition_variable task_cv_;
	std::vector<std::thread> workers;
	std::queue<std::function<void()>> tasks_;
	std::atomic<bool> is_shutdown{false};
};

ThreadPool::ThreadPool(size_t thread_nums){
    for(size_t i = 0; i < thread_nums; ++i){
	workers.emplace_back(std::thread([this]{this->executeTask();}));
    }
}

ThreadPool::~ThreadPool(){
    is_shutdown = true;
    task_cv_.notify_all();
    for(std::thread &th : workers){
	th.join();
    }
}

template <class F, class... Args>
auto ThreadPool::addTask(F &&f, Args&&... args){
    using return_type = typename std::__invoke_result<F, Args...>::type;
    auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    std::future<return_type> res = task->get_future();
    {
	std::lock_guard<std::mutex> locker(mutex_);
	if(is_shutdown){
	    throw std::runtime_error("ThreadPool has shutdowned");
	}
	tasks_.emplace([task]{(*task)();});
    }
    task_cv_.notify_one();
    return res;
}
