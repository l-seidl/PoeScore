#include "poe_score.h"
#include "imgui.h"
#include "item_base_registry.h"
#include "item_info.h"
#include "mod_registry.h"
#include "translation_registry.h"
#include "debug.h"
#include "bitmask.h"

#include <algorithm>
#include <set>
#include <GLFW/glfw3.h>

namespace poe_score
{
    struct PoeScoreContext
    {
        GLFWwindow* pWindow;
        mod_registry::ModRegistry* pModRegistry;
        item_base_registry::ItemBaseRegistry* pItemBaseRegistry;
        translation_registry::TranslationRegistry* pTranslationRegistry;

        std::string currentClipboardText;
        ItemInfo selectedItemInfo;
        float selectedItemScore;

        // TODO: Move this to a type so it can be stored per item class
        std::set< std::string > selectedPrefixes;
        std::set< std::string > selectedSuffixes;
        // BitMask< AffixType > selectedAffixes;
    };

    static void     checkClipboardText( PoeScoreContext* pContext );
    static float    calculateAffixScore( const PoeScoreContext* pContext, const ItemAffix& affix );
    static void     evaluateItemScore( PoeScoreContext* pContext );

    // UI
    void renderScoreWindow( PoeScoreContext* pContext );
    void renderAffixSelectionWindow( PoeScoreContext* pContext );
    void renderLogWindow( PoeScoreContext* pContext );
}

poe_score::PoeScoreContext* poe_score::createPoeScore( GLFWwindow* pWindow )
{
    PoeScoreContext* pContext = new PoeScoreContext;
    pContext->pWindow = pWindow;
    pContext->pModRegistry = mod_registry::createAffixRegistry();
    pContext->pItemBaseRegistry = item_base_registry::createItemBaseRegistry();
    pContext->pTranslationRegistry = translation_registry::createTranslationRegistry();

    pContext->selectedItemScore = 0.0f;

    return pContext;
}

void poe_score::run( PoeScoreContext* pContext )
{
    checkClipboardText( pContext );

    renderScoreWindow( pContext );
    renderAffixSelectionWindow( pContext );
    renderLogWindow( pContext );
}

void poe_score::destroyPoeScore( PoeScoreContext* pContext )
{
    mod_registry::destroyAffixRegistry( pContext->pModRegistry );
    item_base_registry::destroyItemBaseRegistry( pContext->pItemBaseRegistry );
    translation_registry::destroyTranslationRegistry( pContext->pTranslationRegistry );

    delete pContext;
    pContext = nullptr;
}

static void poe_score::checkClipboardText( PoeScoreContext* pContext )
{
    const char* pClipboardString = glfwGetClipboardString( pContext->pWindow );
    if( pClipboardString == nullptr )
    {
        return;
    }

    if( strcmp( pClipboardString, pContext->currentClipboardText.c_str() ) == 0 )
    {
        return;
    }

    pContext->currentClipboardText.assign( pClipboardString, strlen( pClipboardString ) );

    item_info::ParseContext parseContext;
    parseContext.pTarget = &pContext->selectedItemInfo;
    parseContext.pModRegistry = pContext->pModRegistry;
    parseContext.pItemBaseRegistry = pContext->pItemBaseRegistry;
    parseContext.pTranslationRegistry = pContext->pTranslationRegistry;

    debug::clearErrors();
    if( !item_info::parse( &parseContext, pContext->currentClipboardText ) )
    {
        return;
    }

    evaluateItemScore( pContext );
}

static float poe_score::calculateAffixScore( const PoeScoreContext* pContext, const ItemAffix& affix )
{
    float score = 0.0f;

    bool isRequiredAffix = false;
    if( affix.isPrefix )
    {
        isRequiredAffix = pContext->selectedPrefixes.find( affix.type ) != pContext->selectedPrefixes.end();
    }
    else
    {
        isRequiredAffix = pContext->selectedSuffixes.find( affix.type ) != pContext->selectedSuffixes.end();
    }

    score += isRequiredAffix ? 1.0f : 0.0f;
    constexpr float maxTier = 8.0f; // TODO: Parse from data
    const float tierModifier = std::min( std::max( 1.0f - ( affix.tier - 1.0f ) / 8.0f, 0.0f ), 1.0f );
    score *= tierModifier;

    return score;
}

static void poe_score::evaluateItemScore( PoeScoreContext* pContext )
{
    const ItemInfo& item = pContext->selectedItemInfo;

    int selectedPrefixCount = pContext->selectedPrefixes.size();
    int selectedSuffixCount = pContext->selectedSuffixes.size();

    selectedPrefixCount = std::min( ITEM_MAX_PREFIX_COUNT, selectedPrefixCount );
    selectedSuffixCount = std::min( ITEM_MAX_SUFFIX_COUNT, selectedSuffixCount );

    const float maxScore = ( selectedPrefixCount + selectedPrefixCount ) * 1.0f;
    if( maxScore == 0.0f )
    {
        pContext->selectedItemScore = 1.0f;
        return;
    }

    float score = 0.0f;

    for( size_t i = 0; i < item.numPrefixes; ++i )
    {
        score += calculateAffixScore( pContext, item.prefixes[i] ) ;
    }

    for( size_t i = 0; i < item.numSuffixes; ++i )
    {
        score += calculateAffixScore( pContext, item.suffixes[i] );
    }


    pContext->selectedItemScore = score / maxScore;
}

