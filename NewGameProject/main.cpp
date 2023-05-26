//---------------------------------------------------------------------------------------
// This tutorial shows nothing fancy. Only how to setup Ogre to render to
// a window, without any dependency.
//
// This samples has many hardcoded paths (e.g. it assumes the current working directory has R/W access)
// which means it will work in Windows and Linux, Mac may work.
//
// See the next tutorials on how to handles all OSes and how to properly setup a robust render loop
//---------------------------------------------------------------------------------------
#pragma comment(lib, "Comctl32.lib")
#define STRICT

#include "LowLevelOgreNext.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#    include "OSX/macUtils.h"
#endif

HWND g_hwndChild;                           /* Optional child window */

WINDOWPLACEMENT g_wpPrev = { sizeof(g_wpPrev) };

bool* terminate_flag_ptr = new bool;


void FullScreenToggle(HWND hwnd, int x, int y, UINT keyFlags)
{
    DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
    if (dwStyle & WS_POPUP) {
        MONITORINFO mi = { sizeof(mi) };
        if (GetWindowPlacement(hwnd, &g_wpPrev) && GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi))
        {
            SetWindowLong(hwnd, GWL_STYLE, dwStyle & ~WS_POPUP);
            SetWindowPos(hwnd, HWND_TOP,
                mi.rcMonitor.left, mi.rcMonitor.top,
                mi.rcMonitor.right - mi.rcMonitor.left,
                mi.rcMonitor.bottom - mi.rcMonitor.top,
                SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLong(hwnd, GWL_STYLE, dwStyle | WS_POPUP);
        SetWindowPlacement(hwnd, &g_wpPrev);
        SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
            SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}


POINT mouse_cursor;

void OnMouse(HWND hwnd, int x, int y, UINT keyFlags)
{
    if (GetCursorPos(&mouse_cursor))
    {
        ScreenToClient(hwnd, &mouse_cursor);
    }
}


TCHAR buffer[256];

#define HANDLE_WM_INPUT(hwnd, wParam, lParam, fn) \
  ((fn)((hwnd), GET_RAWINPUT_CODE_WPARAM(wParam), \
        (HRAWINPUT)(lParam)), 0)

void OnInput(HWND hwnd, WPARAM code, HRAWINPUT hRawInput)
{
    UINT dwSize;
    GetRawInputData(hRawInput, RID_INPUT, nullptr,
        &dwSize, sizeof(RAWINPUTHEADER));
    RAWINPUT* input = (RAWINPUT*)malloc(dwSize);
    GetRawInputData(hRawInput, RID_INPUT, input,
        &dwSize, sizeof(RAWINPUTHEADER));
    if (input->header.dwType == RIM_TYPEKEYBOARD) {
        TCHAR prefix[80];
        prefix[0] = TEXT('\0');     // go down a line every input
        if (input->data.keyboard.Flags & RI_KEY_E0) {
            StringCchCat(prefix, ARRAYSIZE(prefix), TEXT("E0 "));
        }
        if (input->data.keyboard.Flags & RI_KEY_E1) {
            StringCchCat(prefix, ARRAYSIZE(prefix), TEXT("E1 "));
        }

        StringCchPrintf(buffer, ARRAYSIZE(buffer),
            TEXT("%p, msg=%04x, vk=%04x, scanCode=%s%02x, %s"),
            input->header.hDevice,
            input->data.keyboard.Message,
            input->data.keyboard.VKey,
            prefix,
            input->data.keyboard.MakeCode,
            (input->data.keyboard.Flags & RI_KEY_BREAK)
            ? TEXT("release") : TEXT("press"));
    }
    DefRawInputProc(&input, 1, sizeof(RAWINPUTHEADER));
    free(input);
}


void OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    if (g_hwndChild) {
        MoveWindow(g_hwndChild, 0, 0, cx, cy, TRUE);
    }
}


BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpcs)
{
    RAWINPUTDEVICE dev;
    dev.usUsagePage = 1;
    dev.usUsage = 6;
    dev.dwFlags = 0;
    dev.hwndTarget = hwnd;
    RegisterRawInputDevices(&dev, 1, sizeof(dev));

    return TRUE;
}


void OnDestroy(HWND hwnd)
{
    RAWINPUTDEVICE dev;
    dev.usUsagePage = 1;
    dev.usUsage = 6;
    dev.dwFlags = RIDEV_REMOVE;
    dev.hwndTarget = hwnd;
    RegisterRawInputDevices(&dev, 1, sizeof(dev));

    *terminate_flag_ptr = true;
    PostQuitMessage(0);
}


void PaintContent(HWND hwnd, PAINTSTRUCT* pps)
{
}


void OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    BeginPaint(hwnd, &ps);
    PaintContent(hwnd, &ps);
    EndPaint(hwnd, &ps);
}


void OnPrintClient(HWND hwnd, HDC hdc)
{
    PAINTSTRUCT ps;
    ps.hdc = hdc;
    GetClientRect(hwnd, &ps.rcPaint);
    PaintContent(hwnd, &ps);
}


//------------------------------------------------------------------------------------

const wchar_t g_szClassName[] = L"myWindowClass";

class YieldTimer
{
    Ogre::Timer* mExternalTimer;

public:
    YieldTimer(Ogre::Timer* externalTimer) : mExternalTimer(externalTimer) {}

