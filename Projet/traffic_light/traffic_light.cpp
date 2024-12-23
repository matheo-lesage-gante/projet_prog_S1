#include <SFML/Graphics.hpp>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>
#include <iostream> // Pour afficher des erreurs �ventuelles
#include <random>


// Constantes globales
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// D�finition des �tats du feu de circulation
enum TrafficLightState {
    RedHorizontal,
    GreenHorizontal,
    OrangeHorizontal,
    RedHorizontalOrangeVertical
};

// Classe TrafficLight
class TrafficLight {
private:
    TrafficLightState state;
    sf::RectangleShape lightHorizontalLeft;
    sf::RectangleShape lightHorizontalLeftLeft;
    sf::RectangleShape lightHorizontalRight;
    sf::RectangleShape lightHorizontalRightRight;
    sf::RectangleShape lightVerticalTop;
    sf::RectangleShape lightVerticalBottom;

    sf::Clock stateClock;
    sf::Time stateDuration;
    std::mutex trafficMutex;

public:
    // Constructeur
    TrafficLight() : state(RedHorizontal) {
        // Feu pour les v�hicules venant de gauche
        lightHorizontalLeft.setSize(sf::Vector2f(20, 20));
        lightHorizontalLeft.setPosition(180, 430);
        lightHorizontalLeft.setFillColor(sf::Color::Red);

        // Feu pour les v�hicules venant de droite
        lightHorizontalRight.setSize(sf::Vector2f(20, 20));
        lightHorizontalRight.setPosition(600, 150);
        lightHorizontalRight.setFillColor(sf::Color::Red);

        // Feu pour les v�hicules venant du haut
        lightVerticalTop.setSize(sf::Vector2f(20, 20));
        lightVerticalTop.setPosition(220, 110);
        lightVerticalTop.setFillColor(sf::Color::Green);

        // Feu pour les v�hicules venant du bas
        lightVerticalBottom.setSize(sf::Vector2f(20, 20));
        lightVerticalBottom.setPosition(560, 470);
        lightVerticalBottom.setFillColor(sf::Color::Green);

        lightHorizontalLeftLeft.setSize(sf::Vector2f(20, 20));
        lightHorizontalLeftLeft.setPosition(80, 430);
        lightHorizontalLeftLeft.setFillColor(sf::Color::Red);

        lightHorizontalRightRight.setSize(sf::Vector2f(20, 20));
        lightHorizontalRightRight.setPosition(700, 150);
        lightHorizontalRightRight.setFillColor(sf::Color::Red);

        stateDuration = sf::seconds(30); // Dur�e initiale
    }

    // Met � jour l'�tat du feu
    void update() {
        if (stateClock.getElapsedTime() >= stateDuration) {
            std::lock_guard<std::mutex> lock(trafficMutex);
            switch (state) {
            case RedHorizontal:
                state = GreenHorizontal;
                lightHorizontalLeft.setFillColor(sf::Color::Green);
                lightHorizontalLeftLeft.setFillColor(sf::Color::Green);
                lightHorizontalRight.setFillColor(sf::Color::Green);
                lightHorizontalRightRight.setFillColor(sf::Color::Green);
                lightVerticalTop.setFillColor(sf::Color::Red);
                lightVerticalBottom.setFillColor(sf::Color::Red);
                stateDuration = sf::seconds(30);
                break;

            case GreenHorizontal:
                state = OrangeHorizontal;
                lightHorizontalLeft.setFillColor(sf::Color(255, 165, 0)); // Orange
                lightHorizontalLeftLeft.setFillColor(sf::Color(255, 165, 0)); // Orange
                lightHorizontalRight.setFillColor(sf::Color(255, 165, 0)); // Orange
                lightHorizontalRightRight.setFillColor(sf::Color(255, 165, 0)); // Orange
                stateDuration = sf::seconds(5);
                break;

            case OrangeHorizontal:
                state = RedHorizontalOrangeVertical;
                lightHorizontalLeft.setFillColor(sf::Color::Red);
                lightHorizontalLeftLeft.setFillColor(sf::Color::Red);
                lightHorizontalRight.setFillColor(sf::Color::Red);
                lightHorizontalRightRight.setFillColor(sf::Color::Red);
                lightVerticalTop.setFillColor(sf::Color::Green);
                lightVerticalBottom.setFillColor(sf::Color::Green);
                stateDuration = sf::seconds(30);
                break;

            case RedHorizontalOrangeVertical:
                state = RedHorizontal;
                lightHorizontalLeft.setFillColor(sf::Color::Red);
                lightHorizontalLeftLeft.setFillColor(sf::Color::Red);
                lightHorizontalRight.setFillColor(sf::Color::Red);
                lightHorizontalRightRight.setFillColor(sf::Color::Red);
                lightVerticalTop.setFillColor(sf::Color(255, 165, 0)); // Orange
                lightVerticalBottom.setFillColor(sf::Color(255, 165, 0)); // Orange
                stateDuration = sf::seconds(5);
                break;
            }
            stateClock.restart();
        }
    }

