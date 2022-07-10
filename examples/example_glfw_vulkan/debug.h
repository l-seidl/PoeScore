#pragma once

#include <string>
#include <vector>

namespace debug
{
    void addError( const std::string& text );
    void clearErrors();
    const std::vector<std::string>* getErrors();
}
