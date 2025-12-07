#!/bin/bash

# Script de compilation pour Tesseract
# Usage: ./compile.sh [ACTION] [BUILD_TYPE] [clean] [WITH_KTX]
#   ACTION: shaders|code|all (défaut: all)
#   BUILD_TYPE: release|debug (défaut: release)
#   clean: Efface le dossier build avant de compiler (optionnel)
#   WITH_KTX: ON|OFF (défaut: ON)
#
# Exemples:
#   ./compile.sh                            # Compile tout en Release, WITH_KTX=ON
#   ./compile.sh all debug                  # Compile tout en Debug, WITH_KTX=ON
#   ./compile.sh code release clean         # Nettoie puis compile le code en Release, WITH_KTX=ON
#   ./compile.sh all release clean OFF      # Nettoie puis compile tout en Release, WITH_KTX=OFF
#   ./compile.sh shaders                    # Compile uniquement les shaders

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
# usage: compile_code <BuildType> <WITH_KTX>
compile_code() {
    local build_type=$1
    local with_ktx=${2:-ON}
    echo -e "${BLUE}=== Compilation du code C++ (${build_type}) WITH_KTX=${with_ktx} ===${NC}"
    cmake -B build -DCMAKE_BUILD_TYPE=${build_type} -DWITH_KTX=${with_ktx}
    cmake --build build/ -j$(nproc)
    echo -e "${GREEN}✓ Code compilé avec succès en ${build_type} (WITH_KTX=${with_ktx})${NC}"
}

# Déterminer l'action et le type de build
ACTION="${1:-all}"
BUILD_TYPE_RAW="${2:-release}"
CLEAN_FLAG="${3:-}"
WITH_KTX="${4:-ON}"

# Vérifier si 'clean' est demandé (peut être en 2ème ou 3ème position)
SHOULD_CLEAN=false
if [ "$BUILD_TYPE_RAW" == "clean" ]; then
    SHOULD_CLEAN=true
    BUILD_TYPE_RAW="release"  # Utiliser release par défaut si clean est en 2ème position
elif [ "$CLEAN_FLAG" == "clean" ]; then
    SHOULD_CLEAN=true
    CLEAN_FLAG=""
elif [ "$WITH_KTX" == "clean" ]; then
    # cas improbable où clean passé en 4ème position
    SHOULD_CLEAN=true
    WITH_KTX="ON"
fi

# Normalize WITH_KTX to ON or OFF
case "${WITH_KTX,,}" in
    on|yes|1)
        WITH_KTX="ON"
        ;;
    off|no|0)
        WITH_KTX="OFF"
        ;;
    *)
        # keep as provided (default ON)
        ;;
esac

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
        compile_code "$BUILD_TYPE" "$WITH_KTX"
        ;;
    all)
        compile_shaders
        compile_code "$BUILD_TYPE" "$WITH_KTX"
        ;;
    *)
        echo -e "${RED}Usage: $0 [ACTION] [BUILD_TYPE] [clean] [WITH_KTX]${NC}"
        echo "  ACTION: shaders|code|all (défaut: all)"
        echo "  BUILD_TYPE: release|debug (défaut: release)"
        echo "  clean: Efface le dossier build avant de compiler (optionnel)"
        echo "  WITH_KTX: ON|OFF (défaut: ON)"
        echo ""
        echo "Exemples:"
        echo "  $0                    # Compile tout en Release, WITH_KTX=ON"
        echo "  $0 all debug          # Compile tout en Debug, WITH_KTX=ON"
        echo "  $0 code release       # Compile uniquement le code en Release, WITH_KTX=ON"
        echo "  $0 code debug clean   # Nettoie puis compile le code en Debug, WITH_KTX=ON"
        echo "  $0 all release clean OFF  # Nettoie puis compile tout en Release, WITH_KTX=OFF"
        exit 1
        ;;
esac

echo -e "${GREEN}✓ Compilation terminée !${NC}"