    // Retourne l'�tat actuel
    TrafficLightState getState() {
        std::lock_guard<std::mutex> lock(trafficMutex);
        return state;
    }

    // Dessine les feux sur la fen�tre
    void draw(sf::RenderWindow& window) {
        window.draw(lightHorizontalLeft);
        window.draw(lightHorizontalLeftLeft);
        window.draw(lightHorizontalRight);
        window.draw(lightHorizontalRightRight);
        window.draw(lightVerticalTop);
        window.draw(lightVerticalBottom);
    }
};


// Classe pour les usagers (v�hicules)
class User {
protected:
    sf::Sprite sprite;
    float speed;
    bool isHorizontal;
    bool goingPositive;
    bool hasTurned;       // Indique si la voiture a d�j� tourn�
    bool turnLeftAtCenter; // Indique si cette voiture doit tourner � gauche au centre
    bool turnRightAtCenter; // Indique si la voiture doit tourner � droite au centre

public:
    User(float x, float y, const sf::Texture& texture, float speed, bool isHorizontal, bool goingPositive, bool turnLeftAtCenter = false, bool turnRightAtCenter = false)
        : speed(speed), isHorizontal(isHorizontal), goingPositive(goingPositive), hasTurned(false), turnLeftAtCenter(turnLeftAtCenter), turnRightAtCenter(turnRightAtCenter) {
        sprite.setTexture(texture);
        sprite.setPosition(x, y);

        // Dimensions cibles pour la voiture
        float targetWidth = 40.0f;
        float targetHeight = 20.0f;

        // Calcul de l'�chelle n�cessaire
        float scaleX = targetWidth / texture.getSize().x;
        float scaleY = targetHeight / texture.getSize().y;

        // Appliquer l'�chelle
        sprite.setScale(scaleX, scaleY);

        // Ajuster l'orientation pour les v�hicules
        if (!isHorizontal) {
            sprite.setRotation(90); // Vertical (vers le haut par d�faut)
            if (!goingPositive) {
                sprite.setRotation(270); // Vers le bas
            }
        }
        else {
            if (!goingPositive) {
                sprite.setRotation(180); // Vers la gauche
            }
        }
    }

