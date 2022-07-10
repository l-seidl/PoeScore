#include "item_info.h"
#include "item_base_registry.h"
#include "mod_registry.h"
#include "translation_registry.h"
#include "debug.h"

#include <cctype>
#include <regex>
#include <set>
#include <string>
#include <sstream>
#include <iostream>

namespace item_info
{
    bool parseItemClass( ItemInfo* pTarget, const std::string& line );

    static bool isAffix( const std::string& line );
    static void toLowerString( std::string* pTarget );
    static bool doImplicitTagsMatch( const std::vector<std::string>& tags, const mod_registry::ModDefinition& modifier );
    bool parseAffix( ParseContext* pContext, std::istringstream& iStream, const std::string& affixHeadline );

    static void removeLineBreak( std::string* pTarget );
}

static void item_info::removeLineBreak( std::string* pTarget )
{
    if( pTarget->size() == 0 )
    {
        return;
    }

    *pTarget = pTarget->substr( 0, pTarget->length() - 1 ); // remove line break
}

bool item_info::parseItemClass( ItemInfo* pTarget, const std::string& line )
{
    constexpr char* CLASS_TOKEN = "Item Class:";
    int pos = (int)line.find( CLASS_TOKEN );
    if( pos == -1 )
    {
        return false;
    }

    std::string className = line.substr( pos + strlen( CLASS_TOKEN ) + 1 );
    removeLineBreak( &className );

    if( className == "Gloves" )
    {
        pTarget->itemClass = ItemClass::GLOVES;
    }
    else if( className == "Body Armours" )
    {
        pTarget->itemClass = ItemClass::BODY_ARMOR;
    }
    else
    {
        debug::addError( "Unknown Item class " + className );
    }

    return true;
}

static bool item_info::isAffix( const std::string& line )
{
    return line.find( "Prefix" ) != std::string::npos || line.find( "Suffix" ) != std::string::npos;
}

static void item_info::toLowerString( std::string* pTarget )
{
    std::transform( pTarget->begin(), pTarget->end(), pTarget->begin(),
        []( unsigned char c ) { return std::tolower( c ); } );
}

static bool item_info::doImplicitTagsMatch( const std::vector<std::string>& tags, const mod_registry::ModDefinition& modifier )
{
    for( size_t i = 0; i < tags.size(); ++i )
    {
        bool tagFound = false;
        for( size_t implicitTagIndex = 0; implicitTagIndex < modifier.implicitTags.size(); ++implicitTagIndex )
        {
            if( tags[i] == modifier.implicitTags[implicitTagIndex] )
            {
                tagFound = true;
                break;
            }
        }

        if( !tagFound )
        {
            return false;
        }
    }

    return true;
}

