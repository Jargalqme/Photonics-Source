#pragma once

#include "Common/Transform.h"
#include "GeometricPrimitive.h"
#include <SimpleMath.h>
#include <string>
#include <vector>

struct SceneContext;

struct PrimitiveLayoutPart
{
    DirectX::DX11::GeometricPrimitive* primitive = nullptr;
    Transform localTransform;
};

class LayoutLoader
{
public:
    static bool loadPrimitiveLayout(
        SceneContext& context,
        const std::string& jsonPath,
        std::vector<PrimitiveLayoutPart>& outParts);
};
