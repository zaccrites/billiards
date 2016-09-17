
#include <utility>  // for std::pair
#include <cmath>    // for std::sin
#include <algorithm>  // for std::remove_if
#include <iostream>
#include <glm/geometric.hpp>  // for glm::length, glm::distance, glm::reflect

#include "Game.hpp"
#include "debug.hpp"


// Todo: move to own file
Ball::Ball(SDL_Texture *pTexture) :
    position {0, 0},
    velocity {0, 0},
    m_pTexture {pTexture}
{
}

BallForce::BallForce(Ball *pBall, glm::vec2 force, float duration) :
    m_pBall {pBall},
    m_Force {force},
    m_Duration {duration}
{
}


Game::Game() :
    m_IsRunning {false},
    m_pWindow {nullptr},
    m_pRenderer {nullptr},
    m_BallTextures {},
    m_Balls {},
    m_BallForces {},
    m_ShotPower {0.0}
{
}

Game::~Game()
{
    teardownSDL();
}


SDL_Texture* Game::createBallTexture(uint8_t red, uint8_t green, uint8_t blue, bool hasStripe)
{
    DEBUG_LOG("Creating ball texture: red=%u, green=%u, blue=%u, hasStripe=%s \n", red, green, blue, BOOL_STRING(hasStripe));

    SDL_Texture *pTexture = SDL_CreateTexture(
        m_pRenderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STATIC,
        BALL_DIAMETER,
        BALL_DIAMETER
    );
    if ( ! pTexture)
    {
        return nullptr;
    }

    SDL_SetTextureBlendMode(pTexture, SDL_BLENDMODE_BLEND);

    const auto R = static_cast<int16_t>(BALL_DIAMETER / 2);
    const auto R_EDGE = static_cast<int16_t>(R - 3);
    const auto STRIPE_AREA_WIDTH = static_cast<int16_t>(BALL_DIAMETER / 3);

    uint16_t i = 0;
    uint8_t textureData[BALL_DIAMETER * BALL_DIAMETER * 4];
    for (uint16_t y = 0; y < BALL_DIAMETER; ++y)
    {
        for (uint16_t x = 0; x < BALL_DIAMETER; ++x)
        {
            // Fix coords for calculating the pixels inside the circle.
            const auto x0 = static_cast<int16_t>(x - R);
            const auto y0 = static_cast<int16_t>(y - R);
            const auto qq = static_cast<int16_t>((x0 * x0) + (y0 * y0));

            const bool onEdge {qq >= (R_EDGE * R_EDGE)};
            const bool doDraw {qq <= (R * R)};

            const bool inStripeArea {x < STRIPE_AREA_WIDTH || x > (BALL_DIAMETER - STRIPE_AREA_WIDTH)};
            if (onEdge)
            {
                textureData[i++] = 0x00;
                textureData[i++] = 0x00;
                textureData[i++] = 0x00;
            }
            else if (hasStripe && inStripeArea)
            {
                textureData[i++] = 0xff;
                textureData[i++] = 0xff;
                textureData[i++] = 0xff;
            }
            else
            {
                textureData[i++] = blue;
                textureData[i++] = green;
                textureData[i++] = red;
            }
            textureData[i++] = doDraw ? 0xff : 0x00;  // Alpha
        }
    }
    if (SDL_UpdateTexture(
        pTexture,
        NULL,
        textureData,
        4 * sizeof(uint8_t) * BALL_DIAMETER
    ) != 0) {
        SDL_DestroyTexture(pTexture);
        return nullptr;
    }

    return pTexture;
}


