#pragma once

#include <string>
#include <vector>

class FileUtils {
public:
    // Suppression du constructeur (classe statique)
    FileUtils() = delete;

    /**
     * @brief Lit un fichier binaire et retourne son contenu
     * @param filename Chemin vers le fichier
     * @return Contenu du fichier sous forme de vector<char>
     * @throws std::runtime_error si le fichier ne peut pas être ouvert
     */
    static std::vector<char> readBinaryFile(const std::string& filename);

    /**
     * @brief Lit un fichier texte et retourne son contenu
     * @param filename Chemin vers le fichier
     * @return Contenu du fichier sous forme de string
     * @throws std::runtime_error si le fichier ne peut pas être ouvert
     */
    static std::string readTextFile(const std::string& filename);

    /**
     * @brief Vérifie si un fichier existe
     * @param filename Chemin vers le fichier
     * @return true si le fichier existe
     */
    static bool fileExists(const std::string& filename);

    /**
     * @brief Retourne la taille d'un fichier en octets
     * @param filename Chemin vers le fichier
     * @return Taille du fichier
     * @throws std::runtime_error si le fichier ne peut pas être ouvert
     */
    static size_t getFileSize(const std::string& filename);
};