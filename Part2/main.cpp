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
#include<random>
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
sf::Clock spriteClock, monsterClock;

#define MAX_WANDER_TIME 7
#define MAX_MOVE_CENTER_TIME 4
#define MAX_PATH_FIND_TIME 15
#define MAX_DANCE_TIME 10

// global variables

enum Actions
{
    START,
    WANDER,
    MOVE_CENTER_OF_ROOM,
    PATH_FIND,
    DANCE
};
// action_choice
Actions action_choice=START;

std::string title = "Part 2: Behavior Trees";

// Creating the window and initial variables
sf::RenderWindow window(sf::VideoMode(640, 480), title);


// Breadcrumbs
deque <crumb> breadcrumbs,monster_breadcrumbs;
sf::Vector2f latest_crumb_pos = sf::Vector2f (50.f, 50.f),latest_crumb_pos_monster = sf::Vector2f (360.f, 300.f);
int crumb_count, crumb_count_monster = 0;

vector <int> scripted_path = {2, 0, 2, 3, 2, 1, 2, 5, 6, 5, 4};

// Mouse position based variables
sf::Event event;
sf::Vector2i mouse_curr, mouse_final = sf::Vector2i(0, 0);
sf::Vector2f mouse_final_float;

// Steering Outputs
SteeringOutput steeringArrive;
SteeringOutput steeringAlign;
SteeringOutput steering;
SteeringOutput steeringArrive_monster;
SteeringOutput steeringAlign_monster;
SteeringOutput steering_monster;

// time
float startTime = spriteClock.getElapsedTime().asSeconds();
float prevTime = startTime;

float monster_startTime = monsterClock.getElapsedTime().asSeconds();
float monsterPrevTime = monster_startTime;


// Flag set to true if sprite moves in same room
// Flag set to true if mouse is clicked on a wall
bool same_room = true;
bool moving_to_target_room = false;
bool moving_to_final_position = false; 
// bool mouse_click = false;
// sprite
int current_room = 0, clicked_room = 0, ending_room = 0;



int current_monster_room=5, ending_monster_room=5,clicked_monster_room=5;
bool same_monster_room=true, monster_moving_to_target_room=false, monster_moving_to_final_position=false, wall_click_monster=false;
sf::Vector2i monster_mouse_curr, monster_mouse_final=sf::Vector2i(0, 0);
sf::Vector2f monster_mouse_final_float;



// character, monster and target
Kinematic charData(sf::Vector2f(50.0f,50.0f), 0.0f, sf::Vector2f(0.0f,0.0f), 0.0f);
Kinematic monsterData(sf::Vector2f(360.0f,300.0f), 0.0f, sf::Vector2f(0.0f,0.0f), 0.0f);
Kinematic target(sf::Vector2f(50.0f,50.0f), 0.0f, sf::Vector2f(0.0f,0.0f), 0.0f);
Kinematic monsterTarget(sf::Vector2f(360.0f,300.0f), 0.0f, sf::Vector2f(0.0f,0.0f), 0.0f);

sf::Vector2f wander_point;
sf::Vector2f monster_wander_point;
bool isWanderPointGenerated=false, isDancePointGenerated=false;

//PositionMatching and OrientationMatching
PositionMatching pm(40.0f, 80.0f, 120.0f, 0.5f, 0.1f);
OrientationMatching om(100.0f, 0.5f, 0.20f, 0.001f, 0.1f);

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

// Graph for environment
vector <char> path, monster_path;
Graph g;

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