void poe_score::renderScoreWindow( PoeScoreContext* pContext )
{
    ImGui::Begin( "POE score" );

    const ItemInfo& item = pContext->selectedItemInfo;

    ImVec4 scoreCol = pContext->selectedItemScore < 0.5f ? ImVec4( 1.0f, 0.0f, 0.0f, 1.0f ) : ImVec4( 0.0f, 1.0f, 0.0f, 1.0f );
    ImGui::TextColored( scoreCol, "Score: %.2f%%", pContext->selectedItemScore * 100.0f );

    ImGui::TextUnformatted( "Affixes: " );

    // TODO: Cleanup :)
    for( size_t i = 0; i < item.numPrefixes; ++i )
    {
        const float score = calculateAffixScore( pContext, item.prefixes[i] );
        ImVec4 scoreCol = score < 0.5f ? ImVec4( 1.0f, 0.0f, 0.0f, 1.0f ) : ImVec4( 0.0f, 1.0f, 0.0f, 1.0f );
        //const std::string* pDescription = mod_registry::getAffixDescription( pContext->pModRegistry, item.prefixes[i].type );
        // const std::string description = pDescription != nullptr ? *pDescription : "unknown ";
        const std::string description = item.prefixes[i].type;
        ImGui::TextColored( scoreCol, "%s (Tier %u) => %.2f%%", description.c_str(), item.prefixes[i].tier, score * 100.0f );
    }

    for( size_t i = 0; i < item.numSuffixes; ++i )
    {
        const float score = calculateAffixScore( pContext, item.suffixes[i] );
        ImVec4 scoreCol = score < 0.5f ? ImVec4( 1.0f, 0.0f, 0.0f, 1.0f ) : ImVec4( 0.0f, 1.0f, 0.0f, 1.0f );
        // const std::string* pDescription = mod_registry::getAffixDescription( pContext->pModRegistry, item.suffixes[i].types[0] );
        // const std::string description = pDescription != nullptr ? *pDescription : "unknown ";
        const std::string description = item.suffixes[i].type;
        ImGui::TextColored( scoreCol, "%s (Tier %u) => %.2f%%", description.c_str(), item.suffixes[i].tier, score * 100.0f );
    }

    ImGui::TextUnformatted( pContext->currentClipboardText.c_str() );

    ImGui::End();
}

void poe_score::renderAffixSelectionWindow( PoeScoreContext* pContext )
{
    ImGui::Begin( "Affix selection" );

    ImGui::TextUnformatted( "Selection" );

    const float halfWidth = ImGui::GetContentRegionAvail().x * 0.5f;

    std::vector<const mod_registry::ModDefinition*> mods;
    {
        const std::vector<const mod_registry::ModDefinition*>& availableMods = mod_registry::getAvailableMods( pContext->pModRegistry );
        std::set<std::string> usedTypes;
        for( size_t i = 0; i < availableMods.size(); ++i )
        {
            if( usedTypes.find( availableMods[i]->type ) != usedTypes.end() )
            {
                continue;
            }

            usedTypes.insert( availableMods[i]->type );

            mods.push_back( availableMods[i] );
        }
    }


    {
        ImGui::BeginGroup();
        ImGui::TextUnformatted( "Prefixes" );

        ImGui::InvisibleButton( "Spacer", ImVec2( halfWidth, 1.0f ) );

        for( int i = 0; i < mods.size(); ++i )
        {
            const mod_registry::ModDefinition* pMod = mods[i];
            if( pMod->generationType != "prefix" )
            {
                continue;
            }

            ImGui::PushItemWidth( halfWidth );
            bool isChecked = pContext->selectedPrefixes.find( pMod->type ) != pContext->selectedPrefixes.end();
            if( ImGui::Checkbox( pMod->type.c_str(), &isChecked ) )
            {
                if( isChecked )
                {
                    pContext->selectedPrefixes.insert( pMod->type );
                }
                else
                {
                    pContext->selectedPrefixes.erase( pMod->type );
                }
                evaluateItemScore( pContext );
            }

            ImGui::PopItemWidth();
        }

        ImGui::EndGroup();
    }
    ImGui::SameLine();
    {
        ImGui::BeginGroup();
        ImGui::TextUnformatted( "Suffixes" );

        ImGui::InvisibleButton( "Spacer2", ImVec2( halfWidth, 1.0f ) );

        for( int i = 0; i < mods.size(); ++i )
        {
            const mod_registry::ModDefinition* pMod = mods[i];
            if( pMod->generationType != "suffix" )
            {
                continue;
            }

            ImGui::PushItemWidth( halfWidth );
            bool isChecked = pContext->selectedSuffixes.find( pMod->type ) != pContext->selectedSuffixes.end();
            if( ImGui::Checkbox( pMod->type.c_str(), &isChecked ) )
            {
                if( isChecked )
                {
                    pContext->selectedSuffixes.insert( pMod->type );
                }
                else
                {
                    pContext->selectedSuffixes.erase( pMod->type );
                }
                evaluateItemScore( pContext );
            }

            ImGui::PopItemWidth();
        }

        ImGui::EndGroup();
    }

    ImGui::End();
}

void poe_score::renderLogWindow( PoeScoreContext* pContext )
{
    const std::vector<std::string>* pErrors = debug::getErrors();

    if( pErrors->empty() )
    {
        return;
    }

    ImGui::Begin( "Error log" );

    for( size_t i=0; i < pErrors->size(); ++i )
    {
        ImGui::TextUnformatted( (*pErrors)[i].c_str() );
    }

    ImGui::End();
}
