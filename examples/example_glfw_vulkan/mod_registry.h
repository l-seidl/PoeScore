#pragma once

#include <string>
#include <vector>

enum class AffixType;
struct ItemInfo;

namespace mod_registry
{
    struct ModRegistry;

    struct ModSpawnWeight
    {
        std::string tag;
        int weight;
    };

    struct ModStat
    {
        std::string id;
        int min;
        int max;
    };

    struct ModDefinition
    {
        std::vector<std::string> implicitTags;
        int requiredLevel;
        bool essenceOnly;
        std::vector<ModSpawnWeight> spawnWeights;
        std::vector<ModStat> stats;
        std::string type;
        std::string generationType;
    };

    struct AffixDefinition
    {
        AffixType type;
        std::string displayString;
        std::string searchToken;
    };


    ModRegistry* createAffixRegistry();
    void destroyAffixRegistry( ModRegistry* pRegistry );
    AffixType parsePrefix( const ModRegistry* pRegistry, const std::string& line );
    AffixType parseSuffix( const ModRegistry* pRegistry, const std::string& line );
    const std::vector<AffixDefinition>& getPrefixes( const ModRegistry* pRegistry );
    const std::vector<AffixDefinition>& getSuffixes( const ModRegistry* pRegistry );
    const std::string* getAffixDescription( const ModRegistry* pRegistry, AffixType type );

    void setAvailableModContext( ModRegistry* pReistry, const ItemInfo& item );
    const std::vector<const ModDefinition*>& getAvailableMods( const ModRegistry* pReistry );
}
