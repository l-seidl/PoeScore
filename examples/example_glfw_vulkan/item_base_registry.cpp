#include "item_base_registry.h"
#include "ThirdParty/json.h"
#include "debug.h"

#include <iostream>
#include <fstream>

namespace item_base_registry
{
    struct ItemBaseRegistry
    {
        std::vector<ItemBase> itemBases;
    };

    static void readItemBaseDataFromJson( ItemBaseRegistry* pRegistry );
}

item_base_registry::ItemBaseRegistry* item_base_registry::createItemBaseRegistry()
{
    ItemBaseRegistry* pRegistry = new ItemBaseRegistry;

    readItemBaseDataFromJson( pRegistry );

    return pRegistry;
}

void item_base_registry::destroyItemBaseRegistry( ItemBaseRegistry* pRegistry )
{
    delete pRegistry;
}

const ItemBase* item_base_registry::findItemByName( const ItemBaseRegistry* pRegistry, const std::string& name )
{
    for( size_t i=0; i < pRegistry->itemBases.size(); ++i )
    {
        if( pRegistry->itemBases[i].name == name )
        {
            return &pRegistry->itemBases[i];
        }
    }

    return nullptr;
}

static void item_base_registry::readItemBaseDataFromJson( ItemBaseRegistry* pRegistry )
{
    std::ifstream iFileStream( "base_items_filtered.json" );
    if( !iFileStream.is_open() )
    {
        return;
    }

    nlohmann::json baseItemsJson;
    iFileStream >> baseItemsJson;

    for( nlohmann::json::iterator it = baseItemsJson.begin(); it != baseItemsJson.end(); ++it )
    {
        nlohmann::json itemJson = *it;

        ItemBase itemBase;
        itemBase.name = itemJson["name"].get<std::string>();
        itemBase.itemClass = itemJson["item_class"].get<std::string>();

        for( nlohmann::json::iterator tagIt = itemJson["tags"].begin(); tagIt != itemJson["tags"].end(); ++tagIt )
        {
            itemBase.tags.push_back( tagIt->get<std::string>() );
        }

        pRegistry->itemBases.push_back( itemBase );
    }
}

