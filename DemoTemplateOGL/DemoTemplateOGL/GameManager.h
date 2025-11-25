#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "Base/model.h"
#include "Texto.h"
#include "Scenario.h"
#include "InputDevices/KeyboardInput.h" // por cDelta (mouse)
#include <cstdlib>
#include <ctime>

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
    std::vector<Model*> stars;      // estrellas iniciales
    std::vector<Model*> flags;      // banderas colocadas
    std::vector<Model*> aliens;     // aliens
    AstroFlagState state;
    glm::vec3 spawnPoint{ 5.0f, 0.0f, -5.0f };
    double meteorTimerMs = 0.0;
    double meteorIntervalMs = 30000.0; // 30 segundos
    bool meteorActive = false;
    double meteorRainElapsedMs = 0.0;
    double meteorRainDurationMs = 6000.0;
    std::vector<Billboard2D*> meteorBillboards;
    Texto* messageText = nullptr;

    // Configuración
    float alienBaseSpeed = 2.0f; // unidades / segundo
    float alienChaseRadius = 150.0f;
    float collisionDistancePlayerAlien = 3.0f;
    float collisionDistancePlayerStar = 3.0f;

public:
    void init(Scenario* sce, Model* playerModel, const glm::vec3& spawn) {
        scene = sce;
        player = playerModel;
        spawnPoint = spawn;
        std::srand((unsigned)std::time(nullptr));

        // Texto para mensajes (HUD)
        messageText = new Texto((WCHAR*)L"", 18, 0, 10, 50, 0, player);
        messageText->name = "GameMessages";
        scene->getLoadedText()->emplace_back(messageText);

        // Robot (colocado frente al jugador)
        glm::vec3 robotPos = spawn + glm::vec3(0.0f, 0.0f, 8.0f);
        robot = new Model("models/robot/robot.fbx", robotPos, player->cameraDetails);
        robot->setTranslate(&robotPos);
        scene->getLoadedModels()->emplace_back(robot);

        // Estrellas iniciales
        for (int i = 0; i < state.totalFlags; ++i) {
            glm::vec3 pos = randomNear(spawnPoint, 25.0f);
            pos.y = scene->getTerreno()->Superficie(pos.x, pos.z);
            Model* star = new Model("models/star/star.fbx", pos, player->cameraDetails);
            star->setTranslate(&pos);
            scene->getLoadedModels()->emplace_back(star);
            stars.emplace_back(star);
        }

        // Aliens
        const std::string alienModels[3] = {
            "models/alien/hiphop(1).fbx",
            "models/alien/breakdance.fbx",
            "models/alien/Breakdance2.fbx"
        };
        for (int i = 0; i < 3; ++i) {
            glm::vec3 apos = randomNear(spawnPoint, 35.0f);
            apos.y = scene->getTerreno()->Superficie(apos.x, apos.z);
            Model* alien = new Model(alienModels[i], apos, player->cameraDetails);
            alien->setTranslate(&apos);
            glm::vec3 scale(1.8f);
            alien->setScale(&scale);
            scene->getLoadedModels()->emplace_back(alien);
            aliens.emplace_back(alien);
        }
        updateHUD();
    }

    void update(double dtMs) {
        if (!scene || !player) return;

        if (state.phase == AstroFlagState::Init) {
            handleRobotClick();
        }
        if (state.phase == AstroFlagState::Playing) {
            updateFlags(dtMs);
            updateAliens(dtMs);
            updateMeteors(dtMs);
        }
        if (state.phase == AstroFlagState::Win) {
            setMessage(L"¡HAS GANADO! Todas las banderas colocadas.");
        }
        if (state.phase == AstroFlagState::Lose) {
            setMessage(L"Has perdido. Los aliens te atraparon.");
        }
    }

    bool isGameOver() const {
        return state.phase == AstroFlagState::Win || state.phase == AstroFlagState::Lose;
    }
    bool isWin() const { return state.phase == AstroFlagState::Win; }
    bool isLose() const { return state.phase == AstroFlagState::Lose; }

