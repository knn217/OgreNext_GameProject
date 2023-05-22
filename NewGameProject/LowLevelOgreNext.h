#include <windows.h>
#include <windowsx.h>
#include <ole2.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <fstream>
#include <thread>
#include <strsafe.h>

#include "OgreAbiUtils.h"
#include "OgreArchiveManager.h"
#include "OgreCamera.h"
#include "OgreConfigFile.h"
#include "OgreRoot.h"
#include "OgreWindow.h"

#include "OgreHlmsManager.h"
#include "OgreHlmsPbs.h"
#include "OgreHlmsUnlit.h"
#include "OgreHlmsDiskCache.h"

#include "OgreOverlay.h"
#include "OgreOverlayContainer.h"
#include "OgreOverlayManager.h"
#include "OgreOverlaySystem.h"
#include "OgreTextAreaOverlayElement.h"

#include "OgreSingleton.h"
#include "OgreRenderSystem.h"

#include "OgreTextureGpuManager.h"
#include "OgreSceneManager.h"
#include "OgreMeshManager.h"
#include "OgreMeshManager2.h"
#include "OgreGpuProgramManager.h"
#include "OgreLogManager.h"

#include "OgrePlatformInformation.h"
#include "Compositor/OgreCompositorManager2.h"
#include "OgreWindowEventUtilities.h"
#include "System/Android/AndroidSystems.h"

#include "OgreItem.h"
#include "Animation/OgreSkeletonAnimation.h"
#include "Animation/OgreSkeletonInstance.h"
#include "Animation/OgreTagPoint.h"

#include "Threading/YieldTimer.h"
#include "OgreTimer.h"

#include "OgrePixelFormatGpuUtils.h"
#include "OgreTextureBox.h"
#include "OgreHardwareBufferManager.h"
#include "OgreMesh.h"
#include "OgreMesh2.h"

#include "OgreHlmsPbsDatablock.h"
#include "OgreHlmsSamplerblock.h"
#include "OgreHlmsUnlitDatablock.h"

static const Ogre::uint32 c_numAreaLights = 4u;
static const Ogre::uint32 c_areaLightsPoolId = 759384;
static const Ogre::uint32 c_defaultWidth = 512u;
static const Ogre::uint32 c_defaultHeight = 512u;
static const Ogre::PixelFormatGpu c_defaultFormat = Ogre::PFG_RGBA8_UNORM_SRGB;
static const Ogre::uint8 c_defaultNumMipmaps =
Ogre::PixelFormatGpuUtils::getMaxMipmapCount(c_defaultWidth, c_defaultHeight);

static const char c_lightRadiusKeys[4] = { 'Y', 'U', 'I', 'O' };
static const char c_lightFileKeys[4] = { 'H', 'J', 'K', 'L' };

inline float sdBox(const Ogre::Vector2& point, const Ogre::Vector2& center,
    const Ogre::Vector2& halfSize)
{
    Ogre::Vector2 p = point - center;
    Ogre::Vector2 d = Ogre::Vector2(abs(p.x), abs(p.y)) - halfSize;
    Ogre::Vector2 dCeil(d);
    dCeil.makeCeil(Ogre::Vector2::ZERO);
    return dCeil.length() + std::min(std::max(d.x, d.y), Ogre::Real(0.0f));
}
inline float sdAnnularBox(const Ogre::Vector2& point, const Ogre::Vector2& center,
    const Ogre::Vector2& halfSize, float r)
{
    return fabsf(sdBox(point, center, halfSize)) - r;
}
//------------------------------------------------------------------------------------------------------------------
class LowLevelOgreNext
{
public:
    LowLevelOgreNext(void);
    LowLevelOgreNext(HWND hwnd);
    virtual ~LowLevelOgreNext(void);

