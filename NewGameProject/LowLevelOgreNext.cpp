#include "LowLevelOgreNext.h"

//-------------------------------------------------------------------------------------
LowLevelOgreNext::LowLevelOgreNext(HWND hwnd) :
    mWinHandle(hwnd),
    mRoot(0),
    mCamera(0),
    mSceneMgr(0),
    mTextureMgr(0),
    mMeshMgr(0),
    mMeshMgrV1(0),
    mWindow(0),
    mWorkspace(0),
    mOverlaySystem(0),
    mCompositorManager(0),
    mDebugText(0),
    mDebugTextShadow(0),
    mOverlayManager(0),
    mOverlay(0),
    mItem(0),
    nSceneNode(0),
    mAreaMaskTex(0),
    mAnyAnimation(0),
    OutText("1")
{

}
LowLevelOgreNext::~LowLevelOgreNext(void) { /* Everything is already deleted in DeInit */ }
//-------------------------------------------------------------------------------------


Ogre::Root* LowLevelOgreNext::Init(
    const Ogre::String pluginsFolder,
    const Ogre::String writeAccessFolder,
    bool UseMicrocodeCache, bool UseHlmsDiskCache,
    const char* pluginsFile,
    int& width, int& height, bool& fullscreen)
{

    const Ogre::AbiCookie abiCookie = Ogre::generateAbiCookie();
    mRoot = OGRE_NEW Ogre::Root(&abiCookie, pluginsFolder + pluginsFile,  //
        writeAccessFolder + "ogre.cfg",           //
        writeAccessFolder + "Ogre.log");

    //--------------------------------------------------------------------------------------
    if (!mRoot->showConfigDialog())
        return NULL;

    mRoot->getRenderSystem()->setConfigOption("sRGB Gamma Conversion", "Yes");

    // Initialize Root
    mRoot->initialise(false);
    Ogre::ConfigOptionMap& cfgOpts = mRoot->getRenderSystem()->getConfigOptions();
    Ogre::ConfigOptionMap::iterator opt = cfgOpts.find("Video Mode");
    if (opt != cfgOpts.end() && !opt->second.currentValue.empty())
    {
        // Ignore leading space
        const Ogre::String::size_type start = opt->second.currentValue.find_first_of("012356789");
        // Get the width and height
        Ogre::String::size_type widthEnd = opt->second.currentValue.find(' ', start);
        // we know that the height starts 3 characters after the width and goes until the next space
        Ogre::String::size_type heightEnd = opt->second.currentValue.find(' ', widthEnd + 3);
        // Now we can parse out the values
        width = Ogre::StringConverter::parseInt(opt->second.currentValue.substr(0, widthEnd));
        height = Ogre::StringConverter::parseInt(opt->second.currentValue.substr(widthEnd + 3, heightEnd));
    }
    fullscreen = Ogre::StringConverter::parseBool(cfgOpts["Full Screen"].currentValue);

    Ogre::NameValuePairList params;
    //params["externalWindowHandle"] = Ogre::StringConverter::toString((int)hWnd);
    params.insert(std::make_pair("externalWindowHandle", Ogre::StringConverter::toString((int)mWinHandle)));
    params.insert(std::make_pair("gamma", cfgOpts["sRGB Gamma Conversion"].currentValue));
    if (cfgOpts.find("VSync Method") != cfgOpts.end())
        params.insert(std::make_pair("vsync_method", cfgOpts["VSync Method"].currentValue));
    params.insert(std::make_pair("FSAA", cfgOpts["FSAA"].currentValue));
    params.insert(std::make_pair("vsync", cfgOpts["VSync"].currentValue));
    params.insert(std::make_pair("reverse_depth", "Yes"));

    initMiscParamsListener(params);

    mWindow = mRoot->createRenderWindow("Main RenderWindow", static_cast<uint32_t>(width), static_cast<uint32_t>(height), fullscreen, &params);

    //***************************************************************
    // Setup overlay system
    mOverlaySystem = OGRE_NEW Ogre::v1::OverlaySystem();
    mOverlayManager = Ogre::v1::OverlayManager::getSingletonPtr();
    mOverlay = mOverlayManager->create("DebugText");

    Ogre::String ResourcePath = "";
    SetupResources(ResourcePath);
    LoadResources(ResourcePath, writeAccessFolder, true, true);
    // Create SceneManager
    ChooseSceneManager();
    // Create Overlay 
    CreateTextOverlay();
    // Create the 1st scene
    CreateScene01();
    // Setup a basic compositor with a blue clear colour
    mCompositorManager = mRoot->getCompositorManager2();
    // Setup workspace
    const Ogre::String workspaceName("Demo Workspace");
    const Ogre::ColourValue backgroundColour(0.2f, 0.4f, 0.6f);
    mCompositorManager->createBasicWorkspaceDef(workspaceName, backgroundColour, Ogre::IdString());
    mWorkspace = mCompositorManager->addWorkspace(mSceneMgr, mWindow->getTexture(), mCamera, workspaceName, true);

    return mRoot;
}
//-----------------------------------------------------------------------------------
void LowLevelOgreNext::SetupResources(Ogre::String ResourcePath)
{
    // Load resource paths from config file
    Ogre::ConfigFile cf;
    cf.load(Demo::AndroidSystems::openFile(ResourcePath + "resources2.cfg"));

    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap* settings = seci.getNext();

        if (secName != "Hlms")
        {
            Ogre::ConfigFile::SettingsMultiMap::iterator i;
            for (i = settings->begin(); i != settings->end(); ++i)
            {
                typeName = i->first;
                archName = i->second;
                AddResourceLocation(archName, typeName, secName);
            }
        }
    }
}
//-----------------------------------------------------------------------------------
void LowLevelOgreNext::LoadResources(Ogre::String ResourcePath, const Ogre::String writeAccessFolder, bool UseMicrocodeCache, bool UseHlmsDiskCache)
{
    RegisterHlms(ResourcePath);

    LoadTextureCache(writeAccessFolder);
    LoadHlmsDiskCache(writeAccessFolder, UseMicrocodeCache, UseHlmsDiskCache);

    // Initialise, parse scripts etc
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups(true);
}
//-----------------------------------------------------------------------------------
void LowLevelOgreNext::AddResourceLocation(const Ogre::String& archName, const Ogre::String& typeName, const Ogre::String& secName)
{
#if( OGRE_PLATFORM == OGRE_PLATFORM_APPLE ) || ( OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS )
    // OS X does not set the working directory relative to the app,
    // In order to make things portable on OS X we need to provide
    // the loading with it's own bundle path location
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
        Ogre::String(Ogre::macBundlePath() + "/" + archName), typeName, secName);
