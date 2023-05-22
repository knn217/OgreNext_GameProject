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
    mUseSynchronousMethod(false),
    OutText("1")
{
    OGRE_STATIC_ASSERT(sizeof(c_lightRadiusKeys) / sizeof(c_lightRadiusKeys[0]) >=
        c_numAreaLights);
    OGRE_STATIC_ASSERT(sizeof(c_lightFileKeys) / sizeof(c_lightFileKeys[0]) >=
        c_numAreaLights);

    memset(mAreaLights, 0, sizeof(mLightNodes));
    memset(mAreaLights, 0, sizeof(mAreaLights));
    memset(mUseTextureFromFile, 0, sizeof(mUseTextureFromFile));

    for (size_t i = 0; i < c_numAreaLights; ++i)
    {
        mLightTexRadius[i] = 0.05f;
    }
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
    // Create Camera
    CreateCamera();
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

    // Setup resource for decals and area light texture
    Ogre::String dataFolder = cf.getSetting("DoNotUseAsResource", "Hlms", "");

    if (dataFolder.empty())
        dataFolder = Demo::AndroidSystems::isAndroid() ? "/" : "./";
    else if (*(dataFolder.end() - 1) != '/')
        dataFolder += "/";

    const size_t baseSize = dataFolder.size();

    dataFolder.resize(baseSize);
    dataFolder += "2.0/scripts/materials/PbsMaterials";
    AddResourceLocation(dataFolder, GetMediaReadArchiveType(), "General");
    dataFolder.resize(baseSize);
    dataFolder += "2.0/scripts/materials/UpdatingDecalsAndAreaLightTex";
    AddResourceLocation(dataFolder, GetMediaReadArchiveType(), "General");
}
//-----------------------------------------------------------------------------------
void LowLevelOgreNext::LoadResources(Ogre::String ResourcePath, const Ogre::String writeAccessFolder, bool UseMicrocodeCache, bool UseHlmsDiskCache)
{
    RegisterHlms(ResourcePath);

    LoadTextureCache(writeAccessFolder);
    LoadHlmsDiskCache(writeAccessFolder, UseMicrocodeCache, UseHlmsDiskCache);

    // Initialise, parse scripts etc
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups(true);

    // Initialize resources for LTC area lights and accurate specular reflections (IBL)
    Ogre::Hlms* hlms = mRoot->getHlmsManager()->getHlms(Ogre::HLMS_PBS);
    OGRE_ASSERT_HIGH(dynamic_cast<Ogre::HlmsPbs*>(hlms));
    Ogre::HlmsPbs* hlmsPbs = static_cast<Ogre::HlmsPbs*>(hlms);
    try
    {
        hlmsPbs->loadLtcMatrix();
    }
    catch (Ogre::FileNotFoundException& e)
    {
        Ogre::LogManager::getSingleton().logMessage(e.getFullDescription(), Ogre::LML_CRITICAL);
        Ogre::LogManager::getSingleton().logMessage(
            "WARNING: LTC matrix textures could not be loaded. Accurate specular IBL reflections "
            "and LTC area lights won't be available or may not function properly!",
            Ogre::LML_CRITICAL);
    }
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
    mCamera->setPosition(Ogre::Vector3(0, 5, 15));
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
    // Reserve/create the texture for the area lights
    mAreaMaskTex = mTextureMgr->reservePoolId(c_areaLightsPoolId, c_defaultWidth, c_defaultHeight, c_numAreaLights, c_defaultNumMipmaps, c_defaultFormat);
    // Set the texture mask to PBS.
    Ogre::Hlms* hlms = mRoot->getHlmsManager()->getHlms(Ogre::HLMS_PBS);
    assert(dynamic_cast<Ogre::HlmsPbs*>(hlms));
    Ogre::HlmsPbs* pbs = static_cast<Ogre::HlmsPbs*>(hlms);

    pbs->setAreaLightMasks(mAreaMaskTex);
    pbs->setAreaLightForwardSettings(c_numAreaLights, 0u);
    //-----------------------------------------------------------------------------------------
    mMeshMgr = Ogre::MeshManager::getSingletonPtr();
    mMeshMgrV1 = Ogre::v1::MeshManager::getSingletonPtr();

    Ogre::MeshPtr v2Mesh;
    Ogre::MeshPtr v2Floor = CreatePlaneV2(
        "Floor", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        Ogre::Plane(Ogre::Vector3::UNIT_Y, -10.0f), 50.0f, 50.0f, 1, 1, true, 1, 4.0f, 4.0f,
        Ogre::Vector3::UNIT_Z, Ogre::v1::HardwareBuffer::HBU_STATIC,
        Ogre::v1::HardwareBuffer::HBU_STATIC);

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
        Ogre::Item* item = mSceneMgr->createItem(v2Floor, Ogre::SCENE_DYNAMIC);
        item->setDatablock("MirrorLike");
        Ogre::SceneNode* sceneNode = mSceneMgr->getRootSceneNode(Ogre::SCENE_DYNAMIC)
            ->createChildSceneNode(Ogre::SCENE_DYNAMIC);
        sceneNode->setPosition(0, -1, 0);
        sceneNode->attachObject(item);
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

    // Create the mesh template for all the lights (i.e. the billboard-like plane)

    {
        Ogre::MeshPtr LightPlaneV2 = CreatePlaneV2(
            "LightPlane", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            Ogre::Plane(Ogre::Vector3::UNIT_Z, 0.0f), 1.0f, 1.0f, 1, 1, true, 1, 1.0f, 1.0f,
            Ogre::Vector3::UNIT_Y, Ogre::v1::HardwareBuffer::HBU_STATIC,
            Ogre::v1::HardwareBuffer::HBU_STATIC);
    }


    // Main directional light
    Ogre::Light* light = mSceneMgr->createLight();
    Ogre::SceneNode* rootNode = mSceneMgr->getRootSceneNode();
    Ogre::SceneNode* lightNode = rootNode->createChildSceneNode();
    lightNode->attachObject(light);
    light->setPowerScale(1.0f);
    light->setType(Ogre::Light::LT_DIRECTIONAL);
    light->setDirection(Ogre::Vector3(-1, -1, -1).normalisedCopy());

    for (size_t i = 0; i < c_numAreaLights; ++i)
    {
        // Create light planes at different locations
        CreateLight(
            Ogre::Vector3((Ogre::Real(i) - (c_numAreaLights - 1u) * 0.5f) * 10, 4.0f, 0.0f),
            i);
        // Toggle config for texture usage
        mUseTextureFromFile[i] = !mUseTextureFromFile[i];
        // Setup light texture from config
        SetupLightTexture(i);
    }

    //mUseTextureFromFile[1] = !mUseTextureFromFile[1];
    //SetupLightTexture(1);
}
//___________________________________________________________________________________
void LowLevelOgreNext::Update(float timeSinceLast)
{
    GenerateDebugText("2");
    mAnyAnimation->addTime(timeSinceLast);
}
//-----------------------------------------------------------------------------------
void LowLevelOgreNext::CreateAreaMask(float radius, Ogre::Image2& outImage)
{
    // Please note the texture CAN be coloured. The sample uses a monochrome texture,
    // but you coloured textures are supported too. However they will burn a little
    // more GPU performance.
    const Ogre::uint32 texWidth = c_defaultWidth;
    const Ogre::uint32 texHeight = c_defaultHeight;
    const Ogre::PixelFormatGpu texFormat = c_defaultFormat;

    // Fill the texture with a hollow rectangle, 10-pixel thick.
    size_t sizeBytes = Ogre::PixelFormatGpuUtils::calculateSizeBytes(texWidth, texHeight, 1u, 1u,
        texFormat, 1u, 4u);
    Ogre::uint8* data =
        reinterpret_cast<Ogre::uint8*>(OGRE_MALLOC_SIMD(sizeBytes, Ogre::MEMCATEGORY_GENERAL));
    outImage.loadDynamicImage(data, texWidth, texHeight, 1u, Ogre::TextureTypes::Type2D, texFormat,
        true, 1u);

    const float invTexWidth = 1.0f / texWidth;
    const float invTexHeight = 1.0f / texHeight;
    for (size_t y = 0; y < texHeight; ++y)
    {
        for (size_t x = 0; x < texWidth; ++x)
        {
            const Ogre::Vector2 uv(Ogre::Real(x) * invTexWidth, Ogre::Real(y) * invTexHeight);

            const float d = sdAnnularBox(uv, Ogre::Vector2(0.5f), Ogre::Vector2(0.3f), radius);
            if (d <= 0)
            {
                *data++ = 255;
                *data++ = 255;
                *data++ = 255;
                *data++ = 255;
            }
            else
            {
                *data++ = 0;
                *data++ = 0;
                *data++ = 0;
                *data++ = 0;
            }
        }
    }

    // Generate the mipmaps so roughness works
    outImage.generateMipmaps(true, Ogre::Image2::FILTER_GAUSSIAN_HIGH);

    {
        // Ensure the lower mips have black borders. This is done to prevent certain artifacts,
        // Ensure the higher mips have grey borders. This is done to prevent certain artifacts.
        for (size_t i = 0u; i < outImage.getNumMipmaps(); ++i)
        {
            Ogre::TextureBox dataBox = outImage.getData(static_cast<Ogre::uint8>(i));

            const Ogre::uint8 borderColour = i >= 5u ? 127u : 0u;
            const Ogre::uint32 currWidth = dataBox.width;
            const Ogre::uint32 currHeight = dataBox.height;

            const size_t bytesPerRow = dataBox.bytesPerRow;

            memset(dataBox.at(0, 0, 0), borderColour, bytesPerRow);
            memset(dataBox.at(0, (currHeight - 1u), 0), borderColour, bytesPerRow);

            for (size_t y = 1; y < currWidth - 1u; ++y)
            {
                Ogre::uint8* left = reinterpret_cast<Ogre::uint8*>(dataBox.at(0, y, 0));
                left[0] = borderColour;
                left[1] = borderColour;
                left[2] = borderColour;
                left[3] = borderColour;
                Ogre::uint8* right =
                    reinterpret_cast<Ogre::uint8*>(dataBox.at(currWidth - 1u, y, 0));
                right[0] = borderColour;
                right[1] = borderColour;
                right[2] = borderColour;
                right[3] = borderColour;
            }
        }
    }
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
Ogre::HlmsDatablock* LowLevelOgreNext::SetupDatablockTextureForLight(Ogre::Light* light, size_t idx)
{
    Ogre::Hlms* hlmsUnlit = mRoot->getHlmsManager()->getHlms(Ogre::HLMS_UNLIT);

    // Setup an unlit material, double-sided, with textures
    //(if it has one) and same colour as the light.
    // IMPORTANT: these materials are never destroyed once they're not needed (they will
    // be destroyed by Ogre on shutdown). Watchout for this to prevent memory leaks in
    // a real implementation
    const Ogre::String materialName = "LightPlane Material" + Ogre::StringConverter::toString(idx);
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
void LowLevelOgreNext::CreatePlaneForAreaLight(Ogre::Light* light, size_t idx)
{
    Ogre::HlmsDatablock* datablock = SetupDatablockTextureForLight(light, idx);

    // Create the plane Item
    Ogre::SceneNode* lightNode = light->getParentSceneNode();
    Ogre::SceneNode* planeNode = lightNode->createChildSceneNode();

    Ogre::Item* item = mSceneMgr->createItem(
        "LightPlane", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    item->setCastShadows(false);
    item->setDatablock(datablock);
    planeNode->attachObject(item);

    // Math the plane size to that of the area light
    const Ogre::Vector2 rectSize = light->getRectSize();
    planeNode->setScale(rectSize.x, rectSize.y, 1.0f);

    /* For debugging ranges & AABBs
    Ogre::WireAabb *wireAabb = sceneManager->createWireAabb();
    wireAabb->track( light );*/
}
//-----------------------------------------------------------------------------------
void LowLevelOgreNext::CreateLight(const Ogre::Vector3& position, size_t idx)
{
    Ogre::SceneNode* rootNode = mSceneMgr->getRootSceneNode();

    Ogre::Light* light = mSceneMgr->createLight();
    Ogre::SceneNode* lightNode = rootNode->createChildSceneNode();
    lightNode->attachObject(light);
    light->setDiffuseColour(1.0f, 1.0f, 1.0f);
    light->setSpecularColour(1.0f, 1.0f, 1.0f);
    // Increase the strength 10x to showcase this light. Area approx lights are not
    // physically based so the value is more arbitrary than the other light types
    light->setPowerScale(Ogre::Math::PI);
    light->setType(Ogre::Light::LT_AREA_APPROX);
    light->setRectSize(Ogre::Vector2(5.0f, 5.0f));
    lightNode->setPosition(position);
    light->setDirection(Ogre::Vector3(0, 0, 1).normalisedCopy());
    light->setAttenuationBasedOnRadius(10.0f, 0.01f);

    //        //Control the diffuse mip (this is the default value)
    //        light->mTexLightMaskDiffuseMipStart = (Ogre::uint16)(0.95f * 65535);

    CreatePlaneForAreaLight(light, idx);

    mAreaLights[idx] = light;
    mLightNodes[idx] = lightNode;
}
//-----------------------------------------------------------------------------------
void LowLevelOgreNext::SetupLightTexture(size_t idx)
{
    mTextureMgr = mRoot->getRenderSystem()->getTextureGpuManager();

    if (!mUseTextureFromFile[idx])
    {
        Ogre::TextureGpu* areaTex = mTextureMgr->createOrRetrieveTexture(
            "AreaLightMask" + Ogre::StringConverter::toString(idx),
            Ogre::GpuPageOutStrategy::AlwaysKeepSystemRamCopy, Ogre::TextureFlags::AutomaticBatching,
            Ogre::TextureTypes::Type2D, Ogre::BLANKSTRING, 0u, c_areaLightsPoolId);

        Ogre::Image2 image;
        CreateAreaMask(mLightTexRadius[idx], image);

        bool canUseSynchronousUpload =
            areaTex->getNextResidencyStatus() == Ogre::GpuResidency::Resident &&
            areaTex->isDataReady();
        if (mUseSynchronousMethod && canUseSynchronousUpload)
        {
            // If canUseSynchronousUpload is false, you can use areaTex->waitForData()
            // to still use sync method (assuming the texture is resident)
            image.uploadTo(areaTex, 0, areaTex->getNumMipmaps() - 1u);
        }
        else
        {
            // Asynchronous is preferred due to being done in the background. But the switch
            // Resident -> OnStorage -> Resident may cause undesired effects, so we
            // show how to do it synchronously

            // Tweak via _setAutoDelete so the internal data is copied as a pointer
            // instead of performing a deep copy of the data; while leaving the responsability
            // of freeing memory to imagePtr instead.
            image._setAutoDelete(false);
            Ogre::Image2* imagePtr = new Ogre::Image2(image);
            imagePtr->_setAutoDelete(true);

            if (areaTex->getNextResidencyStatus() == Ogre::GpuResidency::Resident)
                areaTex->scheduleTransitionTo(Ogre::GpuResidency::OnStorage);
            // Ogre will call "delete imagePtr" when done, because we're passing
            // true to autoDeleteImage argument in scheduleTransitionTo
            areaTex->scheduleTransitionTo(Ogre::GpuResidency::Resident, imagePtr, true);

            mAreaLights[idx]->setTexture(areaTex);
        }
    }
    else
    {
        const Ogre::String aliasName = "AreaLightMask" + Ogre::StringConverter::toString(idx);
        Ogre::TextureGpu* areaTex = mTextureMgr->findTextureNoThrow(aliasName);
        if (areaTex)
            mTextureMgr->destroyTexture(areaTex);

        // We know beforehand that floor_bump.PNG & co are 512x512. This is important!!!
        //(because it must match the resolution of the texture created via reservePoolId)
        const char* textureNames[4] = { "floor_bump.PNG", "grassWalpha.tga", "MtlPlat2.jpg",
                                        "Panels_Normal_Obj.png" };

        areaTex = mTextureMgr->createOrRetrieveTexture(
            textureNames[idx % 4u], "AreaLightMask" + Ogre::StringConverter::toString(idx),
            Ogre::GpuPageOutStrategy::Discard, Ogre::CommonTextureTypes::Diffuse,
            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, c_areaLightsPoolId);
        mAreaLights[idx]->setTexture(areaTex);
    }

    SetupDatablockTextureForLight(mAreaLights[idx], idx);

    if (!mUseSynchronousMethod)
    {
        // If we don't wait, textures will flicker during async upload.
        // If you don't care about the glitch, avoid this call
        mTextureMgr->waitForStreamingCompletion();
    }
}
//-----------------------------------------------------------------------------------
void LowLevelOgreNext::DestroyScene()
{
    mTextureMgr = mRoot->getRenderSystem()->getTextureGpuManager();

    for (size_t i = 0; i < c_numAreaLights; ++i)
    {
        if (mAreaLights[i])
            mTextureMgr->destroyTexture(mAreaLights[i]->getTexture());
    }

    // Don't forget to destroy mAreaMaskTex, otherwise this pool will leak!!!
    if (mAreaMaskTex)
    {
        mTextureMgr->destroyTexture(mAreaMaskTex);
        mAreaMaskTex = 0;
    }
}