#pragma once

#include <string>
#include <vector>

struct Translation
{
    std::string text;
    std::vector<std::string> format;
    std::string regexString;
};

struct TranslationDefinition
{
    std::vector<Translation> translations;
    std::vector<std::string> ids;
};

namespace translation_registry
{
    struct TranslationRegistry;

    TranslationRegistry* createTranslationRegistry();
    void destroyTranslationRegistry( TranslationRegistry* pRegistry );
    const std::vector<TranslationDefinition>& getTranslations( const TranslationRegistry* pRegistry );
}