bool Game::createTextures()
{
    DEBUG_LOG("Creating textures \n");

    struct BallColorProfile
    {
        uint8_t r, g, b;
        bool hasStripe;
    };

    std::pair<BallType, BallColorProfile> profiles[] = {
        std::make_pair(BallType::Cue,          BallColorProfile{255, 255, 255, false}),
        std::make_pair(BallType::Black,        BallColorProfile{0,   0,   0,   false}),

        std::make_pair(BallType::Yellow,       BallColorProfile{255, 204, 0,   false}),
        std::make_pair(BallType::Blue,         BallColorProfile{0,   51,  153, false}),
        std::make_pair(BallType::Red,          BallColorProfile{255, 0,   0,   false}),
        std::make_pair(BallType::Purple,       BallColorProfile{61,  0,   153, false}),
        std::make_pair(BallType::Orange,       BallColorProfile{255, 51,  0,   false}),
        std::make_pair(BallType::Green,        BallColorProfile{0,   77,  0,   false}),
        std::make_pair(BallType::Maroon,       BallColorProfile{153, 0,   0,   false}),

        std::make_pair(BallType::YellowStripe, BallColorProfile{255, 204, 0,   true}),
        std::make_pair(BallType::BlueStripe,   BallColorProfile{0,   51,  153, true}),
        std::make_pair(BallType::RedStripe,    BallColorProfile{255, 0,   0,   true}),
        std::make_pair(BallType::PurpleStripe, BallColorProfile{61,  0,   153, true}),
        std::make_pair(BallType::OrangeStripe, BallColorProfile{255, 51,  0,   true}),
        std::make_pair(BallType::GreenStripe,  BallColorProfile{0,   77,  0,   true}),
        std::make_pair(BallType::MaroonStripe, BallColorProfile{153, 0,   0,   true}),
    };
    for (const auto& profile : profiles)
    {
        SDL_Texture* pTexture = createBallTexture(
            profile.second.r,
            profile.second.g,
            profile.second.b,
            profile.second.hasStripe
        );
        if ( ! pTexture)
        {
            return false;
        }
        m_BallTextures[profile.first] = pTexture;
    }

    return true;
}


void Game::printHelp()
{
    std::cout
        << "\n"
        << "How to Play: \n"
        << "----------------------------------------------------------------------- \n"
        << " - To launch the cue-ball, place the cursor over the intended \n"
           "   direction, then left-click. The power of the shot is determined \n"
           "   by the bar on the right-hand side of the screen. \n\n"
           " - Press \"R\" to reset the balls to their starting positions. \n\n"
           " - Press \"F\" to freeze the balls in their tracks! \n\n"
           " - Right-click or press \"Escape\" to exit.\n"
        << std::endl;
}


bool Game::initGame()
{
    DEBUG_LOG("Initializing Game \n");

    if ( ! createTextures())
    {
        return false;
    }

    createBalls();

    printHelp();

    return true;
}

void Game::createBalls()
{
    DEBUG_LOG("Creating Balls \n");

    std::map<BallType, glm::vec2> spots;
    spots[BallType::Cue] = glm::vec2{
        FELT_LEFT_COORD + FELT_WIDTH * 0.15f,
        FELT_TOP_COORD + FELT_HEIGHT / 2
    };

    auto positionBall = [this](int row, int position)
    {
        static const float SPACING_MULTIPLIER {1.1f};

        float baseX {FELT_LEFT_COORD + 0.6f * FELT_WIDTH};
        float x {baseX + row * (SPACING_MULTIPLIER * BALL_DIAMETER)};

        float baseY {FELT_TOP_COORD + FELT_HEIGHT / 2};
        baseY -= static_cast<float>(row) / 2.0f * BALL_DIAMETER;
        float y {baseY + SPACING_MULTIPLIER * BALL_DIAMETER * position};

        return glm::vec2{x, y};
    };

    spots[BallType::Yellow]       = positionBall(0, 0);

    spots[BallType::RedStripe]    = positionBall(1, 0);
    spots[BallType::Red]          = positionBall(1, 1);

    spots[BallType::Green]        = positionBall(2, 0);
    spots[BallType::Black]        = positionBall(2, 1);
    spots[BallType::GreenStripe]  = positionBall(2, 2);

    spots[BallType::OrangeStripe] = positionBall(3, 0);
    spots[BallType::MaroonStripe] = positionBall(3, 1);
    spots[BallType::Purple]       = positionBall(3, 2);
    spots[BallType::YellowStripe] = positionBall(3, 3);

    spots[BallType::Maroon]       = positionBall(4, 0);
    spots[BallType::Blue]         = positionBall(4, 1);
    spots[BallType::BlueStripe]   = positionBall(4, 2);
    spots[BallType::Orange]       = positionBall(4, 3);
    spots[BallType::PurpleStripe] = positionBall(4, 4);


    for (const auto& pair : spots)
    {
        auto pTexture = m_BallTextures[pair.first];
        Ball ball {pTexture};
        ball.position = pair.second;
        m_Balls.push_back(ball);
    }

}

