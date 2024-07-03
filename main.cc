#include "logger.hpp"

#include <string>

int main()
{
    using namespace lap::logger;

    std::string str = "Hello";
    int x           = 1;
    log_trace("{}", str);
    log_info("{}", str);
    log_debug("{}", str);
    log_warning("{}", x);
    log_error("{}", x);
    log_fatal("{}", x);
    LOG_Println(str);
}