    virtual void move(TrafficLightState lightState) {
        // Positions de la ligne d'arr�t
        const float stopLineXLeft = 120;   // Ligne d'arr�t pour les v�hicules venant de la gauche
        const float stopLineXRight = 675; // Ligne d'arr�t pour les v�hicules venant de la droite
        const float stopLineYTop = 75;    // Ligne d'arr�t pour les v�hicules venant du haut
        const float stopLineYBottom = 515; // Ligne d'arr�t pour les v�hicules venant du bas

        // G�rer les virages au centre de l'intersection
        if (!hasTurned) {
            if (turnLeftAtCenter && isHorizontal && sprite.getPosition().x > 370 && sprite.getPosition().x < 380) {
                isHorizontal = false;
                goingPositive = true; // Tourne vers le bas
                sprite.setRotation(90);
                hasTurned = true;
                return;
            }
            if (turnLeftAtCenter && !isHorizontal && sprite.getPosition().y > 275 && sprite.getPosition().y < 285) {
                isHorizontal = true;
                goingPositive = false; // Tourne � gauche
                sprite.setRotation(180);
                hasTurned = true;
                return;
            }
            if (turnRightAtCenter && isHorizontal && sprite.getPosition().x > 435 && sprite.getPosition().x < 445) {
                isHorizontal = false;
                goingPositive = false; // Tourne vers le haut
                sprite.setRotation(270);
                hasTurned = true;
                return;
            }
            if (turnRightAtCenter && !isHorizontal && sprite.getPosition().y > 310 && sprite.getPosition().y < 320) {
                isHorizontal = true;
                goingPositive = true; // Tourne � droite
                sprite.setRotation(0);
                hasTurned = true;
                return;
            }
        }

        // D�placement en fonction des feux de circulation
        if (isHorizontal) {
            // Mouvement horizontal
            if (lightState == GreenHorizontal) {
                sprite.move((goingPositive ? speed : -speed), 0); // D�placement si feu vert
            }
            else if (lightState == OrangeHorizontal || lightState == RedHorizontalOrangeVertical || lightState == RedHorizontal) {
                if (goingPositive && sprite.getPosition().x >= stopLineXLeft && sprite.getPosition().x < 200) {
                    return; // Arr�t pour les v�hicules allant vers la droite
                }
                if (!goingPositive && sprite.getPosition().x <= stopLineXRight && sprite.getPosition().x > 580) {
                    return; // Arr�t pour les v�hicules allant vers la gauche
                }
                sprite.move((goingPositive ? speed : -speed), 0); // Mouvement apr�s avoir pass� la ligne
            }
        }
        else {
            // Mouvement vertical
            if (lightState == RedHorizontalOrangeVertical) {
                sprite.move(0, (goingPositive ? speed : -speed)); // D�placement si feu vert pour direction verticale
            }
            else if (lightState == GreenHorizontal || lightState == OrangeHorizontal || lightState == RedHorizontal) {
                if (goingPositive && sprite.getPosition().y >= stopLineYTop && sprite.getPosition().y < 140) {
                    return; // Arr�t pour les v�hicules descendant
                }
                if (!goingPositive && sprite.getPosition().y <= stopLineYBottom && sprite.getPosition().y > 450) {
                    return; // Arr�t pour les v�hicules montant
                }
                sprite.move(0, (goingPositive ? speed : -speed)); // Mouvement apr�s avoir pass� la ligne
            }
        }
    }



    void draw(sf::RenderWindow& window) { window.draw(sprite); }
    virtual ~User() = default;
};



class Bus : public User {
public:
    Bus(float x, float y, const sf::Texture& texture, bool isHorizontal, bool goingPositive, bool turnLeftAtCenter = false, bool turnRightAtCenter = false)
        : User(x, y, texture, 0.075f, isHorizontal, goingPositive, turnLeftAtCenter, turnRightAtCenter) {
        // Ajuster l'�chelle sp�cifique pour le bus
        float targetWidth = 60.0f;
        float targetHeight = 30.0f;

        // Calcul de l'�chelle n�cessaire
        float scaleX = targetWidth / texture.getSize().x;
        float scaleY = targetHeight / texture.getSize().y;

        // Appliquer l'�chelle
        sprite.setScale(scaleX, scaleY);

        // Ajuster l'orientation pour les bus
        if (!isHorizontal) {
            sprite.setRotation(90); // Vertical (vers le haut par d�faut)
            if (!goingPositive) {
                sprite.setRotation(270); // Vers le bas
            }
        }
        else {
            if (!goingPositive) {
                sprite.setRotation(180); // Vers la gauche
            }
        }
    }