void Game::teardownGame()
{
    for (auto const& pair : m_BallTextures)
    {
        SDL_DestroyTexture(pair.second);
    }
}

void Game::renderFrame()
{
    SDL_Rect rect;

    // Draw inside of power bar
    SDL_SetRenderDrawColor(m_pRenderer, 76, 76, 76, 0xff);
    rect.x = POWER_BAR_LEFT_COORD;
    rect.y = POWER_BAR_TOP_COORD;
    rect.w = POWER_BAR_WIDTH;
    rect.h = POWER_BAR_HEIGHT;
    SDL_RenderFillRect(m_pRenderer, &rect);

    // Draw Power Bar
    SDL_SetRenderDrawColor(m_pRenderer, 206, 13, 13, 0xff);
    rect.h = POWER_BAR_HEIGHT * m_ShotPower + 1;
    rect.w = POWER_BAR_WIDTH;
    rect.x = POWER_BAR_LEFT_COORD;
    rect.y = POWER_BAR_TOP_COORD + POWER_BAR_HEIGHT - rect.h;
    SDL_RenderFillRect(m_pRenderer, &rect);

    // Draw Power Bar Borders
    SDL_SetRenderDrawColor(m_pRenderer, 16, 14, 14, 0xff);

    // Draw Power Bar Border Top
    rect.x = POWER_BAR_LEFT_COORD - POWER_BAR_BORDER_WIDTH;
    rect.y = POWER_BAR_TOP_COORD - POWER_BAR_BORDER_WIDTH;
    rect.w = POWER_BAR_WIDTH + 2 * POWER_BAR_BORDER_WIDTH;
    rect.h = POWER_BAR_BORDER_WIDTH;
    SDL_RenderFillRect(m_pRenderer, &rect);

    // Draw Power Bar Border Bottom
    rect.x = POWER_BAR_LEFT_COORD - POWER_BAR_BORDER_WIDTH;
    rect.y = POWER_BAR_TOP_COORD + POWER_BAR_HEIGHT;
    rect.w = POWER_BAR_WIDTH + 2 * POWER_BAR_BORDER_WIDTH;
    rect.h = POWER_BAR_BORDER_WIDTH;
    SDL_RenderFillRect(m_pRenderer, &rect);

    // Draw Power Bar Border Left
    rect.x = POWER_BAR_LEFT_COORD - POWER_BAR_BORDER_WIDTH;
    rect.y = POWER_BAR_TOP_COORD;
    rect.w = POWER_BAR_BORDER_WIDTH;
    rect.h = POWER_BAR_HEIGHT;
    SDL_RenderFillRect(m_pRenderer, &rect);

    // Draw Power Bar Border Left
    rect.x = POWER_BAR_LEFT_COORD + POWER_BAR_WIDTH;
    rect.y = POWER_BAR_TOP_COORD;
    rect.w = POWER_BAR_BORDER_WIDTH;
    rect.h = POWER_BAR_HEIGHT;
    SDL_RenderFillRect(m_pRenderer, &rect);

    // Draw Felt
    SDL_SetRenderDrawColor(m_pRenderer, 19, 132, 23, 0xff);
    rect.x = FELT_LEFT_COORD;
    rect.y = FELT_TOP_COORD;
    rect.w = FELT_WIDTH;
    rect.h = FELT_HEIGHT;
    SDL_RenderFillRect(m_pRenderer, &rect);

    // Draw Bumpers
    SDL_SetRenderDrawColor(m_pRenderer, 125, 82, 9, 0xff);

    // Draw Left Bumper
    rect.x = FELT_LEFT_COORD - BUMPER_WIDTH;
    rect.y = FELT_TOP_COORD - BUMPER_WIDTH / 2;
    rect.w = BUMPER_WIDTH;
    rect.h = FELT_HEIGHT + BUMPER_WIDTH;
    SDL_RenderFillRect(m_pRenderer, &rect);

    // Draw Right Bumper
    rect.x = FELT_LEFT_COORD + FELT_WIDTH;
    rect.y = FELT_TOP_COORD - BUMPER_WIDTH / 2;
    rect.w = BUMPER_WIDTH;
    rect.h = FELT_HEIGHT + BUMPER_WIDTH;
    SDL_RenderFillRect(m_pRenderer, &rect);

    // Draw Top Bumper
    rect.x = FELT_LEFT_COORD - BUMPER_WIDTH / 2;
    rect.y = FELT_TOP_COORD - BUMPER_WIDTH;
    rect.w = FELT_WIDTH + BUMPER_WIDTH;
    rect.h = BUMPER_WIDTH;
    SDL_RenderFillRect(m_pRenderer, &rect);

    // Draw Bottom Bumper
    rect.x = FELT_LEFT_COORD - BUMPER_WIDTH / 2;
    rect.y = FELT_TOP_COORD + FELT_HEIGHT;
    rect.w = FELT_WIDTH + BUMPER_WIDTH;
    rect.h = BUMPER_WIDTH;
    SDL_RenderFillRect(m_pRenderer, &rect);

    // Draw Balls
    rect.w = BALL_DIAMETER;
    rect.h = BALL_DIAMETER;
    for (const auto& ball : m_Balls)
    {
        // Draw balls relative to center
        rect.x = ball.position.x - (BALL_DIAMETER / 2);
        rect.y = ball.position.y - (BALL_DIAMETER / 2);
        SDL_RenderCopy(m_pRenderer, ball.m_pTexture, NULL, &rect);
    }

}

