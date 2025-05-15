//
// Created by jack on 5/14/2025.
//

#ifndef WORLDSTATECOMPONENT_H
#define WORLDSTATECOMPONENT_H


struct WorldState {
    enum Chapter {
        BEGINNING,
        WORLDCOLOR,
        FOGEND
    };
    Chapter currentChapter = BEGINNING;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(currentChapter);
    }
};

#endif //WORLDSTATECOMPONENT_H
