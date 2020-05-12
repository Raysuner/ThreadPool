#include "ThreadPool.h"
#include <iostream>

void fun(int i){
    std::cout << "fun" << i << std::endl;
}

void fun2(){
    std::cout << "fun2" << std::endl;
}

int main(){
    using task = std::function<void()>;
    ThreadPool pool(2);
    for(int i = 0; i < 10; ++i){
	pool.addTask(fun, 1);
	pool.addTask(fun2);
    }
}
