//
// Created by jack on 6/15/2025.
//

#ifndef HEALTHCOMPONENT_H
#define HEALTHCOMPONENT_H

#include "../PrecompHeader.h"

struct HealthComponent {
  float health;
  float heartRate;

  template<class Archive>
  void serialize(Archive& archive)
  {
    archive(health, heartRate);
  }
};

#endif //HEALTHCOMPONENT_H
