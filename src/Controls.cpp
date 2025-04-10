//
// Created by jack on 1/6/2025.
//

#include "Controls.h"

bool Controls::anyMovement() const
{
    return forward || backward || left || right || jump || secondary1 || secondary2 || crouch;
}