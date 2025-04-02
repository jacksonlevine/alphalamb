//
// Created by jack on 3/26/2025.
//

#ifndef NPPOSITIONCOMPONENT_H
#define NPPOSITIONCOMPONENT_H



class NPPositionComponent {
public:
    glm::vec3 position = glm::vec3(0.f);
    NPPositionComponent(glm::vec3 pos) : position(pos) {};
    NPPositionComponent() = default;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(position);
    }
};



#endif //NPPOSITIONCOMPONENT_H
