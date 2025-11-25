#pragma once
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <glm/glm.hpp>
#include <algorithm>
#include "Scenario.h"
#include "Texto.h"
#include "Billboard2D.h"
#include "InputDevices/KeyboardInput.h"
#include "D:\Documents\POO\PIA\FullDemoTemplateOGL-master\DemoTemplateOGL\DemoTemplateOGL\Base\Animator.h"
#include "D:\Documents\POO\PIA\FullDemoTemplateOGL-master\DemoTemplateOGL\DemoTemplateOGL\Base\Animation.h"

inline void ShowAlertWindow(const wchar_t* msg) {
#ifdef _WIN32
    MessageBoxW(NULL, msg, L"AstroFlag", MB_OK | MB_ICONINFORMATION);
#else
    std::wcout << L"[ALERT] " << msg << std::endl;
#endif 
}

// RADIO DE SPAWN DE ESTRELLAS Y ALIENS
static constexpr float STAR_MIN_RADIUS = 35.0f;   // Distancia mínima desde el spawn para estrellas
static constexpr float STAR_MAX_RADIUS = 85.0f;   // Distancia máxima para estrellas
static constexpr float ALIEN_MIN_RADIUS = 50.0f;   // Distancia mínima para aliens
static constexpr float ALIEN_MAX_RADIUS = 100.0f;   // Distancia máxima para aliens
//METEOROS
static constexpr double METEOR_INTERVAL_MS = 3000.0;   // cada 8 s
static constexpr double METEOR_DEBUG_FIRST_MS = 3000.0;   // primera lluvia = 3 s
static constexpr double METEOR_DURATION_MS = 10000.0;   // dura 6 s
static constexpr int    METEOR_COUNT = 50;
static constexpr float  METEOR_FALL_SPEED = 18.0f;    // unidades por segundo
static constexpr float  METEOR_MIN_RADIUS = 35.0f;
static constexpr float  METEOR_MAX_RADIUS = 85.0f;

struct AstroFlagState {
    enum Phase { Init, Playing, Win, Lose } phase = Init;
    int flagsPlaced = 0;
    int totalFlags = 4;
    int playerHits = 0;
};

class GameManager {
private:
    Scenario* scene = nullptr;
    Model* player = nullptr;
    Model* robot = nullptr;
    Model* medkit = nullptr;
    std::vector<Model*> stars;
    std::vector<Model*> flags;
    std::vector<Model*> aliens;
    AstroFlagState state;
    glm::vec3 spawnPoint{ 50.0f,0.0f,-30.0f };
    // Meteoritos
    double meteorTimerMs = 0.0;
    double meteorRainElapsedMs = 0.0;
    bool meteorActive = false;
    bool medkitSpawned = false;
    std::vector<Billboard*> meteorBillboards;
    float medkitPickupRadius = 3.0f;
    int maxLives = 3;

    Texto* hud = nullptr;
    std::wstring lastMessage = L"";
    float alienBaseSpeed = 2.0f;

public:
    void init(Scenario* sce, Model* playerModel, const glm::vec3& spawn) {
        scene = sce;
        player = playerModel;
        spawnPoint = spawn;
        std::srand((unsigned)std::time(nullptr));

        hud = nullptr;
        auto textos = scene->getLoadedText();
        if (textos->size() >= 2) hud = textos->at(1);
        if (!hud) {
            hud = new Texto((WCHAR*)L"", 20, 0, 10, 80, 0, player);
            hud->name = "GameHUD";
            textos->emplace_back(hud);
        }

        // Robot
        {
            glm::vec3 rPos = spawn + glm::vec3(0, 0, 10); 
            rPos.y = scene->getTerreno()->Superficie(rPos.x, rPos.z);
            robot = new Model("models/robot/robot.obj", player->cameraDetails, false, false);
            robot->setTranslate(&rPos);
            robot->setNextTranslate(&rPos);
            glm::vec3 rScale(10.0f);
            robot->setScale(&rScale);
            robot->setNextRotY(180);
            scene->getLoadedModels()->emplace_back(robot);
        }

        spawnAliens();

        ShowAlertWindow(L"Acércate y haz click al robot para empezar.");
        updateHUD();
        std::cout << "[AstroFlag] init\n";
    }

    void update(double dtMs) {
        if (!scene || !player) return;
        if (state.phase == AstroFlagState::Init) {
            robotClick();
        }
        else if (state.phase == AstroFlagState::Playing) {
            updateFlags();
            updateAliens(dtMs);
            updateMeteors(dtMs);
            updateMedkit();

        }
        updateHUD();
    }

