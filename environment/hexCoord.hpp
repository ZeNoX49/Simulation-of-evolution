#ifndef HEXCOORD_HPP
#define HEXCOORD_HPP

#include <vector>
#include <cmath>
#include <SFML/System/Vector2.hpp>

/**
 * Système de coordonnées hexagonales en "cube coordinates"
 * Constraint: q + r + s = 0
 * 
 * Avantages des cube coordinates:
 * - Distance facile: (|q1-q2| + |r1-r2| + |s1-s2|) / 2
 * - Voisins faciles: +/- 1 sur une coordonnée
 * - Symétrie et calculs simples
 */
struct HexCoord {
    int q, r, s;
    
    // Constructeurs
    HexCoord() : q(0), r(0), s(0) {}
    HexCoord(int q, int r) : q(q), r(r), s(-q - r) {}
    HexCoord(int q, int r, int s) : q(q), r(r), s(s) {}
    
    // Vérifier la validité (q + r + s = 0)
    bool isValid() const {
        return (q + r + s) == 0;
    }
    
    // Opérateurs de comparaison
    bool operator==(const HexCoord& other) const {
        return q == other.q && r == other.r && s == other.s;
    }
    
    bool operator!=(const HexCoord& other) const {
        return !(*this == other);
    }
    
    // Addition
    HexCoord operator+(const HexCoord& other) const {
        return HexCoord(q + other.q, r + other.r, s + other.s);
    }
    
    // Soustraction
    HexCoord operator-(const HexCoord& other) const {
        return HexCoord(q - other.q, r - other.r, s - other.s);
    }
    
    // Multiplication par scalaire
    HexCoord operator*(int k) const {
        return HexCoord(q * k, r * k, s * k);
    }
    
    // Distance de Manhattan entre deux hexagones
    int distance(const HexCoord& other) const {
        return (std::abs(q - other.q) + std::abs(r - other.r) + std::abs(s - other.s)) / 2;
    }
    
    // Les 6 directions possibles
    static const HexCoord directions[6];
    
    // Obtenir un voisin dans une direction (0-5)
    HexCoord neighbor(int direction) const {
        return *this + directions[direction];
    }
    
    // Obtenir tous les voisins (6)
    std::vector<HexCoord> neighbors() const {
        std::vector<HexCoord> result;
        result.reserve(6);
        for (int i = 0; i < 6; ++i) {
            result.push_back(neighbor(i));
        }
        return result;
    }
    
    // Obtenir tous les hexagones dans un rayon donné
    std::vector<HexCoord> hexagonsInRange(int range) const {
        std::vector<HexCoord> results;
        
        for (int dq = -range; dq <= range; ++dq) {
            int r1 = std::max(-range, -dq - range);
            int r2 = std::min(range, -dq + range);
            
            for (int dr = r1; dr <= r2; ++dr) {
                int ds = -dq - dr;
                results.push_back(HexCoord(q + dq, r + dr, s + ds));
            }
        }
        
        return results;
    }
    
    // Conversion vers coordonnées offset (row, col pour grille décalée)
    // Utile pour compatibilité avec ton système actuel
    struct OffsetCoord {
        int col;
        int row;
        
        OffsetCoord(int col, int row) : col(col), row(row) {}
    };
    
    OffsetCoord toOffset() const {
        int col = q;
        int row = r + (q - (q & 1)) / 2;
        return OffsetCoord(col, row);
    }
    
    // Conversion depuis offset vers cube
    static HexCoord fromOffset(int col, int row) {
        int q = col;
        int r = row - (col - (col & 1)) / 2;
        int s = -q - r;
        return HexCoord(q, r, s);
    }
    
    // Conversion vers coordonnées pixel (flat-top orientation)
    sf::Vector2f toPixel(float radius) const {
        float x = radius * std::sqrt(3.0f) * (q + r / 2.0f);
        float y = radius * 3.0f / 2.0f * r;
        return sf::Vector2f(x, y);
    }
    
    // Conversion depuis pixel vers hex (arrondi)
    static HexCoord fromPixel(sf::Vector2f pixel, float radius) {
        float q = (std::sqrt(3.0f) / 3.0f * pixel.x - 1.0f / 3.0f * pixel.y) / radius;
        float r = (2.0f / 3.0f * pixel.y) / radius;
        return hexRound(q, r);
    }
    
    // Arrondir des coordonnées fractionnelles vers l'hexagone le plus proche
    static HexCoord hexRound(float fq, float fr) {
        float fs = -fq - fr;
        
        int q = static_cast<int>(std::round(fq));
        int r = static_cast<int>(std::round(fr));
        int s = static_cast<int>(std::round(fs));
        
        float q_diff = std::abs(q - fq);
        float r_diff = std::abs(r - fr);
        float s_diff = std::abs(s - fs);
        
        if (q_diff > r_diff && q_diff > s_diff) {
            q = -r - s;
        } else if (r_diff > s_diff) {
            r = -q - s;
        } else {
            s = -q - r;
        }
        
        return HexCoord(q, r, s);
    }
    
    // Interpolation linéaire entre deux hexagones
    static HexCoord lerp(const HexCoord& a, const HexCoord& b, float t) {
        float fq = a.q + (b.q - a.q) * t;
        float fr = a.r + (b.r - a.r) * t;
        return hexRound(fq, fr);
    }
    
    // Ligne entre deux hexagones (pour pathfinding, line of sight, etc.)
    static std::vector<HexCoord> line(const HexCoord& a, const HexCoord& b) {
        int N = a.distance(b);
        std::vector<HexCoord> results;
        results.reserve(N + 1);
        
        for (int i = 0; i <= N; ++i) {
            float t = N == 0 ? 0.0f : static_cast<float>(i) / N;
            results.push_back(lerp(a, b, t));
        }
        
        return results;
    }
};

// Hash function pour utiliser HexCoord dans unordered_map
namespace std {
    template<>
    struct hash<HexCoord> {
        size_t operator()(const HexCoord& h) const {
            // Cantor pairing function améliorée
            size_t hash_q = std::hash<int>{}(h.q);
            size_t hash_r = std::hash<int>{}(h.r);
            return hash_q ^ (hash_r << 1);
        }
    };
}

#endif // HEXCOORD_HPP