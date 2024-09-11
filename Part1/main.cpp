#include<SFML/Graphics.hpp>
#include<iostream>
#include<cmath>
#include<deque>
#include<time.h>
#include<unordered_map>
#include<vector>
#include<limits>
#include<algorithm>
#include<string>
#include "./PositionMatching.hpp"
#include "./OrientationMatching.hpp"
#include "./Kinematic.hpp"
#include "./Crumbs.hpp"
#include "./Boid.hpp"

using namespace std;

// Constants
#define NUM_BOUND_WALLS 4
#define NUM_ROOM_WALLS 7

// defines
#define CRUMB_SIZE 15
#define CRUMB_DISTANCE 30
sf::Clock gameClock;

// Attributes for room class are defined here
class Room
{
    public:
        int room_id;
        sf::Vector2f top_left, top_right, bottom_right, bottom_left;
        vector <sf::Vector2f> room_entry;
        sf::Vector2f room_center;
        int num_entries;
        sf::Vector2f wander_point;
        vector<int> adjacent_rooms;

        Room(vector <sf::Vector2f> coordinates, int room_id, vector <sf::Vector2f> room_entry, vector<int> adjacent_rooms)
        {
            this->top_left = coordinates[0];
            this->top_right = coordinates[1];
            this->bottom_right = coordinates[2];
            this->bottom_left = coordinates[3];
            // calculating the coordinates of the center of the room
            this->room_center.x=(this->top_left.x+this->top_right.x)/2;
            this->room_center.y=(this->top_left.y+this->bottom_left.y)/2;
            
            this->room_id = room_id;
            for(auto re: room_entry)
                this->room_entry.push_back(re);

            for(auto r:adjacent_rooms)
                this->adjacent_rooms.push_back(r);
            this->num_entries = room_entry.size();
        }
        sf::Vector2f get_wander_point()
        {
            sf::Vector2f new_wander_point;
            new_wander_point.x = (rand() % (int)(this->top_right.x-this->top_left.x-20))+top_left.x+10;
            new_wander_point.y = (rand() % (int)(this->bottom_right.y-this->top_right.y-20))+top_right.y+10;

            while(new_wander_point.x==wander_point.x || new_wander_point.y==wander_point.y)
            {
                new_wander_point.x = (rand() % (int)(this->top_right.x-this->top_left.x-20))+top_left.x+10;
                new_wander_point.y = (rand() % (int)(this->bottom_right.y-this->top_right.y-20))+top_right.y+10;
            }
            // cout<<"TL Y:"<<this->top_left.y<<"BL Y:"<<bottom_left.y<<"BR X:"<<bottom_right.x<<"BL X:"<<bottom_left.x<<endl;

            // cout<<wander_point.y<<" "<<wander_point.x<<endl;
            return new_wander_point;
        }

};

class Graph
{
    public:

        // Graph data structure, unordered map of unordered maps

        unordered_map<int, const unordered_map<int, int> > vertices;

        void add_vertex(int vertex, const unordered_map<int, int>& edges)
        {
            vertices.insert( unordered_map <int, const unordered_map<int, int> >::value_type(vertex, edges));
        }

        vector<int> dijkstra(int first, int final)
        {
            unordered_map<int, int> previous;
            unordered_map<int, int> weights;
            
            vector<int> nodes;
            vector<int> path;
            
            auto comparator = [&] (int left, int right) { return weights[left] > weights[right]; };

            for (auto& vertex : vertices)
            {
                if (vertex.first == first) weights[vertex.first] = 0;
                else weights[vertex.first] = numeric_limits<int>::max();
                
                nodes.push_back(vertex.first);  
                push_heap(begin(nodes), end(nodes), comparator);
            }

            while (!nodes.empty())
            {
                pop_heap(begin(nodes), end(nodes), comparator);

                char closest = nodes.back();
                nodes.pop_back();
                
                if (closest == final)
                {
                    while (previous.find(closest) != end(previous))
                    {
                        path.push_back(closest);
                        closest = previous[closest];
                    }
                    
                    break;
                }
                
                if (weights[closest] == numeric_limits<int>::max()) break;
                
                for (auto& neighbor : vertices[closest])
                {
                    int alt = weights[closest] + neighbor.second;
                    if (alt < weights[neighbor.first])
                    {
                        weights[neighbor.first] = alt;
                        previous[neighbor.first] = closest;
                        make_heap(begin(nodes), end(nodes), comparator);
                    }
                }
            }
            return path;
        }
};
// Everything in the viewport is a room class, seperated by a wall class
class Wall : sf::RectangleShape
{   
    public:

