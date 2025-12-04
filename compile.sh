#!/bin/bash

# Script de compilation pour Tesseract
# Usage: ./compile.sh [ACTION] [BUILD_TYPE] [clean]
#   ACTION: shaders|code|all (défaut: all)
#   BUILD_TYPE: release|debug (défaut: release)
#   clean: Efface le dossier build avant de compiler (optionnel)
#
# Exemples:
#   ./compile.sh                    # Compile tout en Release
#   ./compile.sh all debug          # Compile tout en Debug
#   ./compile.sh code release       # Compile uniquement le code en Release
#   ./compile.sh code debug clean   # Nettoie puis compile le code en Debug
#   ./compile.sh all release clean  # Nettoie puis compile tout en Release
#   ./compile.sh shaders            # Compile uniquement les shaders

set -e  # Arrête le script en cas d'erreur

# Couleurs pour l'affichage
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Fonction pour compiler les shaders
compile_shaders() {
    echo -e "${BLUE}=== Compilation des shaders ===${NC}"
    mkdir -p src/shaders/spv
    /usr/local/bin/glslc src/shaders/shader.vert -o src/shaders/spv/vert.spv
    /usr/local/bin/glslc src/shaders/shader.frag -o src/shaders/spv/frag.spv
    echo -e "${GREEN}✓ Shaders compilés avec succès${NC}"
}

# Fonction pour nettoyer le dossier build
clean_build() {
    echo -e "${YELLOW}=== Nettoyage du dossier build ===${NC}"
    if [ -d "build" ]; then
        rm -rf build/*
        echo -e "${GREEN}✓ Dossier build nettoyé${NC}"
    else
        echo -e "${BLUE}ℹ Dossier build n'existe pas encore${NC}"
    fi
}

# Fonction pour compiler le code C++
compile_code() {
    local build_type=$1
    echo -e "${BLUE}=== Compilation du code C++ (${build_type}) ===${NC}"
    cmake -B build -DCMAKE_BUILD_TYPE=${build_type}
    cmake --build build/ -j$(nproc)
    echo -e "${GREEN}✓ Code compilé avec succès en ${build_type}${NC}"
}

# Déterminer l'action et le type de build
ACTION="${1:-all}"
BUILD_TYPE_RAW="${2:-release}"
CLEAN_FLAG="${3:-}"

# Vérifier si 'clean' est demandé (peut être en 2ème ou 3ème position)
SHOULD_CLEAN=false
if [ "$BUILD_TYPE_RAW" == "clean" ]; then
    SHOULD_CLEAN=true
    BUILD_TYPE_RAW="release"  # Utiliser release par défaut si clean est en 2ème position
elif [ "$CLEAN_FLAG" == "clean" ]; then
    SHOULD_CLEAN=true
fi

# Nettoyer si demandé
if [ "$SHOULD_CLEAN" == true ]; then
    clean_build
fi

# Convertir le type de build en format CMake (Release/Debug)
case "$BUILD_TYPE_RAW" in
    release|Release|RELEASE)
        BUILD_TYPE="Release"
        ;;
    debug|Debug|DEBUG)
        BUILD_TYPE="Debug"
        ;;
    *)
        echo -e "${RED}Type de build invalide: $BUILD_TYPE_RAW${NC}"
        echo -e "${YELLOW}Utilisez 'release' ou 'debug'${NC}"
        exit 1
        ;;
esac

# Exécuter l'action demandée
case "$ACTION" in
    shaders)
        compile_shaders
        ;;
    code)
        compile_code "$BUILD_TYPE"
        ;;
    all)
        compile_shaders
        compile_code "$BUILD_TYPE"
        ;;
    *)
        echo -e "${RED}Usage: $0 [ACTION] [BUILD_TYPE] [clean]${NC}"
        echo "  ACTION: shaders|code|all (défaut: all)"
        echo "  BUILD_TYPE: release|debug (défaut: release)"
        echo "  clean: Efface le dossier build avant de compiler (optionnel)"
        echo ""
        echo "Exemples:"
        echo "  $0                    # Compile tout en Release"
        echo "  $0 all debug          # Compile tout en Debug"
        echo "  $0 code release       # Compile uniquement le code en Release"
        echo "  $0 code debug clean   # Nettoie puis compile le code en Debug"
        echo "  $0 all clean          # Nettoie puis compile tout en Release"
        echo "  $0 shaders            # Compile uniquement les shaders"
        exit 1
        ;;
esac

echo -e "${GREEN}✓ Compilation terminée !${NC}"