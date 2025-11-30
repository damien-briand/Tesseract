cmake_minimum_required(VERSION 3.28.3)
project(vulkan CXX)

# C++17 minimum requis pour std::optional
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# Liste des fichiers sources - Core
set(CORE_SOURCES
    src/core/Window.cpp
    src/core/Instance.cpp
    src/core/Device.cpp
    src/core/SwapChain.cpp
)

# Liste des fichiers sources - Rendering
set(RENDERING_SOURCES
    src/rendering/RenderPass.cpp
    src/rendering/Pipeline.cpp
    src/rendering/CommandBuffer.cpp
    src/rendering/Renderer.cpp
)

# Liste des fichiers sources - Utils
set(UTILS_SOURCES
    src/utils/FileUtils.cpp
    src/utils/ShaderLoader.cpp
    src/utils/VulkanUtils.cpp
)

# Liste des fichiers headers (optionnel, pour IDEs)
set(CORE_HEADERS
    src/core/Window.hpp
    src/core/Instance.hpp
    src/core/Device.hpp
    src/core/SwapChain.hpp
)

set(RENDERING_HEADERS
    src/rendering/RenderPass.hpp
    src/rendering/Pipeline.hpp
    src/rendering/CommandBuffer.hpp
    src/rendering/Renderer.hpp
)

set(UTILS_HEADERS
    src/utils/FileUtils.hpp
    src/utils/ShaderLoader.hpp
    src/utils/VulkanUtils.hpp
)

# Combiner toutes les sources
set(SOURCES
    src/main.cpp
    ${CORE_SOURCES}
    ${RENDERING_SOURCES}
    ${UTILS_SOURCES}
)

set(HEADERS
    ${CORE_HEADERS}
    ${RENDERING_HEADERS}
    ${UTILS_HEADERS}
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Ajout du répertoire src/ pour les includes
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(${PROJECT_NAME} 
    Vulkan::Vulkan 
    glfw
)

# Optionnel : Copier les shaders compilés dans le build directory
# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#     ${CMAKE_SOURCE_DIR}/src/shaders/spv
#     ${CMAKE_BINARY_DIR}/shaders/spv
# )

# Optionnel : Afficher un message de succès
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Build completed successfully!"
)