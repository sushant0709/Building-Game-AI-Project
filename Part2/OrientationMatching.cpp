#include<cmath>
#include<iostream>
#include<cmath>
#include "./OrientationMatching.hpp"

float OrientationMatching::MapToRange(float theta) const
{
    float modifiedRot = fmod(theta, M_PI);
    if(std::abs(modifiedRot)<=M_PI)
    {
        return modifiedRot;
    }
    else if(modifiedRot>M_PI)
    {
        return modifiedRot-2*M_PI;
    }
    return modifiedRot+2*M_PI;
}

SteeringOutput OrientationMatching::CalculateSteering(Kinematic &character, Kinematic target) const
{
    SteeringOutput steering(sf::Vector2f(0.0f, 0.0f), 0);
    /* Find the naive direction to the target */
    float rotation = target.orientation - character.orientation;

    /* Map roation to [-PI, PI]radians range */
    rotation = MapToRange(rotation);
    float rotationSize = std::abs(rotation);
    /* Check if we are within the region of TargetRadius */
    if(rotationSize<this->targetRadius)
    {
        // std::cout<<"Steering Angular 0: "<<steering.angular<<"\n";
        return steering;
    }
    float targetRot;
    /* Check if we are outside the region of SlowRadius */
    if(rotationSize>this->slowRadius)
    {
        targetRot = this->maxRot;
        // std::cout<<"Rotation: "<<targetRot<<"\n";
    }
    /* Character is inside the region of SlowRadius but Outside of TargetRadius*/
    else
    {
        targetRot = rotationSize*this->maxRot/this->slowRadius;
    }
    /* target Rotation */
    targetRot *= rotation/rotationSize;
    // std::cout<<"Target Roation: "<<targetRot<<"\n";

    /* Angular acceleration tries to align with the target */
    steering.angular = (targetRot - character.rotation)/this->timeToTarget;
    // std::cout<<"Steering Angular 1: "<<steering.angular<<"\n";

    float angularAcc = std::abs(steering.angular);
    if (angularAcc > this->maxAngularAcc)
    {
        steering.angular /= angularAcc;
        steering.angular *= this->maxAngularAcc;
        // std::cout<<"Steering Angular 2: "<<steering.angular<<"\n";
    }
    // std::cout<<"Steering Angular 3: "<<steering.angular<<"\n";
    steering.linear = sf::Vector2f(0.0f,0.0f);
    return steering;
}