    Ogre::uint64 yield(double frameTime, Ogre::uint64 startTime)
    {
        Ogre::uint64 endTime = mExternalTimer->getMicroseconds();

        while (frameTime * 1000000.0 > double(endTime - startTime))
        {
            endTime = mExternalTimer->getMicroseconds();

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            SwitchToThread();
#elif OGRE_PLATFORM == OGRE_PLATFORM_LINUX || OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
            sched_yield();
#endif
        }

        return endTime;
    }
};

void render_thread(LowLevelOgreNext app, Ogre::Root* m_Root, bool* terminate_flag_ptr)
{
    const Ogre::String writeAccessFolder = "./";
    Ogre::Timer timer;
    YieldTimer yieldTimer(&timer);  // Create yield timer for fixed frame rate
    Ogre::uint64 startTime = timer.getMicroseconds();
    double render_FrameTime = 1.0 / 60.0;
    std::string str;
    std::wstring wstr;

    while (!*terminate_flag_ptr)
    {
#ifndef UNICODE
        str = buffer;
#else
        wstr = buffer;
        str = std::string(wstr.begin(), wstr.end());
#endif

        app.GenerateDebugText(
            Ogre::StringConverter::toString(startTime / 1000000.0)
            + "\nanimation: "
            + app.mAnyAnimation->getName().getFriendlyText()
            + "\nmouse:"
            + Ogre::StringConverter::toString(mouse_cursor.x)
            + ", "
            + Ogre::StringConverter::toString(mouse_cursor.y)
            + "\nmulti-keyboard inputs:\n"
            + str);
        app.mAnyAnimation->addTime(render_FrameTime);
        m_Root->renderOneFrame();
        startTime = yieldTimer.yield(render_FrameTime, startTime);
    }
    //app.DeInit(writeAccessFolder, true, true);  
}

// Step 4: the Window Procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uiMsg) {
        HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
        HANDLE_MSG(hwnd, WM_SIZE, OnSize);
        HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
        HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
        HANDLE_MSG(hwnd, WM_LBUTTONUP, FullScreenToggle);
        HANDLE_MSG(hwnd, WM_MOUSEMOVE, OnMouse);
        HANDLE_MSG(hwnd, WM_INPUT, OnInput);
    case WM_PRINTCLIENT: OnPrintClient(hwnd, (HDC)wParam); return 0;
    }
    return DefWindowProc(hwnd, uiMsg, wParam, lParam);
}

BOOL InitApp(HINSTANCE hInst)
{
    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return FALSE;
    }
    InitCommonControls();               /* In case we use a common control */
    return TRUE;
}


#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR strCmdLine, INT nCmdShow)
#else
int main(int argc, const char* argv[])
#endif
{
    //using namespace Ogre;

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
    const String pluginsFolder = macResourcesPath();
    const String writeAccessFolder = macLogPath();
#else
    const Ogre::String pluginsFolder = "./";
    const Ogre::String writeAccessFolder = pluginsFolder;
#endif

#ifndef OGRE_STATIC_LIB
#    if OGRE_DEBUG_MODE && \
        !( ( OGRE_PLATFORM == OGRE_PLATFORM_APPLE ) || ( OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS ) )
    const char* pluginsFile = "plugins_d.cfg";
#    else
    const char* pluginsFile = "plugins.cfg";
#    endif
#else
    const char* pluginsFile = 0;  // TODO
#endif

    HWND hwnd;
    MSG Msg;
    int width, height;
    bool fullscreen;
    float frame_time = 1.0 / 60.0;

    //Step 1: Registering the Window Class
    if (!InitApp(hInst)) return 0;

    // Step 2: Creating the Window
    hwnd = CreateWindowEx(NULL, g_szClassName, L"The title of my window", WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, 240, 120, NULL, NULL, hInst, NULL);

    if (hwnd == NULL)
    {
        MessageBox(NULL, L"Window Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    UpdateWindow(hwnd);

    LowLevelOgreNext app(hwnd);// Create application object

    Ogre::Root* m_Root = app.Init(pluginsFolder, writeAccessFolder, true, true, pluginsFile, width, height, fullscreen);
    if (m_Root == NULL)
        return 1;

    // Set the window to config options: fullscreen, wifth, height
    DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
    MONITORINFO mi = { sizeof(mi) };
    if (GetWindowPlacement(hwnd, &g_wpPrev) && GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi))
    {
        SetWindowPos(hwnd, HWND_TOP,
            (mi.rcMonitor.right - mi.rcMonitor.left - width) / 2,
            (mi.rcMonitor.bottom - mi.rcMonitor.top - height) / 2,
            width, height,
            SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
    if (fullscreen)
    {
        if (dwStyle & WS_POPUP)
        {
            if (GetWindowPlacement(hwnd, &g_wpPrev) && GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi))
            {
                SetWindowLong(hwnd, GWL_STYLE, dwStyle & ~WS_POPUP);
                SetWindowPos(hwnd, HWND_TOP,
                    mi.rcMonitor.left, mi.rcMonitor.top,
                    mi.rcMonitor.right - mi.rcMonitor.left,
                    mi.rcMonitor.bottom - mi.rcMonitor.top,
                    SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
            }
        }
        else
        {
            SetWindowLong(hwnd, GWL_STYLE, dwStyle | WS_POPUP);
            SetWindowPlacement(hwnd, &g_wpPrev);
            SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }

    *terminate_flag_ptr = false;
    std::thread t1(render_thread, app, m_Root, terminate_flag_ptr);
    ShowWindow(hwnd, nCmdShow);

    // Step 3: The Message Loop
    while (GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    t1.join();
    app.DeInit(writeAccessFolder, true, true);
    delete terminate_flag_ptr;

    return Msg.wParam;
}
