//
// Created by jack on 3/16/2025.
//

#ifndef RENDERCOMPONENT_H
#define RENDERCOMPONENT_H

#include "BillboardInstanceShader.h"


struct RenderComponent {
    ChunkGLInfo handledBlockMeshInfo;
    Billboard billboard;
    AnimationState animation_state;
    template<class Archive>
    void serialize(Archive& archive)
    {
        //deliberately empty
    }
};


#endif //RENDERCOMPONENT_H
