# OgreNext_GameProject
Simple project for learning to use OgreNext as renderer

LowLevelOgreNext class:
_Used as renderer, does not affect logic
_Has basic utils functions
_Can load meshes, skeleton and animations
_Can blit images to 2d planes (for GUI)

WinMain:
_Has fullscreen toggle
_Can separate inputs from multiple keyboards with raw kb input
_Receive messages and run renderer on different threads(renderer run at fixed framerate)
