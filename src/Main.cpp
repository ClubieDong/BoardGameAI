#include "Server/Server.hpp"
#include <iostream>

int main() {
    Server(std::cin, std::cout).Run();
    return 0;
}
