#include "item_tag_mapping.h"
#include "item_info.h"

#include <set>
#include <map>

namespace item_tag_mapping
{
    struct ItemTagMapping
    {
        std::set <std::string> tagBlacklist; // List of tags we don't need
        std::map <ItemTag, std::string> tagToStringMap;
        std::map <std::string, ItemTag> stringToTagMap;

        ItemTagMapping()
        {
            tagBlacklist.insert( "not_for_sale" );
            // TODO: Do I need this optimization?
            // tagToStringMap.insert( ItemTag:: )
        }
    };
}

namespace
{
    item_tag_mapping::ItemTagMapping g_itemTagMapping;
}


const ItemTag* item_tag_mapping::getItemTagByName( const std::string& name )
{
    if( g_itemTagMapping.stringToTagMap.find( name ) == g_itemTagMapping.stringToTagMap.end() )
    {
        return nullptr;
    }

    return &g_itemTagMapping.stringToTagMap[name];
}

const std::string* item_tag_mapping::getItemTagName( const ItemTag& tag )
{
    if( g_itemTagMapping.tagToStringMap.find( tag ) == g_itemTagMapping.tagToStringMap.end() )
    {
        return nullptr;
    }

    return &g_itemTagMapping.tagToStringMap[tag];
}
