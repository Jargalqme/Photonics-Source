#pragma once
#include "Common/Transform.h"

class GameObject
{
public:
    virtual ~GameObject() = default;

    virtual void initialize() {}
    virtual void update(float) {}
    virtual void finalize() {}

protected:
    Transform m_transform;
    bool m_active = true;
};
