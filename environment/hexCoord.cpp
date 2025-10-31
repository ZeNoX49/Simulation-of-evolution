#include "hexCoord.hpp"

// Définition des 6 directions hexagonales (flat-top orientation)
// Direction 0 = droite, puis sens horaire
const HexCoord HexCoord::directions[6] = {
    HexCoord(+1, 0, -1),   // Direction 0: Est
    HexCoord(+1, -1, 0),   // Direction 1: Nord-Est
    HexCoord(0, -1, +1),   // Direction 2: Nord-Ouest
    HexCoord(-1, 0, +1),   // Direction 3: Ouest
    HexCoord(-1, +1, 0),   // Direction 4: Sud-Ouest
    HexCoord(0, +1, -1)    // Direction 5: Sud-Est
};