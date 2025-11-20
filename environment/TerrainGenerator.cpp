// TerrainGenerator.cpp
#include "TerrainGenerator.hpp"
#include <chrono>
#include <limits>
#include <random>
#include <algorithm>
#include <numeric>
#include <cmath>

TerrainGenerator::TerrainGenerator(unsigned seed)
    : noise(seed), seed(seed) {}

void TerrainGenerator::Normalize(std::vector<double> &m) {
    double mn = 1e12, mx = -1e12;
    for (double v : m) { mn = std::min(mn, v); mx = std::max(mx, v); }
    double span = mx - mn;
    if (span < 1e-12) span = 1.0;
    for (double &v : m) v = (v - mn) / span;
}

void TerrainGenerator::GenerateHeightmap() {
    heights.assign(width * height, 0.0);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double nx = (double)x / width * scale;
            double ny = (double)y / height * scale;
            heights[idx(x,y)] = noise.FractalNoise(nx, ny, octaves, persistence, lacunarity);
        }
    }
    Normalize(heights);
}

void TerrainGenerator::ApplyErosion(int iterations) {
    if (width < 3 || height < 3) return;
    if (heights.empty()) return;

    std::mt19937 rng(seed + 1234);
    std::uniform_int_distribution<int> ux(1, width - 2);
    std::uniform_int_distribution<int> uy(1, height - 2);

    for (int it = 0; it < iterations; ++it) {
        int x = ux(rng), y = uy(rng);
        double current = heights[idx(x,y)];

        int bestX = x, bestY = y;
        double maxDrop = 0.0;

        // Cherche la pente la plus forte (D8)
        for (int oy = -1; oy <= 1; ++oy) {
            for (int ox = -1; ox <= 1; ++ox) {
                if (ox == 0 && oy == 0) continue;

                int nx = x + ox, ny = y + oy;
                double neighbour = heights[idx(nx, ny)];
                double drop = current - neighbour;

                if (drop > maxDrop) {
                    maxDrop = drop;
                    bestX = nx;
                    bestY = ny;
                }
            }
        }

        // Déplacement de matière
        if (maxDrop > 0.0) {
            double move = std::min(erosionTalus, maxDrop * 0.5);
            heights[idx(x,y)] -= move;
            heights[idx(bestX,bestY)] += move;
        }
    }
}

void TerrainGenerator::ComputeFlowAccumulation() {
    if (heights.empty()) return;
    flow.assign(width * height, 1.0); // rainfall = 1 per cell

    // sort indices by descending height
    std::vector<int> order(width*height);
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](int a, int b){ return heights[a] > heights[b]; });

    for (int k = 0; k < (int)order.size(); ++k) {
        int i = order[k];
        int x = i % width, y = i / width;
        double bestH = heights[i];
        int bx = x, by = y;
        for (int oy = -1; oy <= 1; ++oy) for (int ox = -1; ox <= 1; ++ox) {
            if (ox == 0 && oy == 0) continue;
            int nx = x + ox, ny = y + oy;
            if (nx < 0 || nx >= width || ny < 0 || ny >= height) continue;
            double nh = heights[idx(nx,ny)];
            if (nh < bestH) { bestH = nh; bx = nx; by = ny; }
        }
        if (bx != x || by != y) flow[idx(bx,by)] += flow[i];
    }
}


std::vector<std::vector<std::pair<int,int>>> TerrainGenerator::ExtractRivers(double threshold) {
    std::vector<std::vector<std::pair<int,int>>> rivers;
    if (flow.empty()) return rivers;
    std::vector<char> used(width*height, 0);

    for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x) {
        int i = idx(x,y);
        if (used[i]) continue;
        if (flow[i] >= threshold) {
            std::vector<std::pair<int,int>> path;
            int cx = x, cy = y;
            int safety = 0;
            while (true) {
                int ci = idx(cx,cy);
                if (used[ci]) break;
                used[ci] = 1;
                path.emplace_back(cx,cy);
                int nx = cx, ny = cy;
                double bestFlow = flow[ci];
                for (int oy = -1; oy <= 1; ++oy) for (int ox = -1; ox <= 1; ++ox) {
                    if (ox == 0 && oy == 0) continue;
                    int tx = cx + ox, ty = cy + oy;
                    if (tx < 0 || tx >= width || ty < 0 || ty >= height) continue;
                    if (flow[idx(tx,ty)] > bestFlow) { bestFlow = flow[idx(tx,ty)]; nx = tx; ny = ty; }
                }
                if (nx == cx && ny == cy) break;
                if (flow[idx(nx,ny)] < threshold) { path.emplace_back(nx,ny); break; }
                cx = nx; cy = ny;
                if (++safety > 100000) break;
            }
            if ((int)path.size() >= minRiverLength) rivers.push_back(std::move(path));
        }
    }
    return rivers;
}

void TerrainGenerator::CarveRivers(const std::vector<std::vector<std::pair<int,int>>> &rivers,
                                  double carveDepth, int widthHalf, bool smoothAfter) {
    if (heights.empty()) return;
    for (auto &path : rivers) {
        for (auto &pt : path) {
            int cx = pt.first, cy = pt.second;
            for (int oy = -widthHalf; oy <= widthHalf; ++oy) for (int ox = -widthHalf; ox <= widthHalf; ++ox) {
                int nx = cx + ox, ny = cy + oy;
                if (nx < 0 || nx >= width || ny < 0 || ny >= height) continue;
                double dist = std::sqrt(double(ox*ox + oy*oy));
                double factor = std::max(0.0, 1.0 - dist / (widthHalf + 1e-6));
                heights[idx(nx,ny)] = std::max(0.0, heights[idx(nx,ny)] - carveDepth * factor);
            }
        }
    }
    if (smoothAfter) SmoothHeights(1);
}

void TerrainGenerator::SmoothHeights(int passes) {
    if (heights.empty()) return;
    std::vector<double> tmp = heights;
    for (int p = 0; p < passes; ++p) {
        for (int y = 1; y < height-1; ++y) for (int x = 1; x < width-1; ++x) {
            double s = 0.0; int n=0;
            for (int oy=-1; oy<=1; ++oy) for (int ox=-1; ox<=1; ++ox) {
                s += tmp[idx(x+ox,y+oy)]; ++n;
            }
            heights[idx(x,y)] = s / n;
        }
        tmp = heights;
    }
}

std::vector<std::vector<std::pair<int,int>>> TerrainGenerator::GenerateAll() {
    GenerateHeightmap();
    ApplyErosion(erosionIterations);
    ComputeFlowAccumulation();
    auto rivers = ExtractRivers(riverFlowThreshold);
    CarveRivers(rivers, 0.02, 2, true);
    // recompute flow after carving for better accuracy if needed
    ComputeFlowAccumulation();
    return rivers;
}