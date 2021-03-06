/*
 * Copyright 2013 Simon Brakhane
 *
 * This file is part of AnJaRoot.
 *
 * AnJaRoot is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * AnJaRoot is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * AnJaRoot. If not, see http://www.gnu.org/licenses/.
 */
#include <algorithm>
#include <system_error>
#include <errno.h>
#include <linux/user.h>
#include <signal.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>

#include "trace.h"
#include "shared/util.h"

trace::Tracee::Tracee(pid_t pid_) : pid(pid_), syscallBegin(false)
{
}

trace::Tracee::~Tracee()
{
}

pid_t trace::Tracee::getPid() const
{
    return pid;
}

bool trace::Tracee::detach() const
{
    int ret = ptrace(PTRACE_DETACH, pid, NULL, NULL);
    if(ret == -1)
    {
        util::logError("Failed to detach from %d: %s", pid, strerror(errno));
        return false;
    }

    util::logVerbose("Detached from %d", pid);
    return true;
}

void trace::Tracee::resume(int signal) const
{
    int ret = ptrace(PTRACE_CONT, pid, NULL, reinterpret_cast<void*>(signal));
    if(ret == -1)
    {
        util::logError("Failed to continue %d: %s", pid, strerror(errno));
        throw std::system_error(errno, std::system_category());
    }
}

void trace::Tracee::waitForSyscallResume() const
{
    int ret = ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
    if(ret == -1)
    {
        util::logError("Failed to syscall resume %d: %s",
                pid, strerror(errno));
        throw std::system_error(errno, std::system_category());
    }
}

void trace::Tracee::setupSyscallTrace() const
{
    int ret = ptrace(PTRACE_SETOPTIONS, pid, NULL,
            reinterpret_cast<void*>(PTRACE_O_TRACESYSGOOD));
    if(ret == -1)
    {
        util::logError("Failed to setup syscall signaling on %d: %s",
                pid, strerror(errno));
        throw std::system_error(errno, std::system_category());
    }
}

void trace::Tracee::setupChildTrace() const
{
    int ret = ptrace(PTRACE_SETOPTIONS, pid, NULL,
            reinterpret_cast<void*>(PTRACE_O_TRACEFORK));
    if(ret == -1)
    {
        util::logError("Failed to setup fork tracing on %d: %s",
                pid, strerror(errno));
        throw std::system_error(errno, std::system_category());
    }
}

unsigned long trace::Tracee::getEventMsg() const
{
    unsigned long result;
    int ret = ptrace(PTRACE_GETEVENTMSG, pid, NULL, &result);
    if(ret == -1)
    {
        util::logError("Failed to get event msg from %d: %s", pid,
                strerror(errno));
        throw std::system_error(errno, std::system_category());
    }

    return result;
}

siginfo_t trace::Tracee::getSignalInfo() const
{
    siginfo_t result;
    int ret = ptrace(PTRACE_GETSIGINFO, pid, NULL, &result);
    if(ret == -1)
    {
        util::logError("Failed to get signal info from %d: %s", pid,
                strerror(errno));
        throw std::system_error(errno, std::system_category());
    }

    return result;
}

bool trace::Tracee::isSyscallBegin() const
{
    return syscallBegin;
}

void trace::Tracee::setSyscallBegin(bool value)
{
    syscallBegin = value;
}

trace::WaitResult::WaitResult(pid_t pid_, int status_) : pid(pid_),
    status(status_)
{
}

void trace::WaitResult::logDebugInfo() const
{
    util::logError("PID: %d STATUS: %d EVENT: %d INSYSCALL: %d", pid, status,
            getEvent(), inSyscall());

    if(hasExited())
    {
        util::logError("WIFEXITED: %d WEXITSTATUS: %d",
                hasExited(), getExitStatus());
    }

    if(wasSignaled())
    {
        util::logError("WIFSIGNALED: %d WTERMSIG: %d",
                wasSignaled(), getTermSignal());
    }

    if(hasExited())
    {
        util::logError("WIFEXITED: %d WEXITSTATUS: %d WCOREDUMP: %d",
                hasExited(), getExitStatus(), wasCoredumped());
    }

    if(hasStopped())
    {
        util::logError("WIFSTOPPED: %d WSTOPSIG: %d",
                hasStopped(), getStopSignal());
    }
}

pid_t trace::WaitResult::getPid() const
{
    return pid;
}

int trace::WaitResult::getStatus() const
{
    return status;
}

bool trace::WaitResult::hasExited() const
{
    return WIFEXITED(status);
}

int trace::WaitResult::getExitStatus() const
{
    if(!hasExited())
    {
        util::logError("getExitStatus() called, but hasExited() is false");
    }

    return WEXITSTATUS(status);
}

bool trace::WaitResult::wasSignaled() const
{
    return WIFSIGNALED(status);
}

int trace::WaitResult::getTermSignal() const
{
    if(!wasSignaled())
    {
        util::logError("getTermSignal() called, but wasSignaled() is false");
    }

    return WTERMSIG(status);
}

bool trace::WaitResult::wasCoredumped() const
{
    if(!wasSignaled())
    {
        util::logError("wasCoredumped() called, but wasSignaled() is false");
    }

    return WCOREDUMP(status);
}

bool trace::WaitResult::hasStopped() const
{
    return WIFSTOPPED(status);
}

int trace::WaitResult::getStopSignal() const
{
    if(!hasStopped())
    {
        util::logError("getStopSignal() called, but hasStopped() is false");
    }

    return WSTOPSIG(status);
}

int trace::WaitResult::getEvent() const
{
    return status >> 16;
}

bool trace::WaitResult::inSyscall() const
{
    return getStopSignal() == (SIGTRAP | 0x80);
}

trace::Tracee::Ptr trace::attach(pid_t pid)
{
    int ret = ptrace(PTRACE_ATTACH, pid, NULL, NULL);
    if(ret == -1)
    {
        util::logError("Failed to attach to process: %s", strerror(errno));
        throw std::system_error(errno, std::system_category());
    }

    return std::make_shared<Tracee>(pid);
}

trace::WaitResult trace::waitChilds()
{
    int status;
    pid_t pid = wait(&status);
    return WaitResult(pid, status);
}

trace::WaitResult trace::waitChild(pid_t pid)
{
    int status;
    pid_t retpid = waitpid(pid, &status, NULL);
    return WaitResult(retpid, status);
}
