#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>

using namespace std;

Color green = { 173,204,96,255 };
Color darkgreen = { 43,51,24,255 };

int cellSize = 30;
int cellCount = 25;
int offset = 75;

double lastUpdateTime = 0;

bool ElementIndeque(Vector2 element, deque<Vector2> deque)
{
    for (unsigned int i = 0; i < deque.size(); i++)
    {
        if (Vector2Equals(deque[i], element))
        {
            return true;
        }
    }
    return false;
}

bool eventTriggered(double interval)
{
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval)
    {
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

class Snake
{
public:
    deque<Vector2> body = { Vector2{6,9}, Vector2{5,9},Vector2{4,9} };
    Vector2 direction{ 1,0 };
    bool addSegment = false;

    void Draw()
    {
        for (int i = 0; i < body.size(); i++)
        {
            float x = body[i].x;
            float y = body[i].y;
            Rectangle segment = Rectangle{
                offset + x * cellSize,
                offset + y * cellSize,
                (float)cellSize,
                (float)cellSize
            };
            DrawRectangleRounded(segment, 0.5, 6, darkgreen);
        }
    }

    void Update()
    {
        body.push_front(Vector2Add(body[0], direction));
        if (addSegment)
        {
            addSegment = false;
        }
        else
        {
            body.pop_back();
        }
    }

    void Reset()
    {
        body = { Vector2{6,9}, Vector2{5,9},Vector2{4,9} };
        direction = { 1,0 };
    }
};

class Food
{
public:
    Vector2 position;
    Texture2D texture;

    Food(deque<Vector2> snakeBody)
    {
        Image image = LoadImage("Graphics/food2.png");
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
        position = GenerateRandompos(snakeBody);
    }

    ~Food()
    {
        UnloadTexture(texture);
    }

    void Draw()
    {
        DrawTexture(texture,
            offset + position.x * cellSize,
            offset + position.y * cellSize,
            WHITE);
    }

    Vector2 GenerateRandomCell()
    {
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{ x,y };
    }

    Vector2 GenerateRandompos(deque<Vector2> snakeBody)
    {
        Vector2 pos = GenerateRandomCell();
        while (ElementIndeque(pos, snakeBody))
        {
            pos = GenerateRandomCell();
        }
        return pos;
    }
};

class Game
{
public:
    Snake snake;
    Food food = Food(snake.body);
    bool running = true;
    int score = 0;

    Sound eatSound;
    Sound wallHitSound;

    Game()
    {
        InitAudioDevice();
        eatSound = LoadSound("Sounds/eat.wav");
        wallHitSound = LoadSound("Sounds/gameover.wav");
    }

    ~Game()
    {
        UnloadSound(eatSound);
        UnloadSound(wallHitSound);
        CloseAudioDevice();
    }

    void Draw()
    {
        food.Draw();
        snake.Draw();
    }

    void Update()
    {
        if (running)
        {
            snake.Update();
            CheckCollisionWithFood();
            CheckCollisionwithEdges();
            CheckCollisionWithTail();
        }
    }

    void CheckCollisionWithFood()
    {
        if (Vector2Equals(snake.body[0], food.position))
        {
            food.position = food.GenerateRandompos(snake.body);
            snake.addSegment = true;
            score++;
            PlaySound(eatSound);
        }
    }

    void CheckCollisionwithEdges()
    {
        if (snake.body[0].x == cellCount || snake.body[0].x == -1 ||
            snake.body[0].y == cellCount || snake.body[0].y == -1)
        {
            GameOver();
        }
    }

    void CheckCollisionWithTail()
    {
        deque<Vector2> headless = snake.body;
        headless.pop_front();

        if (ElementIndeque(snake.body[0], headless))
        {
            GameOver();
        }
    }

    void GameOver()
    {
        snake.Reset();
        food.position = food.GenerateRandompos(snake.body);
        running = false;
        score = 0;
        PlaySound(wallHitSound);
    }
};

int main()
{
    cout << "Starting the game..." << endl;

    InitWindow(2 * offset + cellSize * cellCount,2 * offset + cellSize * cellCount,"RETRO SNAKE");
    SetTargetFPS(60);

    Game game;

    while (!WindowShouldClose())
    {
        bool gamepadAvailable = IsGamepadAvailable(0);

        // ----------- KEYBOARD INPUT -----------
        if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1)
        {
            game.snake.direction = { 0,-1 };
            game.running = true;
        }
        if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1)
        {
            game.snake.direction = { 0,1 };
            game.running = true;
        }
        if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1)
        {
            game.snake.direction = { -1,0 };
            game.running = true;
        }
        if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1)
        {
            game.snake.direction = { 1,0 };
            game.running = true;
        }

        // ----------- GAMEPAD INPUT -----------
        if (gamepadAvailable)
        {
            // D-PAD
            if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_UP) && game.snake.direction.y != 1)
            {
                game.snake.direction = { 0,-1 };
                game.running = true;
            }
            if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN) && game.snake.direction.y != -1)
            {
                game.snake.direction = { 0,1 };
                game.running = true;
            }
            if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT) && game.snake.direction.x != 1)
            {
                game.snake.direction = { -1,0 };
                game.running = true;
            }
            if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT) && game.snake.direction.x != -1)
            {
                game.snake.direction = { 1,0 };
                game.running = true;
            }

            // ANALOG STICK
            float axisX = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
            float axisY = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);

            if (axisY < -0.5f && game.snake.direction.y != 1)
            {
                game.snake.direction = { 0,-1 };
                game.running = true;
            }
            else if (axisY > 0.5f && game.snake.direction.y != -1)
            {
                game.snake.direction = { 0,1 };
                game.running = true;
            }

            if (axisX < -0.5f && game.snake.direction.x != 1)
            {
                game.snake.direction = { -1,0 };
                game.running = true;
            }
            else if (axisX > 0.5f && game.snake.direction.x != -1)
            {
                game.snake.direction = { 1,0 };
                game.running = true;
            }
        }

        BeginDrawing();

        if (eventTriggered(0.2))
        {
            game.Update();
        }

        ClearBackground(green);

        DrawRectangleLinesEx(
            Rectangle{
                (float)offset - 5,
                (float)offset - 5,
                (float)cellSize * cellCount + 10,
                (float)cellSize * cellCount + 10
            },
            5,
            darkgreen
        );

        DrawText("RETRO SNAKE", offset, 20, 40, darkgreen);
        DrawText(TextFormat("SCORE: %i", game.score),
            offset + cellSize * cellCount - 150,
            20, 40, darkgreen);

        if (gamepadAvailable)
        {
            DrawText(GetGamepadName(0), 10, 10, 20, DARKGRAY);
        }

        game.Draw();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}