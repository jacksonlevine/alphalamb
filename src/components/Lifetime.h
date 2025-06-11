//
// Created by jack on 6/10/2025.
//

#ifndef LIFETIME_H
#define LIFETIME_H

#include "../PrecompHeader.h"

struct Lifetime {
    uint8_t ttl = 5;

    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(ttl);
    }
};

inline void updateOrDestroyLifetimeHavers(entt::registry& registry, uint8_t ttlToDeleteAt)
{
    auto view = registry.view<Lifetime>();

    std::vector<entt::entity> toDestroy = {};

    for (auto entity : view)
    {
        auto& lifetime = view.get<Lifetime>(entity);
        if (lifetime.ttl > ttlToDeleteAt)
        {
            lifetime.ttl--;
        } else
        {
            toDestroy.emplace_back(entity);
        }
    }

    for (auto entity : toDestroy)
    {
        registry.destroy(entity);
    }


}

// inline void updatingOrDestroyingLifetimeHaversSystem(float deltatime, float& timer, entt::registry& registry)
// {
//     [[likely]]if (timer < 1.0f)
//     {
//         timer += deltatime;
//     } else
//     {
//         timer = 0.f;
//         updateOrDestroyLifetimeHavers(registry);
//     }
// }

#endif //LIFETIME_H
