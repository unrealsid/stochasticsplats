#include "imu_sensor.h"

#include "imu_sensor.h"
#include "core/log.h"

ImuSensor::ImuSensor() :
        sensorManager(nullptr),
        rotationSensor(nullptr),
        accelSensor(nullptr),
        sensorEventQueue(nullptr),
        rotation(1.0f, 0.0f, 0.0f, 0.0f), // Identity quaternion
        acceleration(0.0f, 0.0f, 0.0f)
{}

ImuSensor::~ImuSensor()
{
    // Clean up the event queue if we created one
    if (sensorManager && sensorEventQueue)
    {
        ASensorManager_destroyEventQueue(sensorManager, sensorEventQueue);
    }
}

bool ImuSensor::Init(ALooper* looper)
{
    sensorManager = ASensorManager_getInstance();
    if (!sensorManager)
    {
        Log::E("Failed to get ASensorManager!");
        return false;
    }

    rotationSensor = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_ROTATION_VECTOR);
    accelSensor = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_LINEAR_ACCELERATION);

    sensorEventQueue = ASensorManager_createEventQueue(sensorManager, looper, LOOPER_ID_SENSOR, nullptr, nullptr);

    if (rotationSensor)
    {
        ASensorEventQueue_enableSensor(sensorEventQueue, rotationSensor);
        ASensorEventQueue_setEventRate(sensorEventQueue, rotationSensor, 1000000 / 60); // 60Hz
    }
    if (accelSensor)
    {
        ASensorEventQueue_enableSensor(sensorEventQueue, accelSensor);
        ASensorEventQueue_setEventRate(sensorEventQueue, accelSensor, 1000000 / 60); // 60Hz
    }

    Log::D("ImuSensor initialized successfully!");
    return true;
}

void ImuSensor::ProcessEvents()
{
    if (!sensorEventQueue) return;

    ASensorEvent event;
    // Drain the queue of all pending events
    while (ASensorEventQueue_getEvents(sensorEventQueue, &event, 1) > 0)
    {
        if (event.type == ASENSOR_TYPE_ROTATION_VECTOR)
        {
            // Android gives [x, y, z, w]. GLM expects (w, x, y, z).
            rotation = glm::quat(event.data[3], event.data[0], event.data[1], event.data[2]);
        }
        else if (event.type == ASENSOR_TYPE_LINEAR_ACCELERATION)
        {
            acceleration = glm::vec3(event.acceleration.x, event.acceleration.y, event.acceleration.z);
        }
    }
}