# Tesseract

## Compilation Instructions

To compile the Tesseract project, use the following commands for a release build:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build/
```

For debugging builds, use the following commands:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build/
```