void Game::simulateFrame()
{
    static float time {0.0};
    time += 0.001;

    handleInput();

    simulatePhysics(time);

    m_ShotPower = 0.5 * (std::sin(8 * time) + 1.0);

}

void Game::simulatePhysics(float time)
{
    static float lastTime {0.0};
    const float deltaTime {time - lastTime};

    // Check for collisions between balls
    for (auto& ball : m_Balls)
    {
        for (auto& otherBall : m_Balls)
        {
            // Skip if same ball
            if (&ball == &otherBall)
            {
                continue;
            }

            // If the circles don't intersect, then there is no collision
            if (glm::distance(ball.position, otherBall.position) > static_cast<float>(BALL_DIAMETER))
            {
                continue;
            }

            // Perfectly elastic collision. Operates in the reference frame
            // of the target (i.e. the target is at rest).
            auto force = 0.01f * (otherBall.position - ball.position) * glm::length(ball.velocity);
            m_BallForces.push_back({&otherBall, force, 0.0f});

            // The equal and opposite reaction force (Newton's Third Law)
            m_BallForces.push_back({&ball, -force, 0.0f});

        }
    }

    // Check for collisions between balls and bumpers
    const auto BALL_RADIUS = static_cast<float>(BALL_DIAMETER) / 2.0f;
    for (auto &ball : m_Balls)
    {
        // Right Bumper
        {
            // Check if the rightmost point of the circle is behind the line
            // of the bumper.
            const auto WALL_COORD = static_cast<float>(FELT_LEFT_COORD + FELT_WIDTH);
            if (ball.position.x + BALL_RADIUS >= WALL_COORD)
            {
                // Do a simple reflection off of the surface: perfectly
                // elastic collision. Doesn't impart a BallForce (for now).
                ball.velocity = glm::reflect(ball.velocity, glm::vec2{-1.0f, 0.0f});
                continue;
            }
        }

        // Left Bumper
        {
            const auto WALL_COORD = static_cast<float>(FELT_LEFT_COORD);
            if (ball.position.x - BALL_RADIUS <= WALL_COORD)
            {
                ball.velocity = glm::reflect(ball.velocity, glm::vec2{1.0f, 0.0f});
                continue;
            }
        }

        // Top Bumper
        {
            const auto WALL_COORD = static_cast<float>(FELT_TOP_COORD);
            if (ball.position.y - BALL_RADIUS <= WALL_COORD)
            {
                ball.velocity = glm::reflect(ball.velocity, glm::vec2{0.0f, -1.0f});
                continue;
            }
        }

        // Bottom Bumper
        {
            const auto WALL_COORD = static_cast<float>(FELT_TOP_COORD + FELT_HEIGHT);
            if (ball.position.y + BALL_RADIUS >= WALL_COORD)
            {
                ball.velocity = glm::reflect(ball.velocity, glm::vec2{0.0f, 1.0f});
                continue;
            }
        }
    }

    // Apply friction forces
    for (auto& ball : m_Balls)
    {
        const auto speed = glm::length(ball.velocity);

        // This is a rough (bad) analog of static friction.
        if (speed < STATIC_FRICTION_THRESHOLD)
        {
            ball.velocity = {0.0f, 0.0f};
        }
        else
        {
            // This is a rough analog of dynamic friction.
            // F = (coefficient)(normal force)
            //   The normal force is, on a flat pool table, just gravity.
            //   This can be approximated with the mass of the ball.

            // The friction should oppose the direction of motion, but
            // I'm not sure how the proper physics equations work at the
            // moment. I'll just fake it for now.
            const glm::vec2 friction = -ball.velocity * (FRICTION_COEFFICIENT / BALL_MASS / speed);
            m_BallForces.push_back({&ball, friction, 0.0f});
        }
    }

    // (Poorly) integrate acceleration
    for (auto& force : m_BallForces)
    {
        // F = ma  =>  a = F/m
        // V = (integral of a from t=0 to t=time) = (sum acceleration from each frame)
        glm::vec2 acceleration = force.m_Force / BALL_MASS;
        force.m_pBall->velocity += acceleration;
    }

    for (auto& ball : m_Balls)
    {
        ball.position += 1000.0f * deltaTime * ball.velocity;
    }

    // Update forces, removing expired ones
    for (auto& force : m_BallForces)
    {
        force.m_Duration -= deltaTime;
    }
    auto it = std::remove_if(begin(m_BallForces), end(m_BallForces), [](const auto& force)
    {
        return force.m_Duration < 0.0f;
    });
    m_BallForces.erase(it, end(m_BallForces));

    // printf("# forces = %zu \n", m_BallForces.size());

    lastTime = time;
}

