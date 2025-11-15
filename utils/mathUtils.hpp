#pragma once

#ifndef MATHUTILS_HPP
#define MATHUTILS_HPP

#include <cmath>
#include <algorithm>

namespace MathUtils {
    
    // Interpolation linéaire
    inline float lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }
    
    // min <= value <= max
    inline float clamp(float value, float min, float max) {
        return std::max(min, std::min(max, value));
    }
    
    // // Map une valeur d'un range à un autre
    // inline float map(float value, float inMin, float inMax, float outMin, float outMax) {
    //     return outMin + (outMax - outMin) * ((value - inMin) / (inMax - inMin));
    // }
    
    // // Smoothstep (interpolation smooth)
    // inline float smoothstep(float t) {
    //     return t * t * (3.0f - 2.0f * t);
    // }
    
    // // Distance euclidienne 2D
    // inline float distance(const sf::Vector2f& a, const sf::Vector2f& b) {
    //     float dx = b.x - a.x;
    //     float dy = b.y - a.y;
    //     return std::sqrt(dx * dx + dy * dy);
    // }
    
    // // Distance Manhattan 2D
    // inline float manhattanDistance(const sf::Vector2f& a, const sf::Vector2f& b) {
    //     return std::abs(b.x - a.x) + std::abs(b.y - a.y);
    // }
    
    // // Normaliser un vecteur
    // inline sf::Vector2f normalize(const sf::Vector2f& v) {
    //     float length = std::sqrt(v.x * v.x + v.y * v.y);
    //     if (length == 0.0f) return sf::Vector2f(0.0f, 0.0f);
    //     return sf::Vector2f(v.x / length, v.y / length);
    // }
    
    // // Longueur d'un vecteur
    // inline float length(const sf::Vector2f& v) {
    //     return std::sqrt(v.x * v.x + v.y * v.y);
    // }
    
    // // Produit scalaire
    // inline float dot(const sf::Vector2f& a, const sf::Vector2f& b) {
    //     return a.x * b.x + a.y * b.y;
    // }
    
    // // Angle entre deux vecteurs (en radians)
    // inline float angleBetween(const sf::Vector2f& a, const sf::Vector2f& b) {
    //     float dotProduct = dot(normalize(a), normalize(b));
    //     return std::acos(clamp(dotProduct, -1.0f, 1.0f));
    // }
    
    // // Vérifier si un point est dans un cercle
    // inline bool isInCircle(const sf::Vector2f& point, const sf::Vector2f& center, float radius) {
    //     return distance(point, center) <= radius;
    // }
    
    // // Normaliser un angle entre 0 et 2*PI
    // inline float normalizeAngle(float angle) {
    //     const float TWO_PI = 2.0f * 3.14159265359f;
    //     angle = std::fmod(angle, TWO_PI);
    //     if (angle < 0) angle += TWO_PI;
    //     return angle;
    // }
    
    // // Différence angulaire la plus courte entre deux angles
    // inline float shortestAngleDiff(float angle1, float angle2) {
    //     const float PI = 3.14159265359f;
    //     float diff = normalizeAngle(angle2 - angle1);
    //     if (diff > PI) diff -= 2.0f * PI;
    //     return diff;
    // }
    
    // // Rolling average (moyenne glissante)
    // class RollingAverage {
    // private:
    //     float sum;
    //     int count;
    //     int maxCount;
        
    // public:
    //     RollingAverage(int maxCount = 100) 
    //         : sum(0.0f), count(0), maxCount(maxCount) {}
        
    //     void add(float value) {
    //         if (count < maxCount) {
    //             sum += value;
    //             count++;
    //         } else {
    //             // Simplified: just keep adding (not perfect but fast)
    //             sum = sum * 0.99f + value;
    //         }
    //     }
        
    //     float average() const {
    //         return count > 0 ? sum / count : 0.0f;
    //     }
        
    //     void reset() {
    //         sum = 0.0f;
    //         count = 0;
    //     }
    // };
    
    // // Weighted random choice
    // // weights = tableau des poids, retourne l'index choisi
    // template<typename T>
    // int weightedRandomChoice(const std::vector<T>& weights, float randomValue) {
    //     T totalWeight = 0;
    //     for (const auto& w : weights) {
    //         totalWeight += w;
    //     }
        
    //     T threshold = randomValue * totalWeight;
    //     T cumulative = 0;
        
    //     for (size_t i = 0; i < weights.size(); ++i) {
    //         cumulative += weights[i];
    //         if (cumulative >= threshold) {
    //             return static_cast<int>(i);
    //         }
    //     }
        
    //     return static_cast<int>(weights.size() - 1);
    // }
    
} // namespace MathUtils

#endif // MATHUTILS_HPP