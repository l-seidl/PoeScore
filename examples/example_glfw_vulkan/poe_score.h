#pragma once

struct GLFWwindow;

namespace poe_score
{
    struct PoeScoreContext;

    PoeScoreContext* createPoeScore( GLFWwindow* pWindow );
    void run( PoeScoreContext* pContext );
    void destroyPoeScore( PoeScoreContext* pContext );
}
