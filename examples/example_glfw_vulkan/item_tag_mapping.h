#pragma once

#include <string>

enum class ItemTag;

namespace item_tag_mapping
{
    const ItemTag* getItemTagByName( const std::string& name );
    const std::string* getItemTagName( const ItemTag& tag );
}
