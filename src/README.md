# Architecture
```bash
src/
├── main.cpp
├── core/
│   ├── Window.h/cpp           # Gestion de la fenêtre GLFW
│   ├── Instance.h/cpp         # Instance Vulkan + validation layers
│   ├── Device.h/cpp           # Physical + Logical device + queues
│   └── SwapChain.h/cpp        # Swapchain + image views + framebuffers
├── rendering/
│   ├── RenderPass.h/cpp       # Render pass
│   ├── Pipeline.h/cpp         # Graphics pipeline
│   ├── CommandBuffer.h/cpp    # Command pool + buffers
│   └── Renderer.h/cpp         # Orchestration du rendu
└── utils/
    ├── ShaderLoader.h/cpp     # Chargement des shaders
    └── VulkanUtils.h/cpp      # Utilitaires divers
```

# Core 
Relations entre les classes

```bash
Window (indépendant)
   ↓
Instance (utilise Window pour extensions)
   ↓
Device (dépend de Instance + Surface)
   ↓
SwapChain (dépend de Device + Window)
```

# Rendering
Relations entre les classes

```bash
Device → RenderPass
       ↓
Device + RenderPass + SwapChain → Pipeline
       ↓
Device + Pipeline + RenderPass + SwapChain → CommandBuffer
       ↓
Tout → Renderer (orchestration)
```