#include "stdl.hpp"
#include <iostream>

int main(){
    auto scene = STDL::LoadFile("example.stdl");
    if(!scene){
        std::cerr<<"Failed to load STDL file\n";
        return 1;
    }

    std::cout << "=== All nodes in scene ===\n";
    for(auto& node : scene->nodes){
        std::cout << "Type: " << node->type << ", Name: " << node->name << "\n";
    }
    std::cout << "=========================\n";

    auto player = scene->getNodeByName("MyPlayer");
    std::cout<<"Player found! Type: "<<player->type<<"\n";
    int health;
    if(player->get("health",health))
        std::cout<<"Player health = "<<health<<"\n";

    std::string escaped;
    if (player->get("escaped",escaped))
    {
        std::cout << "Test escaped: " << escaped << "\n";
    }

    auto forest = scene->getNodeByName("Forest");
    if(!forest){
        std::cerr << "Forest not found!\n";
        return 1;
    }

    std::cout << "Forest found! It has " << forest->children.size() << " children\n";

    NodePtr tree = forest->getChild("Oak");
    if(!tree){
        std::cerr << "Oak not found!\n";
        return 1;
    }

    std::cout << "Oak found!\n";
    std::cout<<"Tree found! Type: "<<tree->type<<"\n";
    int height;
    if(tree->get("height",height))
        std::cout<<"Tree Height: "<<height<<"\n";

    auto enemy = std::make_shared<Node>();
    enemy->type="enemy";
    enemy->name="Orc";
    enemy->set("health",80);
    scene->addNode(enemy);

    STDL::SaveFile(scene,"out.stdl");
    std::cout<<"Saved scene to out.stdl\n";
    return 0;
}