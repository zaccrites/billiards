
#ifndef GAME_HPP
#define GAME_HPP

#include <stdint.h>
#include <vector>
#include <map>

#include <SDL2/SDL.h>
#include <glm/vec2.hpp>


enum class BallType
{
    Cue,
    Black,

    Yellow,
    Blue,
    Red,
    Purple,
    Orange,
    Green,
    Maroon,

    YellowStripe,
    BlueStripe,
    RedStripe,
    PurpleStripe,
    OrangeStripe,
    GreenStripe,
    MaroonStripe,
};


struct Ball
{
    glm::vec2 position, velocity;
    SDL_Texture* m_pTexture;

    Ball(SDL_Texture* pTexture);
};


struct BallForce
{
    Ball *m_pBall;
    glm::vec2 m_Force;
    float m_Duration;  // Time left before force stops (0.0 for one frame)

    BallForce(Ball *pBall, glm::vec2 force, float duration);
};


class Game
{
public:
    Game();
    ~Game();

    bool start();
    bool setupSDL();

private:
    void teardownSDL();

    bool initGame();
    void simulateFrame();
    void renderFrame();
    void teardownGame();

    void printHelp();

    void simulatePhysics(float time);

    SDL_Texture* createBallTexture(uint8_t red, uint8_t green, uint8_t blue, bool hasStripe);
    bool createTextures();

    void createBalls();

    void handleInput();
    void handleKeyPress(SDL_Keycode sym);
    void handleMouseClick(const SDL_MouseButtonEvent* pEvent);

public:
    static const uint16_t WINDOW_WIDTH {1100};
    static const uint16_t WINDOW_HEIGHT {600};

private:
    bool m_IsRunning;

    SDL_Window* m_pWindow;
    SDL_Renderer* m_pRenderer;

    // SDL_Texture* m_pPowerTexture;

    std::map<BallType, SDL_Texture*> m_BallTextures;
    std::vector<Ball> m_Balls;
    std::vector<BallForce> m_BallForces;

    static constexpr float SHOT_POWER_MULTIPLIER {0.05f};
    float m_ShotPower;

    static constexpr float BALL_MASS {15.0f};
    static constexpr float FRICTION_COEFFICIENT {0.04f};
    static constexpr float STATIC_FRICTION_THRESHOLD {0.002f};

    static const uint16_t BUMPER_WIDTH {50};
    static const uint16_t FELT_LEFT_COORD {BUMPER_WIDTH * 2};
    static const uint16_t FELT_TOP_COORD {100};
    static const uint16_t FELT_WIDTH {800};
    static const uint16_t FELT_HEIGHT {400};

    static const uint16_t BALL_DIAMETER {35};

    static const uint16_t POWER_BAR_LEFT_COORD {FELT_WIDTH + BUMPER_WIDTH + 150};
    static const uint16_t POWER_BAR_TOP_COORD {FELT_TOP_COORD};
    static const uint16_t POWER_BAR_HEIGHT {FELT_HEIGHT};
    static const uint16_t POWER_BAR_WIDTH {40};
    static const uint16_t POWER_BAR_BORDER_WIDTH {10};


private:
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

};


#endif