    bool isGameOver() const {
        return state.phase == AstroFlagState::Win || state.phase == AstroFlagState::Lose;
    }

    void triggerMeteorRain() {
        if (!meteorActive) startRain();
    }

    bool isWin() const { return state.phase == AstroFlagState::Win; }
    bool isLose() const { return state.phase == AstroFlagState::Lose; }


private:
    glm::vec3 randomAnnulus(const glm::vec3& center, float minR, float maxR) {
        float angle = ((std::rand() % 10000) / 10000.f) * glm::pi<float>() * 2.0f;
        float r = minR + ((std::rand() % 10000) / 10000.f) * (maxR - minR);
        float x = center.x + r * std::sin(angle);
        float z = center.z + r * std::cos(angle);
        return { x, center.y, z };
    }

    void spawnStars() {
        for (int i = 0; i < state.totalFlags; i++) {
            glm::vec3 p = randomAnnulus(spawnPoint, STAR_MIN_RADIUS, STAR_MAX_RADIUS);
            p.y = scene->getTerreno()->Superficie(p.x, p.z);
            Model* star = new Model("models/star/star.fbx", player->cameraDetails);
            star->setTranslate(&p);
            star->setNextTranslate(&p);
            stars.emplace_back(star);
            scene->getLoadedModels()->emplace_back(star);
        }
    }

    void spawnAliens() {
        const char* walkPath = "models/alien/Walking.fbx"; 
        const int numAliens = 3;

        for (int i = 0; i < numAliens; i++) {
            glm::vec3 ap = randomAnnulus(spawnPoint, ALIEN_MIN_RADIUS, ALIEN_MAX_RADIUS);
            ap.y = scene->getTerreno()->Superficie(ap.x, ap.z) + 1.5f;

            Model* alien = new Model(walkPath, player->cameraDetails);
            alien->setTranslate(&ap);
            alien->setNextTranslate(&ap);

            glm::vec3 sc(0.0005); 
            alien->setScale(&sc);
            glm::vec3 forward = *player->getTranslate() - ap;
            float angDeg = glm::degrees(std::atan2(forward.x, forward.z));
            alien->setRotY(angDeg);
            alien->setNextRotY(angDeg);

            try {
                std::vector<Animation> animations =
                    Animation::loadAllAnimations(walkPath,
                        alien->GetBoneInfoMap(),
                        alien->getBonesInfo(),
                        alien->GetBoneCount());

                for (Animation& clip : animations) {
                    alien->setAnimator(Animator(clip));
                }

                alien->setAnimation(0);
            }
            catch (...) {
                ERRORL("No se pudo cargar la animación de Walking.fbx", "ANIMACION");
            }

            alien->setActive(true);
            aliens.emplace_back(alien);
            scene->getLoadedModels()->emplace_back(alien);
        }
    }

    void removeCurrentStars() {
        auto& models = *scene->getLoadedModels();
        for (auto* star : stars) {
            if (!star) continue;
            auto it = std::find(models.begin(), models.end(), star);
            if (it != models.end()) models.erase(it);
            delete star;
        }
        stars.clear();
    }

    void removeCurrentAliens() {
        auto& models = *scene->getLoadedModels();
        for (auto* alien : aliens) {
            if (!alien) continue;
            auto it = std::find(models.begin(), models.end(), alien);
            if (it != models.end()) models.erase(it);
            delete alien;
        }
        aliens.clear();
    }

    void robotClick() {
        if (!robot) return;
        float d = glm::length(*player->getTranslate() - *robot->getTranslate());
        if (d < 10.0f) { 
            if (cDelta.getLbtn()) {
                state.phase = AstroFlagState::Playing;
                ShowAlertWindow(L"Hola Bob! Recuerda colocar todas las banderas antes de que los aliens nos atrapen, sigue las estrellas del terreno.");
                extern bool KEYS[256]; // limpia el estado de teclas para que no se atoren
                for (int i = 0; i < 256; ++i) KEYS[i] = false;
                cDelta.setLbtn(false);
                // Estrellas al iniciar el juego
                spawnStars(); 
                //meddkit
                spawnMedkit();

            }
        }
    }