    void move(TrafficLightState lightState) override {
        // Positions de la ligne d'arr�t
        const float stopLineXLeft = 100;   // Ligne d'arr�t pour les v�hicules venant de la gauche
        const float stopLineXRight = 695; // Ligne d'arr�t pour les v�hicules venant de la droite
        const float stopLineYTop = 55;    // Ligne d'arr�t pour les v�hicules venant du haut
        const float stopLineYBottom = 535; // Ligne d'arr�t pour les v�hicules venant du bas

        // G�rer les virages au centre de l'intersection
        if (!hasTurned) {
            if (turnLeftAtCenter && isHorizontal && sprite.getPosition().x > 305 && sprite.getPosition().x < 315) {
                isHorizontal = false;
                goingPositive = true; // Tourne vers le bas
                sprite.setRotation(90);
                hasTurned = true;
                return;
            }
            if (turnLeftAtCenter && !isHorizontal && sprite.getPosition().y > 230 && sprite.getPosition().y < 240) {
                isHorizontal = true;
                goingPositive = false; // Tourne � gauche
                sprite.setRotation(180);
                hasTurned = true;
                return;
            }
            if (turnRightAtCenter && isHorizontal && sprite.getPosition().x > 475 && sprite.getPosition().x < 485) {
                isHorizontal = false;
                goingPositive = false; // Tourne vers le haut
                sprite.setRotation(270);
                hasTurned = true;
                return;
            }
            if (turnRightAtCenter && !isHorizontal && sprite.getPosition().y > 355 && sprite.getPosition().y < 365) {
                isHorizontal = true;
                goingPositive = true; // Tourne � droite
                sprite.setRotation(0);
                hasTurned = true;
                return;
            }
        }

        // D�placement en fonction des feux de circulation
        if (isHorizontal) {
            // Mouvement horizontal
            if (lightState == GreenHorizontal) {
                sprite.move((goingPositive ? speed : -speed), 0); // D�placement si feu vert
            }
            else if (lightState == OrangeHorizontal || lightState == RedHorizontalOrangeVertical || lightState == RedHorizontal) {
                if (goingPositive && sprite.getPosition().x >= stopLineXLeft && sprite.getPosition().x < 200) {
                    return; // Arr�t pour les v�hicules allant vers la droite
                }
                if (!goingPositive && sprite.getPosition().x <= stopLineXRight && sprite.getPosition().x > 580) {
                    return; // Arr�t pour les v�hicules allant vers la gauche
                }
                sprite.move((goingPositive ? speed : -speed), 0); // Mouvement apr�s avoir pass� la ligne
            }
        }
        else {
            // Mouvement vertical
            if (lightState == RedHorizontalOrangeVertical) {
                sprite.move(0, (goingPositive ? speed : -speed)); // D�placement si feu vert pour direction verticale
            }
            else if (lightState == GreenHorizontal || lightState == OrangeHorizontal || lightState == RedHorizontal) {
                if (goingPositive && sprite.getPosition().y >= stopLineYTop && sprite.getPosition().y < 140) {
                    return; // Arr�t pour les v�hicules descendant
                }
                if (!goingPositive && sprite.getPosition().y <= stopLineYBottom && sprite.getPosition().y > 450) {
                    return; // Arr�t pour les v�hicules montant
                }
                sprite.move(0, (goingPositive ? speed : -speed)); // Mouvement apr�s avoir pass� la ligne
            }
        }
    }
};

