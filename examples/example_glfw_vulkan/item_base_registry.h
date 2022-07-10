#pragma once


#include <vector>
#include <string>

struct ItemBase
{
    std::string itemClass;
    std::string name;
    std::vector< std::string > tags;
};

namespace item_base_registry
{
    struct ItemBaseRegistry;

    ItemBaseRegistry* createItemBaseRegistry();
    void destroyItemBaseRegistry( ItemBaseRegistry* pRegistry );
    const ItemBase* findItemByName( const ItemBaseRegistry* pRegistry, const std::string& name );
}
