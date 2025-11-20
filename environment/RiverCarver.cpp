// RiverCarver.cpp
#include "RiverCarver.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

void RiverCarver::Carve(std::vector<double> &heights, int w, int h,
                        const std::vector<std::vector<std::pair<int,int>>> &rivers,
                        double carveDepth, int widthHalf, bool smoothAfter)
{
    if (heights.size() != (size_t)w*h) return;
    auto idx = [w](int x, int y){ return y*w + x; };

    for (const auto &path : rivers) {
        for (const auto &pt : path) {
            int cx = pt.first, cy = pt.second;
            for (int oy = -widthHalf; oy <= widthHalf; ++oy) for (int ox = -widthHalf; ox <= widthHalf; ++ox) {
                int nx = cx + ox, ny = cy + oy;
                if (nx < 0 || nx >= w || ny < 0 || ny >= h) continue;
                double dist = std::sqrt(double(ox*ox + oy*oy));
                double factor = std::max(0.0, 1.0 - dist / (widthHalf + 1e-6));
                heights[idx(nx,ny)] = std::max(0.0, heights[idx(nx,ny)] - carveDepth * factor);
            }
        }
    }

    if (smoothAfter) Smooth(heights, w, h, 1);
}

void RiverCarver::Smooth(std::vector<double> &heights, int w, int h, int passes) {
    std::vector<double> tmp = heights;
    auto idx = [w](int x, int y){ return y*w + x; };
    for (int p = 0; p < passes; ++p) {
        for (int y = 1; y < h-1; ++y) for (int x = 1; x < w-1; ++x) {
            double s = 0.0; int n=0;
            for (int oy=-1; oy<=1; ++oy) for (int ox=-1; ox<=1; ++ox) {
                s += tmp[idx(x+ox,y+oy)]; ++n;
            }
            heights[idx(x,y)] = s / n;
        }
        tmp = heights;
    }
}