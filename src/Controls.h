//
// Created by jack on 11/18/2024.
//

#ifndef CONTROLS_H
#define CONTROLS_H

#include "PrecompHeader.h"

struct Controls
{
    bool forward = false;
    bool backward = false;
    bool left = false;
    bool right = false;
    bool sprint = false;
    bool jump = false;
    bool secondary1 = false;
    bool secondary2 = false;
    bool crouch = false;
    [[nodiscard]] bool anyMovement() const;
};


inline std::ostream& operator<<(std::ostream& os, const Controls& controls) {
    os << "{Forward: " << (controls.forward ? "true" : "false")
       << ", Backward: " << (controls.backward ? "true" : "false")
       << ", Left: " << (controls.left ? "true" : "false")
       << ", Right: " << (controls.right ? "true" : "false")
       << ", Sprint: " << (controls.sprint ? "true" : "false")
       << ", Jump: " << (controls.jump ? "true" : "false") << "}"
        << ", Secondary1: " << (controls.secondary1 ? "true" : "false")
       << ", Secondary2: " << (controls.secondary2 ? "true" : "false") << "}"
    << ", Crouch: " << (controls.crouch ? "true" : "false")
    ;
    return os;
}

inline bool operator==(const Controls& c1, const Controls& c2)
{
    return c1.backward == c2.backward && c1.forward == c2.forward &&
            c1.jump == c2.jump && c1.left == c2.left && c1.right == c2.right && c1.sprint == c2.sprint
            && c1.secondary1 == c2.secondary1 && c1.secondary2 == c2.secondary2 && c1.crouch == c2.crouch;
}


#endif //CONTROLS_H