void pathFindMonster(Graph &g, Boid &monster,sf::Vector2f mouse_final)
{
    // Logic that checks if mouse_click is in same room
    if(mouse_final.x > rooms[current_monster_room].top_left.x && mouse_final.x < rooms[current_monster_room].top_right.x && mouse_final.y < rooms[current_monster_room].bottom_left.y && mouse_final.y > rooms[current_monster_room].top_right.y )
    {
        same_monster_room = true;
        monster_mouse_final_float = sf::Vector2f(static_cast < float > (mouse_final.x), static_cast < float > (mouse_final.y) );
        if(mouse_final.x != monster_mouse_curr.x || mouse_final.y != monster_mouse_curr.y)
        {
            monsterTarget.position = monster_mouse_final_float;
            monsterTarget.orientation = std::atan2(monsterTarget.position.y - monster.data.position.y,monsterTarget.position.x - monster.data.position.x);
        }
        // std::cout<<"Inside path find: "<<mouse_final.x<<" "<<mouse_final.y<<endl;
    }
    // if different room
    else
    {
        same_monster_room = false;

        // calculate the target room id
        for(int i = 0; i < 7;i++)
        {
            if(mouse_final.x > rooms[i].top_left.x && mouse_final.x < rooms[i].top_right.x && mouse_final.y > rooms[i].top_left.y && mouse_final.y < rooms[i].bottom_left.y)
            {
                clicked_monster_room = rooms[i].room_id;
                break;
            }
        }

        // calculate path using Dijkstra's algorithm
        for (int vertex : g.dijkstra(current_monster_room, clicked_monster_room) )
        {
            monster_path.push_back(vertex);
        }
        ending_monster_room = monster_path[0];
        monster_moving_to_target_room = true;
    }

    // if the final position within same room, calc steering, move and continue it without doing anything else
    if(same_monster_room)
    {
        monsterTarget.position = monster_mouse_final_float;
        monsterTarget.orientation = std::atan2(monsterTarget.position.y -monster.data.position.y,monsterTarget.position.x - monster.data.position.x);

        steeringArrive_monster = pm.CalculateSteering(monster.data, monsterTarget);
        steeringAlign_monster = om.CalculateSteering(monster.data, monsterTarget);
        steering_monster.linear = steeringArrive_monster.linear;
        steering_monster.angular = steeringAlign_monster.angular;

        float currentTime = monsterClock.getElapsedTime().asSeconds();
        float elapsedSeconds = currentTime - monsterPrevTime;
        monsterPrevTime = currentTime;

        monster.data.update(steering_monster, elapsedSeconds);
        monster.move();
    }
    // if the final position is in different room
    else
    {
        if(monster_moving_to_target_room)
        {
            if(monster_path.size())
            {
                monsterTarget.position = calculateCommonEntry(current_monster_room, monster_path[monster_path.size()-1]);

                monsterTarget.orientation = std::atan2(monsterTarget.position.y - monster.data.position.y,monsterTarget.position.x - monster.data.position.x);

                steeringArrive_monster = pm.CalculateSteering(monster.data, monsterTarget);
                steeringAlign_monster = om.CalculateSteering(monster.data, monsterTarget);
                steering_monster.linear = steeringArrive_monster.linear;
                steering_monster.angular = steeringAlign_monster.angular;

                float currentTime = monsterClock.getElapsedTime().asSeconds();
                float elapsedSeconds = currentTime - monsterPrevTime;
                monsterPrevTime = currentTime;

                monster.data.update(steering_monster, elapsedSeconds);
                monster.move();

                if(spriteWithinThreshold(monster.s, monsterTarget.position))
                {
                    current_monster_room = monster_path.back();
                    monster_path.pop_back();
                    if(monster_path.size()==0)
                    {
                        monster_moving_to_target_room = false;
                        monster_moving_to_final_position = true;
                    }
                }
            }
            
        }
        else if(monster_moving_to_final_position)
        {
            monsterTarget.position = monster_mouse_final_float;
            monsterTarget.orientation = std::atan2(monsterTarget.position.y - monster.data.position.y,monsterTarget.position.x - monster.data.position.x);

            steeringArrive_monster = pm.CalculateSteering(monster.data, monsterTarget);
            steeringAlign_monster = om.CalculateSteering(monster.data, monsterTarget);
            steering_monster.linear = steeringArrive_monster.linear;
            steering_monster.angular = steeringAlign_monster.angular;

            float currentTime = monsterClock.getElapsedTime().asSeconds();
            float elapsedSeconds = currentTime - monsterPrevTime;
            monsterPrevTime = currentTime;

            monster.data.update(steering_monster, elapsedSeconds);
            monster.move();

            if(spriteWithinThreshold(monster.s, monsterTarget.position))
            {
                monster_moving_to_final_position = false;
            }
        }
    }
    if(monster.s.getPosition() != monsterTarget.position)
    {
        // Movement and Breadcrumb logic using double ended queue
        if( std::sqrt(std::pow(monster.data.position.x - latest_crumb_pos_monster.x, 2.0) + pow(monster.data.position.y - latest_crumb_pos_monster.y, 2.0 ))  > CRUMB_DISTANCE) 
        {
            crumb c(crumb_count_monster,&window,true);
            c.drop(latest_crumb_pos_monster); 
            latest_crumb_pos_monster = monster.s.getPosition();

            if (crumb_count_monster > CRUMB_SIZE ){ monster_breadcrumbs.pop_front(); monster_breadcrumbs.push_back(c); }
            else monster_breadcrumbs.push_back(c);

            crumb_count_monster++;
        }
    }
    return;
}

