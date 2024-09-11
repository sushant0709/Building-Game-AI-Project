#ifndef POSITION_MATCHING_HPP
#define POSITION_MATCHING_HPP
#include "./SteeringBehavior.hpp"
#include <SFML/System/Vector2.hpp>

class PositionMatching : public SteeringBehavior
{
    public:
        float maxAcc;
        float maxSpeed;
        float slowRadius;
        float targetRadius;
        float timeToTarget = 0.2;
        PositionMatching(float ma, float ms, float sr, float tr, float ttg)
        {
            this->maxAcc = ma;
            this->maxSpeed = ms;
            this->slowRadius = sr;
            this->targetRadius = tr;
            this->timeToTarget = ttg;
        }
        SteeringOutput CalculateSteering(Kinematic &character, Kinematic target) const override;
};
#endif /*POSITION_MATCHING_HPP*/