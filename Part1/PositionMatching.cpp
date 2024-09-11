#include<cmath>
#include<iostream>
#include "./PositionMatching.hpp"

SteeringOutput PositionMatching::CalculateSteering(Kinematic &character, Kinematic target) const
{
    SteeringOutput steering(sf::Vector2f(0.0f, 0.0f), 0);
    /* Find the direction to the target */
    sf::Vector2f direction = target.position - character.position;
    float distance = sqrt(std::pow(direction.x,2)+std::pow(direction.y,2));
    // std::cout<<"Distance: "<<distance<<"\n";
    float targetSpeed;
    /* Check if we are within the region of TargetRadius */
    if(distance<this->targetRadius)
    {

        return steering;
    }
    /* Check if we are outside the region of SlowRadius */
    if(distance>this->slowRadius)
    {
        targetSpeed = this->maxSpeed;
    }
    /* Character is inside the region of SlowRadius but Outside of TargetRadius*/
    else
    {
        targetSpeed = distance*this->maxSpeed/this->slowRadius;
    }
    /* target velocity */
    sf::Vector2f targetVelocity = direction;
    float mag = std::sqrt(std::pow(targetVelocity.x,2)+std::pow(targetVelocity.y,2));
    targetVelocity = targetVelocity/mag*targetSpeed;
    // std::cout<<"Target Velocity: "<<targetVelocity.x<<" "<<targetVelocity.y<<"\n";
    steering.linear = targetVelocity - character.velocity;
    // std::cout<<"Steering Linear 1: "<<steering.linear.x<<" "<<steering.linear.y<<"\n";
    steering.linear /= this->timeToTarget;
    // std::cout<<"Steering Linear 2: "<<steering.linear.x<<" "<<steering.linear.y<<"\n";
    float linearAccMag = std::sqrt(std::pow(steering.linear.x,2)+std::pow(steering.linear.y,2));
    if (linearAccMag > this->maxAcc)
    {
        steering.linear /= linearAccMag;
        // std::cout<<"Steering Linear 3: "<<steering.linear.x<<" "<<steering.linear.y<<"\n";
        steering.linear *= this->maxAcc;
    }
    // std::cout<<"Steering Linear 4: "<<steering.linear.x<<" "<<steering.linear.y<<"\n";
    steering.angular = 0.0f;
    return steering;
}