struct DecisionTree
{
    int node_id;
    DecisionTree* left;
    DecisionTree* right;
    DecisionTree(): node_id(0),left(nullptr), right(nullptr){}
    DecisionTree(int x): node_id(x),left(nullptr), right(nullptr){}
    DecisionTree(int x, DecisionTree* left, DecisionTree* right): node_id(x),left(left), right(right){}
};
enum NodeState
{
    RUNNING,
    SUCCESS,
    FAILURE
};

class BehaviorTreeNode {
public:
    NodeState state;
    virtual ~BehaviorTreeNode() {}
    virtual NodeState execute(Boid &monster) = 0;
};

class SelectorNode : public BehaviorTreeNode {
private:
    std::vector<BehaviorTreeNode*> children;

public:
    SelectorNode(std::vector<BehaviorTreeNode*> children) : children(children) {}
    void add_child(BehaviorTreeNode* child) {
        children.push_back(child);
    }

    virtual NodeState execute(Boid &monster) override {
        for (BehaviorTreeNode* child : children) {
            NodeState ch = child->execute(monster);
            if (SUCCESS==ch) {
                return SUCCESS; // Make state of current node SUCCESS if any child succeeds
            }
            else if(RUNNING==ch){
                return RUNNING;
            }
        }
        return FAILURE; //failure if all children fail
    }
};

class SequenceNode : public BehaviorTreeNode {
private:
    std::vector<BehaviorTreeNode*> children;
    int currentNodeIndex;

public:
    SequenceNode(std::vector<BehaviorTreeNode*> children) : children(children),currentNodeIndex(0) {}
    void add_child(BehaviorTreeNode* child) {
        children.push_back(child);
    }

    virtual NodeState execute(Boid &monster) override {

        bool isRunning=false;
        for (BehaviorTreeNode* child : children) {
            NodeState ch = child->execute(monster);

            if (FAILURE==ch) {
                return FAILURE; //Failure if any child fails
            }
            else if(RUNNING==ch){
                isRunning=true;
            }
        }
        return (isRunning)?RUNNING:SUCCESS;
    }
};

class IsSpriteinNeighbouringOrSameRoom : public BehaviorTreeNode {
    public:
    virtual NodeState execute(Boid &monster) override {
        for(auto r:rooms[current_monster_room].adjacent_rooms){
            if(r==current_room){
                cout<<"Character in same or neighbouring room of the monster"<<endl;
                return SUCCESS;
            }
        }
        return FAILURE;
    }
};

// Decorator base class
class DecoratorNode : public BehaviorTreeNode {
protected:
    BehaviorTreeNode* child;
    bool hasExecuted;

public:
    DecoratorNode(BehaviorTreeNode* child) : child(child), hasExecuted(false) {}

