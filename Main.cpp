#include <iostream>

int main(int argc, char** argv)
{
    __noop(argc);
    __noop(argv);

    std::cout << "Hello World!" << std::endl; 

    return 0;
}