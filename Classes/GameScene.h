/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "cocos2d.h"
#include <vector>

 // ---- window / tuning constants ----
static const float WIN_W = 800.0f, WIN_H = 600.0f;
static const float SHIP_TURN = 180.0f;   // deg/sec
static const float SHIP_ACCEL = 200.0f;  // px/s^2
static const float SHIP_MAXSPD = 350.0f;
static const float BULLET_SPD = 500.0f;
static const float BULLET_LIFE = 1.2f;
static const float FIRE_CD = 0.25f;
static const int   MAX_BULLETS = 4;
static const float RESPAWN_DELAY = 1.5f;
static const float INVULN_TIME = 2.0f;
static const float SAFE_RADIUS = 100.0f;

// asteroid size table: 0=large, 1=medium, 2=small
static const float AST_RADIUS[3] = { 40.0f, 24.0f, 12.0f };
static const float AST_SPEED[3] = { 60.0f, 90.0f, 120.0f };
static const int   AST_SCORE[3] = { 20, 50, 100 };

struct Ship {
    cocos2d::Vec2 pos, vel;
    float angle = 90.0f;   // degrees, 90 = up
};

struct Asteroid {
    cocos2d::Vec2 pos, vel;
    int size = 0;
    std::vector<cocos2d::Vec2> shape; // rough polygon offsets
};

struct Bullet {
    cocos2d::Vec2 pos, vel;
    float life = 0.0f;
};

class GameScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    CREATE_FUNC(GameScene);
    void update(float dt) override;

private:
    void startGame();
    void spawnWave(int count);
    void spawnAsteroid(int size, cocos2d::Vec2 at);
    void fire();
    void killShip();
    void wrap(cocos2d::Vec2& p);

    void onKeyPressed(cocos2d::EventKeyboard::KeyCode c, cocos2d::Event*);
    void onKeyReleased(cocos2d::EventKeyboard::KeyCode c, cocos2d::Event*);

    void updateShip(float dt);
    void updateBullets(float dt);
    void updateAsteroids(float dt);
    void checkHits();
    void drawFrame();

    cocos2d::DrawNode* _gfx = nullptr;
    cocos2d::DrawNode* _hud = nullptr;
    cocos2d::Label* _scoreLbl = nullptr;
    cocos2d::Label* _waveLbl = nullptr;
    cocos2d::Label* _overLbl = nullptr;
    cocos2d::Label* _restartLbl = nullptr;

    Ship _ship;
    std::vector<Asteroid> _asteroids;
    std::vector<Bullet> _bullets;
    int _score = 0, _lives = 3, _wave = 1;
    bool _gameOver = false;
    bool _shipAlive = true;
    float _respawnTimer = 0.0f;
    float _invuln = 0.0f;
    float _fireTimer = 0.0f;
    float _waveTimer = 0.0f;

    bool _kL = false, _kR = false, _kU = false;
};

#endif