class Bike : public User {
public:
    Bike(float x, float y, const sf::Texture& texture, bool isHorizontal, bool goingPositive, bool turnLeftAtCenter = false, bool turnRightAtCenter = false)
        : User(x, y, texture, 0.05f, isHorizontal, goingPositive, turnLeftAtCenter, turnRightAtCenter) {

        // Ajuster l'�chelle sp�cifique pour le v�lo
        float targetWidth = 30.0f;
        float targetHeight = 15.0f;

        float scaleX = targetWidth / texture.getSize().x;
        float scaleY = targetHeight / texture.getSize().y;

        sprite.setScale(scaleX, scaleY);

        if (!isHorizontal) {
            sprite.setRotation(90); // Mouvement vertical par d�faut
            if (!goingPositive) {
                sprite.setRotation(270); // Mouvement vers le bas
            }
        }
        else {
            if (!goingPositive) {
                sprite.setRotation(180); // Mouvement vers la gauche
            }
        }
    }

    void move(TrafficLightState lightState) override {
        // Positions de la ligne d'arr�t
        const float stopLineXLeft = 130;   // Ligne d'arr�t pour les v�hicules venant de la gauche
        const float stopLineXRight = 665; // Ligne d'arr�t pour les v�hicules venant de la droite
        const float stopLineYTop = 85;    // Ligne d'arr�t pour les v�hicules venant du haut
        const float stopLineYBottom = 505; // Ligne d'arr�t pour les v�hicules venant du bas

        // G�rer les virages au centre de l'intersection
        if (!hasTurned) {
            if (turnLeftAtCenter && isHorizontal && sprite.getPosition().x > 263 && sprite.getPosition().x < 267) {
                isHorizontal = false;
                goingPositive = true; // Tourne vers le bas
                sprite.setRotation(90);
                hasTurned = true;
                return;
            }
            if (turnLeftAtCenter && !isHorizontal && sprite.getPosition().y > 188 && sprite.getPosition().y < 192) {
                isHorizontal = true;
                goingPositive = false; // Tourne � gauche
                sprite.setRotation(180);
                hasTurned = true;
                return;
            }
            if (turnRightAtCenter && isHorizontal && sprite.getPosition().x > 528 && sprite.getPosition().x < 532) {
                isHorizontal = false;
                goingPositive = false; // Tourne vers le haut
                sprite.setRotation(270);
                hasTurned = true;
                return;
            }
            if (turnRightAtCenter && !isHorizontal && sprite.getPosition().y > 403 && sprite.getPosition().y < 407) {
                isHorizontal = true;
                goingPositive = true; // Tourne � droite
                sprite.setRotation(0);
                hasTurned = true;
                return;
            }
        }

        // D�placement en fonction des feux de circulation
        if (isHorizontal) {
            // Mouvement horizontal
            if (lightState == GreenHorizontal) {
                sprite.move((goingPositive ? speed : -speed), 10); // D�placement si feu vert
            }
            else if (lightState == OrangeHorizontal || lightState == RedHorizontalOrangeVertical || lightState == RedHorizontal) {
                if (goingPositive && sprite.getPosition().x >= stopLineXLeft && sprite.getPosition().x < 200) {
                    return; // Arr�t pour les v�hicules allant vers la droite
                }
                if (!goingPositive && sprite.getPosition().x <= stopLineXRight && sprite.getPosition().x > 580) {
                    return; // Arr�t pour les v�hicules allant vers la gauche
                }
                sprite.move((goingPositive ? speed : -speed), 0); // Mouvement apr�s avoir pass� la ligne
            }
        }
        else {
            // Mouvement vertical
            if (lightState == RedHorizontalOrangeVertical) {
                sprite.move(0, (goingPositive ? speed : -speed)); // D�placement si feu vert pour direction verticale
            }
            else if (lightState == GreenHorizontal || lightState == OrangeHorizontal || lightState == RedHorizontal) {
                if (goingPositive && sprite.getPosition().y >= stopLineYTop && sprite.getPosition().y < 140) {
                    return; // Arr�t pour les v�hicules descendant
                }
                if (!goingPositive && sprite.getPosition().y <= stopLineYBottom && sprite.getPosition().y > 450) {
                    return; // Arr�t pour les v�hicules montant
                }
                sprite.move(0, (goingPositive ? speed : -speed)); // Mouvement apr�s avoir pass� la ligne
            }
        }
    }
};