    Ogre::Root* Init(
        const Ogre::String pluginsFolder,
        const Ogre::String writeAccessFolder,
        bool UseMicrocodeCache, bool UseHlmsDiskCache,
        const char* pluginsFile,
        int& width, int& height, bool& fullscreen);
    void SetupResources(Ogre::String ResourcePath);
    void LoadResources(Ogre::String ResourcePath, const Ogre::String writeAccessFolder, bool UseMicrocodeCache, bool UseHlmsDiskCache);
    void AddResourceLocation(const Ogre::String& archName,
        const Ogre::String& typeName,
        const Ogre::String& secName);
    void LoadTextureCache(const Ogre::String writeAccessFolder);
    void SaveTextureCache(const Ogre::String writeAccessFolder);
    void LoadHlmsDiskCache(const Ogre::String writeAccessFolder,
        bool UseMicrocodeCache, bool UseHlmsDiskCache);
    void SaveHlmsDiskCache(const Ogre::String writeAccessFolder,
        bool UseMicrocodeCache, bool UseHlmsDiskCache);
    void RegisterHlms(Ogre::String ResourcePath);

    void CreateTextOverlay();
    void ChooseSceneManager();
    void CreateCamera();
    void initMiscParamsListener(Ogre::NameValuePairList& params) {}
    void GenerateDebugText(Ogre::String text);
    void DeInit(const Ogre::String writeAccessFolder, bool UseMicrocodeCache, bool UseHlmsDiskCache);
    void CreateScene01();
    void Update(float timeSinceLast);
    void CreateAreaMask(float radius, Ogre::Image2& outImage);
    Ogre::MeshPtr CreatePlaneV2(
        const Ogre::String& name, const Ogre::String& groupName, const Ogre::Plane& plane, Ogre::Real width, Ogre::Real height,
        Ogre::uint32 xsegments = 1, Ogre::uint32 ysegments = 1, bool normals = true,
        unsigned short numTexCoordSets = 1, Ogre::Real uTile = 1.0f, Ogre::Real vTile = 1.0f,
        const Ogre::Vector3& upVector = Ogre::Vector3::UNIT_Y,
        Ogre::v1::HardwareBuffer::Usage vertexBufferUsage = Ogre::v1::HardwareBuffer::HBU_STATIC_WRITE_ONLY,
        Ogre::v1::HardwareBuffer::Usage indexBufferUsage = Ogre::v1::HardwareBuffer::HBU_STATIC_WRITE_ONLY,
        bool vertexShadowBuffer = true, bool indexShadowBuffer = true);
    Ogre::HlmsDatablock* SetupDatablockTextureForLight(Ogre::Light* light, size_t idx);
    void CreatePlaneForAreaLight(Ogre::Light* light, size_t idx);
    void CreateLight(const Ogre::Vector3& position, size_t idx);
    void SetupLightTexture(size_t idx);
    void DestroyScene();
    const char* GetMediaReadArchiveType() const;

    HWND mWinHandle;
    Ogre::Root* mRoot;
    Ogre::Camera* mCamera;
    Ogre::SceneManager* mSceneMgr;
    Ogre::TextureGpuManager* mTextureMgr;
    Ogre::MeshManager* mMeshMgr;
    Ogre::v1::MeshManager* mMeshMgrV1;
    Ogre::Window* mWindow;
    Ogre::CompositorWorkspace* mWorkspace;
    Ogre::v1::OverlaySystem* mOverlaySystem;
    Ogre::CompositorManager2* mCompositorManager;
    Ogre::v1::TextAreaOverlayElement* mDebugText;
    Ogre::v1::TextAreaOverlayElement* mDebugTextShadow;
    Ogre::v1::OverlayManager* mOverlayManager;
    Ogre::v1::Overlay* mOverlay;
    Ogre::v1::OverlayContainer* mPanel;
    Ogre::String OutText;
    Ogre::Item* mItem;
    Ogre::SceneNode* nSceneNode;
    Ogre::SceneNode* mLightNodes[c_numAreaLights];
    Ogre::Light* mAreaLights[c_numAreaLights];
    Ogre::TextureGpu* mAreaMaskTex;
    Ogre::SkeletonAnimation* mAnyAnimation;
    Ogre::SkeletonAnimationVec mAnyAnimations;

    bool mUseTextureFromFile[c_numAreaLights];
    float mLightTexRadius[c_numAreaLights];
    bool mUseSynchronousMethod;
};


