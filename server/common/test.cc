#include "utils.hpp"
#include <iostream>
int main()
{
    for (int i = 0; i < 10; i++)
    {
        std::cout << MY_IM::Uuid() << std::endl;
    }
    return 0;
}