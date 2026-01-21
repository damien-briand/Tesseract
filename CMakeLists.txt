cmake_minimum_required(VERSION 3.28.3)
project(vulkan CXX)

# C++17 minimum requis pour std::optional
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Flags d'optimisation
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # Mode Release : optimisations maximales
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")
    message(STATUS "Build type: Release (optimisations -O3 activées)")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Mode Debug : pas d'optimisation, symboles de debug
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wall -Wextra")
    message(STATUS "Build type: Debug (pas d'optimisation)")
else()
    # Par défaut : Release
    set(CMAKE_BUILD_TYPE "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")
    message(STATUS "Build type non spécifié, utilisation de Release par défaut")
endif()

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# Liste des fichiers sources - Core
set(CORE_SOURCES
    src/core/Window.cpp
    src/core/Instance.cpp
    src/core/Device.cpp
    src/core/SwapChain.cpp
    src/core/Camera.cpp
    src/core/ThreadPool.cpp
    src/scene/GameObject.cpp
    src/scene/Scene.cpp
    src/scene/Light.cpp
    src/scene/LightManager.cpp 
)

# Liste des fichiers sources - Rendering
set(RENDERING_SOURCES
    src/rendering/RenderPass.cpp
    src/rendering/Pipeline.cpp
    src/rendering/Renderer.cpp
    src/rendering/CommandBuffer.cpp
    src/rendering/AdvancedCommandRecorder.cpp
    src/rendering/DepthBuffer.cpp
    src/rendering/MSAABuffer.cpp
)

# Liste des fichiers sources - Rendering/Buffer
set(RENDERING_BUFFER_SOURCES
    src/rendering/Buffer/Buffer.cpp
    src/rendering/Buffer/VertexBuffer.cpp
    src/rendering/Buffer/IndexBuffer.cpp
    src/rendering/Buffer/UniformBuffer.cpp
)

# Liste des fichiers sources - Rendering/Descriptor
set(RENDERING_DESCRIPTOR_SOURCES
    src/rendering/Descriptor/DescriptorSetLayout.cpp
    src/rendering/Descriptor/DescriptorPool.cpp
)

# Liste des fichiers sources - Utils
set(UTILS_SOURCES
    src/utils/FileUtils.cpp
    src/utils/ShaderLoader.cpp
    src/utils/VulkanUtils.cpp
    src/utils/ModelLoader.cpp
    src/utils/PrimitiveGenerator.cpp
)

# Liste des fichiers sources - ImGui
set(IMGUI_SOURCES
    src/utils/external/imgui/imgui.cpp
    src/utils/external/imgui/imgui_demo.cpp
    src/utils/external/imgui/imgui_draw.cpp
    src/utils/external/imgui/imgui_tables.cpp
    src/utils/external/imgui/imgui_widgets.cpp
    src/utils/external/imgui/backends/imgui_impl_glfw.cpp
    src/utils/external/imgui/backends/imgui_impl_vulkan.cpp
)

set(UI_SOURCES
    src/ui/ImGuiManager.cpp
    src/ui/StatsWindow.cpp
    src/ui/SceneInspector.cpp
)

# Liste des fichiers headers (optionnel, pour IDEs)
set(CORE_HEADERS
    src/core/Window.hpp
    src/core/Instance.hpp
    src/core/Device.hpp
    src/core/SwapChain.hpp
    src/core/Camera.hpp
    src/core/ThreadPool.hpp
    src/scene/GameObject.hpp
    src/scene/Scene.hpp
    src/scene/Light.hpp
    src/scene/LightManager.hpp
)

set(RENDERING_HEADERS
    src/rendering/RenderPass.hpp
    src/rendering/Pipeline.hpp
    src/rendering/Renderer.hpp
    src/rendering/Vertex.hpp
    src/rendering/CommandBuffer.hpp
     src/rendering/AdvancedCommandRecorder.hpp
    src/rendering/DepthBuffer.hpp
    src/rendering/MSAABuffer.hpp
)

set(RENDERING_BUFFER_HEADERS
    src/rendering/Buffer/Buffer.hpp
    src/rendering/Buffer/VertexBuffer.hpp
    src/rendering/Buffer/IndexBuffer.hpp
    src/rendering/Buffer/UniformBuffer.hpp
    src/rendering/Buffer/UniformBufferObject.hpp
)

set(RENDERING_DESCRIPTOR_HEADERS
    src/rendering/Descriptor/DescriptorSetLayout.hpp
    src/rendering/Descriptor/DescriptorPool.hpp
)

set(UTILS_HEADERS
    src/utils/FileUtils.hpp
    src/utils/ShaderLoader.hpp
    src/utils/VulkanUtils.hpp
    src/utils/ModelLoader.hpp
    src/utils/PrimitiveGenerator.hpp
)

set(UI_HEADERS
    src/ui/ImGuiWindow.hpp
    src/ui/ImGuiManager.hpp
    src/ui/StatsWindow.hpp
    src/ui/SceneInspector.hpp
)

# ImGui headers
set(IMGUI_HEADERS
    src/utils/external/imgui/imgui.h
    src/utils/external/imgui/imconfig.h
    src/utils/external/imgui/backends/imgui_impl_glfw.h
    src/utils/external/imgui/backends/imgui_impl_vulkan.h
)

set(RENDERING_TEXTURE_SOURCES
    src/rendering/Texture/Texture.cpp
    src/rendering/Material/Material.cpp
)

set(RENDERING_TEXTURE_HEADERS
    src/rendering/Texture/Texture.hpp
    src/rendering/Material/Material.hpp
)

# Dans la section SOURCES
set(SOURCES
    src/main.cpp
    ${CORE_SOURCES}
    ${RENDERING_SOURCES}
    ${RENDERING_BUFFER_SOURCES}
    ${RENDERING_DESCRIPTOR_SOURCES}
    ${RENDERING_TEXTURE_SOURCES} 
    ${UTILS_SOURCES}
    ${IMGUI_SOURCES}
    ${UI_SOURCES}
)

set(HEADERS
    ${CORE_HEADERS}
    ${RENDERING_HEADERS}
    ${RENDERING_BUFFER_HEADERS}
    ${RENDERING_DESCRIPTOR_HEADERS}
    ${RENDERING_TEXTURE_HEADERS}
    ${UTILS_HEADERS}
    ${IMGUI_HEADERS}
    ${UI_HEADERS}
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Ajout du répertoire src/ pour les includes
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(${PROJECT_NAME} 
    Vulkan::Vulkan 
    glfw
)

# Afficher les flags de compilation
message(STATUS "Flags de compilation C++ : ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")

# Optionnel : Afficher un message de succès
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "✓ Build completed successfully!"
)