class Pedestrian : public User {
public:
    Pedestrian(float x, float y, const sf::Texture& texture, bool isHorizontal, bool goingPositive, bool turnLeftAtCenter = false, bool turnRightAtCenter = false)
        : User(x, y, texture, 0.03f, isHorizontal, goingPositive, turnLeftAtCenter, turnRightAtCenter) {

        // Ajuster l'�chelle sp�cifique pour le v�lo
        float targetWidth = 15.0f;
        float targetHeight = 30.0f;

        float scaleX = targetWidth / texture.getSize().x;
        float scaleY = targetHeight / texture.getSize().y;

        sprite.setScale(scaleX, scaleY);

        if (!isHorizontal) {
            sprite.setRotation(90); // Mouvement vertical par d�faut
            if (!goingPositive) {
                sprite.setRotation(270); // Mouvement vers le bas
            }
        }
        else {
            if (!goingPositive) {
                sprite.setRotation(180); // Mouvement vers la gauche
            }
        }
    }

    void move(TrafficLightState lightState) override {
        // Positions de la ligne d'arr�t
        const float stopLineXLeft = 145;   // Ligne d'arr�t pour les v�hicules venant de la gauche
        const float stopLineXRight = 650; // Ligne d'arr�t pour les v�hicules venant de la droite
        const float stopLineYTop = 100;    // Ligne d'arr�t pour les v�hicules venant du haut
        const float stopLineYBottom = 490; // Ligne d'arr�t pour les v�hicules venant du bas
        const float stopLineXLeftLeft = 45;

        // G�rer les virages au centre de l'intersection
        if (!hasTurned) {
            if (turnLeftAtCenter && isHorizontal && sprite.getPosition().x > 220 && sprite.getPosition().x < 230) {
                isHorizontal = false;
                goingPositive = true; // Tourne vers le bas
                sprite.setRotation(90);
                hasTurned = true;
                return;
            }
            if (turnLeftAtCenter && !isHorizontal && sprite.getPosition().y > 155 && sprite.getPosition().y < 165) {
                isHorizontal = true;
                goingPositive = false; // Tourne � gauche
                sprite.setRotation(180);
                hasTurned = true;
                return;
            }
            if (turnRightAtCenter && isHorizontal && sprite.getPosition().x > 560 && sprite.getPosition().x < 570) {
                isHorizontal = false;
                goingPositive = false; // Tourne vers le haut
                sprite.setRotation(270);
                hasTurned = true;
                return;
            }
            if (turnRightAtCenter && !isHorizontal && sprite.getPosition().y > 435 && sprite.getPosition().y < 445) {
                isHorizontal = true;
                goingPositive = true; // Tourne � droite
                sprite.setRotation(0);
                hasTurned = true;
                return;
            }
        }

        // D�placement en fonction des feux de circulation
        if (isHorizontal) {
            // Mouvement horizontal
            if (lightState == GreenHorizontal) {
                sprite.move((goingPositive ? speed : -speed), 0); // D�placement si feu vert
            }
            else if (lightState == OrangeHorizontal || lightState == RedHorizontalOrangeVertical || lightState == RedHorizontal) {
                if (goingPositive && sprite.getPosition().x >= stopLineXLeft && sprite.getPosition().x < 200) {
                    return; // Arr�t pour les v�hicules allant vers la droite
                }
                if (!goingPositive && sprite.getPosition().x <= stopLineXRight && sprite.getPosition().x > 580) {
                    return; // Arr�t pour les v�hicules allant vers la gauche
                }
                sprite.move((goingPositive ? speed : -speed), 0); // Mouvement apr�s avoir pass� la ligne
            }
        }
        else {
            // Mouvement vertical
            if (lightState == RedHorizontalOrangeVertical) {
                sprite.move(0, (goingPositive ? speed : -speed)); // D�placement si feu vert pour direction verticale
            }
            else if (lightState == GreenHorizontal || lightState == OrangeHorizontal || lightState == RedHorizontal) {
                if (goingPositive && sprite.getPosition().y >= stopLineYTop && sprite.getPosition().y < 140) {
                    return; // Arr�t pour les v�hicules descendant
                }
                if (!goingPositive && sprite.getPosition().y <= stopLineYBottom && sprite.getPosition().y > 450) {
                    return; // Arr�t pour les v�hicules montant
                }
                sprite.move(0, (goingPositive ? speed : -speed)); // Mouvement apr�s avoir pass� la ligne
            }
        }
    }
};



