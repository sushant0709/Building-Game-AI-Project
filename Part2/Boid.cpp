#include "./Boid.hpp"

Boid::Boid(const Boid& other)
{
    this->texture = other.texture;
    this->data = other.data;
    this->s = other.s;
}

Boid::Boid(sf::Texture *t, Kinematic d, bool isMonster)
{

    this->texture = t;
    this->data = d;
    this->s.setTexture(*this->texture);
    this->s.getTexture();
    // size of sprite-> 46.5x31.35
    // this->s.scale(sf::Vector2f(0.0806f,0.0797f));
    if(isMonster)
        this->s.scale(sf::Vector2f(0.15f,0.15f));
    else
        this->s.scale(sf::Vector2f(0.02f,0.02f));
    this->s.setOrigin(this->s.getLocalBounds().width/ 2.0f, this->s.getLocalBounds().height/ 2.0f);
    this->s.setPosition(this->data.position);
    this->s.setRotation(this->data.orientation);

}
void Boid::setPosition(sf::Vector2f position)
{
    this->s.setPosition(position);
}

sf::Vector2f Boid::checkBorderP(sf::Vector2f location)
{
    if(location.x<0) location.x=630.0f;
    if(location.x>630.0f) location.x=0.0f;
    if(location.y<0) location.y=470.0f;
    if(location.y>470.0f) location.y=0.0f;
    
    return location;
}

void Boid::move()
{
    this->data.position = checkBorderP(this->data.position);
    this->s.setPosition(this->data.position);
    this->s.setRotation(this->data.orientation*180.0f/M_PI);
}

