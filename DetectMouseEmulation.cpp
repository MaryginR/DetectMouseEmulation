#include "DetectMouseEmulationHeader.h"

std::chrono::steady_clock::time_point LastMouseMessage;
bool EmulatingDetected = false;

static LRESULT CALLBACK targetWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INPUT: {
            UINT dwSize;
            static BYTE lpb[40];
            unsigned short defaultmove = 0;

            GetRawInputData((HRAWINPUT)lParam, RID_INPUT,
                lpb, &dwSize, sizeof(RAWINPUTHEADER));

            RAWINPUT* raw = (RAWINPUT*)lpb;

            if (raw->header.dwType == RIM_TYPEMOUSE) 
            {
                //hDevice == 0x0000000000000000 означает что девайс, с которого получен raw пакет не определен
                if (raw->header.hDevice == 0x0000000000000000)
                {
                    EmulatingDetected = true;
                }

                LastMouseMessage = std::chrono::steady_clock::now();
            }

            break;
            return 0;
        }
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

static bool CheckCursor()
{
    POINT CurrentCursorPos, LastCursorPos;

    GetCursorPos(&LastCursorPos);

    while (true)
    {
        GetCursorPos(&CurrentCursorPos);

        int deltaX = CurrentCursorPos.x - LastCursorPos.x;
        int deltaY = CurrentCursorPos.y - LastCursorPos.y;

        if (deltaX != 0 || deltaY != 0)
        {
            auto msFromLastMessage = std::chrono::steady_clock::now();

            auto elapsedTicks = std::chrono::duration_cast<std::chrono::milliseconds>(msFromLastMessage - LastMouseMessage);

            if (elapsedTicks.count() > 200)
            {
                EmulatingDetected = true;
            }

            LastCursorPos = CurrentCursorPos;
        }

        Sleep(1);
    }
}

static void ScanMouseMoves()
{
    HINSTANCE hInstance = GetModuleHandle(NULL);

    WNDCLASS wc = {};
    wc.lpfnWndProc = targetWindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = TEXT("MyRawInputWnd");

    if (!RegisterClass(&wc))
        throw "failed to register WNDCLASS!";

    HWND targetWindow = CreateWindowEx(0, wc.lpszClassName, NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);
    if (!targetWindow)
        throw "failed to create target window for scanning!";

    //register the monitoring device
    RAWINPUTDEVICE rid = {};
    rid.usUsagePage = 0x01; //Mouse
    rid.usUsage = 0x02;
    rid.dwFlags = RIDEV_INPUTSINK;
    rid.hwndTarget = targetWindow;

    if (!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
        throw "failed to register raw input device!";

    MSG msg;

    LastMouseMessage = std::chrono::steady_clock::now();
    std::future<bool> future_result = std::async(std::launch::async, CheckCursor);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        Sleep(1);
    }

    DestroyWindow(targetWindow);
}