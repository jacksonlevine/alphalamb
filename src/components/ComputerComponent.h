//
// Created by jack on 3/26/2025.
//

#ifndef COMPUTERCOMPONENT_H
#define COMPUTERCOMPONENT_H
#include "../TextEditor.h"



class ComputerComponent {
public:
    TextEditor editor = {};

    ///Whether this computer component-ed entity has got a renderer assigned to it yet
    bool gotRenderer = false;

    ComputerComponent() {
        editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Python());
    }

    ComputerComponent& operator=(const ComputerComponent& other) = delete;
    ComputerComponent(ComputerComponent& other) = delete;
    ComputerComponent& operator=(ComputerComponent&& other) noexcept {
        if(this != &other) {
            editor = other.editor;
        }
        return *this;
    }
    ComputerComponent(ComputerComponent&& other) noexcept {
        editor = other.editor;
    }
    template<class Archive>
    void serialize(Archive& archive) {}
	~ComputerComponent()
    {
    }
};



#endif //COMPUTERCOMPONENT_H
