//
// Created by jack on 3/26/2025.
//

#ifndef COMPUTERCOMPONENT_H
#define COMPUTERCOMPONENT_H
#include "../TextEditor.h"


class ComputerComponent {
public:
    TextEditor textEditor = {};


    template<class Archive>
    void serialize(Archive& archive)
    {
        //deliberately empty
    }
};



#endif //COMPUTERCOMPONENT_H