    virtual NodeState execute(Boid &monster) override {
        if (!hasExecuted && child) {
            NodeState result = child->execute(monster);
            if (result == NodeState::SUCCESS || result == NodeState::FAILURE) {
                hasExecuted = true;
            }
            return result;
        }
        return NodeState::FAILURE; // If the child has already executed, return failure
    }
};

// wallHit decorator class
class WallHitNode: public DecoratorNode {
    public:
    WallHitNode(BehaviorTreeNode* child) : DecoratorNode(child) {}
    virtual NodeState execute(Boid &monster) override {
        if(monster.data.position.x-rooms[current_monster_room].top_left.x<=15.0f || monster.data.position.y-rooms[current_monster_room].top_left.y<=15.0f || rooms[current_monster_room].top_right.x - monster.data.position.x<=20.0f || rooms[current_monster_room].bottom_right.y - monster.data.position.y<=20.0f)
        {
            cout<<"Monster hits the wall"<<endl;
            return FAILURE;
        }
        return child->execute(monster);
    }
};

// action nodes
class WanderAction: public BehaviorTreeNode {
public:
    virtual NodeState execute(Boid &monster) override {
        // Implement action here
        action_choice = WANDER;
        if(monsterClock.getElapsedTime().asSeconds()-monster_startTime > MAX_WANDER_TIME)
        {
            cout<<"WanderAction failed due to timeout"<<endl;
            monsterClock.restart();
            monster_startTime = 0;
            monsterPrevTime = monster_startTime;
            isWanderPointGenerated = false;
            return FAILURE;
        }
        else
        {
            if(!isWanderPointGenerated)
            {
                monster_wander_point = rooms[current_monster_room].get_wander_point();
                isWanderPointGenerated = true;
            }
            if (abs(monster.data.position.x - monster_wander_point.x) < 2 && abs(monster.data.position.y - monster_wander_point.y) < 2) 
            {
                cout << "Monster wandered to the random point" << endl;
                isWanderPointGenerated = false;
                return SUCCESS; // Successfully reached the wander point
            }
        }
        cout << "Monster is wandering in the same room" << endl;
        return RUNNING; // Still moving towards the wander point
    }
};
class MoveToCenterOfRoomAction: public BehaviorTreeNode{
public:
    virtual NodeState execute(Boid &monster) override {
        action_choice = MOVE_CENTER_OF_ROOM;
        if(monsterClock.getElapsedTime().asSeconds()-monster_startTime > MAX_MOVE_CENTER_TIME)
        {
            cout<<"MoveToCenter action failed due to timeout"<<endl;
            monsterClock.restart();
            monster_startTime = 0;
            monsterPrevTime = monster_startTime;
            return FAILURE;
        }
        else if(abs(monster.data.position.x - rooms[current_monster_room].room_center.x) < 2 && abs(monster.data.position.y - rooms[current_monster_room].room_center.y) < 2)
        {
            cout<<"Monster moved to the center of the room"<<endl;
            return SUCCESS;
        }
        else
        {
            cout<<"Monster moving to the center of the room"<<endl;
            return RUNNING;
        }
    }
};

class FollowAction : public BehaviorTreeNode {
public:
    virtual NodeState execute(Boid &monster) override {
        // Implement logic for following action
        if(monsterClock.getElapsedTime().asSeconds()-monster_startTime > MAX_PATH_FIND_TIME)
        {
            cout<<"PathFinding failed due to timeout"<<endl;
            monsterClock.restart();
            monster_startTime = 0;
            monsterPrevTime = monster_startTime;
            return FAILURE;
        }
        action_choice = PATH_FIND;
        std::cout<<"Monster is following the character" <<endl;
        return RUNNING; // For simplicity, always return success
    }
};

