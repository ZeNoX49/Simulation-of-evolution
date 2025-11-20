// TileClimate.cpp
#include "TileClimate.hpp"
#include <cmath>
#include <queue>
#include <limits>
#include <vector>

// Helper: multi-source BFS (4-neighbors) distance to mask true in cells
static std::vector<int> distanceToMaskInt(const std::vector<char> &mask, int w, int h) {
    const int INF = 1e9;
    std::vector<int> dist(w*h, INF);
    std::deque<std::pair<int,int>> dq;
    for (int y=0;y<h;++y) for (int x=0;x<w;++x) {
        if (mask[y*w + x]) { dist[y*w + x] = 0; dq.emplace_back(x,y); }
    }
    const int dx[4] = {1,-1,0,0}, dy[4] = {0,0,1,-1};
    while (!dq.empty()) {
        auto p = dq.front(); dq.pop_front();
        int x = p.first, y = p.second;
        int d0 = dist[y*w + x];
        for (int k=0;k<4;++k) {
            int nx = x + dx[k], ny = y + dy[k];
            if (nx<0||nx>=w||ny<0||ny>=h) continue;
            if (dist[ny*w + nx] > d0 + 1) {
                dist[ny*w + nx] = d0 + 1;
                dq.emplace_back(nx, ny);
            }
        }
    }
    return dist;
}

void TileClimate::ComputeClimate(const std::vector<double> &heights, int w, int h,
                                 std::vector<double> &outTemp,
                                 std::vector<double> &outPrecip,
                                 std::vector<Biome> &outBiome,
                                 double seaLevel) const
{
    outTemp.assign(w*h, 0.0);
    outPrecip.assign(w*h, 0.0);
    outBiome.assign(w*h, Biome::Ocean);

    // compute ocean mask
    std::vector<char> isOcean(w*h, 0);
    for (int i=0;i<w*h;++i) isOcean[i] = (heights[i] <= seaLevel ? 1 : 0);

    auto dist = distanceToMaskInt(isOcean, w, h);

    for (int y=0;y<h;++y) {
        double latFactor = 1.0 - std::fabs((double(y)/(h-1.0) * 2.0 - 1.0)); // 1 at equator
        for (int x=0;x<w;++x) {
            int i = y*w + x;
            double elevKm = heights[i] * 3.0; // scale
            double baseLatTemp = poleTemp * (1.0 - latFactor) + equatorTemp * latFactor;
            double oceanEffect = std::exp(- (double)dist[i] / oceanInfluenceScale);
            double t = baseLatTemp - lapseRateCperKm * elevKm + coastalTempBoost * oceanEffect;
            outTemp[i] = t;

            double basePrecip = 200.0 * latFactor + 50.0 * (1.0 - latFactor); // rough mm/year
            double p = basePrecip * (1.0 + 2.0 * oceanEffect);
            outPrecip[i] = p;

            if (isOcean[i]) { outBiome[i] = Biome::Ocean; continue; }
            if (t <= -5.0) outBiome[i] = Biome::Snow;
            else if (t < 0.0) outBiome[i] = Biome::Tundra;
            else if (t < 8.0) outBiome[i] = Biome::Taiga;
            else {
                if (p < 80.0) outBiome[i] = Biome::Desert;
                else if (p < 200.0) outBiome[i] = Biome::Grassland;
                else outBiome[i] = Biome::Forest;
            }
            if (heights[i] > 0.8) outBiome[i] = Biome::Mountain;
        }
    }
}