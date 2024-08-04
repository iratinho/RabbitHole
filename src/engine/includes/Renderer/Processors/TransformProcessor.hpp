#pragma once
#include "Core/Scene.hpp"
#include "Core/Utils.hpp"
#include "Components/TransformComponent.hpp"

class TransformProcessor {
public:
    static void Process(Scene* scene) {
        // Pre-fetch all transforms for cache coherency
        auto view = scene->GetRegistry().view<TransformComponent>();
        // We cant use this type of map, consider lvm maps since they are stack allocated
        std::unordered_map<uint32_t, TransformComponent*> transforms;
        std::vector<uint32_t> rootTransforms;
        uint32_t rootTransform = -1;
        for(auto& entity : view) {
            TransformComponent* component = &view.get<TransformComponent>(entity);
            transforms[component->_id] = component;
            
            component->_computedMatrix.reset();

            if(component->_isRootTransform) {
                rootTransforms.push_back(component->_id);
            }
        }
                
        const auto buildMatrix = [](TransformComponent* component) -> glm::mat4  {
            glm::mat4 matrix = glm::identity<glm::mat4>();
             // Translation
            matrix = glm::translate(glm::mat4(1.0f), component->m_Position);
            // Rotation
            matrix = matrix * glm::mat4_cast(component->m_Rotation);
            // Scale
            matrix = glm::scale(matrix, component->m_Scale);
            
            return matrix;
        };
        
        if(!rootTransforms.empty()) {
            const std::function<void(uint32_t)> processTransform = [&buildMatrix, &processTransform, &transforms](uint32_t id) {
                TransformComponent* component = transforms[id];
                glm::mat4 matrix = glm::identity<glm::mat4>();
                
                if(!component->_computedMatrix.has_value()) {
                    matrix = buildMatrix(component);
                    component->_computedMatrix = component->_matrix * matrix;
                }
                
                for(uint32_t child : component->_childs) {
                    TransformComponent* childComponent = transforms[child];
                    matrix = buildMatrix(childComponent);
                    childComponent->_computedMatrix = component->_computedMatrix.value() * childComponent->_matrix * matrix;
                    
                    processTransform(childComponent->_id);
                    int i = 0;
                }
            };
            
            std::for_each(rootTransforms.begin(), rootTransforms.end(), processTransform);
        }
    };
};