class DanceAction : public BehaviorTreeNode {
public:
    virtual NodeState execute(Boid &monster) override{
        // Implement logic for dancing action
        if(!isDancePointGenerated)
        {
            int next_room = scripted_path.back();
            scripted_path.pop_back();
            scripted_path.insert(scripted_path.begin(), next_room);
            monster_wander_point = rooms[next_room].get_wander_point();
            isDancePointGenerated = true;
        }
        if(monsterClock.getElapsedTime().asSeconds()-monster_startTime > MAX_DANCE_TIME)
        {
            cout<<"DanceAction failed due to timeout"<<endl;
            monsterClock.restart();
            monster_startTime = 0;
            monsterPrevTime = monster_startTime;
            isDancePointGenerated = false;
            return FAILURE;
        }
        action_choice = DANCE;
        std::cout << "Monster is dancing, wandering to different rooms" << std::endl;
        return RUNNING;
    }
};
BehaviorTreeNode* constructBehaviorTree()
{
    // Create action nodes
    BehaviorTreeNode* wander = new WanderAction();
    BehaviorTreeNode* moveToCenter = new MoveToCenterOfRoomAction();
    BehaviorTreeNode* follow = new FollowAction();
    BehaviorTreeNode* dance = new DanceAction();

    // Create decorator nodes for conditions
    BehaviorTreeNode* wallHitCheck = new WallHitNode(wander); // Decorate wander action with wall hit check
    BehaviorTreeNode* chaseCheck = new DecoratorNode(follow); // Decorate follow action with chase condition check

    // Create sequence nodes
    std::vector<BehaviorTreeNode*> wanderSeqNodes = { wallHitCheck, moveToCenter };
    BehaviorTreeNode* wanderSeq = new SequenceNode(wanderSeqNodes);

    std::vector<BehaviorTreeNode*> chaseSeqNodes = { new IsSpriteinNeighbouringOrSameRoom(), chaseCheck };
    BehaviorTreeNode* chaseSeq = new SequenceNode(chaseSeqNodes);

    std::vector<BehaviorTreeNode*> danceSeqNodes = { dance };
    BehaviorTreeNode* danceSeq = new SequenceNode(danceSeqNodes);

    // Create selector node to choose between sequences
    std::vector<BehaviorTreeNode*> selNode = {chaseSeq, danceSeq, wanderSeq};
    BehaviorTreeNode* selector = new SelectorNode(selNode);

    return selector;
}