private:
    glm::vec3 randomNear(const glm::vec3& center, float radiusXZ) {
        float rx = ((std::rand() % 20000) / 10000.f - 1.f) * radiusXZ;
        float rz = ((std::rand() % 20000) / 10000.f - 1.f) * radiusXZ;
        return glm::vec3(center.x + rx, center.y, center.z + rz);
    }

    void handleRobotClick() {
        // Simulación de click en robot: si LMB presionado y distancia < 6
        if (cDelta.getLbtn()) {
            float d = glm::length(*player->getTranslate() - *robot->getTranslate());
            if (d < 6.0f) {
                setMessage(L"Hola Bob! Coloca las banderas siguiendo las estrellas antes de que los aliens te atrapen.");
                state.phase = AstroFlagState::Playing;
            }
        }
    }

    void updateFlags(double /*dt*/) {
        // Colisión jugador - estrellas
        for (size_t i = 0; i < stars.size(); ++i) {
            Model* star = stars[i];
            if (!star) continue;
            float d = glm::length(*player->getTranslate() - *star->getTranslate());
            if (d < collisionDistancePlayerStar) {
                // Convertir a bandera
                glm::vec3 pos = *star->getTranslate();
                // removemos star visual (no la borramos del vector de escena para mantener integridad, solo setActive false)
                star->setActive(false);
                stars[i] = nullptr;

                Model* flag = new Model("models/flag/flag2.obj", pos, player->cameraDetails);
                flag->setTranslate(&pos);
                glm::vec3 sc(1.5f);
                flag->setScale(&sc);
                scene->getLoadedModels()->emplace_back(flag);
                flags.emplace_back(flag);

                state.flagsPlaced++;
                setMessage(L"Bandera colocada!");
                updateHUD();
                if (state.flagsPlaced >= state.totalFlags) {
                    state.phase = AstroFlagState::Win;
                    return;
                }
            }
        }
    }

    void updateAliens(double dtMs) {
        float speedMultiplier = 1.0f + 0.15f * state.flagsPlaced;
        float dtSec = static_cast<float>(dtMs) / 1000.0f;
        glm::vec3 playerPos = *player->getTranslate();

        for (Model* alien : aliens) {
            if (!alien || !alien->getActive()) continue;
            glm::vec3 pos = *alien->getTranslate();
            glm::vec3 dir = playerPos - pos;
            float dist = glm::length(dir);
            if (dist < alienChaseRadius && dist > 0.001f) {
                dir /= dist;
                pos += dir * alienBaseSpeed * speedMultiplier * dtSec;
                // Ajustar altura al terreno
                pos.y = scene->getTerreno()->Superficie(pos.x, pos.z);
                alien->setTranslate(&pos);
                alien->setNextTranslate(&pos);
            }
            // Colisión
            if (dist < collisionDistancePlayerAlien) {
                state.playerHits++;
                if (state.playerHits >= 3) {
                    state.phase = AstroFlagState::Lose;
                    return;
                }
                else {
                    setMessage(L"¡Has recibido daño! Vuelve al inicio.");
                    // Reset jugador
                    glm::vec3 resetPos = spawnPoint;
                    resetPos.y = scene->getTerreno()->Superficie(resetPos.x, resetPos.z);
                    player->setTranslate(&resetPos);
                    player->setNextTranslate(&resetPos);
                    // Reubicar aliens
                    for (Model* a2 : aliens) {
                        if (!a2) continue;
                        glm::vec3 r = randomNear(spawnPoint, 35.0f);
                        r.y = scene->getTerreno()->Superficie(r.x, r.z);
                        a2->setTranslate(&r);
                        a2->setNextTranslate(&r);
                    }
                    updateHUD();
                }
                break;
            }
        }
    }

    void updateMeteors(double dtMs) {
        meteorTimerMs += dtMs;
        if (!meteorActive && meteorTimerMs >= meteorIntervalMs) {
            startMeteorRain();
        }
        if (meteorActive) {
            meteorRainElapsedMs += dtMs;
            // Actualizar caída
            for (auto* b : meteorBillboards) {
                if (!b) continue;
                glm::vec3 pos = *b->getTranslate();
                pos.y -= 0.12f * static_cast<float>(dtMs); // caída proporcional al tiempo (0.12 * ms ~ 72 u/s)
                b->setTranslate(&pos);
                b->setNextTranslate(&pos);
                if (pos.y < -5.0f) {
                    b->setActive(false);
                }
            }
            if (meteorRainElapsedMs >= meteorRainDurationMs) {
                stopMeteorRain();
            }
        }
    }

    void startMeteorRain() {
        meteorActive = true;
        meteorRainElapsedMs = 0.0;
        meteorBillboards.clear();
        // Generar lluvia (billboards 2D)
        for (int i = 0; i < 50; ++i) {
            glm::vec3 p(
                spawnPoint.x + ((std::rand() % 20000) / 10000.f - 1.f) * 60.0f,
                35.0f + (std::rand() % 1000) / 100.0f,
                spawnPoint.z + ((std::rand() % 20000) / 10000.f - 1.f) * 60.0f
            );
            Billboard2D* m = new Billboard2D((WCHAR*)L"textures/meteor.png", 4.0f, 4.0f, p.x, p.y, p.z, player->cameraDetails);
            meteorBillboards.emplace_back(m);
            scene->getLoadedBillboards2D()->emplace_back(m);
        }
        setMessage(L"Lluvia de meteoritos!");
    }

    void stopMeteorRain() {
        meteorActive = false;
        meteorTimerMs = 0.0;
        for (auto* b : meteorBillboards) {
            if (b) b->setActive(false);
        }
        meteorBillboards.clear();
        setMessage(L"Fin de la lluvia de meteoritos");
    }

    void updateHUD() {
        wchar_t buffer[256];
        swprintf(buffer, 256, L"Banderas: %d/%d  Vidas: %d/3",
            state.flagsPlaced, state.totalFlags, 3 - state.playerHits);
        if (messageText) {
            messageText->initTexto(buffer);
        }
    }

    void setMessage(const wchar_t* msg) {
        if (!messageText) return;
        // concatenar estado principal arriba
        wchar_t base[512];
        swprintf(base, 512, L"%s", msg);
        messageText->initTexto(base);
    }
};