#pragma once
#include <vector>
#include <memory>
#include <stack>
using std::vector, std::stack, std::unique_ptr;

#include "Classes/GraphicsEngine/Entities.hpp"

#ifndef GRAPHICS_ENGINE_CLASS_HPP
#define GRAPHICS_ENGINE_CLASS_HPP

class GraphicsEngine {
    public:
        vector<unique_ptr<BaseEntity>> Entities{};
        stack<int> FreeIds{};
        
        int AddEntity(unique_ptr<BaseEntity> Entity) {
            const int Id = !FreeIds.empty() ? FreeIds.top() : Entities.size();
            
            if (!FreeIds.empty()) {
                FreeIds.pop();
                Entities[Id] = std::move(Entity);
            } else {
                Entities.push_back(std::move(Entity));
            }
            
            return Id;
        }
        
        void RemoveEntity(int Id) {
            Entities[Id].reset();
            FreeIds.push(Id);
        }
        
        void Render(GLuint DrawType) {
            for (auto &Entity : Entities) {
                if (Entity) Entity->Render(DrawType);
            }
        }
        
        GraphicsEngine() = default;
        ~GraphicsEngine() = default;
    private:
        
};

#endif