    Wall(sf::Vector2f position, sf::Vector2f size, float rotation)
    {
        this->setFillColor(sf::Color::Blue);
        this->setPosition(position);
        this->setSize(size);
        this->setRotation(rotation);
    }

    void draw(sf::RenderWindow* window)
    {
        window->draw(*this);
    }

    float get_rot()
    {
        return this->getRotation();
    }

    private:

    void setPos(sf::Vector2f position)
    {
        this->setPosition(position);
    }

    sf::Vector2f get_pos()
    {
        return this->getPosition();
    }
};
struct DecisionTree
{
    int node_id;
    DecisionTree* left;
    DecisionTree* right;
    DecisionTree(): node_id(0),left(nullptr), right(nullptr){}
    DecisionTree(int x): node_id(x),left(nullptr), right(nullptr){}
    DecisionTree(int x, DecisionTree* left, DecisionTree* right): node_id(x),left(left), right(right){}
};

// Defining an array of boundary walls
sf::Vector2f init_pos[] = { sf::Vector2f(0, 0), sf::Vector2f(640, 0), sf::Vector2f(640, 480), sf::Vector2f(0,480)};
sf::Vector2f init_size = sf::Vector2f(680, 10);
float init_rot[] = { 0, 90, 180, 270};
Wall bound_walls[] = { Wall(init_pos[0], init_size, init_rot[0]), 
                        Wall(init_pos[1], init_size, init_rot[1]), 
                        Wall(init_pos[2], init_size, init_rot[2]), 
                        Wall(init_pos[3], init_size, init_rot[3]) 
                        };
// Defining an array of obstacle walls

sf::Vector2f wall_pos[] = { sf::Vector2f(200,0), sf::Vector2f(0, 200), sf::Vector2f(150, 200), sf::Vector2f(200, 160), sf::Vector2f(450, 160), sf::Vector2f(300, 160), sf::Vector2f(550, 160)};
sf::Vector2f wall_size[] = { sf::Vector2f(200, 8), sf::Vector2f(150, 8), sf::Vector2f(230, 8), sf::Vector2f(200, 8), sf::Vector2f(200, 8), sf::Vector2f(260, 8), sf::Vector2f(270, 8)};
Wall room_walls[] = { 
                        Wall(wall_pos[0], wall_size[0], 90), 
                        Wall(wall_pos[1], wall_size[1], 0), 
                        Wall(wall_pos[2], wall_size[2], 90), 
                        Wall(wall_pos[3], wall_size[3], 0), 
                        Wall(wall_pos[4], wall_size[4], 0), 
                        Wall(wall_pos[5], wall_size[5], 90), 
                        Wall(wall_pos[6], wall_size[6], 90)
                    };

// Defining range of values where the sprite could move through

