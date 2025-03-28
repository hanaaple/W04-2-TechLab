#include "UnrealEd/SceneMgr.h"
#include "JSON/json.hpp"
#include "UObject/Object.h"
#include <fstream>
#include "Components/LightComponent.h"
#include "Components/SkySphereComponent.h"
#include "Core/HAL/PlatformType.h"
#include <sstream>

#include "Utils/PathUtil.h"

using json = nlohmann::json;
using namespace std;

constexpr int X = 0;
constexpr int Y = 1;
constexpr int Z = 2;

FSceneData FSceneMgr::CurrentSceneData = FSceneData();

bool FSceneMgr::LoadSceneData(const FString& FileName)
{
    std::filesystem::path targetDir;
    if (!IsDebuggerPresent())
    {
        targetDir = std::filesystem::current_path() / TEXT("Assets\\Scenes");
    }
    else
    {
        targetDir = std::filesystem::current_path() / TEXT("Assets\\Scenes");
    }

    // 디렉토리가 없으면 생성합니다.
    if (!filesystem::exists(targetDir))
    {
        filesystem::create_directories(targetDir);
    }

    // FileName은 FString이므로 c_str()을 통해 문자열로 변환합니다.
    filesystem::path fullPath = targetDir / FileName.c_str();

    // 파일을 열어 전체 내용을 읽어들임
    std::ifstream inFile(fullPath);
    if (!inFile.is_open())
    {
        return false;
    }

    std::stringstream buffer;
    buffer << inFile.rdbuf();
    inFile.close();

    // 파일 내용을 파싱
    json j;
    try
    {
        j = json::parse(buffer.str());
    }
    catch (json::parse_error& e)
    {
        return false;
    }
    // NextUUID 파싱
    CurrentSceneData.NextUUID = j["NextUUID"].get<int>();
    
    // PerspectiveCamera 파싱
    auto pc = j["PerspectiveCamera"];
    CurrentSceneData.PerspectiveCamera.FOV = pc["FOV"][0].get<float>();
    CurrentSceneData.PerspectiveCamera.FarClip = pc["FarClip"][0].get<float>();
    CurrentSceneData.PerspectiveCamera.NearClip = pc["NearClip"][0].get<float>();

    CurrentSceneData.PerspectiveCamera.Location.x = pc["Location"][X].get<float>();
    CurrentSceneData.PerspectiveCamera.Location.y = pc["Location"][Y].get<float>();
    CurrentSceneData.PerspectiveCamera.Location.z = pc["Location"][Z].get<float>();

    CurrentSceneData.PerspectiveCamera.Rotation.x = pc["Rotation"][X].get<float>();
    CurrentSceneData.PerspectiveCamera.Rotation.y = pc["Rotation"][Y].get<float>();
    CurrentSceneData.PerspectiveCamera.Rotation.z = pc["Rotation"][Z].get<float>();

    // Primitives 파싱
    auto primitivesJson = j["Primitives"];
    for (auto it = primitivesJson.begin(); it != primitivesJson.end(); ++it)
    {
        uint32 key = std::stoi(it.key());
        auto prim = it.value();
        FPrimitiveData pd;
    
        pd.Location.x = prim["Location"][X].get<float>();
        pd.Location.y = prim["Location"][Y].get<float>();
        pd.Location.z = prim["Location"][Z].get<float>();

        pd.Rotation.x = prim["Rotation"][X].get<float>();
        pd.Rotation.y = prim["Rotation"][Y].get<float>();
        pd.Rotation.z = prim["Rotation"][Z].get<float>();

        pd.Scale.x = prim["Scale"][X].get<float>();
        pd.Scale.y = prim["Scale"][Y].get<float>();
        pd.Scale.z = prim["Scale"][Z].get<float>();
#if USE_WIDECHAR
        pd.ObjStaticMeshAsset = prim["ObjStaticMeshAsset"].get<std::wstring>();
        pd.Type = prim["Type"].get<std::wstring>();
#else
        pd.ObjStaticMeshAsset = prim["ObjStaticMeshAsset"].get<std::string>();
        pd.Type = prim["Type"].get<std::string>();
#endif

        CurrentSceneData.Primitives[key] = pd;
    }

    return true;
}

bool FSceneMgr::NewSceneData()
{
    CurrentSceneData = FSceneData();
    return true;
}

bool FSceneMgr::SaveSceneData(const FString& FileName, FSceneData InSceneData)
{
    // JSON 객체 생성
    json j;
    
    // NextUUID 파싱
    j["NextUUID"] = InSceneData.NextUUID;
    
    // PerspectiveCamera 파싱 (배열 형식으로 저장)
    j["PerspectiveCamera"]["FOV"]      = { InSceneData.PerspectiveCamera.FOV };
    j["PerspectiveCamera"]["FarClip"]  = { InSceneData.PerspectiveCamera.FarClip };
    j["PerspectiveCamera"]["NearClip"] = { InSceneData.PerspectiveCamera.NearClip };
    j["PerspectiveCamera"]["Location"] = { 
        InSceneData.PerspectiveCamera.Location.x,
        InSceneData.PerspectiveCamera.Location.y,
        InSceneData.PerspectiveCamera.Location.z
    };
    j["PerspectiveCamera"]["Rotation"] = {
        InSceneData.PerspectiveCamera.Rotation.x,
        InSceneData.PerspectiveCamera.Rotation.y,
        InSceneData.PerspectiveCamera.Rotation.z
    };

    // Primitives 파싱
    for (const auto& pair : InSceneData.Primitives)
    {
        const uint32& key = pair.Key;
        const FPrimitiveData& pd = pair.Value;

        j["Primitives"][key]["Location"] = {
            pd.Location.x, pd.Location.y, pd.Location.z
        };
        j["Primitives"][key]["Rotation"] = {
            pd.Rotation.x, pd.Rotation.y, pd.Rotation.z
        };
        j["Primitives"][key]["Scale"] = {
            pd.Scale.x, pd.Scale.y, pd.Scale.z
        };
        j["Primitives"][key]["ObjStaticMeshAsset"] = pd.ObjStaticMeshAsset.c_str();
        j["Primitives"][key]["Type"] = pd.Type.c_str();
    }
    
    // JSON 문자열로 변환 (들여쓰기는 4칸)
    std::string jsonString = j.dump(4);
    
    // 파일 저장 (FilePath.c_wstr()를 사용하여 wide 문자열 경로로 열기)
    std::filesystem::path targetDir;
    if (!IsDebuggerPresent())
    {
        targetDir = std::filesystem::current_path() / TEXT("Assets\\Scenes");
    }
    else
    {
        targetDir = std::filesystem::current_path() / TEXT("Assets\\Scenes");
    }

    // 디렉토리가 없으면 생성합니다.
    if (!filesystem::exists(targetDir))
    {
        filesystem::create_directories(targetDir);
    }

    // FileName은 FString이므로 c_str()을 통해 문자열로 변환합니다.
    filesystem::path fullPath = targetDir / FileName.c_str();
    
    std::ofstream outFile(fullPath);
    if (!outFile.is_open())
    {
        return false;
    }
    
    outFile << jsonString;
    outFile.close();
    
    return true;
}

