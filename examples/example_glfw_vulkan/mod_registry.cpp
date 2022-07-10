#include "mod_registry.h"
#include "item_info.h"
#include "ThirdParty/json.h"
#include "debug.h"
#include "item_base_registry.h"

#include <vector>
#include <iostream>
#include <fstream>

namespace mod_registry
{
    struct ModRegistry
    {
        std::vector<AffixDefinition> prefixes;
        std::vector<AffixDefinition> suffixes;
        std::vector<ModDefinition> mods;

        ItemInfo currentModContext;
        std::vector<const ModDefinition*> filteredMods;
    };

    static void readPoeData( ModRegistry* pRegistry );
    static bool isNewModContext( const ModRegistry* pRegistry, const ItemInfo& item );
    static int getSpawnWeight( const ModDefinition& mod, const ItemInfo& item );

    AffixDefinition*    registerAffix( ModRegistry* pRegistry, bool isPrefix, AffixType type, std::string displayString, std::string searchToken );
    AffixType           parseAffix( const std::vector<AffixDefinition>& targetAffixes, const std::string& line );
}

mod_registry::ModRegistry* mod_registry::createAffixRegistry()
{
    ModRegistry* pRegistry = new ModRegistry();

    // Prefixes
    registerAffix( pRegistry, true, AffixType::PREFIX_INCREASED_LIFE, "+# to Maximum Life", "to maximum Life" );
    registerAffix( pRegistry, true, AffixType::PREFIX_INCREASED_ARMOUR, "+# to Armour", "to Armour" );
    registerAffix( pRegistry, true, AffixType::PREFIX_INCREASED_ARMOUR_PERC, "+% to Armour", "increased Armour" );
    registerAffix( pRegistry, true, AffixType::PREFIX_LIGHTNING_DAMAGE, "+# Lightning Damage to Attacks", "Lightning Damage" );
    registerAffix( pRegistry, true, AffixType::PREFIX_COLD_DAMAGE, "+# Cold Damage to Attacks", "Cold Damage" );


    // Suffixes
    registerAffix( pRegistry, false, AffixType::SUFFIX_INTELLIGENCE, "+# to Intelligence", "to Intelligence" );
    registerAffix( pRegistry, false, AffixType::SUFFIX_STRENGTH, "+# to Strength", "to Strength" );
    registerAffix( pRegistry, false, AffixType::SUFFIX_MANA, "+# to Maximum Mana", "to Maximum Mana" );
    registerAffix( pRegistry, false, AffixType::SUFFIX_COLD_RESISTANCE, "+% to Cold Resistance", "to Cold Resistance" );
    registerAffix( pRegistry, false, AffixType::SUFFIX_FIRE_RESISTANCE, "+% to Fire Resistance", "to Fire Resistance" );
    registerAffix( pRegistry, false, AffixType::SUFFIX_LIGHTNING_RESISTANCE, "+% to Lightning Resistance", "to Lightning Resistance" );
    registerAffix( pRegistry, false, AffixType::SUFFIX_CHAOS_RESISTANCE, "+% to Chaos Resistance", "to Chaos Resistance" );
    registerAffix( pRegistry, false, AffixType::SUFFIX_LIFE_REGEN, "Life Regeneration", "Life per second" );
    registerAffix( pRegistry, false, AffixType::SUFFIX_MOVEMENT_SPEED, "+% Movement Speed", "Increased Movement Speed" );

    readPoeData( pRegistry );

    return pRegistry;
}

void mod_registry::destroyAffixRegistry( ModRegistry* pRegistry )
{
    delete pRegistry;
    pRegistry = nullptr;
}

static void mod_registry::readPoeData( ModRegistry* pRegistry )
{
    nlohmann::json jsonMods;
    // std::ifstream modFile( "./../Data/RePoE/RePoE/data/mods.json" );
    // std::ifstream modFile( "mods.json" );
    std::ifstream modFile( R"(C:\Users\lseidl\Documents\PoeCraftingScore\ImGUI\examples\example_glfw_vulkan\data\converted\mods_filtered.json)" );

    // std::ifstream modFile( "../Data/RePoE/RePoE/data/m1231ods.json" );
    if( !modFile.is_open() )
    {
        debug::addError( "Could not open mod file" );
        return;
    }

    modFile >> jsonMods;

    for( nlohmann::json::iterator it = jsonMods.begin(); it != jsonMods.end(); ++it )
    {
        nlohmann::json jsonMod = *it;

        ModDefinition mod;
        mod.requiredLevel = jsonMod["required_level"].get<int>();
        mod.essenceOnly = jsonMod["is_essence_only"].get<bool>();
        mod.type = jsonMod["types"].get<std::string>();
        mod.generationType = jsonMod["generation_type"];

        for( nlohmann::json::iterator spawnWeightIt = jsonMod["spawn_weights"].begin(); spawnWeightIt != jsonMod["spawn_weights"].end(); ++spawnWeightIt )
        {
            nlohmann::json jsonSpawnWeight = *spawnWeightIt;

            ModSpawnWeight spawnWeight;
            spawnWeight.tag = jsonSpawnWeight["tag"].get<std::string>();
            spawnWeight.weight = jsonSpawnWeight["weight"].get<int>();

            mod.spawnWeights.push_back( spawnWeight );
        }

        for( nlohmann::json::iterator statIt = jsonMod["stats"].begin(); statIt != jsonMod["stats"].end(); ++statIt )
        {
            nlohmann::json jsonStat = *statIt;

            ModStat stat;
            stat.id = jsonStat["id"].get<std::string>();
            stat.min = jsonStat["min"].get<int>();
            stat.max = jsonStat["max"].get<int>();

            mod.stats.push_back( stat );
        }


        for( nlohmann::json::iterator tagIt = jsonMod["implicit_tags"].begin(); tagIt != jsonMod["implicit_tags"].end(); ++tagIt )
        {
            mod.implicitTags.push_back( tagIt->get<std::string>() );
        }

        pRegistry->mods.push_back( mod );
    }
}

