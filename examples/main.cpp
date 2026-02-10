#include "stdl.hpp"
#include <iostream>

int main(){
    auto scene = STDL::LoadFile("C:\\Users\\Humoy\\Projects\\STDL\\STDL\\examples\\example.stdl");
    if(!scene){
        std::cerr << "Failed to load STDL file\n";
        return 1;
    }

    auto player = scene->getNodeByName("MyPlayer");
    if(player){
        int health;
        if(player->get("health", health))
            std::cout << "Player health = " << health << "\n";
    }

    auto enemy = std::make_shared<Node>();
    enemy->type = "enemy";
    enemy->name = "Orc";
    enemy->set("health", 80);
    scene->addNode(enemy);

    STDL::SaveFile(scene, "out.stdl");

    std::cout << "Saved scene to out.stdl\n";
    return 0;
}