bool item_info::parseAffix( ParseContext* pContext, std::istringstream& iStream, const std::string& affixHeadline )
{
    bool isPrefix = affixHeadline.find( "Prefix" ) != std::string::npos;
    size_t tier = 0u;

    // Read tier
    {
        std::regex tierRegex( "\\(Tier: (\\d+)\\)", std::regex_constants::ECMAScript );
        std::smatch match;
        if( std::regex_search( affixHeadline, match, tierRegex ) )
        {
            tier = std::stoi( match.str( 1u ) );
        }
    }

    std::vector<std::string> modifierTypes;

    try
    {
        std::regex typeRegex( " ([a-zA-Z, ]+) \\}", std::regex_constants::ECMAScript );
        std::smatch typesMatch;
        if( std::regex_search( affixHeadline, typesMatch, typeRegex ) )
        {
            std::string types = typesMatch.str( 1u );

            size_t pos = 0;
            std::string type;
            while( ( pos = types.find( ", " ) ) != std::string::npos )
            {
                type = types.substr( 0, pos );
                toLowerString( &type );
                modifierTypes.push_back( type );
                types.erase( 0, pos + 2 );
            }

            toLowerString( &types );
            modifierTypes.push_back( types );
        }
    }
    catch (std::regex_error& e)
    {
        debug::addError( e.what() );
    }

    // Filter out possible mods based on prefix/ affix and implicit tags
    std::vector<const mod_registry::ModDefinition*> possibleMods;
    {
        std::set< std::string > usedTypes;

        const std::vector<const mod_registry::ModDefinition*>& availableMods = mod_registry::getAvailableMods( pContext->pModRegistry );
        for( size_t i = 0; i < availableMods.size(); ++i )
        {
            if( usedTypes.find( availableMods[i]->type ) != usedTypes.end() )
            {
                continue;
            }

            usedTypes.insert( availableMods[i]->type );

            if( !doImplicitTagsMatch( modifierTypes, *availableMods[i] ) )
            {
                continue;
            }

            possibleMods.push_back( availableMods[i] );
        }
    }

    std::vector<std::string> statLines;
    std::string nextAffixHeadline;
    // Read n lines of stats
    {
        bool nextAffixFound = false;
        while( !nextAffixFound )
        {
            std::string nextLine;
            if( !std::getline( iStream, nextLine ) )
            {
                nextAffixHeadline = "";
                nextAffixFound = true;
                break;
            }

            if( isAffix( nextLine ) )
            {
                nextAffixHeadline = nextLine;
                nextAffixFound = true;
                break;
            }

            statLines.push_back( nextLine );
        }
    }

    // Filter possible mods based on stat count
    /*
    {
        std::vector<const mod_registry::ModDefinition*> statPossibleMods;
        for( size_t i = 0; i < possibleMods.size(); ++i )
        {
            if( possibleMods[i]->stats.size() == statLines.size() )
            {
                statPossibleMods.push_back( possibleMods[i] );
            }
        }

        possibleMods = statPossibleMods;
    }*/

    const mod_registry::ModDefinition* pFoundMod = nullptr;

    const std::vector<TranslationDefinition>& translations = translation_registry::getTranslations( pContext->pTranslationRegistry );
    for( size_t modIndex = 0; modIndex < possibleMods.size(); ++modIndex )
    {
        const mod_registry::ModDefinition* pMod = possibleMods[modIndex];
        for( size_t i = 0; i < translations.size(); ++i )
        {
            const TranslationDefinition& translationDefinition = translations[i];
            /*
            if( pMod->stats.size() != translationDefinition.ids.size() )
            {
                continue;
            }*/

            bool allIdsMatch = true;
            for( size_t j = 0; j < pMod->stats.size(); ++j )
            {
                bool idFound = false;

                for( size_t k = 0; k < translationDefinition.ids.size(); ++k )
                {
                    if( pMod->stats[j].id == translationDefinition.ids[k] )
                    {
                        idFound = true;
                        break;
                    }
                }

                if( !idFound )
                {
                    allIdsMatch = false;
                    break;
                }
            }

            if( !allIdsMatch )
            {
                continue;
            }

            // Check if all translations match
            for( size_t j = 0; j < translationDefinition.translations.size(); ++j )
            {
                std::regex translationRegex( translationDefinition.translations[j].regexString, std::regex_constants::ECMAScript );
                // TODO: Merge stat lines
                if( std::regex_search( statLines[0], translationRegex ) )
                {
                    pFoundMod = pMod;
                    break;
                }
            }

            if( pFoundMod != nullptr )
            {
                break;
            }
        }

        if( pFoundMod != nullptr )
        {
            break;
        }
    }

    if( pFoundMod != nullptr )
    {
        ItemAffix* pAffix = nullptr;

        if( isPrefix )
        {
            pAffix = &pContext->pTarget->prefixes[pContext->pTarget->numPrefixes];
            ++pContext->pTarget->numPrefixes;
        }
        else
        {
            pAffix = &pContext->pTarget->suffixes[pContext->pTarget->numSuffixes];
            ++pContext->pTarget->numSuffixes;
        }

        pAffix->tier = tier;
        pAffix->isPrefix = isPrefix;
        pAffix->type = pFoundMod->type;
    }
    else
    {
        debug::addError( "Could not parse mod " + affixHeadline + "\n" + statLines[0] );
    }

    if( nextAffixHeadline.empty() )
    {
        return true;
    }

     return parseAffix( pContext, iStream, nextAffixHeadline );

    /*
    // Read combined modifier
    if( isPrefix )
    {
        type = mod_registry::parsePrefix( pContext->pModRegistry, affixValues );
    }
    else
    {
        type = mod_registry::parseSuffix( pContext->pModRegistry, affixValues );
    }

    if( type == AffixType::NONE )
    {
        debug::addError( "Unknown affix " + affixValues );
    }

    pAffix->types.push_back( type );
    */

    return true;
}

bool item_info::parse( ParseContext* pContext, const std::string& infoText )
{
    std::istringstream iStream( infoText );
    std::string line;

    pContext->pTarget->numPrefixes = 0u;
    pContext->pTarget->numSuffixes = 0u;

    // First line should be item
    std::getline( iStream, line );
    if( !parseItemClass( pContext->pTarget, line ) )
    {
        return false;
    }

    // Search for the item base name
    {
        bool nameFound = false;
        std::string name;
        while( !nameFound )
        {
            if( !std::getline( iStream, line ) )
            {
                return false;
            }

            if( line.find( "--------" ) == std::string::npos )
            {
                name = line;
            }
            else
            {
                nameFound = true;
            }
        }

        removeLineBreak( &name );
        pContext->pTarget->pItemBase = item_base_registry::findItemByName( pContext->pItemBaseRegistry, name );
    }

    mod_registry::setAvailableModContext( pContext->pModRegistry, *pContext->pTarget );

    while( std::getline( iStream, line ) )
    {
        if( isAffix( line ) )
        {
            parseAffix( pContext, iStream, line );
        }
    }

    return true;
}