#else
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName);
#endif
}
//-----------------------------------------------------------------------------------
void LowLevelOgreNext::LoadTextureCache(const Ogre::String writeAccessFolder)
{
#if !OGRE_NO_JSON
    Ogre::ArchiveManager& archiveManager = Ogre::ArchiveManager::getSingleton();
    Ogre::Archive* rwAccessFolderArchive =
        archiveManager.load(writeAccessFolder, "FileSystem", true);
    try
    {
        const Ogre::String filename = "textureMetadataCache.json";
        if (rwAccessFolderArchive->exists(filename))
        {
            Ogre::DataStreamPtr stream = rwAccessFolderArchive->open(filename);
            std::vector<char> fileData;
            fileData.resize(stream->size() + 1);
            if (!fileData.empty())
            {
                stream->read(&fileData[0], stream->size());
                // Add null terminator just in case (to prevent bad input)
                fileData.back() = '\0';
                Ogre::TextureGpuManager* textureManager = mRoot->getRenderSystem()->getTextureGpuManager();
                textureManager->importTextureMetadataCache(stream->getName(), &fileData[0], false);
            }
        }
        else
        {
            Ogre::LogManager::getSingleton().logMessage("[INFO] Texture cache not found at " +
                writeAccessFolder +
                "/textureMetadataCache.json");
        }
    }
    catch (Ogre::Exception& e)
    {
        Ogre::LogManager::getSingleton().logMessage(e.getFullDescription());
    }

    archiveManager.unload(rwAccessFolderArchive);
#endif
}
//-----------------------------------------------------------------------------------
void LowLevelOgreNext::SaveTextureCache(const Ogre::String writeAccessFolder)
{
    if (mRoot->getRenderSystem())
    {
        Ogre::TextureGpuManager* textureManager = mRoot->getRenderSystem()->getTextureGpuManager();
        if (textureManager)
        {
            Ogre::String jsonString;
            textureManager->exportTextureMetadataCache(jsonString);
            const Ogre::String path = writeAccessFolder + "/textureMetadataCache.json";
            std::ofstream file(path.c_str(), std::ios::binary | std::ios::out);
            if (file.is_open())
                file.write(jsonString.c_str(), static_cast<std::streamsize>(jsonString.size()));
            file.close();
        }
    }
}
//-----------------------------------------------------------------------------------
void LowLevelOgreNext::LoadHlmsDiskCache(const Ogre::String writeAccessFolder, bool UseMicrocodeCache, bool UseHlmsDiskCache)
{
    if (!UseMicrocodeCache && !UseHlmsDiskCache)
        return;

    Ogre::HlmsManager* hlmsManager = mRoot->getHlmsManager();
    Ogre::HlmsDiskCache diskCache(hlmsManager);

    Ogre::ArchiveManager& archiveManager = Ogre::ArchiveManager::getSingleton();

    Ogre::Archive* rwAccessFolderArchive =
        archiveManager.load(writeAccessFolder, "FileSystem", true);

    if (UseMicrocodeCache)
    {
        // Make sure the microcode cache is enabled.
        Ogre::GpuProgramManager::getSingleton().setSaveMicrocodesToCache(true);
        const Ogre::String filename = "microcodeCodeCache.cache";
        if (rwAccessFolderArchive->exists(filename))
        {
            Ogre::DataStreamPtr shaderCacheFile = rwAccessFolderArchive->open(filename);
            Ogre::GpuProgramManager::getSingleton().loadMicrocodeCache(shaderCacheFile);
        }
    }

    if (UseHlmsDiskCache)
    {
        for (size_t i = Ogre::HLMS_LOW_LEVEL + 1u; i < Ogre::HLMS_MAX; ++i)
        {
            Ogre::Hlms* hlms = hlmsManager->getHlms(static_cast<Ogre::HlmsTypes>(i));
            if (hlms)
            {
                Ogre::String filename =
                    "hlmsDiskCache" + Ogre::StringConverter::toString(i) + ".bin";

                try
                {
                    if (rwAccessFolderArchive->exists(filename))
                    {
                        Ogre::DataStreamPtr diskCacheFile = rwAccessFolderArchive->open(filename);
                        diskCache.loadFrom(diskCacheFile);
                        diskCache.applyTo(hlms);
                    }
                }
                catch (Ogre::Exception&)
                {
                    Ogre::LogManager::getSingleton().logMessage(
                        "Error loading cache from " + writeAccessFolder + "/" + filename +
                        "! If you have issues, try deleting the file "
                        "and restarting the app");
                }
            }
        }
    }

    archiveManager.unload(writeAccessFolder);
}
//-----------------------------------------------------------------------------------
void LowLevelOgreNext::SaveHlmsDiskCache(const Ogre::String writeAccessFolder, bool UseMicrocodeCache, bool UseHlmsDiskCache)
{
    if (mRoot->getRenderSystem() && Ogre::GpuProgramManager::getSingletonPtr() &&
        (UseMicrocodeCache || UseHlmsDiskCache))
    {
        Ogre::HlmsManager* hlmsManager = mRoot->getHlmsManager();
        Ogre::HlmsDiskCache diskCache(hlmsManager);

        Ogre::ArchiveManager& archiveManager = Ogre::ArchiveManager::getSingleton();

        Ogre::Archive* rwAccessFolderArchive =
            archiveManager.load(writeAccessFolder, "FileSystem", false);

        if (UseHlmsDiskCache)
        {
            for (size_t i = Ogre::HLMS_LOW_LEVEL + 1u; i < Ogre::HLMS_MAX; ++i)
            {
                Ogre::Hlms* hlms = hlmsManager->getHlms(static_cast<Ogre::HlmsTypes>(i));
                if (hlms)
                {
                    diskCache.copyFrom(hlms);

                    Ogre::DataStreamPtr diskCacheFile = rwAccessFolderArchive->create(
                        "hlmsDiskCache" + Ogre::StringConverter::toString(i) + ".bin");
                    diskCache.saveTo(diskCacheFile);
                }
            }
        }

        if (Ogre::GpuProgramManager::getSingleton().isCacheDirty() && UseMicrocodeCache)
        {
            const Ogre::String filename = "microcodeCodeCache.cache";
            Ogre::DataStreamPtr shaderCacheFile = rwAccessFolderArchive->create(filename);
            Ogre::GpuProgramManager::getSingleton().saveMicrocodeCache(shaderCacheFile);
        }

        archiveManager.unload(writeAccessFolder);
    }
}
//-----------------------------------------------------------------------------------
void LowLevelOgreNext::RegisterHlms(Ogre::String ResourcePath)
{
    //using namespace Ogre;

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
    // Note:  macBundlePath works for iOS too. It's misnamed.
    const String resourcePath = Ogre::macResourcesPath();
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
    const String resourcePath = Ogre::macBundlePath() + "/";
#else

#endif

    Ogre::ConfigFile cf;
    cf.load(ResourcePath + "resources2.cfg");

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
    String rootHlmsFolder = macBundlePath() + '/' + cf.getSetting("DoNotUseAsResource", "Hlms", "");
#else
    Ogre::String rootHlmsFolder = ResourcePath + cf.getSetting("DoNotUseAsResource", "Hlms", "");
#endif

    if (rootHlmsFolder.empty())
        rootHlmsFolder = "./";
    else if (*(rootHlmsFolder.end() - 1) != '/')
        rootHlmsFolder += "/";

    // At this point rootHlmsFolder should be a valid path to the Hlms data folder

    Ogre::HlmsUnlit* hlmsUnlit = 0;
    Ogre::HlmsPbs* hlmsPbs = 0;

    // For retrieval of the paths to the different folders needed
    Ogre::String mainFolderPath;
    Ogre::StringVector libraryFoldersPaths;
    Ogre::StringVector::const_iterator libraryFolderPathIt;
    Ogre::StringVector::const_iterator libraryFolderPathEn;

    Ogre::ArchiveManager& archiveManager = Ogre::ArchiveManager::getSingleton();

    {
        // Create & Register HlmsUnlit
        // Get the path to all the subdirectories used by HlmsUnlit
        Ogre::HlmsUnlit::getDefaultPaths(mainFolderPath, libraryFoldersPaths);
        Ogre::Archive* archiveUnlit =
            archiveManager.load(rootHlmsFolder + mainFolderPath, "FileSystem", true);
        Ogre::ArchiveVec archiveUnlitLibraryFolders;
        libraryFolderPathIt = libraryFoldersPaths.begin();
        libraryFolderPathEn = libraryFoldersPaths.end();
        while (libraryFolderPathIt != libraryFolderPathEn)
        {
            Ogre::Archive* archiveLibrary =
                archiveManager.load(rootHlmsFolder + *libraryFolderPathIt, "FileSystem", true);
            archiveUnlitLibraryFolders.push_back(archiveLibrary);
            ++libraryFolderPathIt;
        }

        // Create and register the unlit Hlms
        hlmsUnlit = OGRE_NEW Ogre::HlmsUnlit(archiveUnlit, &archiveUnlitLibraryFolders);
        Ogre::Root::getSingleton().getHlmsManager()->registerHlms(hlmsUnlit);
    }

    {
        // Create & Register HlmsPbs
        // Do the same for HlmsPbs:
        Ogre::HlmsPbs::getDefaultPaths(mainFolderPath, libraryFoldersPaths);
        Ogre::Archive* archivePbs = archiveManager.load(rootHlmsFolder + mainFolderPath, "FileSystem", true);

        // Get the library archive(s)
        Ogre::ArchiveVec archivePbsLibraryFolders;
        libraryFolderPathIt = libraryFoldersPaths.begin();
        libraryFolderPathEn = libraryFoldersPaths.end();
        while (libraryFolderPathIt != libraryFolderPathEn)
        {
            Ogre::Archive* archiveLibrary =
                archiveManager.load(rootHlmsFolder + *libraryFolderPathIt, "FileSystem", true);
            archivePbsLibraryFolders.push_back(archiveLibrary);
            ++libraryFolderPathIt;
        }

        // Create and register
        hlmsPbs = OGRE_NEW Ogre::HlmsPbs(archivePbs, &archivePbsLibraryFolders);
        Ogre::Root::getSingleton().getHlmsManager()->registerHlms(hlmsPbs);
    }

    Ogre::RenderSystem* renderSystem = Ogre::Root::getSingletonPtr()->getRenderSystem();
    if (renderSystem->getName() == "Direct3D11 Rendering Subsystem")
    {
        // Set lower limits 512kb instead of the default 4MB per Hlms in D3D 11.0
        // and below to avoid saturating AMD's discard limit (8MB) or
        // saturate the PCIE bus in some low end machines.
        bool supportsNoOverwriteOnTextureBuffers;
        renderSystem->getCustomAttribute("MapNoOverwriteOnDynamicBufferSRV",
            &supportsNoOverwriteOnTextureBuffers);

        if (!supportsNoOverwriteOnTextureBuffers)
        {
            hlmsPbs->setTextureBufferDefaultSize(512 * 1024);
            hlmsUnlit->setTextureBufferDefaultSize(512 * 1024);
        }
    }
}
//-----------------------------------------------------------------------------------
void LowLevelOgreNext::ChooseSceneManager()
{
#if OGRE_DEBUG_MODE >= OGRE_DEBUG_HIGH
    // Debugging multithreaded code is a PITA, disable it.
    const size_t numThreads = 1;
#else
    // getNumLogicalCores() may return 0 if couldn't detect
    const size_t numThreads = std::max<size_t>(1, Ogre::PlatformInformation::getNumLogicalCores());
#endif
    // Create the SceneManager, in this case a generic one
    mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC, numThreads, "ExampleSMInstance");

    mSceneMgr->addRenderQueueListener(mOverlaySystem);
    mSceneMgr->getRenderQueue()->setSortRenderQueue(
        Ogre::v1::OverlayManager::getSingleton().mDefaultRenderQueueId,
        Ogre::RenderQueue::StableSort);

    // Set sane defaults for proper shadow mapping
    mSceneMgr->setShadowDirectionalLightExtrusionDistance(500.0f);
    mSceneMgr->setShadowFarDistance(500.0f);
}
//-----------------------------------------------------------------------------------
void LowLevelOgreNext::CreateTextOverlay()
{
    mPanel = static_cast<Ogre::v1::OverlayContainer*>(mOverlayManager->createOverlayElement("Panel", "DebugPanel"));
    mPanel->setMetricsMode(Ogre::v1::GuiMetricsMode::GMM_RELATIVE);
    mPanel->setDimensions(1, 1);
    mPanel->setPosition(0, 0);
    mDebugText = static_cast<Ogre::v1::TextAreaOverlayElement*>(mOverlayManager->createOverlayElement("TextArea", "DebugText"));
    mDebugText->setMetricsMode(Ogre::v1::GuiMetricsMode::GMM_RELATIVE);
    mDebugText->setPosition(0, 0);
    mDebugText->setFontName("DebugFont");
    mDebugText->setCharHeight(0.025f);

    mDebugTextShadow = static_cast<Ogre::v1::TextAreaOverlayElement*>(mOverlayManager->createOverlayElement("TextArea", "0DebugTextShadow"));
    mDebugTextShadow->setFontName("DebugFont");
    mDebugTextShadow->setCharHeight(0.025f);
    mDebugTextShadow->setColour(Ogre::ColourValue::Black);
    mDebugTextShadow->setPosition(0.002f, 0.002f);

    mPanel->addChild(mDebugTextShadow);
    mPanel->addChild(mDebugText);
    mOverlay->add2D(mPanel);
    mOverlay->show();
}
//-----------------------------------------------------------------------------------
void LowLevelOgreNext::CreateCamera()
{
    // Create & setup camera
    mCamera = mSceneMgr->createCamera("Main Camera");
    // Position it at 500 in Z direction
    mCamera->setPosition(Ogre::Vector3(0, 0, 25));
    // Look back along -Z
    mCamera->lookAt(Ogre::Vector3(0, 0, 0));
    mCamera->setNearClipDistance(0.2f);
    mCamera->setFarClipDistance(1000.0f);
    mCamera->setAutoAspectRatio(true);
}
//-----------------------------------------------------------------------------------
const char* LowLevelOgreNext::GetMediaReadArchiveType() const
{
#if OGRE_PLATFORM != OGRE_PLATFORM_ANDROID
    return "FileSystem";
#else
    return "APKFileSystem";
#endif
}
//-----------------------------------------------------------------------------------
void LowLevelOgreNext::GenerateDebugText(Ogre::String text)
{
    OutText.reserve(128);
    OutText = text;

    mDebugText->setCaption(OutText);
    mDebugTextShadow->setCaption(OutText);
}
//___________________________________________________________________________________
void LowLevelOgreNext::DeInit(const Ogre::String writeAccessFolder, bool UseMicrocodeCache, bool UseHlmsDiskCache)
{
    SaveTextureCache(writeAccessFolder);
    SaveHlmsDiskCache(writeAccessFolder, UseMicrocodeCache, UseHlmsDiskCache);

    if (mSceneMgr)
    {
        Ogre::AtmosphereComponent* atmosphere = mSceneMgr->getAtmosphereRaw();
        OGRE_DELETE atmosphere;

        mSceneMgr->removeRenderQueueListener(mOverlaySystem);
    }

    OGRE_DELETE mOverlaySystem;
    mOverlaySystem = 0;

#if OGRE_USE_SDL2
    delete mInputHandler;
    mInputHandler = 0;
#endif

    OGRE_DELETE mRoot;
    mRoot = 0;

#if OGRE_USE_SDL2
    if (mSdlWindow)
    {
        // Restore desktop resolution on exit
        SDL_SetWindowFullscreen(mSdlWindow, 0);
        SDL_DestroyWindow(mSdlWindow);
        mSdlWindow = 0;
    }

    SDL_Quit();
#endif
}
//___________________________________________________________________________________
void LowLevelOgreNext::CreateScene01()
{
    mTextureMgr = mRoot->getRenderSystem()->getTextureGpuManager();
    //-----------------------------------------------------------------------------------------
    mMeshMgr = Ogre::MeshManager::getSingletonPtr();
    mMeshMgrV1 = Ogre::v1::MeshManager::getSingletonPtr();

    Ogre::MeshPtr v2Mesh;
    Ogre::MeshPtr v2Floor;

    {  // Prepare the mesh
        v2Mesh = mMeshMgr->load(
            "Cube.003.mesh", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
            Ogre::BufferType::BT_DEFAULT, Ogre::BufferType::BT_DEFAULT);
        //Ogre::TextureGpuManager::getSingleton()
    }

    nSceneNode = mSceneMgr->getRootSceneNode(Ogre::SCENE_DYNAMIC)->createChildSceneNode(Ogre::SCENE_DYNAMIC);

    mItem = mSceneMgr->createItem(v2Mesh, Ogre::SCENE_DYNAMIC);  // Create an Item (v2mesh)
    nSceneNode->attachObject(mItem);                                // Attach Item (v2mesh) to a Node, now Item execute comands through the Node 
    nSceneNode->setPosition(0, 0, 0);                               // Set the node (v2mesh)'s initial position
    //nSceneNode->setDirection(0, 0, 0);                              // Set the node (v2mesh)'s initial direction
    //nSceneNode->setOrientation(Ogre::Quaternion(1, 1, 1, 1));       // Set the node (v2mesh)'s initial orientation

    {
        // Set up floor
        v2Floor = CreatePlaneV2(
            "Floor", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            Ogre::Plane(Ogre::Vector3::UNIT_Y, -10.0f), 25.0f, 25.0f, 1, 1, true, 1, 4.0f, 4.0f,
            Ogre::Vector3::UNIT_Z, Ogre::v1::HardwareBuffer::HBU_STATIC,
            Ogre::v1::HardwareBuffer::HBU_STATIC);

        Ogre::Item* item = mSceneMgr->createItem(v2Floor, Ogre::SCENE_DYNAMIC);
        item->setDatablock("MirrorLike");
        Ogre::SceneNode* sceneNode = mSceneMgr->getRootSceneNode(Ogre::SCENE_DYNAMIC)->createChildSceneNode(Ogre::SCENE_DYNAMIC);
        sceneNode->attachObject(item);
        sceneNode->setPosition(0, -1, 0);
    }

    {
        // Import animation from .skeleton file
        Ogre::SkeletonInstance* skeletonInstance = mItem->getSkeletonInstance();
        skeletonInstance->addAnimationsFromSkeleton(
            "Cube.003.skeleton", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
        mAnyAnimations = skeletonInstance->getAnimations();
        mAnyAnimation = &mAnyAnimations[3];
        mAnyAnimation->setEnabled(true);
    }

    // Main directional light
    Ogre::Light* light = mSceneMgr->createLight();
    Ogre::SceneNode* rootNode = mSceneMgr->getRootSceneNode();
    Ogre::SceneNode* lightNode = rootNode->createChildSceneNode();
    lightNode->attachObject(light);
    light->setPowerScale(1.0f);
    light->setType(Ogre::Light::LT_DIRECTIONAL);
    light->setDirection(Ogre::Vector3(-1, -1, -1).normalisedCopy());

    // Create a 2D plane and load texture from .PNG file
    Ogre::SceneNode* PlaneNode_1 = CreateTexturePlane(
        Ogre::Plane(Ogre::Vector3::UNIT_Z, 0.0f), Ogre::Vector3::UNIT_Y,
        -Ogre::Plane(Ogre::Vector3::UNIT_Z, 0.0f).normal,
        "plane_1", "material_1", "alias_1", "10points.PNG", 759384);
    PlaneNode_1->setPosition(5, 0, 0);
    // Rotate the plane around the x_axis
    //PlaneNode_1->pitch(Ogre::Radian(Ogre::Degree(45.0f)), Ogre::Node::TS_LOCAL);

    // Create Camera
    CreateCamera();
    //mCamera->setPosition(Ogre::Vector3(0, 0, 1));
    //mCamera->setProjectionType(Ogre::PT_ORTHOGRAPHIC);

}
//___________________________________________________________________________________
void LowLevelOgreNext::Update(float timeSinceLast)
{

}
//-----------------------------------------------------------------------------------
Ogre::MeshPtr LowLevelOgreNext::CreatePlaneV2(
    const Ogre::String& name, const Ogre::String& groupName, const Ogre::Plane& plane, Ogre::Real width, Ogre::Real height,
    Ogre::uint32 xsegments, Ogre::uint32 ysegments, bool normals, unsigned short numTexCoordSets, Ogre::Real uTile, Ogre::Real vTile,
    const Ogre::Vector3& upVector, Ogre::v1::HardwareBuffer::Usage vertexBufferUsage, Ogre::v1::HardwareBuffer::Usage indexBufferUsage,
    bool vertexShadowBuffer, bool indexShadowBuffer)
{
    // Create manual mesh which calls back self to load
    Ogre::v1::MeshPtr planeMeshV1 = mMeshMgrV1->createPlane(
        name + "v1", groupName, plane, width, height, xsegments, ysegments, normals, numTexCoordSets, uTile, vTile,
        upVector, vertexBufferUsage, indexBufferUsage, vertexShadowBuffer, indexShadowBuffer);

    Ogre::MeshPtr planeMesh = mMeshMgr->createByImportingV1(
        name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, planeMeshV1.get(), true,
        true, true);

    planeMeshV1->unload();
    //mMeshMgrV1->remove(planeMeshV1);

    return planeMesh;
}
//-----------------------------------------------------------------------------------
Ogre::HlmsDatablock* LowLevelOgreNext::SetupDatablockTextureForLight(Ogre::Light* light, const Ogre::String materialName)
{
    Ogre::Hlms* hlmsUnlit = mRoot->getHlmsManager()->getHlms(Ogre::HLMS_UNLIT);

    // Setup an unlit material, double-sided, with textures
    //(if it has one) and same colour as the light.
    // IMPORTANT: these materials are never destroyed once they're not needed (they will
    // be destroyed by Ogre on shutdown). Watchout for this to prevent memory leaks in
    // a real implementation
    Ogre::HlmsMacroblock macroblock;
    macroblock.mCullMode = Ogre::CULL_NONE;
    Ogre::HlmsDatablock* datablockBase = hlmsUnlit->getDatablock(materialName);

    if (!datablockBase)
    {
        datablockBase = hlmsUnlit->createDatablock(materialName, materialName, macroblock,
            Ogre::HlmsBlendblock(), Ogre::HlmsParamVec());
    }

    assert(dynamic_cast<Ogre::HlmsUnlitDatablock*>(datablockBase));
    Ogre::HlmsUnlitDatablock* datablock = static_cast<Ogre::HlmsUnlitDatablock*>(datablockBase);

    if (light->mTextureLightMaskIdx != std::numeric_limits<Ogre::uint16>::max())
    {
        Ogre::HlmsSamplerblock samplerblock;
        samplerblock.mMaxAnisotropy = 8.0f;
        samplerblock.setFiltering(Ogre::TFO_ANISOTROPIC);

        datablock->setTexture(0, light->getTexture(), &samplerblock);
    }

    datablock->setUseColour(true);
    datablock->setColour(light->getDiffuseColour());

    return datablock;
}
//-----------------------------------------------------------------------------------
Ogre::SceneNode* LowLevelOgreNext::CreateTexturePlane(
    const Ogre::Plane& plane, const Ogre::Vector3& upVector, const Ogre::Vector3& lightVector,
    const Ogre::String meshName, const Ogre::String materialName,
    const Ogre::String aliasName, const Ogre::String textureName,
    static const Ogre::uint32 areaLightsPoolId)
{
    // Create Light for the plane-------------------------------------------------
    Ogre::Light* Planelight = mSceneMgr->createLight();
    Ogre::SceneNode* PlanelightNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    PlanelightNode->attachObject(Planelight);
    Planelight->setDiffuseColour(1.0f, 1.0f, 1.0f);
    Planelight->setSpecularColour(1.0f, 1.0f, 1.0f);
    // Increase the strength 10x to showcase this light. Area approx lights are not
    // physically based so the value is more arbitrary than the other light types
    Planelight->setPowerScale(Ogre::Math::PI);
    Planelight->setType(Ogre::Light::LT_AREA_APPROX);
    Planelight->setRectSize(Ogre::Vector2(5.0f, 5.0f));
    PlanelightNode->setPosition(Ogre::Vector3(0.0f, 0.0f, 0.0f));
    Planelight->setDirection(lightVector.normalisedCopy());
    //Planelight->setDirection(-plane.normal.normalisedCopy());
    Planelight->setAttenuationBasedOnRadius(10.0f, 0.01f);

    // Create the mesh for light plane
    Ogre::MeshPtr v2LightPlane = CreatePlaneV2(
        meshName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        plane, 1.0f, 1.0f, 1, 1, true, 1, 1.0f, 1.0f,
        upVector, Ogre::v1::HardwareBuffer::HBU_STATIC,
        Ogre::v1::HardwareBuffer::HBU_STATIC);

    // Setup datablock texture for light
    Ogre::HlmsDatablock* datablock = SetupDatablockTextureForLight(Planelight, materialName);

    // Create the plane Item
    //Ogre::SceneNode* planeNode = PlanelightNode->createChildSceneNode(Ogre::SCENE_DYNAMIC);

    //Ogre::Item* item = mSceneMgr->createItem(meshName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    Ogre::Item* item = mSceneMgr->createItem(v2LightPlane, Ogre::SCENE_DYNAMIC);
    item->setCastShadows(false);
    item->setDatablock(datablock);
    //planeNode->attachObject(item);
    PlanelightNode->attachObject(item);

    // Match the plane size to that of the area light
    const Ogre::Vector2 rectSize = Planelight->getRectSize();
    //planeNode->setScale(rectSize.x, rectSize.y, 1.0f);
    PlanelightNode->setScale(rectSize.x, rectSize.y, 1.0f);

    // Setup light texture-------------------------------------------------------
    Ogre::TextureGpu* areaTex = mTextureMgr->findTextureNoThrow(aliasName);
    if (areaTex)
        mTextureMgr->destroyTexture(areaTex);

    areaTex = mTextureMgr->createOrRetrieveTexture(
        textureName, aliasName,
        Ogre::GpuPageOutStrategy::Discard, Ogre::CommonTextureTypes::Diffuse,
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, areaLightsPoolId);

    Planelight->setTexture(areaTex);
    SetupDatablockTextureForLight(Planelight, materialName);

    return PlanelightNode;
}