    void updateFlags() {
        for (size_t i = 0; i < stars.size(); i++) {
            Model* star = stars[i];
            if (!star) continue;
            float d = glm::length(*player->getTranslate() - *star->getTranslate());
            if (d < 3.0f) {
                glm::vec3 pos = *star->getTranslate();
                // Eliminar estrella
                if (!star->getModelAttributes()->empty()) {
                    Model* AABB = (Model*)star->getModelAttributes()->at(0).hitbox;
                    if (AABB) delete AABB;
                    star->getModelAttributes()->at(0).hitbox = NULL;
                }
                star->setActive(false);
                auto& models = *scene->getLoadedModels();
                auto it = std::find(models.begin(), models.end(), star);
                if (it != models.end()) models.erase(it);
                delete star;
                stars[i] = nullptr;

                // Crear bandera
                Model* flag = new Model("models/bandera/flag2.obj", player->cameraDetails, false, false);
                flag->setTranslate(&pos);
                flag->setNextTranslate(&pos);
                glm::vec3 sc(0.5f);
                flag->setScale(&sc);
                scene->getLoadedModels()->emplace_back(flag);
                flags.emplace_back(flag);

                state.flagsPlaced++;
                ShowAlertWindow(L"¡Bandera colocada!");
                extern bool KEYS[256]; // limpia el estado de teclas para que no se atoren
                for (int i = 0; i < 256; ++i) KEYS[i] = false; 
                cDelta.setLbtn(false);
                if (state.flagsPlaced >= state.totalFlags) {
                    state.phase = AstroFlagState::Win;
                    ShowAlertWindow(L"¡HAS GANADO! Todas las banderas colocadas.");
                    extern bool KEYS[256]; // limpia el estado de teclas para que no se atoren
                    for (int i = 0; i < 256; ++i) KEYS[i] = false;
                    cDelta.setLbtn(false);
                    return;
                }
            }
        }
    }

    void updateAliens(double dtMs) {
        float speedMult = 1.0f + 0.15f * state.flagsPlaced;
        float dt = (float)dtMs / 1000.f;
        glm::vec3 playerPos = *player->getTranslate();
        for (Model* alien : aliens) {
            glm::vec3 pos = *alien->getTranslate();
            glm::vec3 dir = playerPos - pos;
            float dist = glm::length(dir);
            if (dist > 0.001f) dir /= dist;
            pos += dir * (alienBaseSpeed * speedMult) * dt;
            pos.y = scene->getTerreno()->Superficie(pos.x, pos.z);
            alien->setTranslate(&pos);
            alien->setNextTranslate(&pos);

            // Rotar alien para mirar al jugador
            glm::vec3 forward = playerPos - pos;
            float angDeg = glm::degrees(std::atan2(forward.x, forward.z));
            alien->setNextRotY(angDeg);

            if (dist < 3.0f) {
                state.playerHits++;
                if (state.playerHits >= 3) {
                    state.phase = AstroFlagState::Lose;
                    ShowAlertWindow(L"Has perdido. Los aliens te atraparon.");
                    extern bool KEYS[256]; // limpia el estado de teclas para que no se atoren
                    for (int i = 0; i < 256; ++i) KEYS[i] = false;
                    cDelta.setLbtn(false);
                    return;
                }
                else {
                    ShowAlertWindow(L"¡Has recibido daño! Regresando al spawn.");
                    extern bool KEYS[256]; // limpia el estado de teclas para que no se atoren
                    for (int i = 0; i < 256; ++i) KEYS[i] = false;
                    cDelta.setLbtn(false);
                    glm::vec3 reset = spawnPoint;
                    reset.y = scene->getTerreno()->Superficie(reset.x, reset.z);
                    player->setTranslate(&reset);
                    player->setNextTranslate(&reset);
                    // Reubicar aliens lejos otra vez
                    for (Model* a2 : aliens) {
                        glm::vec3 r = randomAnnulus(spawnPoint, ALIEN_MIN_RADIUS, ALIEN_MAX_RADIUS);
                        r.y = scene->getTerreno()->Superficie(r.x, r.z);
                        a2->setTranslate(&r);
                        a2->setNextTranslate(&r);
                    }
                }
                break;
            }
        }
    }

