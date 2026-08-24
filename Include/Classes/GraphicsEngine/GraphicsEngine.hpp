#pragma once
#include <vector>
#include <stack>
using std::vector, std::stack;

#include "Classes/GraphicsEngine/Entities.hpp"

#ifndef GRAPHICS_ENGINE_CLASS_HPP
#define GRAPHICS_ENGINE_CLASS_HPP

class GraphicsEngine {
    public:
        vector<BaseEntity> Entities{};
        stack<int> FreeIds{};
        
        int AddEntity(BaseEntity Entity) {
            const int Id = !FreeIds.empty() ? FreeIds.top() : Entities.size();
            if (!FreeIds.empty()) FreeIds.pop();
            
            Entities[Id] = Entity;
            
            return Id;
        }
        
        void RemoveEntity(int Id) {
            Entities.erase(Entities.begin() + Id);
        }
        
        void Render(GLuint DrawType) {
            for (auto& Entity : Entities) {
                Entity.Render(DrawType);
            }
        }
        
        GraphicsEngine() = default;
        ~GraphicsEngine() = default;
    private:
        
};

#endif