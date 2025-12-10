cmake_minimum_required(VERSION 3.10)
project(vulkan CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Build flags
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -flto")
    message(STATUS "Build type: Release, flags: ${CMAKE_CXX_FLAGS_RELEASE}")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wall -Wextra")
    message(STATUS "Build type: Debug")
else()
    message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
endif()

# Options
option(WITH_KTX "Enable KTX2 support (requires libktx installed)" ON)

# Find dependencies
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# Try to find libktx (optional)
if(WITH_KTX)
  find_package(ktx QUIET)
  if(ktx_FOUND)
    message(STATUS "libktx found: enabling KTX2 support")
    add_compile_definitions(WITH_KTX=1)
  else()
    message(STATUS "libktx not found: KTX2 support disabled")
  endif()
endif()

# Add include for bundled headers (tinygltf, stb, etc.) if you keep them in src/utils/external
set(EXTERNAL_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/external)

# Source lists
set(CORE_SOURCES
    src/core/Window.cpp
    src/core/Instance.cpp
    src/core/Device.cpp
    src/core/SwapChain.cpp
    src/core/Camera.cpp
    src/scene/GameObject.cpp
    src/scene/Scene.cpp
)

set(RENDERING_SOURCES
    src/rendering/RenderPass.cpp
    src/rendering/Pipeline.cpp
    src/rendering/Renderer.cpp
    src/rendering/CommandBuffer.cpp
    src/rendering/DepthBuffer.cpp
    src/rendering/MSAABuffer.cpp
    src/rendering/Texture/Texture.cpp
)

set(RENDERING_BUFFER_SOURCES
    src/rendering/Buffer/Buffer.cpp
    src/rendering/Buffer/VertexBuffer.cpp
    src/rendering/Buffer/IndexBuffer.cpp
    src/rendering/Buffer/UniformBuffer.cpp
)

set(RENDERING_DESCRIPTOR_SOURCES
    src/rendering/Descriptor/DescriptorSetLayout.cpp
    src/rendering/Descriptor/DescriptorPool.cpp
)

set(UTILS_SOURCES
    src/utils/FileUtils.cpp
    src/utils/ShaderLoader.cpp
    src/utils/VulkanUtils.cpp
    src/utils/ModelLoader.cpp
    src/utils/Asset/AssetLoader.cpp
    src/utils/Asset/GltfLoader.cpp
    src/utils/Asset/TextureLoader.cpp
)

set(SOURCES
    src/main.cpp
    ${CORE_SOURCES}
    ${RENDERING_SOURCES}
    ${RENDERING_BUFFER_SOURCES}
    ${RENDERING_DESCRIPTOR_SOURCES}
    ${UTILS_SOURCES}
)

set(HEADERS
    src/core/Window.hpp
    src/core/Instance.hpp
    src/core/Device.hpp
    src/core/SwapChain.hpp
    src/core/Camera.hpp
    src/scene/GameObject.hpp
    src/scene/Scene.hpp
)

    src/rendering/RenderPass.hpp
    src/rendering/Pipeline.hpp
    src/rendering/Renderer.hpp
    src/rendering/Vertex.hpp
    src/rendering/CommandBuffer.hpp
    src/rendering/DepthBuffer.hpp
    src/rendering/MSAABuffer.hpp
    src/rendering/Texture/Texture.hpp

    src/rendering/Buffer/Buffer.hpp
    src/rendering/Buffer/VertexBuffer.hpp
    src/rendering/Buffer/IndexBuffer.hpp
    src/rendering/Buffer/UniformBuffer.hpp

    src/rendering/Descriptor/DescriptorSetLayout.hpp
    src/rendering/Descriptor/DescriptorPool.hpp

    src/utils/FileUtils.hpp
    src/utils/ShaderLoader.hpp
    src/utils/VulkanUtils.hpp
    src/utils/ModelLoader.hpp
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
)

set(HEADERS
    ${CORE_HEADERS}
    ${RENDERING_HEADERS}
    ${RENDERING_BUFFER_HEADERS}
    ${RENDERING_DESCRIPTOR_HEADERS}
    ${RENDERING_TEXTURE_HEADERS}
    ${UTILS_HEADERS}
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include dirs
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${EXTERNAL_INCLUDE})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan glfw glm::glm)

# Link libktx if available
if(WITH_KTX AND ktx_FOUND)
  target_link_libraries(${PROJECT_NAME} PRIVATE ktx::ktx)
endif()

# If tinygltf is bundled, its header is in external; no link required (header-only)
# If you got tinygltf via package manager, link if necessary.

# Post build message
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "✓ Build completed successfully!"
)