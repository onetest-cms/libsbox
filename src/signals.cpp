/*
 * Copyright (c) 2019 Andrei Odintsov <forestryks1@gmail.com>
 */

#include "signals.h"
#include "context_manager.h"
#include "utils.h"

#include <sys/signal.h>
#include <sys/time.h>
#include <cstring>
#include <map>

namespace {
class SignalAction {
public:
    static const SignalAction IGNORE;
    static const SignalAction DEFAULT;
    static const SignalAction ABORT;
    static const SignalAction TERMINATE;
    static const SignalAction TIMER;

    void apply_to(int signum, bool restart = false) const {
        if (restart) {
            sigaction.sa_flags |= SA_RESTART;
        }
        if (::sigaction(signum, &sigaction, nullptr)) {
            ContextManager::get().die(format("Cannot set signal action for %s: %m", strsignal(signum)));
        }
        if (restart) {
            sigaction.sa_flags &= (~SA_RESTART);
        }
    }

    explicit SignalAction(void (*handler)(int)) {
        memset(&sigaction, 0, sizeof(sigaction));
        sigaction.sa_handler = handler;
    }

private:
    mutable struct sigaction sigaction{};
};

void abort_handler(int signum) {
    ContextManager::get().die(format("Received fatal signal %d (%s)", signum, strsignal(signum)));
}

void terminate_handler(int) {
    timer_interrupt = false;
    ContextManager::get().terminate();
}

void timer_handler(int) {
    timer_interrupt = true;
}

const SignalAction SignalAction::IGNORE(SIG_IGN);
const SignalAction SignalAction::DEFAULT(SIG_DFL);
const SignalAction SignalAction::ABORT(abort_handler);
const SignalAction SignalAction::TERMINATE(terminate_handler);
const SignalAction SignalAction::TIMER(timer_handler);

const std::map<int, std::reference_wrapper<const SignalAction>> signal_actions = {
    {SIGUSR1, std::ref(SignalAction::IGNORE)},
    {SIGUSR2, std::ref(SignalAction::IGNORE)},
    {SIGPIPE, std::ref(SignalAction::IGNORE)},
    {SIGHUP, std::ref(SignalAction::ABORT)},
    {SIGQUIT, std::ref(SignalAction::ABORT)},
    {SIGILL, std::ref(SignalAction::ABORT)},
    {SIGABRT, std::ref(SignalAction::ABORT)},
    {SIGIOT, std::ref(SignalAction::ABORT)},
    {SIGBUS, std::ref(SignalAction::ABORT)},
    {SIGFPE, std::ref(SignalAction::ABORT)},
    {SIGINT, std::ref(SignalAction::ABORT)},
    {SIGTERM, std::ref(SignalAction::TERMINATE)},
    {SIGALRM, std::ref(SignalAction::TIMER)},
    {SIGCHLD, std::ref(SignalAction::DEFAULT)}
};
} // namespace

bool timer_interrupt = false;

void prepare_signals() {
    for (const auto &signal_action : signal_actions) {
        signal_action.second.get().apply_to(signal_action.first);
    }
}

void reset_signals() {
    for (const auto &signal_action : signal_actions) {
        SignalAction::DEFAULT.apply_to(signal_action.first);
    }
}

void set_standard_handler_restart(int signum, bool restart) {
    try {
        signal_actions.at(signum).get().apply_to(signum, restart);
    } catch (std::out_of_range &e) {
        ContextManager::get().die(format("%d is invalid signum", signum));
    }
}

void set_custom_handler(int signum, void (*handler)(int), bool restart) {
    SignalAction signal_action(handler);
    signal_action.apply_to(signum, restart);
}

void start_timer(long interval) {
    struct itimerval timer = {};
    memset(&timer, 0, sizeof(timer));
    timer.it_interval.tv_usec = interval * 1000;
    timer.it_value.tv_usec = interval * 1000;
    setitimer(ITIMER_REAL, &timer, nullptr);
}

void stop_timer() {
    struct itimerval timer = {};
    memset(&timer, 0, sizeof(timer));
    setitimer(ITIMER_REAL, &timer, nullptr);
}