void generateRandomVehicle(std::vector<User>& users, std::vector<Bus>& buses, std::vector<Bike>& bikes, std::vector<Pedestrian>& pedestrians,
    const sf::Texture& carTexture, const sf::Texture& busTexture, const sf::Texture& bikeTexture, const sf::Texture& pedestrianTexture) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> vehicleTypeDist(0, 3); // 0: Voiture, 1: Bus, 2: V�lo 3:pieton
    std::uniform_int_distribution<int> directionDist(0, 3);
    std::uniform_int_distribution<int> turnDecisionDist(0, 2);

    int vehicleType = vehicleTypeDist(gen);
    int direction = directionDist(gen);
    int turnDecision = turnDecisionDist(gen);

    bool isHorizontal = direction < 2;
    bool goingPositive = (direction == 0 || direction == 2);
    bool turnLeftAtCenter = (turnDecision == 1);
    bool turnRightAtCenter = (turnDecision == 2);

    float x = 0, y = 0;

    float xOffsetForBus = 60.0f; // Le d�calage entre la voie des voitures et celle des bus

    // D�terminer la position initiale en fonction de la direction
    if (direction == 0) { // Gauche -> Droite
        x = 0;
        y = 315;  // Position pour voiture allant � droite
    }
    else if (direction == 1) { // Droite -> Gauche
        x = WINDOW_WIDTH;
        y = 280;  // Position pour voiture allant � gauche
    }
    else if (direction == 2) { // Haut -> Bas
        x = 375;  // Position pour voiture allant vers le bas
        y = 0;
    }
    else if (direction == 3) { // Bas -> Haut
        x = 440;  // Position pour voiture allant vers le haut
        y = WINDOW_HEIGHT;
    }

    // Ajouter un v�hicule (User ou Bus)
    if (vehicleType == 0) {
        // Positionner les voitures
        users.emplace_back(x, y, carTexture, 0.1f, isHorizontal, goingPositive, turnLeftAtCenter, turnRightAtCenter);
    }
    else if (vehicleType == 1) {
        // Positionner les bus
        if (direction == 0) { // Bus allant � droite
            y = 360;
        }
        else if (direction == 1) { // Bus allant � gauche
            y = 235;
        }
        else if (direction == 2) { // Bus allant vers le bas
            x = 260;
            y = 0;
        }
        else if (direction == 3) { // Bus allant vers le haut
            x = 430;
            y = WINDOW_HEIGHT;
        }

        // Appliquer le d�calage pour les bus
        x += xOffsetForBus;

        // Ajouter le bus � la liste
        buses.emplace_back(x, y, busTexture, isHorizontal, goingPositive, turnLeftAtCenter, turnRightAtCenter);
    }
    else if (vehicleType == 2) {
        // Positionner les bus
        if (direction == 0) { // velo allant � droite
            y = 405;
        }
        else if (direction == 1) { // velo allant � gauche
            y = 190;
        }
        else if (direction == 2) { // velo allant vers le bas
            x = 260;
            y = 0;
        }
        else if (direction == 3) { // velo allant vers le haut
            x = 535;
            y = WINDOW_HEIGHT;
        }

        bikes.emplace_back(x, y, bikeTexture, isHorizontal, goingPositive, turnLeftAtCenter, turnRightAtCenter);
    }
    else {
        // 
        if (direction == 0) { // pieotn allant � droite
            y = 440;
        }
        else if (direction == 1) { // pieton allant � gauche
            y = 160;
        }
        else if (direction == 2) { // pieton allant vers le bas
            x = 225;
            y = 0;
        }
        else if (direction == 3) { // pieton allant vers le haut
            x = 565;
            y = WINDOW_HEIGHT;
        }

        pedestrians.emplace_back(x, y, pedestrianTexture, isHorizontal, goingPositive, turnLeftAtCenter, turnRightAtCenter);
    }
}





