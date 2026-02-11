namespace conf
{
    inline int width = 1000;
    inline int height = 700;
    inline float width_f = static_cast<float>(width);
    inline float height_f = static_cast<float>(height);

    // int const multiplicateur = 300;
    // int const vitesse_deplacement = 100;
    // int const vitesse_mouvement_horizontal = 60;
    // int const vitesse_mouvement_vertical = 40;

    // float const vd = static_cast<float>(vitesse_deplacement) / 10000;
    // float const vmh = static_cast<float>(vitesse_mouvement_horizontal) / 10000;
    // float const vmv = static_cast<float>(vitesse_mouvement_vertical) / 10000;
    
    // Game window
    int const game_window_width = 1200;
    int const game_window_height = 1000;
    float const game_window_width_f = static_cast<float>(game_window_width);
    float const game_window_height_f = static_cast<float>(game_window_height);
    int const offsetX = 360;
    int const offsetY = 40;

    //
    float const model_size_div = 50.0f;
}

inline void resize_window(int newWidth, int newHeight) {
    conf::width = newWidth;
    conf::height = newHeight;
    conf::width_f = static_cast<float>(newWidth);
    conf::height_f = static_cast<float>(newHeight);
}