#pragma once

#include "Renderer/RenderSystemV2.hpp"
#include "Core/GeometryLoaderSystem.hpp"
#include "Core/CameraSystem.hpp"
#include "Core/Scene.hpp"
#include "window.hpp"

namespace app {
    class Application {
        public:
            Application() = default;
            ~Application();
        
            bool Initialize();

            static void Shutdown();
            void Update() const;

    private:
        void InitializeInternal();
        static void CreateDefaultCamera(Scene* _scene);
        static void CreateDefaultLights(Scene* _scene);
        static void CreateFloorGridMesh(Scene* _scene);
        static void HandleResize(const void* callback_context, int width, int height);
        static void HandleDragAndDrop(const void* callback_context, int count, const char** paths);
        
        std::unique_ptr<Window> _mainWindow;
        std::unique_ptr<RenderSystemV2> _renderSystem;
        std::unique_ptr<CameraSystem> _cameraSystem;
        std::unique_ptr<GeometryLoaderSystem> _geometryLoaderSystem;
        std::unique_ptr<Scene> _scene;
    };
}
