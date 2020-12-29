#pragma once

#include <stdio.h>
#include <iostream>
#include <QtCore>

#ifdef Q_OS_LINUX

#include <signal.h>
#include <ucontext.h>
#include <execinfo.h>

void linuxSignalStacktraceHandler(int signalNumber, siginfo_t *, void * secret ) noexcept;

#endif

void installSignalHandlers() noexcept;