int num_openings = 5;
sf::Vector2f room_openings[] = { sf::Vector2f(165, 195), sf::Vector2f(137, 452), sf::Vector2f(245, 200), sf::Vector2f(290, 440), sf::Vector2f(540,445), sf::Vector2f(417, 160)};
vector <Room> rooms  { 
                        Room( {sf::Vector2f(0, 0),sf::Vector2f(200,0),sf::Vector2f(200, 200),sf::Vector2f(0,200)}, 0, { sf::Vector2f(165, 200) }, { 2 }), 
                        Room( {sf::Vector2f(0,200),sf::Vector2f(140,200),sf::Vector2f(140,480),sf::Vector2f(0,480)}, 1, {sf::Vector2f(150, 445)}, { 2 } ),
                        Room( {sf::Vector2f(140,200), sf::Vector2f(300,200), sf::Vector2f(300,480), sf::Vector2f(140,480)}, 2, {sf::Vector2f(165, 200), sf::Vector2f(150, 445), sf::Vector2f(245, 200), sf::Vector2f(290, 440)}, { 1, 3, 5} ) ,  
                        Room( {sf::Vector2f(170,160), sf::Vector2f(300,160), sf::Vector2f(300,200), sf::Vector2f(170, 200)}, 3, {sf::Vector2f(245, 200) }, { 2 }), 
                        Room( {sf::Vector2f(170,0), sf::Vector2f(640,0), sf::Vector2f(640,160), sf::Vector2f(200,160)}, 4, {sf::Vector2f(417, 160)}, { 5 }),
                        Room( {sf::Vector2f(300,160), sf::Vector2f(540,160), sf::Vector2f(540,480), sf::Vector2f(140,480)}, 5, {sf::Vector2f(290, 440), sf::Vector2f(417, 160), sf::Vector2f(550,455)}, { 2, 6 }) , 
                        Room( {sf::Vector2f(540,160), sf::Vector2f(640,160), sf::Vector2f(640,480), sf::Vector2f(540,480)}, 6, {sf::Vector2f(550,455)}, { 5 })
                    };

sf::Vector2f calculateCommonEntry(int current_room, int neighbouring_room)
{
    sf::Vector2f entry;
   // Check if any of the entrances in the current neighboring room match the entrances of the current room
    for (const auto& entrance : rooms[neighbouring_room].room_entry) {
        for (const auto& current_entrance : rooms[current_room].room_entry) {
            if (entrance == current_entrance) {
                entry = entrance;
                break;
            }
        }
    }
    return entry;
}
bool spriteWithinThreshold(sf::Sprite s, sf::Vector2f target){
    // Calculate the absolute difference in x and y coordinates
    float x_diff = std::abs(s.getPosition().x - target.x);
    float y_diff = std::abs(s.getPosition().y - target.y);

    return (x_diff <= 1.75 && y_diff <= 1.75);
}

