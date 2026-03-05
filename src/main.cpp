#include <vector>
#include <unordered_map>
#include <cmath>
#include "raylib.h"


#define ENERGY_GAIN 0.5f
class Ball{
    public:
        Vector2 ballPosition;
        Vector2 ballVelocity;

        int ballRadius;
        Color ballColor; 

        //const
        Ball(   Vector2 ballPosition = {(float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2},
                Vector2 ballVelocity = {0, 0}, 
                int ballRadius = 10 ){

            this->ballPosition = ballPosition;
            this->ballVelocity = ballVelocity;

            this->ballRadius = ballRadius;
            ballColor = {
                (unsigned char)GetRandomValue(0, 255),
                (unsigned char)GetRandomValue(0, 255),
                (unsigned char)GetRandomValue(0, 255),
                255
            };
        }

        void SetPosition(Vector2 newVector){
            this->ballPosition = newVector;
        }

        void Update(float deltaTime){

            //floor
            if(ballPosition.y + ballRadius >= GetScreenHeight()){
                ballPosition.y = GetScreenHeight() - ballRadius;
                ballVelocity.y = -ballVelocity.y * ENERGY_GAIN;
            }

            //ceil
            if(ballPosition.y - ballRadius <= 0){
                ballPosition.y = ballRadius;
                ballVelocity.y = -ballVelocity.y * ENERGY_GAIN;
            }

            //rightwall
            if(ballPosition.x + ballRadius >= GetScreenWidth()){
                ballPosition.x = GetScreenWidth() - ballRadius;
                ballVelocity.x = -ballVelocity.x * ENERGY_GAIN;
            }

            //leftwall
            if(ballPosition.x - ballRadius <= 0){
                ballPosition.x = ballRadius;
                ballVelocity.x = -ballVelocity.x * ENERGY_GAIN;
            }

            ballVelocity.y += 980.f * deltaTime;
            ballPosition = {ballPosition.x + ballVelocity.x * deltaTime, ballPosition.y + ballVelocity.y * deltaTime};
        }

        void Draw(){
            DrawCircle(ballPosition.x, ballPosition.y, ballRadius, ballColor);
        }
};

class Grid{
    public:
    int cellSize;
    std::unordered_map<int, std::vector<Ball*>> cells;

    Grid(int cellSize){
        this->cellSize = cellSize;
    }

    int Key(int cx, int cy){
        return ((cx * 100000) + cy);
    }

    void Clear(){
        cells.clear();
    }
    void Insert(Ball* ball){
        int cx = (int)(ball->ballPosition.x / cellSize);
        int cy = (int)(ball->ballPosition.y / cellSize);
        cells[Key(cx, cy)].push_back(ball);
    }

    std::vector<Ball*> Extract(Ball* ball){
        std::vector<Ball*> balls;

        int cx = (int)(ball->ballPosition.x / cellSize);
        int cy = (int)(ball->ballPosition.y / cellSize);

        for (int i = cx - 1; i <= cx + 1; i++){
            for(int j = cy - 1; j <= cy + 1; j++){
                auto it = cells.find(Key(i, j));

                if(it != cells.end()){
                    for(Ball* b : it->second){
                        balls.push_back(b);
                    }
                }
            }
        }
        return balls;
    }

};

Vector2 GetRandomScreenPos(){
    float x = GetRandomValue(0, GetScreenWidth());
    float y = GetRandomValue(0, GetScreenHeight());

    return {x,  y};
}

float GetRandomVelocityX(){
    float x = GetRandomValue(-100, 100);

    return x;
}
void ResolveCollision(Ball& a, Ball& b) {
    float dx = b.ballPosition.x - a.ballPosition.x;
    float dy = b.ballPosition.y - a.ballPosition.y;
    float dist = sqrtf(dx*dx + dy*dy);
    float minDist = a.ballRadius + b.ballRadius;

    if (dist == 0 || dist >= minDist) return;

    float nx = dx / dist;
    float ny = dy / dist;

    float overlap = minDist - dist;
    float correction = overlap * 0.5f;

    a.ballPosition.x -= nx * correction;
    a.ballPosition.y -= ny * correction;
    b.ballPosition.x += nx * correction;
    b.ballPosition.y += ny * correction;

    float rvx = b.ballVelocity.x - a.ballVelocity.x;
    float rvy = b.ballVelocity.y - a.ballVelocity.y;

    float velAlongNormal = rvx * nx + rvy * ny;

    if (velAlongNormal > 0) return;

    float e = 1.0f;

    float j = -(1 + e) * velAlongNormal;
    j /= 2.0f;

    float impulseX = j * nx;
    float impulseY = j * ny;

    a.ballVelocity.x -= impulseX;
    a.ballVelocity.y -= impulseY;
    b.ballVelocity.x += impulseX;
    b.ballVelocity.y += impulseY;
}

int main(){

    InitWindow(720, 1080, "Ball Duel");

    SetTargetFPS(60);
    ToggleFullscreen();
    std::vector<Ball> ballPit;

    int targetBallCount = 1000;

    for(int i = 0; i < targetBallCount; i++){
        ballPit.push_back(Ball(GetRandomScreenPos(), {GetRandomVelocityX(), 0}));
    }


    Grid spatialGrid(50);

    while (!WindowShouldClose())
    {

        float dt = GetFrameTime(); 

        //Event
        
        //Init Spatial Grid
        int solverIterations = 4;

        for(int i = 0; i < solverIterations; i++){
            spatialGrid.Clear();
            for(Ball& b : ballPit){
                spatialGrid.Insert(&b);
            }

            for (Ball& ball : ballPit) {
                std::vector<Ball*> neighbors = spatialGrid.Extract(&ball);
                for (Ball* other : neighbors) {
                    if (other <= &ball) continue;
                    ResolveCollision(ball, *other);
                }
            }
        }
        //Position
        for(Ball& ball : ballPit){
            ball.Update(dt);
        }

        //Draw
        BeginDrawing();
        ClearBackground(BLACK);
        for(Ball& ball : ballPit){
            ball.Draw();
        }
        EndDrawing();
    }
    

    return 0;
}