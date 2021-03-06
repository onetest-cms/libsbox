/*
 * Copyright (c) 2019 Andrei Odintsov <forestryks1@gmail.com>
 */

#include "testing.h"

#include <time.h>

static int invoker_main(const std::vector<std::string> &args) {
    GenericTarget target = GenericTarget::from_current_executable("target", args[0]);
    target.set_time_limit_ms(stoi(args[0]));
    Testing::safe_run({&target});
    target.print_stats(std::cerr);
    assert(target.is_time_limit_exceeded());
    assert(!target.is_wall_time_limit_exceeded());
    return 0;
}

static int target_main(const std::vector<std::string> &args) {
    double time_limit = static_cast<double>(stoi(args[0])) / 1000;
    while (static_cast<double>(clock()) / CLOCKS_PER_SEC < time_limit * 1.1) {
        asm volatile ("" : : : "memory");
    }
    return 0;
}

int main(int argc, char *argv[]) {
    Testing::add_handler("invoker", invoker_main);
    Testing::add_handler("target", target_main);
    Testing::start(argc, argv);
}
