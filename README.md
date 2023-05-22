# OgreNext_GameProject
Simple project for learning to use OgreNext as renderer

LowLevelOgreNext class:\
_Used as renderer, does not affect logic\
_Has basic utils functions\
_Can load meshes, skeleton and animations\
_Can blit images to 2d planes (for GUI)\

WinMain:\
_Has fullscreen toggle\
_Can separate inputs from multiple keyboards with raw kb input\
_Receive messages and run renderer on different threads(renderer run at fixed framerate: 60fps)\

Setting up:\
_This project is windows-only at the moment\
_Install OgreNext\
_Go to the folder where you built OgreNext => bin => debug (or release), copy all the .dll and .cfg files and replace the files I provided in "this repository/bin/debug-x64(or release-x64)/NewGameProject" for good measure\
_The paths in resources2.cfg should be updated to fit the location where you installed OgreNext\
_In resource2.cfg, add a path to "this repository/OGRE_NEXT_MESH" (This is for the custom resouces in this project, the other resouces should come with OgreNext after you built it)\