    void updateMeteors(double dtMs) {
        if (state.phase != AstroFlagState::Playing) return; // Solo en juego activo
        meteorTimerMs += dtMs;
        if (!meteorActive && meteorTimerMs >= METEOR_INTERVAL_MS) {
            startRain();
        }
        if (meteorActive) {
            float dtSec = (float)dtMs / 1000.0f;
            meteorRainElapsedMs += dtMs;
            for (auto* b : meteorBillboards) {
                if (!b) continue;
                glm::vec3 pos = *b->getTranslate();
                pos.y -= METEOR_FALL_SPEED * dtSec;
                b->setTranslate(&pos);
                b->setNextTranslate(&pos);
                if (pos.y < -5.0f) {
                    b->setActive(false);
                }
            }
            if (meteorRainElapsedMs >= METEOR_DURATION_MS) {
                stopRain();
            }
        }
    }


    void startRain() {
        meteorActive = true;
        meteorRainElapsedMs = 0.0;
        meteorBillboards.clear();
        for (int i = 0; i < METEOR_COUNT; i++) {
            glm::vec3 p = randomAnnulus(spawnPoint, METEOR_MIN_RADIUS, METEOR_MAX_RADIUS);
            p.y = 35.f + (std::rand() % 800) / 40.f;
            Billboard* m = new Billboard((WCHAR*)L"billboards/meteoro.png", 3.f, 3.f, p.x, p.y, p.z, player->cameraDetails);
            meteorBillboards.emplace_back(m);
            scene->getLoadedBillboards()->emplace_back(m);
        }
        setMessage(L"¡Lluvia de meteoritos!");
    }

    void stopRain() {
        meteorActive = false;
        meteorTimerMs = 0.0;
        for (auto* b : meteorBillboards) {
            if (b) b->setActive(false);
        }
        meteorBillboards.clear();
        setMessage(L"Fin de la lluvia de meteoritos.");
    }

    void updateHUD() {
        if (!hud) return;
        wchar_t buf[256];
        swprintf(buf, 256, L"Banderas: %d/%d  Vidas: %d/3",
            state.flagsPlaced, state.totalFlags, 3 - state.playerHits);
        hud->initTexto(buf);
    }

    void setMessage(const wchar_t* msg) {
        lastMessage = msg;
    }

    // medkit vida 
    void spawnMedkit() {
        if (medkitSpawned || !scene) return;

        Model* floor = nullptr;
        for (Model* m : *scene->getLoadedModels()) {
            if (m && m->name == "sueloNave") {
                floor = m;
                break;
            }
        }
        glm::vec3 estimatedFloorPos;
        if (!floor) {
            float baseY = scene->getTerreno()->Superficie(60.0f, 20.0f) - 1.0f;
            estimatedFloorPos = glm::vec3(60.0f, baseY, 20.0f);
        }

        glm::vec3 basePos = floor ? *floor->getTranslate() : estimatedFloorPos;
        basePos.y += 2.0f; 
        basePos.x += 1.0f;
        basePos.z += 0.5f;

        // Crear medkit
        medkit = new Model("models/medkit/medkit.obj", player->cameraDetails, false, false);
        medkit->name = "Medkit";
        medkit->setTranslate(&basePos);
        medkit->setNextTranslate(&basePos);
        glm::vec3 mkScale(2.0f, 2.0f, 2.0f);
        medkit->setScale(&mkScale);
        medkit->setActive(true);
        scene->getLoadedModels()->emplace_back(medkit);
        medkitSpawned = true;
    }

    void removeMedkit() {
        if (!medkit) return;
        auto& models = *scene->getLoadedModels();
        auto it = std::find(models.begin(), models.end(), medkit);
        if (it != models.end()) models.erase(it);
        delete medkit;
        medkit = nullptr;
        medkitSpawned = false;
    }

    void updateMedkit() {
        if (!medkit || !medkit->getActive()) return;
        glm::vec3 playerPos = *player->getTranslate();
        glm::vec3 mkPos = *medkit->getTranslate();
        float dist = glm::length(playerPos - mkPos);
        if (dist < medkitPickupRadius) {
            if (state.playerHits > 0) {
                state.playerHits = std::max(0, state.playerHits - 1);
                ShowAlertWindow(L"¡Has recogido el botiquín! Vida +1.");
                extern bool KEYS[256]; // limpia el estado de teclas para que no se atoren
                for (int i = 0; i < 256; ++i) KEYS[i] = false;
                cDelta.setLbtn(false);
            }
            else {
                ShowAlertWindow(L"¡Botiquín recogido! Ya estabas a vida completa.");
                extern bool KEYS[256]; // limpia el estado de teclas para que no se atoren
                for (int i = 0; i < 256; ++i) KEYS[i] = false;
                cDelta.setLbtn(false);
            }
            removeMedkit();
        }
    }

};