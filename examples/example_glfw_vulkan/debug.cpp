#include "debug.h"

namespace debug
{
    struct DebugData
    {
        std::vector<std::string> errors;
    };
}

namespace
{
    debug::DebugData debugData;
}

void debug::addError( const std::string& text )
{
    debugData.errors.push_back( text );
}

void debug::clearErrors()
{
    debugData.errors.clear();
}

const std::vector<std::string>* debug::getErrors()
{
    return &debugData.errors;
}
