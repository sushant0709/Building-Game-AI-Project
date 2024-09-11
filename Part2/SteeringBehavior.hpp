#ifndef STEERING_BEHAVIOR_HPP
#define STEERING_BEHAVIOR_HPP
#include "./Kinematic.hpp"

/* Pure Virtual SteeringBehavior class */
class SteeringBehavior
{
    public:
    virtual ~SteeringBehavior() = default;
    virtual SteeringOutput CalculateSteering(Kinematic &character, Kinematic target) const = 0;
};
#endif /* STEERING_BEHAVIOR_HPP */
