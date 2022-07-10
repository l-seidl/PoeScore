#pragma once

#include <string>
#include <vector>

constexpr int ITEM_MAX_PREFIX_COUNT = 3u;
constexpr int ITEM_MAX_SUFFIX_COUNT = 3u;

struct ItemBase;

enum class ItemTag
{
    DEFAULT,
    TALISMAN,
    AMULET,
    ARMOUR,
    STR_ARMOUR,
    INT_ARMOUR,
    DEX_ARMOUR,
    STR_INT_ARMOUR,
    STR_DEX_ARMOUR,
    DEX_INT_ARMOUR,
    BODY_ARMOUR,
    BOOTS,
    GLOVES,
    HELMET,
    BELT,
    SHIELD,
    DEX_SHIELD,
    FOCUS,
    QUIVER,
    RING,
    CLAW,
    ONE_HAND_WEAPON,
    ONEHAND,
    WEAPON,
    DAGGER,
    MACE,
    ATTACK_DAGGER,
    WARD_ARMOUR,
    ATLAS_BASE_TYPE,
    GLOVES_ATLAS_STR,
};

enum class ItemClass
{
    WEAPON,
    BODY_ARMOR,
    GLOVES,
};

enum class ItemRarity
{
    NORMAL,
    MAGIC,
    RARE,
    UNIQUE,
};

enum class AffixType
{
    NONE,
    PREFIX_INCREASED_ARMOUR,
    PREFIX_INCREASED_ARMOUR_PERC,
    PREFIX_INCREASED_STUN_AND_BLOCK_RECOVERY,
    PREFIX_INCREASED_LIFE,
    PREFIX_REFLECTS_PHYSICAL_DAMAGE,
    PREFIX_LIGHTNING_DAMAGE,
    PREFIX_COLD_DAMAGE,

    SUFFIX_INTELLIGENCE,
    SUFFIX_STRENGTH,
    SUFFIX_MANA,
    SUFFIX_FIRE_RESISTANCE,
    SUFFIX_COLD_RESISTANCE,
    SUFFIX_LIGHTNING_RESISTANCE,
    SUFFIX_CHAOS_RESISTANCE,
    SUFFIX_LIFE_REGEN,
    SUFFIX_MOVEMENT_SPEED,
    COUNT,
};

constexpr AffixType SUFFIX_START = AffixType::SUFFIX_INTELLIGENCE;

struct ItemAffix
{
    bool                        isPrefix;
    std::string                 type;
    size_t                      tier;
};

struct ItemInfo
{
    const ItemBase* pItemBase;
    ItemClass itemClass;
    ItemRarity rarity;

    size_t numPrefixes;
    ItemAffix prefixes[ITEM_MAX_PREFIX_COUNT];
    size_t numSuffixes;
    ItemAffix suffixes[ITEM_MAX_SUFFIX_COUNT];
};

namespace mod_registry
{
    struct ModRegistry;
}

namespace item_base_registry
{
    struct ItemBaseRegistry;
}

namespace translation_registry
{
    struct TranslationRegistry;
}


namespace item_info
{
    struct ParseContext
    {
        ItemInfo* pTarget;
        mod_registry::ModRegistry* pModRegistry;
        const item_base_registry::ItemBaseRegistry* pItemBaseRegistry;
        const translation_registry::TranslationRegistry* pTranslationRegistry;
    };

    bool parse( ParseContext* pContext, const std::string& infoText );
}
