#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <iostream>
#include "./Kinematic.hpp"

// Default constructor
Kinematic::Kinematic()
{
    this->position = sf::Vector2f(std::rand()%561, std::rand()%421);
    this->orientation = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2 * M_PI;
    this->orientation = 0.0f;
    this->velocity = sf::Vector2f((std::rand() % 80) - 40, (std::rand() % 80) - 40);
    // this->rotation = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * M_PI;
    this->rotation = 0.0f;
}
Kinematic::Kinematic(sf::Vector2f p, float o, sf::Vector2f v, float r)
{
    this->position = p;
    this->orientation = o;
    this->velocity = v;
    this->rotation = r;
}
void Kinematic::update(SteeringOutput steering, float time)
{
    // /* Update the position and orientation */
    this->position += this->velocity*time;
    this->orientation += this->rotation*time;

    /* Update the velocity and rotation based on steering input */
    this->velocity += steering.linear*time;
    this->rotation += steering.angular*time;
}
