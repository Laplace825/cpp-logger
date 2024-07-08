/**
 * @author: "laplace"
 * @date: 2024-07-03T15:35:29
 * @lastmod: 2024-07-08T16:50:37
 * @description:
 * @filePath: /logger/main.cc
 * @lastEditor: Laplace825
 * @Copyright: Copyright (c) 2024 by Laplace825, All Rights Reserved.
 */
#include "logger.hpp"

#include <string>
#include <vector>

int main()
{
    using namespace lap::logger;

    set_log_file("./log.txt");
    std::string str = "Hello";
    int x           = 1;
    log_trace("{},{}", str, x);
    log_info("{},{}", str, x);
    log_debug("{}", str);
    log_warn("{}", x);
    log_error("{}", x);
    log_fatal("{}", x);

    log_with_value_name(fatal, str);

    log(LogLevel::info, "{}", str);
    std::vector< int > vec{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    for (std::size_t i = 0; i < vec.size(); i++)
    {
        log_with_value_name(debug, i, vec[i]);
    }
}
