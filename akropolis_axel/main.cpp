// main.cpp
#include "akropolis_sim.h"


int main() {
    int p = 0;
    do {
        std::cout << "Combien de joueurs ? (2-4) : ";
        std::cin >> p;
        if (!std::cin || p < 2 || p > 4) {
            std::cin.clear(); std::cin.ignore(1000, '\n');
            p = 0;
            std::cout << "Saisie invalide.\n";
        }
    } while (p == 0);

    simuler(p);
    return 0;
}