AffixType mod_registry::parseAffix( const std::vector<AffixDefinition>& targetAffixes, const std::string& line )
{
    for( size_t i = 0; i < targetAffixes.size(); ++i )
    {
        const AffixDefinition& affix = targetAffixes[i];
        if( line.find( affix.searchToken ) != std::string::npos )
        {
            return affix.type;
        }
    }

    return AffixType::NONE;
}

AffixType mod_registry::parsePrefix( const ModRegistry* pRegistry, const std::string& line )
{
    return parseAffix( pRegistry->prefixes, line );
}

AffixType mod_registry::parseSuffix( const ModRegistry* pRegistry, const std::string& line )
{
    return parseAffix( pRegistry->suffixes, line );
}

const std::vector<mod_registry::AffixDefinition>& mod_registry::getPrefixes( const ModRegistry* pRegistry )
{
    return pRegistry->prefixes;
}

const std::vector<mod_registry::AffixDefinition>& mod_registry::getSuffixes( const ModRegistry* pRegistry )
{
    return pRegistry->suffixes;
}

const std::string* mod_registry::getAffixDescription( const ModRegistry* pRegistry, AffixType type )
{
    // TODO: Store in map
    for( size_t i = 0; i < pRegistry->prefixes.size(); ++i )
    {
        if( pRegistry->prefixes[i].type == type )
        {
            return &pRegistry->prefixes[i].displayString;
        }
    }

    for( size_t i = 0; i < pRegistry->suffixes.size(); ++i )
    {
        if( pRegistry->suffixes[i].type == type )
        {
            return &pRegistry->suffixes[i].displayString;
        }
    }

    return nullptr;
}

mod_registry::AffixDefinition* mod_registry::registerAffix( ModRegistry* pRegistry, bool isPrefix, AffixType type, std::string displayString, std::string searchToken )
{
    AffixDefinition affix;
    affix.type = type;
    affix.displayString = displayString;
    affix.searchToken = searchToken;

    if( isPrefix )
    {
        pRegistry->prefixes.push_back( affix );
        return &pRegistry->prefixes[pRegistry->prefixes.size() - 1];
    }
    else
    {
        pRegistry->suffixes.push_back( affix );
        return &pRegistry->suffixes[pRegistry->suffixes.size() - 1];
    }
}

static bool mod_registry::isNewModContext( const ModRegistry* pRegistry, const ItemInfo& item )
{
    if( pRegistry->currentModContext.pItemBase == nullptr )
    {
        return true;
    }

    if( item.pItemBase == nullptr )
    {
        return false;
    }

    if( pRegistry->currentModContext.itemClass != item.itemClass )
    {
        return true;
    }

    if( pRegistry->currentModContext.pItemBase->tags != item.pItemBase->tags )
    {
        return true;
    }
}

static int mod_registry::getSpawnWeight( const ModDefinition& mod, const ItemInfo& item )
{
    for( size_t spawnWeightIndex = 0u; spawnWeightIndex < mod.spawnWeights.size(); ++spawnWeightIndex )
    {
        for( size_t tagIndex = 0u; tagIndex < item.pItemBase->tags.size(); ++tagIndex )
        {
            if( mod.spawnWeights[spawnWeightIndex].tag == item.pItemBase->tags[tagIndex] )
            {
                return mod.spawnWeights[spawnWeightIndex].weight;
            }
        }
    }

    return 0;
}

void mod_registry::setAvailableModContext( ModRegistry* pRegistry, const ItemInfo& item )
{
    if( !isNewModContext( pRegistry, item ) )
    {
        return;
    }

    pRegistry->filteredMods.clear();

    // Check tags for spawn weights
    for( size_t i = 0; i < pRegistry->mods.size(); ++i )
    {
        const ModDefinition& mod = pRegistry->mods[i];

        if( getSpawnWeight( mod, item ) == 0 )
        {
            continue;
        }

        pRegistry->filteredMods.push_back( &mod );
    }
}

const std::vector<const mod_registry::ModDefinition*>& mod_registry::getAvailableMods( const ModRegistry* pReistry )
{
    return pReistry->filteredMods;
}