int main()
{
    // Creating the window and intial variables
    sf::Texture *boidTexture;
    boidTexture = new sf::Texture();
    boidTexture->loadFromFile("./imgs/boid.png");
    std::string title;

    title = "Part 1: Decision Trees";

    // Creating the window and initial variables
    sf::RenderWindow window(sf::VideoMode(640, 480), title);

    // Seed the random number generator with the current time
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // character and target
    Kinematic charData(sf::Vector2f(50.0f,50.0f), 0.0f, sf::Vector2f(0.0f,0.0f), 0.0f);
    Kinematic target(sf::Vector2f(50.0f,50.0f), 0.0f, sf::Vector2f(0.0f,0.0f), 0.0f);

    // Sprite
    Boid sprite(boidTexture, charData);

    //PositionMatching and OrientationMatching
    PositionMatching pm(40.0f, 80.0f, 120.0f, 0.5f, 0.1f);
    OrientationMatching om(100.0f, 0.5f, 0.20f, 0.001f, 0.1f);


    // Breadcrumbs
    deque <crumb> breadcrumbs;
    sf::Vector2f latest_crumb_pos = sf::Vector2f (0.f, 0.f);
    int crumb_count = 0;

    // Mouse position based variables
    sf::Event event;
    sf::Vector2i mouse_curr, mouse_final = sf::Vector2i(0, 0);
    sf::Vector2f mouse_final_float;

    // Steering Outputs
    SteeringOutput steeringArrive;
    SteeringOutput steeringAlign;
    SteeringOutput steering;

    // time
    float startTime = gameClock.getElapsedTime().asSeconds();
    float prevTime = startTime;

    // Graph for environment
    vector <char> path;
    Graph g;
    g.add_vertex(0, {{2, 1}});
    g.add_vertex(1, {{2, 1}});
    g.add_vertex(2, {{0, 1}, {1, 1}, {3, 1}, {5, 1}});
    g.add_vertex(3, {{2, 1}});
    g.add_vertex(4, {{5, 1}});
    g.add_vertex(5, {{2,1},{4,1},{6, 1}});
    g.add_vertex(6, {{5, 1}});

    // Flag set to true if sprite moves in same room
    // Flag set to true if mouse is clicked on a wall
    bool same_room = true;
    bool moving_to_target_room = false;
    bool moving_to_final_position = false; 
    // bool mouse_click = false;
    int current_room = 0, clicked_room = 0, ending_room = 0;
    sf::Vector2f wander_point = sprite.data.position;

    // Decision Tree
    struct DecisionTree* root = new DecisionTree(0);
    root->left = new DecisionTree(1);
    root->right = new DecisionTree(2);
    root->right->left = new DecisionTree(3);
    root->right->right = new DecisionTree(4);
    root->right->right->left = new DecisionTree(5);
    root->right->right->right = new DecisionTree(6);

    while (window.isOpen())
    {
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                std::cout<<"Closing Window";
                window.close();
            }
        } 
        window.clear(sf::Color::White); 
        DecisionTree* tree_node = root;
        // Set the mouse_click variable
        // mouse_click = true;
        // Erasing path vector 
        path.erase(path.begin(), path.end());
        while(tree_node->left!=NULL)
        {
            if(tree_node->node_id==0)
            {
                // if wander point within radius of 3 from sprite's position
                if(abs(sprite.data.position.x-wander_point.x)<=3 && abs(sprite.data.position.y-wander_point.y)<=3)
                {
                    tree_node = tree_node->right;
                }
                else
                {
                    tree_node = tree_node->left;
                }
            }
            else if(tree_node->node_id==2)
            {
                // if time elapsed from starting time is greater than 10 sec
                if(gameClock.getElapsedTime().asSeconds()-startTime > 10.0)
                {
                    tree_node = tree_node->left;
                }
                else
                {
                    tree_node = tree_node->right;
                }
            }
            else if(tree_node->node_id==4)
            {
                // check if the sprite is near to the walls
                if(sprite.data.position.x-rooms[current_room].top_left.x<=20.0f || sprite.data.position.y-rooms[current_room].top_left.y<=20.0f || rooms[current_room].top_right.x - sprite.data.position.x<=20.0f || rooms[current_room].bottom_right.y - sprite.data.position.y<=20.0f)
                {
                    tree_node = tree_node->right;
                }
                else
                {
                    tree_node = tree_node->left;
                }
            }
        }
        if(tree_node->node_id==3)
        {
            cout<<"Moving character to a random room"<<endl;
            int random_room = rand()%7;
            while(random_room == current_room)
                random_room = rand()%7;
            wander_point = rooms[random_room].get_wander_point();
            startTime = 0;
            // gameClock.restart();
        }
        else if(tree_node->node_id==5)
        {
            wander_point = rooms[current_room].get_wander_point();
            cout<<"Moving character to a random point within the same room"<<endl;
        }
        else if(tree_node->node_id==6)
        {
            wander_point=rooms[current_room].room_center;
            cout<<"Moving character to the center of the current room"<<endl;
        }

        // Setting mouse pointer
        mouse_curr = mouse_final;
        mouse_final.x = wander_point.x;
        mouse_final.y = wander_point.y;  
        mouse_final_float = sf::Vector2f(static_cast <float> (mouse_final.x), static_cast <float> (mouse_final.y) );

        // Logic that checks if mouse_click is in same room
        if(mouse_final.x > rooms[current_room].top_left.x && mouse_final.x < rooms[current_room].top_right.x && mouse_final.y < rooms[current_room].bottom_left.y && mouse_final.y > rooms[current_room].top_right.y )
        {
            same_room = true;
            if(mouse_final.x != mouse_curr.x || mouse_final.y != mouse_curr.y)
            {
                target.position = mouse_final_float;
                target.orientation = std::atan2(target.position.y - sprite.data.position.y,target.position.x - sprite.data.position.x);
            }
        }
        // if different room
        else
        {
            same_room = false;

            // calculate the target room id
            for(int i = 0; i < 7;i++)
            {
                if(mouse_final.x > rooms[i].top_left.x && mouse_final.x < rooms[i].top_right.x && mouse_final.y > rooms[i].top_left.y && mouse_final.y < rooms[i].bottom_left.y)
                {
                    clicked_room = rooms[i].room_id;
                    break;
                }
            }

            // calculate path using Dijkstra's algorithm
            for (int vertex : g.dijkstra(current_room, clicked_room) )
            {
                path.push_back(vertex);
            }
            ending_room = path[0];
            moving_to_target_room = true;
        }

        // if the final position within same room, calc steering, move and continue it without doing anything else
        if(same_room)
        {
            target.position = mouse_final_float;
            target.orientation = std::atan2(target.position.y - sprite.data.position.y,target.position.x - sprite.data.position.x);

            steeringArrive = pm.CalculateSteering(sprite.data, target);
            steeringAlign = om.CalculateSteering(sprite.data, target);
            steering.linear = steeringArrive.linear;
            steering.angular = steeringAlign.angular;

            float currentTime = gameClock.getElapsedTime().asSeconds();
            float elapsedSeconds = currentTime - prevTime;
            prevTime = currentTime;

            sprite.data.update(steering, elapsedSeconds);
            sprite.move();
        }
        // if the final position is in different room
        else
        {
            if(moving_to_target_room)
            {
                if(path.size())
                {
                    target.position = calculateCommonEntry(current_room, path[path.size()-1]);

                    target.orientation = std::atan2(target.position.y - sprite.data.position.y,target.position.x - sprite.data.position.x);

                    steeringArrive = pm.CalculateSteering(sprite.data, target);
                    steeringAlign = om.CalculateSteering(sprite.data, target);
                    steering.linear = steeringArrive.linear;
                    steering.angular = steeringAlign.angular;

                    float currentTime = gameClock.getElapsedTime().asSeconds();
                    float elapsedSeconds = currentTime - prevTime;
                    prevTime = currentTime;

                    sprite.data.update(steering, elapsedSeconds);
                    sprite.move();
                    if(spriteWithinThreshold(sprite.s, target.position))
                    {
                        current_room = path.back();
                        path.pop_back();
                        if(path.size()==0)
                        {
                            moving_to_target_room = false;
                            moving_to_final_position = true;
                        }
                    }
                }
                
            }
            else if(moving_to_final_position)
            {
                target.position = mouse_final_float;
                target.orientation = std::atan2(target.position.y - sprite.data.position.y,target.position.x - sprite.data.position.x);

                steeringArrive = pm.CalculateSteering(sprite.data, target);
                steeringAlign = om.CalculateSteering(sprite.data, target);
                steering.linear = steeringArrive.linear;
                steering.angular = steeringAlign.angular;

                float currentTime = gameClock.getElapsedTime().asSeconds();
                float elapsedSeconds = currentTime - prevTime;
                prevTime = currentTime;

                sprite.data.update(steering, elapsedSeconds);
                sprite.move();
                if(spriteWithinThreshold(sprite.s, target.position))
                {
                    moving_to_final_position = false;
                }
            }
        }
        if(sprite.s.getPosition() != target.position)
        {
            // Movement and Breadcrumb logic using double ended queue
            if( std::sqrt(std::pow(sprite.data.position.x - latest_crumb_pos.x, 2.0) + pow(sprite.data.position.y - latest_crumb_pos.y, 2.0 ))  > CRUMB_DISTANCE) 
            {
                crumb c(crumb_count,&window);
                c.drop(latest_crumb_pos); 
                latest_crumb_pos = sprite.s.getPosition();

                if (crumb_count > CRUMB_SIZE ){ breadcrumbs.pop_front(); breadcrumbs.push_back(c); }
                else breadcrumbs.push_back(c);

                crumb_count++;
            }
        }
        
        // Drawing walls and boundaries
        for(int i = 0; i < NUM_BOUND_WALLS; i++)
        {
            bound_walls[i].draw(&window);
        }
        for(int i = 0; i < NUM_ROOM_WALLS;i++)
        {
            room_walls[i].draw(&window);
        }
        
        // Drawing the sprite
        window.draw(sprite.s);

        // Draw breadcrumbs
        for(auto x : breadcrumbs) x.draw();

        // Displaying the window
        window.display();
    }
    return 0;
}
