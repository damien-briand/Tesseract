#pragma once

#include <string>

/**
 * @brief Classe abstraite représentant une fenêtre ImGui personnalisée
 * 
 * Pour créer une nouvelle fenêtre ImGui :
 * 1. Hériter de cette classe
 * 2. Implémenter la méthode render() avec votre code ImGui
 * 3. Ajouter l'instance au ImGuiManager
 */
class ImGuiWindow
{
public:
    explicit ImGuiWindow(const std::string& name)
        : m_name(name)
        , m_visible(true)
    {
    }

    virtual ~ImGuiWindow() = default;

    /**
     * @brief Méthode à implémenter pour dessiner le contenu de la fenêtre
     * Appelée automatiquement chaque frame si la fenêtre est visible
     */
    virtual void render() = 0;

    /**
     * @brief Retourne le nom de la fenêtre
     */
    const std::string& getName() const { return m_name; }

    /**
     * @brief Vérifie si la fenêtre est visible
     */
    bool isVisible() const { return m_visible; }

    /**
     * @brief Active/désactive la visibilité de la fenêtre
     */
    void setVisible(bool visible) { m_visible = visible; }

    /**
     * @brief Toggle la visibilité
     */
    void toggleVisible() { m_visible = !m_visible; }

protected:
    std::string m_name;
    bool m_visible;
};
