#pragma once

#include <android/sensor.h>
#include <android/looper.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class ImuSensor
{
public:
    static constexpr int LOOPER_ID_SENSOR = 3;

    ImuSensor();
    ~ImuSensor();

    bool Init(ALooper* looper);
    void ProcessEvents();

    // Clean getters to pull the current IMU state anytime
    glm::quat GetRotation() const { return rotation; }
    glm::vec3 GetAcceleration() const { return acceleration; }

private:
    ASensorManager* sensorManager;
    const ASensor* rotationSensor;
    const ASensor* accelSensor;
    ASensorEventQueue* sensorEventQueue;

    glm::quat rotation;
    glm::vec3 acceleration;
};

