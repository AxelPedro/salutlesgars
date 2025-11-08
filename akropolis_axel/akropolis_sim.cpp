#include "akropolis_sim.h"
#include <random>
#include <algorithm>
#include <cstdio>

/*==========================  Tas  ==========================*/

Tas::Tas(int nbjoueurs) : nb_tuiles(static_cast<size_t>(nbjoueurs + 1)) {
    tuiles.reserve(nb_tuiles);
}
Tas::~Tas() {} // pas de delete 

void Tas::pushTuile(const Tuile* t) {
    if (tuiles.size() < nb_tuiles) tuiles.push_back(t);
}

/*=========================  Pioche  ========================*/

void Pioche::ajouterTas(const Tas* t) {
    tas.push_back(t);
    curseur.push_back(0);
}

bool Pioche::estVide() const {
    for (size_t i = 0; i < tas.size(); ++i)
        if (curseur[i] < tas[i]->getNbTuiles()) return false;
    return true;
}

const Tuile* Pioche::piocherDepuisTas(size_t i) {
    if (i >= tas.size()) return nullptr;
    const Tas* t = tas[i];
    size_t& c = curseur[i];
    if (c >= t->getNbTuiles()) return nullptr;
    const Tuile* tu = t->getTuilesTas(c);
    ++c; // on “consomme” logiquement 1 tuile de ce tas
    return tu;
}

int Pioche::prendreIndiceTasNonVide() const {
    for (size_t i = 0; i < tas.size(); ++i)
        if (curseur[i] < tas[i]->getNbTuiles()) return static_cast<int>(i);
    return -1;
}

/*==========================  Table  ========================*/

Table::Table(size_t nb_joueurs, std::vector<const Tuile*> res)
    : capacite(nb_joueurs + 2), visibles(), reservoir(std::move(res)) {
    remplirDepuisReservoir(); // initialisation du chantier
}

void Table::remplirDepuisReservoir() {
    while (visibles.size() < capacite && !reservoir.empty()) {
        visibles.push_back(reservoir.front());
        reservoir.erase(reservoir.begin()); // équivalent pop_front() (sans deque)
    }
}

void Table::remplirDepuisReservoirOuTas(Pioche& p) {
    // 1) d'abord le réservoir
    remplirDepuisReservoir();
    // 2) puis des tas si nécessaire
    while (visibles.size() < capacite) {
        int idx = p.prendreIndiceTasNonVide();
        if (idx < 0) break;
        while (visibles.size() < capacite) {
            const Tuile* t = p.piocherDepuisTas(static_cast<size_t>(idx));
            if (!t) break;
            visibles.push_back(t);
        }
    }
}

void Table::regleUneTuileRestante(Pioche& p) {
    if (visibles.size() != 1) return;
    // déplacer explicitement la seule tuile en position 0 (déjà le cas)
    const Tuile* seule = visibles.front();
    visibles.erase(visibles.begin());
    visibles.insert(visibles.begin(), seule);
    // compléter
    remplirDepuisReservoirOuTas(p);
}

const Tuile* Table::prendre(size_t i) {
    if (i >= visibles.size()) return nullptr;
    const Tuile* t = visibles[i];
    visibles.erase(visibles.begin() + static_cast<long>(i)); // décale à gauche
    return t;
}

void Table::recharger(Pioche& p) {
    if (visibles.empty()) {
        remplirDepuisReservoirOuTas(p);
    }
    else if (visibles.size() == 1) {
        regleUneTuileRestante(p);
    }
}

void Table::print() const {
    std::cout << "\nChantier (" << visibles.size() << "/" << capacite << ") [cout=index]\n";
    for (size_t i = 0; i < visibles.size(); ++i) {
        auto* t = visibles[i];
        std::cout << "  [" << i << "] " << t->id
            << "  (minJ=" << t->minPlayers << ", cout=" << i << ")\n";
    }
    std::cout << "Reservoir restant: " << reservoir.size() << "\n";
}


static std::vector<Tuile> fabriquer61() {
    std::vector<Tuile> v; v.reserve(61);
    // 37 x 2+
    for (int i = 1; i <= 37; ++i) {
        char id[16]; std::snprintf(id, 16, "T2_%02d", i);
        v.push_back({ id, 2 });
    }
    // 12 x 3+
    for (int i = 1; i <= 12; ++i) {
        char id[16]; std::snprintf(id, 16, "T3_%02d", i);
        v.push_back({ id, 3 });
    }
    // 12 x 4+
    for (int i = 1; i <= 12; ++i) {
        char id[16]; std::snprintf(id, 16, "T4_%02d", i);
        v.push_back({ id, 4 });
    }
    return v;
}

/* ---------- construire : version simple avec rand() ---------- */
Build construire(int nbJoueurs) {
    assert(nbJoueurs >= 2 && nbJoueurs <= 4);
    const size_t NB_TAS = 11;
    const size_t S = static_cast<size_t>(nbJoueurs + 1); // tuiles par tas

    Build b{};
    b.pool = fabriquer61();

    // 1) filtrer selon minPlayers
    std::vector<const Tuile*> eligibles;
    eligibles.reserve(b.pool.size());
    for (auto& t : b.pool)
        if (t.minPlayers <= nbJoueurs) eligibles.push_back(&t);

    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    for (size_t i = 0; i < eligibles.size(); ++i) {
        size_t j = static_cast<size_t>(std::rand()) % eligibles.size();
        std::swap(eligibles[i], eligibles[j]);
    }

    // 3) 11 tas de S tuiles
    b.tas.reserve(NB_TAS);
    auto it = eligibles.begin();
    auto end = eligibles.end();
    for (size_t k = 0; k < NB_TAS; ++k) {
        Tas ts(nbJoueurs);
        for (size_t j = 0; j < S && it != end; ++j, ++it)
            ts.pushTuile(*it);
        b.tas.push_back(std::move(ts));
    }

    // 4) brancher les tas dans la pioche
    for (auto& t : b.tas) b.pioche.ajouterTas(&t);

    // 5) reste -> réservoir
    b.reservoir.insert(b.reservoir.end(), it, end);

    return b;
}

void simuler(int nbJoueurs) {
    auto b = construire(nbJoueurs);

    // Chantier initial (remplit d’abord depuis le réservoir)
    Table table(nbJoueurs, std::move(b.reservoir));
    table.print();

    size_t tour = 0;
    while (!(table.estVide() && b.pioche.estVide())) {
        std::cout << "\n--- Tour " << ++tour << " ---\n";
        if (table.taille() > 0) {
            const Tuile* prise = table.prendre(0); // on prend toujours la [0] (gratuite)
            if (prise) {
                std::cout << "Prise auto: " << prise->id
                    << " (minJ=" << prise->minPlayers << ", cout=0)\n";
            }
        }
        table.recharger(b.pioche); // 0 -> remplir ; 1 -> règle “1 tuile”
        table.print();
    }
    std::cout << "\nFin de la simulation : plus aucune tuile.\n";
}
