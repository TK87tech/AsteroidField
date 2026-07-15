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

#include "GameScene.h"
#include <cmath>

USING_NS_CC;

static float deg2rad(float d) { return d * (float)M_PI / 180.0f; }
static Vec2 fromAngle(float deg) { return Vec2(std::cos(deg2rad(deg)), std::sin(deg2rad(deg))); }

Scene* GameScene::createScene() { return GameScene::create(); }

bool GameScene::init() {
    if (!Scene::init()) return false;

    _gfx = DrawNode::create(); addChild(_gfx, 1);
    _hud = DrawNode::create(); addChild(_hud, 10);

    _scoreLbl = Label::createWithSystemFont("SCORE: 0", "Arial", 20);
    _scoreLbl->setAnchorPoint(Vec2(0, 1));
    _scoreLbl->setPosition(10, WIN_H - 8);
    addChild(_scoreLbl, 10);

    _waveLbl = Label::createWithSystemFont("WAVE: 1", "Arial", 20);
    _waveLbl->setAnchorPoint(Vec2(0.5f, 1));
    _waveLbl->setPosition(WIN_W * 0.5f, WIN_H - 8);
    addChild(_waveLbl, 10);

    _overLbl = Label::createWithSystemFont("GAME OVER", "Arial", 48);
    _overLbl->setPosition(WIN_W * 0.5f, WIN_H * 0.5f + 20);
    _overLbl->setVisible(false);
    addChild(_overLbl, 10);

    _restartLbl = Label::createWithSystemFont("PRESS R TO RESTART", "Arial", 22);
    _restartLbl->setPosition(WIN_W * 0.5f, WIN_H * 0.5f - 30);
    _restartLbl->setVisible(false);
    addChild(_restartLbl, 10);

    auto kb = EventListenerKeyboard::create();
    kb->onKeyPressed = CC_CALLBACK_2(GameScene::onKeyPressed, this);
    kb->onKeyReleased = CC_CALLBACK_2(GameScene::onKeyReleased, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(kb, this);

    startGame();
    scheduleUpdate();
    return true;
}

void GameScene::startGame() {
    _asteroids.clear();
    _bullets.clear();
    _score = 0; _lives = 3; _wave = 1;
    _gameOver = false; _shipAlive = true;
    _respawnTimer = 0; _invuln = 0; _fireTimer = 0; _waveTimer = 0;
    _ship.pos = Vec2(WIN_W * 0.5f, WIN_H * 0.5f);
    _ship.vel = Vec2::ZERO;
    _ship.angle = 90.0f;
    _scoreLbl->setString("SCORE: 0");
    _waveLbl->setString("WAVE: 1");
    _overLbl->setVisible(false);
    _restartLbl->setVisible(false);
    spawnWave(4);
}

void GameScene::spawnWave(int count) {
    for (int i = 0; i < count; ++i) {
        Vec2 p;
        do { p = Vec2(random(0.0f, WIN_W), random(0.0f, WIN_H)); } while (p.distance(_ship.pos) < SAFE_RADIUS);
        spawnAsteroid(0, p);
    }
}

void GameScene::spawnAsteroid(int size, Vec2 at) {
    Asteroid a;
    a.size = size;
    a.pos = at;
    a.vel = fromAngle(random(0.0f, 360.0f)) * AST_SPEED[size];
    float r = AST_RADIUS[size];
    int verts = 10;
    for (int i = 0; i < verts; ++i) {
        float ang = (360.0f / verts) * i;
        a.shape.push_back(fromAngle(ang) * (r * random(0.75f, 1.1f)));
    }
    _asteroids.push_back(a);
}

void GameScene::fire() {
    if (_fireTimer > 0 || (int)_bullets.size() >= MAX_BULLETS) return;
    _fireTimer = FIRE_CD;
    Bullet b;
    b.pos = _ship.pos + fromAngle(_ship.angle) * 14.0f;
    b.vel = fromAngle(_ship.angle) * BULLET_SPD;
    b.life = 0.0f;
    _bullets.push_back(b);
}

void GameScene::killShip() {
    _lives--;
    _shipAlive = false;
    _respawnTimer = RESPAWN_DELAY;
    if (_lives <= 0) {
        _gameOver = true;
        _overLbl->setVisible(true);
        _restartLbl->setVisible(true);
    }
}

void GameScene::wrap(Vec2& p) {
    if (p.x < 0) p.x += WIN_W; else if (p.x > WIN_W) p.x -= WIN_W;
    if (p.y < 0) p.y += WIN_H; else if (p.y > WIN_H) p.y -= WIN_H;
}

void GameScene::onKeyPressed(EventKeyboard::KeyCode c, Event*) {
    using K = EventKeyboard::KeyCode;
    if (c == K::KEY_LEFT_ARROW)  _kL = true;
    if (c == K::KEY_RIGHT_ARROW) _kR = true;
    if (c == K::KEY_UP_ARROW)    _kU = true;
    if (c == K::KEY_SPACE && !_gameOver) fire();
    if (c == K::KEY_R && _gameOver) startGame();
}

void GameScene::onKeyReleased(EventKeyboard::KeyCode c, Event*) {
    using K = EventKeyboard::KeyCode;
    if (c == K::KEY_LEFT_ARROW)  _kL = false;
    if (c == K::KEY_RIGHT_ARROW) _kR = false;
    if (c == K::KEY_UP_ARROW)    _kU = false;
}

void GameScene::update(float dt) {
    if (_gameOver) { drawFrame(); return; }

    if (_fireTimer > 0) _fireTimer -= dt;

    if (!_shipAlive) {
        _respawnTimer -= dt;
        if (_respawnTimer <= 0) {
            _shipAlive = true;
            _invuln = INVULN_TIME;
            _ship.pos = Vec2(WIN_W * 0.5f, WIN_H * 0.5f);
            _ship.vel = Vec2::ZERO;
            _ship.angle = 90.0f;
        }
    }
    if (_invuln > 0) _invuln -= dt;

    if (_shipAlive) updateShip(dt);
    updateBullets(dt);
    updateAsteroids(dt);
    checkHits();

    if (_asteroids.empty()) {
        if (_waveTimer <= 0) _waveTimer = 2.0f;
        _waveTimer -= dt;
        if (_waveTimer <= 0) {
            _wave++;
            _waveLbl->setString(StringUtils::format("WAVE: %d", _wave));
            spawnWave(3 + _wave);
            _waveTimer = 0;
        }
    }

    drawFrame();
}

void GameScene::updateShip(float dt) {
    if (_kL) _ship.angle += SHIP_TURN * dt;
    if (_kR) _ship.angle -= SHIP_TURN * dt;
    if (_kU) {
        _ship.vel += fromAngle(_ship.angle) * SHIP_ACCEL * dt;
        if (_ship.vel.length() > SHIP_MAXSPD)
            _ship.vel = _ship.vel.getNormalized() * SHIP_MAXSPD;
    }
    _ship.pos += _ship.vel * dt;
    wrap(_ship.pos);
}

void GameScene::updateBullets(float dt) {
    for (auto it = _bullets.begin(); it != _bullets.end(); ) {
        it->life += dt;
        it->pos += it->vel * dt;
        if (it->life >= BULLET_LIFE) it = _bullets.erase(it);
        else ++it;
    }
}

void GameScene::updateAsteroids(float dt) {
    for (auto& a : _asteroids) {
        a.pos += a.vel * dt;
        wrap(a.pos);
    }
}

void GameScene::checkHits() {
    for (auto bit = _bullets.begin(); bit != _bullets.end(); ) {
        bool hit = false;
        for (auto ait = _asteroids.begin(); ait != _asteroids.end(); ++ait) {
            if (bit->pos.distance(ait->pos) <= AST_RADIUS[ait->size]) {
                _score += AST_SCORE[ait->size];
                _scoreLbl->setString(StringUtils::format("SCORE: %d", _score));
                int sz = ait->size;
                Vec2 at = ait->pos;
                _asteroids.erase(ait);
                if (sz < 2) { spawnAsteroid(sz + 1, at); spawnAsteroid(sz + 1, at); }
                hit = true;
                break;
            }
        }
        if (hit) bit = _bullets.erase(bit);
        else ++bit;
    }

    if (_shipAlive && _invuln <= 0) {
        for (auto& a : _asteroids) {
            if (_ship.pos.distance(a.pos) <= AST_RADIUS[a.size] + 8.0f) {
                killShip();
                break;
            }
        }
    }
}

void GameScene::drawFrame() {
    _gfx->clear();
    _hud->clear();

    for (auto& a : _asteroids) {
        int n = (int)a.shape.size();
        for (int i = 0; i < n; ++i)
            _gfx->drawLine(a.pos + a.shape[i], a.pos + a.shape[(i + 1) % n], Color4F::WHITE);
    }

    for (auto& b : _bullets)
        _gfx->drawDot(b.pos, 2.0f, Color4F::WHITE);

    bool showShip = _shipAlive && !(_invuln > 0 && ((int)(_invuln * 10) % 2 == 0));
    if (showShip) {
        Vec2 nose = _ship.pos + fromAngle(_ship.angle) * 14.0f;
        Vec2 l = _ship.pos + fromAngle(_ship.angle + 130) * 12.0f;
        Vec2 r = _ship.pos + fromAngle(_ship.angle - 130) * 12.0f;
        _gfx->drawLine(nose, l, Color4F::WHITE);
        _gfx->drawLine(l, r, Color4F::WHITE);
        _gfx->drawLine(r, nose, Color4F::WHITE);
    }

    for (int i = 0; i < _lives; ++i) {
        Vec2 c(WIN_W - 30 - i * 28.0f, WIN_H - 22);
        _hud->drawLine(c + Vec2(0, 10), c + Vec2(-8, -8), Color4F::WHITE);
        _hud->drawLine(c + Vec2(-8, -8), c + Vec2(8, -8), Color4F::WHITE);
        _hud->drawLine(c + Vec2(8, -8), c + Vec2(0, 10), Color4F::WHITE);
    }
}