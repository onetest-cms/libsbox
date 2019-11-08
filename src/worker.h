/*
 * Copyright (c) 2019 Andrei Odintsov <forestryks1@gmail.com>
 */

#ifndef LIBSBOX_WORKER_H
#define LIBSBOX_WORKER_H

#include "context_manager.h"
#include "shared_id_getter.h"
#include "shared_barrier.h"
#include "container.h"

#include <sys/signal.h>

class Worker : public ContextManager {
public:
    Worker(fd_t server_socket_fd, SharedIdGetter *id_getter);

    static Worker &get();
    pid_t start();

    [[noreturn]]
    void _die(const std::string &error) override;
    void terminate() override;

    [[nodiscard]] pid_t get_pid() const;

    std::pair<fd_t, fd_t> get_pipe(const std::string &pipe_name);
    SharedBarrier *get_run_start_barrier();
private:
    static Worker *worker_;
    fd_t server_socket_fd_;
    fd_t socket_fd_ = -1;
    SharedIdGetter *id_getter_;
    SharedBarrier run_start_barrier_{1};
    pid_t pid_{-1};

    volatile bool terminated_ = false;

    std::vector<std::unique_ptr<Container>> persistent_containers_;
    std::vector<std::unique_ptr<Container>> temporary_containers_;
    std::vector<Container *> containers_;

    std::map<std::string, std::pair<fd_t, fd_t>> pipes_;
    void close_pipes();

    [[noreturn]]
    void serve();
    std::string process(const std::string &request);
    nlohmann::json parse_json(const std::string &request);
    void prepare_containers(const nlohmann::json &json_request);
    void write_tasks(const nlohmann::json &json_request);
    void run_tasks();
    std::string collect_results();

    static void sigchld_action(int, siginfo_t *siginfo, void *);
};

#endif //LIBSBOX_WORKER_H
