#include "translation_registry.h"
#include "ThirdParty/json.h"
#include "debug.h"

#include <iostream>
#include <fstream>

namespace translation_registry
{
    struct TranslationRegistry
    {
        std::vector<TranslationDefinition> translations;
    };

    static void parseFromJson( TranslationRegistry *pRegistry);
}

translation_registry::TranslationRegistry* translation_registry::createTranslationRegistry()
{
    TranslationRegistry* pRegistry = new TranslationRegistry();

    parseFromJson( pRegistry );

    return pRegistry;
}

void translation_registry::destroyTranslationRegistry( TranslationRegistry* pRegistry )
{
    delete pRegistry;
    pRegistry = nullptr;
}

static void translation_registry::parseFromJson( TranslationRegistry *pRegistry )
{

    nlohmann::json jsonTranslations;
    std::ifstream translationFile( "mod_translations.json" );

    if( !translationFile.is_open() )
    {
        debug::addError( "Could not open translation file" );
        return;
    }

    translationFile >> jsonTranslations;

    for( nlohmann::json::iterator it = jsonTranslations.begin(); it != jsonTranslations.end(); ++it )
    {
        nlohmann::json jsonTranslationDefinition = *it;

        TranslationDefinition translationDefinition;

        for( nlohmann::json::iterator translationIt = jsonTranslationDefinition["translations"].begin(); translationIt != jsonTranslationDefinition["translations"].end(); ++translationIt )
        {
            nlohmann::json jsonTranslation = *translationIt;

            Translation translation;
            translation.text = jsonTranslation["string"].get<std::string>();
            translation.regexString = jsonTranslation["regex"].get<std::string>();

            for( nlohmann::json::iterator formatIt = jsonTranslation["format"].begin(); formatIt != jsonTranslation["format"].end(); ++formatIt )
            {
                translation.format.push_back( formatIt->get<std::string>() );
            }

            translationDefinition.translations.push_back( translation );
        }

        for( nlohmann::json::iterator idIt = jsonTranslationDefinition["ids"].begin(); idIt != jsonTranslationDefinition["ids"].end(); ++idIt )
        {
            translationDefinition.ids.push_back( idIt->get<std::string>() );
        }

        pRegistry->translations.push_back( translationDefinition );
    }
}

const std::vector<TranslationDefinition>& translation_registry::getTranslations( const TranslationRegistry* pRegistry )
{
    return pRegistry->translations;
}
