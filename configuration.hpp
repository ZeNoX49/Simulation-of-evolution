namespace conf
{
    /* ----- Window ----- */
    uint32_t const max_framerate = 144;
    float const dt = 1.0f / static_cast<float>(max_framerate);

    //Principal window
    sf::Vector2u const principal_window_size = {1920, 1080};
    sf::Vector2f const principal_window_size_f = static_cast<sf::Vector2f>(principal_window_size);

    // Game window
    sf::Vector2u const game_window_size = {1200, 1000};
    int const offsetX = 360;
    int const offsetY = 40;
}
