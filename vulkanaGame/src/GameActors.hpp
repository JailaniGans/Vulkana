#pragma once

#include <memory>
#include "core/Application.hpp"
#include "gameplay/Actor.hpp"

// Simple player actor
class APlayer : public Vulkana::Actor
{
public:
    APlayer(Vulkana::World* world = nullptr)
        : Actor(world)
    {
        SetActorName("Player");
    }

    void BeginPlay() override
    {
        Vulkana::Actor::BeginPlay();
    }

    void Tick(float dt) override
    {
        Vulkana::Actor::Tick(dt);
    }
};
