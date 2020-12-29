#include "stacktrace.h"

#ifdef Q_OS_LINUX

#include <Logger.h>

namespace {
const static int STACKTRACE_DEPTH = 64;
}

std::string exec( const char * cmd )
{
    std::array< char, 128 > buffer;
    std::string result;
    std::unique_ptr< FILE, decltype( &pclose ) > pipe( popen( cmd, "r" ), pclose );
    if( !pipe )
    {
        return "popen() failed!";
    }
    while( fgets( buffer.data(), buffer.size(), pipe.get() ) != nullptr )
    {
        result += buffer.data();
    }
    return result;
}

void linuxSignalStacktraceHandler( int signalNumber, siginfo_t * /*info*/, void * secret ) noexcept
{

    auto * ucontext = static_cast<ucontext_t *>(secret);
    if ( signalNumber == SIGSEGV )
    {
        LOG_ERROR( QString("Got signal %1 from %2")
                   .arg(signalNumber)
//                   .arg(reinterpret_cast<int>(info->si_addr))
                   .arg(ucontext->uc_mcontext.gregs[ REG_RIP ] ));
    }
    else
    {
        LOG_ERROR("Got signal %d", signalNumber);
    }

    void * stackTrace[ STACKTRACE_DEPTH ];
    char ** messages;

    auto stackLength = backtrace( stackTrace, STACKTRACE_DEPTH );
    stackTrace[ 1 ] = reinterpret_cast< void * > (ucontext->uc_mcontext.gregs[ REG_RIP ]);
    messages = backtrace_symbols( stackTrace, stackLength );

    LOG_INFO( "Execution path:" );
    for ( auto index = 1; index < stackLength; ++index )
    {
        LOG_INFO( "%s", messages[ index ] );

        int position = 0;
        while (
                messages[ index ][ position ] != '('
                && messages[ index ][ position ] != ' '
                && messages[ index ][ position ] != 0 )
        {
            ++position;
        }

        char syscom[ 256 ];
        sprintf( syscom, "addr2line %p -e %.*s", stackTrace[ index ], position, messages[ index ] );
        auto infoStr = exec( syscom );
        LOG_INFO( "%s", infoStr.c_str() );
    }
    exit( 0 );
}

#else

#endif

void installSignalHandlers() noexcept
{
#ifdef Q_OS_LINUX
    struct sigaction signalAction{};
    signalAction.sa_sigaction = linuxSignalStacktraceHandler;
    signalAction.sa_flags = SA_RESTART | SA_SIGINFO;
    sigemptyset( &signalAction.sa_mask );

    sigaction( SIGFPE, &signalAction, nullptr );
    sigaction( SIGILL, &signalAction, nullptr );
    sigaction( SIGBUS, &signalAction, nullptr );
    sigaction( SIGSEGV, &signalAction, nullptr );
    sigaction( SIGABRT, &signalAction, nullptr );
#else

#endif
}
 