int main()
{
    // Creating the window and intial variables
    sf::Texture *boidTexture = new sf::Texture();
    boidTexture->loadFromFile("./imgs/boid.png");
    sf::Texture *monsterTexture = new sf::Texture();
    monsterTexture->loadFromFile("./imgs/monster_1.png");

    // Sprite
    Boid sprite(boidTexture, charData,false);

    // Monster
    Boid monster(monsterTexture, monsterData, true);

    wander_point = sprite.data.position;
    monster_wander_point = monster.data.position;

    NodeState treeState = RUNNING; // Initial state of the behavior tree

    g.add_vertex(0, {{2, 1}});
    g.add_vertex(1, {{2, 1}});
    g.add_vertex(2, {{0, 1}, {1, 1}, {3, 1}, {5, 1}});
    g.add_vertex(3, {{2, 1}});
    g.add_vertex(4, {{5, 1}});
    g.add_vertex(5, {{2,1},{4,1},{6, 1}});
    g.add_vertex(6, {{5, 1}});

    // Seed the random number generator with the current time
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    
    // Decision Tree for character
    struct DecisionTree* root = new DecisionTree(0);
    root->left = new DecisionTree(1);
    root->right = new DecisionTree(2);
    root->right->left = new DecisionTree(3);
    root->right->right = new DecisionTree(4);
    root->right->right->left = new DecisionTree(5);
    root->right->right->right = new DecisionTree(6);

    // Behavior Tree
    BehaviorTreeNode* base = constructBehaviorTree();

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
        if(abs(monster.data.position.x - sprite.data.position.x ) < 15 && abs(monster.data.position.y - sprite.data.position.y) < 15)
        {    
            std::cout<<"Monster catched the character, Reset"<<endl;
            monsterClock.restart();
            spriteClock.restart();
            monster_startTime = monsterClock.getElapsedTime().asSeconds();
            monsterPrevTime = monster_startTime;
            startTime = spriteClock.getElapsedTime().asSeconds();
            prevTime = startTime;
            mouse_final = sf::Vector2i(0,0);
            monster_mouse_final = sf::Vector2i(360 ,300);
            sprite.data.position = sf::Vector2f(50.f, 50.f);
            monster.data.position = sf::Vector2f(360.f, 300.f);
            sprite.data.velocity = sf::Vector2f(0.0f, 0.0f);
            monster.data.velocity = sf::Vector2f(0.0f,0.0f);
            path.clear();
            monster_path.clear();

            same_room = true;
            same_monster_room = true;

            current_room = 0;
            current_monster_room = 5;
            clicked_room = 0;
            clicked_monster_room = 5;
            ending_room = 0;
            ending_monster_room = 5;

            breadcrumbs.clear();
            monster_breadcrumbs.clear();

            crumb_count = 0;
            crumb_count_monster = 0;

            action_choice = START;
            
            latest_crumb_pos = sf::Vector2f (50.f, 50.f);
            
            latest_crumb_pos_monster = sf::Vector2f (360.f, 300.f);

            scripted_path = {2, 0, 2, 3, 2, 1, 2, 5, 6, 5, 4};

            // Add a delay before resetting
            sf::sleep(sf::seconds(0.4)); // 0.4 seconds delay

            wander_point = sprite.data.position;
            monster_wander_point = monster.data.position;
        }

        DecisionTree* tree_node = root;
        
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
                if(spriteClock.getElapsedTime().asSeconds()-startTime > 10.0)
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

            float currentTime = spriteClock.getElapsedTime().asSeconds();
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

                    float currentTime = spriteClock.getElapsedTime().asSeconds();
                    float elapsedSeconds = currentTime - prevTime;
                    // 
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

                float currentTime = spriteClock.getElapsedTime().asSeconds();
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
                crumb c(crumb_count,&window,false);
                c.drop(latest_crumb_pos); 
                latest_crumb_pos = sprite.s.getPosition();

                if (crumb_count > CRUMB_SIZE ){ breadcrumbs.pop_front(); breadcrumbs.push_back(c); }
                else breadcrumbs.push_back(c);

                crumb_count++;
            }
        }
        // behavior tree logic
        treeState = base->execute(monster);

        switch(action_choice)
        {
            case START: {
                break;
            }
            case WANDER: {
                // wander within same room

                pathFindMonster(g, monster, monster_wander_point);
                break;
            }
            case MOVE_CENTER_OF_ROOM: {
                // move the monster ro the center of the room
                monster_wander_point = rooms[current_monster_room].room_center;
                pathFindMonster(g, monster, monster_wander_point);
                break;
            }
            case PATH_FIND: {
                // pathFind the character
                monster_wander_point = sprite.data.position;
                pathFindMonster(g, monster, monster_wander_point);
                break;
            }
            case DANCE: {
                // make monstor follow a scripted path
                if(isDancePointGenerated)
                {
                    if(abs(monster.data.position.x - monster_wander_point.x) < 2 && abs(monster.data.position.y - monster_wander_point.y) < 2)
                    {
                        int next_room = scripted_path.back();
                        scripted_path.pop_back();
                        scripted_path.insert(scripted_path.begin(), next_room);
                        monster_wander_point = rooms[next_room].get_wander_point();
                    }
                    
                }
                pathFindMonster(g, monster, monster_wander_point);
                break;
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

        // Drawing the monster
        window.draw(monster.s);

        // Draw breadcrumbs
        for(auto x : breadcrumbs) x.draw();

        // Draw monster breadcrumbs
        for(auto x : monster_breadcrumbs) x.draw();

        // Displaying the window
        window.display();
    }
    return 0;
}
