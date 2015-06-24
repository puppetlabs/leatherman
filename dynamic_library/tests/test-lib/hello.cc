#include <iostream>

void goodbye();

extern "C" void hello(){
    std::cout << "Hello world!";
}
