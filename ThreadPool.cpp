#include "test.h"
#include <iostream>

void fun(int i){
    std::cout << "fun" << i << std::endl;
}

int main(){
    ThreadPool pool(3);
    pool.addTask<std::function<void ()>(fun, 1);
    pool.addTask(fun2);
}