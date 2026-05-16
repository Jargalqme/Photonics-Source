#include "pch.h"
#include "Render/LayoutLoader.h"
#include "Render/MeshCache.h"
#include "Services/SceneContext.h"
#include "ThirdParty/json.hpp"
#include <fstream>

using namespace DirectX;
using namespace DirectX::SimpleMath;

bool LayoutLoader::loadPrimitiveLayout(
    SceneContext& context,
    const std::string& jsonPath,
    std::vector<PrimitiveLayoutPart>& outParts)
{
    outParts.clear();

    if (!context.meshes)
    {
        return false;
    }

    std::ifstream file(jsonPath);
    if (!file.is_open())
    {
        return false;
    }

    nlohmann::json data;
    file >> data;

    for (const auto& part : data["parts"])
    {
        PrimitiveLayoutPart layoutPart;
        const std::string type = part["type"];
        float scaleCorrection = 1.0f;

        if (type == "box")
        {
            layoutPart.primitive = context.meshes->getCube();
        }
        else if (type == "cylinder")
        {
            layoutPart.primitive = context.meshes->getCylinder();
            scaleCorrection = 2.0f;
        }
        else if (type == "sphere")
        {
            layoutPart.primitive = context.meshes->getSphere();
            scaleCorrection = 2.0f;
        }
        else if (type == "torus")
        {
            layoutPart.primitive = context.meshes->getTorus(1.0f, 0.333f);
            scaleCorrection = 2.0f;
        }
        else
        {
            continue;
        }

        const auto& position = part["pos"];
        const auto& rotation = part["rot"];
        const auto& scale = part["scale"];

        // Existing primitive layout data was authored in the opposite X
        // convention, so keep the flip centralized here instead of in callers.
        layoutPart.localTransform.position = Vector3(
            -(float)position[0],
            (float)position[1],
            (float)position[2]);

        layoutPart.localTransform.rotation = Vector3(
            XMConvertToRadians((float)rotation[0]),
            XMConvertToRadians((float)rotation[1]),
            XMConvertToRadians((float)rotation[2]));

        layoutPart.localTransform.scale = Vector3(
            (float)scale[0] * scaleCorrection,
            (float)scale[1] * scaleCorrection,
            (float)scale[2] * scaleCorrection);

        outParts.push_back(std::move(layoutPart));
    }

    return true;
}