int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Traffic Simulation with Background");

    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("C:/Users/matheo.lesage-gante/Desktop/OneDrive/CIR2/Prog/Projet/img/background.jpg")) {
        std::cerr << "Erreur : Impossible de charger l'image de fond !" << std::endl;
        return -1;
    }
    sf::Sprite backgroundSprite;
    backgroundSprite.setTexture(backgroundTexture);
    backgroundSprite.setScale(
        float(WINDOW_WIDTH) / backgroundTexture.getSize().x,
        float(WINDOW_HEIGHT) / backgroundTexture.getSize().y
    );

    sf::Texture carTexture;
    if (!carTexture.loadFromFile("C:/Users/matheo.lesage-gante/Desktop/OneDrive/CIR2/Prog/Projet/img/car.png")) {
        std::cerr << "Erreur : Impossible de charger l'image de la voiture !" << std::endl;
        return -1;
    }

    sf::Texture busTexture;
    if (!busTexture.loadFromFile("C:/Users/matheo.lesage-gante/Desktop/OneDrive/CIR2/Prog/Projet/img/bus.png")) {
        std::cerr << "Erreur : Impossible de charger l'image du bus !" << std::endl;
        return -1;
    }

    sf::Texture bikeTexture;
    if (!bikeTexture.loadFromFile("C:/Users/matheo.lesage-gante/Desktop/OneDrive/CIR2/Prog/Projet/img/cyclist.png")) {
        std::cerr << "Erreur : Impossible de charger l'image du v�lo !" << std::endl;
        return -1;
    }
    sf::Texture pedestrianTexture;
    if (!pedestrianTexture.loadFromFile("C:/Users/matheo.lesage-gante/Desktop/OneDrive/CIR2/Prog/Projet/img/pieton.png")) {
        std::cerr << "Erreur : Impossible de charger l'image du pi�ton !" << std::endl;
        return -1;
    }

    TrafficLight trafficLight;

    std::vector<User> users;
    std::vector<Bus> buses;
    std::vector<Bike> bikes;
    std::vector<Pedestrian> pedestrians;


    sf::Clock clock;
    sf::Time spawnInterval = sf::seconds(3); // Intervalle pour ajouter un v�hicule

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        if (clock.getElapsedTime() >= spawnInterval) {
            generateRandomVehicle(users, buses, bikes, pedestrians, carTexture, busTexture, bikeTexture, pedestrianTexture);
            clock.restart();
        }
        trafficLight.update();
        window.clear();
        window.draw(backgroundSprite);
        trafficLight.draw(window);

        TrafficLightState currentState = trafficLight.getState();

        // Mouvement et affichage des voitures
        for (auto& user : users) {
            user.move(currentState);
            user.draw(window);
        }

        // Mouvement et affichage des bus
        for (auto& bus : buses) {
            bus.move(currentState);
            bus.draw(window);
        }

        // Mouvement et affichage des v�los
        for (auto& bike : bikes) {
            bike.move(currentState);
            bike.draw(window);
        }

        // Mouvement et affichage des pi�tons
        for (auto& pedestrian : pedestrians) {
            pedestrian.move(currentState);
            pedestrian.draw(window);
        }

        window.display();
    }



    return 0;
}
