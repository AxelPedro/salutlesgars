#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <cassert>

// ---------- Tuile ----------
struct Tuile {
    std::string id;
    int minPlayers;
};

// ---------- Tas ----------
class Tas {
private:
    size_t nb_tuiles;
    std::vector<const Tuile*> tuiles;
public:
    explicit Tas(int nbjoueurs);
    const Tuile* getTuilesTas(size_t i) const { return tuiles.at(i); }
    size_t getNbTuiles() const { return nb_tuiles; }
    ~Tas();
    void pushTuile(const Tuile* t);
};

// ---------- Pioche ----------
class Pioche {
private:
    std::vector<const Tas*> tas;
    std::vector<size_t> curseur;
public:
    Pioche() = default;
    ~Pioche() = default;
    void ajouterTas(const Tas* t);
    bool estVide() const;
    const Tuile* piocherDepuisTas(size_t i);
    int prendreIndiceTasNonVide() const;
};

// ---------- Table (chantier) ----------
class Table {
private:
    size_t capacite;
    std::vector<const Tuile*> visibles;
    std::vector<const Tuile*> reservoir;
public:
    Table(size_t nb_joueurs, std::vector<const Tuile*> res);
    size_t taille() const { return visibles.size(); }
    bool estVide() const { return visibles.empty(); }
    size_t cout(size_t i) const { return i; }
    void print() const;
    const Tuile* prendre(size_t i);
    void recharger(Pioche& p);
private:
    void remplirDepuisReservoir();
    void remplirDepuisReservoirOuTas(Pioche& p);
    void regleUneTuileRestante(Pioche& p);
};

// ---------- Build & Simulation ----------
struct Build {
    std::vector<Tuile> pool;                // propriétaire des tuiles
    std::vector<Tas>   tas;                 // 11 tas
    Pioche             pioche;              // vue const* sur tas
    std::vector<const Tuile*> reservoir;    // reste pour le chantier initial
};

// crée 61 tuiles, filtre selon nb joueurs, mélange (rand/srand), répartit 11 tas
Build construire(int nbJoueurs);

// petite démo terminal : on prend toujours la [0] pour voir le rechargement
void simuler(int nbJoueurs);