void Game::handleInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
            {
                m_IsRunning = false;
                break;
            }

            case SDL_KEYDOWN:
            {
                handleKeyPress(event.key.keysym.sym);
                break;
            }

            // case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEBUTTONDOWN:
            {
                handleMouseClick(&event.button);
                break;
            }

        }
    }
}

void Game::handleKeyPress(SDL_Keycode keycode)
{
    switch (keycode)
    {
        case SDLK_ESCAPE:
        {
            m_IsRunning = false;
            break;
        }

        case SDLK_r:
        {
            m_BallForces.clear();
            m_Balls.clear();
            createBalls();
            break;
        }

        case SDLK_f:
        {
            for (auto& ball : m_Balls)
            {
                ball.velocity = {0, 0};
            }
            m_BallForces.clear();
            break;
        }

    }
}

void Game::handleMouseClick(const SDL_MouseButtonEvent* pEvent)
{
    switch (pEvent->button)
    {
        case SDL_BUTTON_LEFT:
        {
            Ball *pCueBall = &m_Balls[0];

            glm::vec2 force {
                (pEvent->x - pCueBall->position.x),
                (pEvent->y - pCueBall->position.y)
            };
            force *= SHOT_POWER_MULTIPLIER * m_ShotPower;

            m_BallForces.push_back({pCueBall, force, 0.0f});

            break;
        }

        case SDL_BUTTON_RIGHT:
        {
            m_IsRunning = false;
            break;
        }
    }
}


bool Game::start()
{

    if ( ! initGame())
    {
        return false;
    }

    m_IsRunning = true;
    while (m_IsRunning)
    {
        simulateFrame();

        SDL_SetRenderDrawColor(m_pRenderer, 0x64, 0x95, 0xed, 0xff);
        SDL_RenderClear(m_pRenderer);
        renderFrame();
        SDL_RenderPresent(m_pRenderer);

        SDL_Delay(2);
    }

    teardownGame();

    return true;
}


bool Game::setupSDL()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        return false;
    }

    m_pWindow = SDL_CreateWindow(
        "Billiards",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if ( ! m_pWindow)
    {
        return false;
    }

    m_pRenderer = SDL_CreateRenderer(m_pWindow, -1, SDL_RENDERER_ACCELERATED);
    if ( ! m_pRenderer)
    {
        return false;
    }

    if (SDL_SetRenderDrawBlendMode(m_pRenderer, SDL_BLENDMODE_BLEND) != 0)
    {
        return false;
    }

    return true;
}

void Game::teardownSDL()
{
    if (m_pRenderer)
    {
        SDL_DestroyRenderer(m_pRenderer);
        m_pRenderer = nullptr;
    }

    if (m_pWindow)
    {
        SDL_DestroyWindow(m_pWindow);
        m_pWindow = nullptr;
    }

    SDL_Quit();
}
