//--------------------------------------------------------------------------

#include <windows.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <time.h>

//--------------------------------------------------------------------------

#define EXIT_STATUS_SUCCESS 0
#define EXIT_STATUS_FAILURE -1
#define TIMER_SET_EVENT     1
#define TIMER_REMOVE_EVENT  2

//------------------------- Defining Global Constants ----------------------

const LPCSTR gc_BSODText = "Your PC didn't ran into any problems and its completely fine. We're \nnot collecting any info"
                        " nor harming your pc so everything is \ngood.\n\n20% complete";
const LPCSTR gc_BSODInfoText = "For More information about this issue and more fixes, don't visit https://doors.com/stopcode\n"
                               "\n\nIf You call a support person, give them this info:\n"
                               "Stop code: YOUR_MOM_IS_FAT";

const LPCSTR gc_windowClass = "BSODWindowClass";

//------------------------- Defining Global Variables ----------------------

IAudioEndpointVolume* g_iAudioEndpointVolume = NULL;
int g_CurrentTimerEvent;

//------------------------- Defining Function Headers -----------------------

void WindowTimerProcedure(HWND windowHandle, UINT uMessage, UINT_PTR idEvent, DWORD dwCurrentTime);
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
HWND CreateBSODWindowHandle(HINSTANCE hInstance, LPCSTR lpcWindowClass);
ATOM CreateBSODWindowClass(HINSTANCE hInstance, LPCSTR lpcWindowClass);
void LoadQrCode(HDC hdc, LPCSTR path, INT x, INT y);
void ShowErrorBox(LPCSTR message, BOOLEAN bExit);
int  RandomInRange(int nMin, int nMax);
int MinuteToMiliSec(int nMinutes);
void SetMute(WINBOOL bMute);
LOGFONT CreateDefaultFontStruct();
BOOL InitVolumeEndPoint();
WPARAM RunMessageLoop();

//------------------------- Program Entry Point ----------------------------

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    ShowWindow(GetConsoleWindow(), SW_HIDE); // Hides console window

    if (!CreateBSODWindowClass(hInstance, gc_windowClass)) {
        ShowErrorBox("Failed to Register Window Class!", TRUE);
    }

    HWND windowHandle;
    if ((windowHandle = CreateBSODWindowHandle(hInstance, gc_windowClass)) == NULL) {
        ShowErrorBox("Failed to create window handle!", TRUE);
    }
    ShowWindow(windowHandle, SW_HIDE);
    UpdateWindow(windowHandle);

    if (InitVolumeEndPoint() == FALSE){
        ShowErrorBox("Failed to initialize AudioEndpoint!", TRUE);
    }

    g_CurrentTimerEvent = TIMER_SET_EVENT;
    SetTimer(windowHandle, 0, MinuteToMiliSec(10), WindowTimerProcedure);

    return (int) RunMessageLoop();
}

//------------------------- Function Definitions ----------------------------

ATOM CreateBSODWindowClass(HINSTANCE hInstance, LPCSTR lpcWindowClass){
    /* Creates and registers the window class used by the program
     */
    WNDCLASSEX  windowClass;
    windowClass.cbSize          =   sizeof(windowClass);
    windowClass.style           =   CS_NOCLOSE | CS_HREDRAW;
    windowClass.lpfnWndProc     =   WindowProcedure;
    windowClass.cbClsExtra      =   0;
    windowClass.cbWndExtra      =   0;
    windowClass.hInstance       =   hInstance;
    windowClass.hIcon           =   LoadIcon(NULL, IDI_APPLICATION);
    windowClass.hCursor         =   LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground   =   CreateSolidBrush(RGB(0,120,215));
    windowClass.lpszClassName   =   lpcWindowClass;
    windowClass.lpszMenuName    =   NULL;
    windowClass.hIconSm         =   LoadIcon(NULL, IDI_APPLICATION);
    return RegisterClassEx(&windowClass);
}

HWND CreateBSODWindowHandle(HINSTANCE hInstance, LPCSTR lpcWindowClass){
    /* Creates a window handle from the registered window class name
     * then returns the handle.
     */
    HWND windowHandle;
    windowHandle = CreateWindowEx(
            WS_EX_TOPMOST,
            lpcWindowClass,
            "",
            0,
            0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
            NULL, NULL, hInstance, NULL);
    SetWindowLong(windowHandle, GWL_STYLE, 0);
    return windowHandle;

}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
    /* the main window message handler.
     */
    switch(msg)
    {
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(EXIT_STATUS_SUCCESS);
            break;
        case WM_PAINT:{

            // Getting BSOD window device context handle, and setting background and foreground of the window.
            HDC hdc = GetWindowDC(hwnd);
            SetTextColor(hdc, RGB(255, 255, 255));
            SetBkColor(hdc, RGB(0,120,215));

            // this font struct will be used as the base of each text output, with some minor changes.
            LOGFONT fontStruct = CreateDefaultFontStruct();

            HFONT textFont;
            textFont = CreateFontIndirect(&fontStruct);
            SelectObject(hdc, textFont); // Setting DCs font to the new font.

            // writing the main part of the text
            RECT bsodTextRect = {207, 400, 1000, 600};
            DrawText(hdc, gc_BSODText, (int) strlen(gc_BSODText), &bsodTextRect, DT_LEFT);

            fontStruct.lfHeight = 130; // changing the size of the font
            textFont = CreateFontIndirect(&fontStruct);
            SelectObject(hdc, textFont); // re-setting the font the new one

            RECT smileRect = {205, 240, 320, 365};
            DrawText(hdc, ":)", 2, &smileRect, DT_LEFT);

            fontStruct.lfHeight = 20;
            textFont = CreateFontIndirect(&fontStruct);
            SelectObject(hdc, textFont);

            RECT infoRect = {327, 600, 1000, 800};
            DrawText(hdc, gc_BSODInfoText, (int) strlen(gc_BSODInfoText), &infoRect, DT_LEFT);

            DeleteObject(textFont);

            LoadQrCode(hdc, "qr.bmp", 207, 600);

            ReleaseDC(hwnd, hdc);
        }
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void WindowTimerProcedure(HWND windowHandle, UINT uMessage, UINT_PTR idEvent, DWORD dwCurrentTime){
    /* The timer handle that shows and hides the bsod windows at random times
     */

    if (g_CurrentTimerEvent == TIMER_SET_EVENT){
        MessageBeep(MB_ICONWARNING);
        Sleep(1000);
        SetMute(TRUE);

        // Setting window to the Top of the Z-Order and showing it.
        SetWindowPos(windowHandle, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN),
                     GetSystemMetrics(SM_CYSCREEN), SWP_SHOWWINDOW);

        // setting the next timer event and re-setting the timer
        g_CurrentTimerEvent = TIMER_REMOVE_EVENT;
        SetTimer(windowHandle, 0, 5000, WindowTimerProcedure);

    }else if (g_CurrentTimerEvent == TIMER_REMOVE_EVENT){
        SetMute(FALSE);
        ShowWindow(windowHandle, SW_HIDE);
        g_CurrentTimerEvent = TIMER_SET_EVENT;

        //Setting a timer that will get triggered in a time between 20 and 60 minutes
        SetTimer(windowHandle, 0, RandomInRange(MinuteToMiliSec(20),
                                                MinuteToMiliSec(60)), WindowTimerProcedure);
    }


}

WPARAM RunMessageLoop(){
    /* starts the window message loop that keeps the window running.
     */
    MSG windowMessage;
    while(GetMessage(&windowMessage, NULL, 0, 0) > 0)
    {
        TranslateMessage(&windowMessage);
        DispatchMessage(&windowMessage);
    }
    return windowMessage.wParam;
}


LOGFONT CreateDefaultFontStruct(){
    /* Returns a base font for all the texts used in the BSOD window.
     */
    LOGFONT fontStruct;
    memset(&fontStruct, 0, sizeof(LOGFONT)); // clearing garbage from struct
    fontStruct.lfHeight         = 30;
    fontStruct.lfEscapement     = 0;
    fontStruct.lfOrientation    = 0;
    fontStruct.lfWeight         = FW_THIN;
    fontStruct.lfItalic         = FALSE;
    fontStruct.lfStrikeOut      = FALSE;
    fontStruct.lfUnderline      = FALSE;
    fontStruct.lfCharSet        = DEFAULT_CHARSET;
    fontStruct.lfOutPrecision   = OUT_OUTLINE_PRECIS;
    fontStruct.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
    fontStruct.lfQuality        = CLEARTYPE_QUALITY;
    fontStruct.lfPitchAndFamily = VARIABLE_PITCH;
    strcpy(fontStruct.lfFaceName, "Arial");
    return fontStruct;
}

void LoadQrCode(HDC hdc, LPCSTR path, INT x, INT y){
    /* Loads and outputs the qr-code image into the window.
     */
    BITMAP qrBitMap;
    HBITMAP qrBitMapHandle = (HBITMAP) LoadImage(NULL, path, IMAGE_BITMAP, 0, 0,
                                                 LR_DEFAULTSIZE | LR_LOADFROMFILE);
    HDC hdcMem = CreateCompatibleDC(hdc);
    HBITMAP hbmOld = SelectObject(hdcMem, qrBitMapHandle);
    GetObject(qrBitMapHandle, sizeof(qrBitMap), &qrBitMap);
    StretchBlt(hdc, x, y, 100, 100, hdcMem, 0, 0, qrBitMap.bmWidth, qrBitMap.bmHeight, SRCCOPY);
    SelectObject(hdcMem, hbmOld);
    DeleteObject(hdcMem);
}

BOOL InitVolumeEndPoint(){
    /* Initializing the audio endpoint for muting/unmuting the master volume
     */
    HRESULT res;

    CoInitialize(NULL);

    // Creating an IMMDeviceEnum for getting the Audio Endpoint.
    IMMDeviceEnumerator* immDeviceEnumerator = NULL;
    res = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_INPROC_SERVER, &IID_IMMDeviceEnumerator,
                     (LPVOID*) &immDeviceEnumerator);
    if (res != S_OK){return FALSE;}

    // Getting the Audio endpoint from the IMMDeviceEnum
    IMMDevice* immDevice = NULL;
    res = immDeviceEnumerator->lpVtbl->GetDefaultAudioEndpoint(
            immDeviceEnumerator, eRender, eConsole, &immDevice);
    if (res != S_OK){return FALSE;}

    immDeviceEnumerator->lpVtbl->Release(immDeviceEnumerator);
    immDeviceEnumerator = NULL;

    // Getting an AudioEndpointVolume interface from the MMDevice inteface and setting the global var.
    res = immDevice->lpVtbl->Activate(immDevice, &IID_IAudioEndpointVolume, CLSCTX_INPROC_SERVER, NULL,
                                      (LPVOID *) &g_iAudioEndpointVolume);
    if (res != S_OK){return FALSE;}

    immDevice->lpVtbl->Release(immDevice);
    immDevice = NULL;

    return TRUE;
}

void SetMute(WINBOOL bMute){
    /* Sends a mute or unmute command to the global AudioEndpointVolume interface
     */
    g_iAudioEndpointVolume->lpVtbl->SetMute(g_iAudioEndpointVolume, bMute, NULL);
}

int RandomInRange(int nMin, int nMax){
    /* returns a random number within [nMin, nMax)
     */
    int unixTime = (int)time(NULL);     // getting unix time stamps
    srand(unixTime);                   // setting rand() seed
    return nMin +  rand() % (nMax - nMin);  // taking a random number within [min, max)
}

void ShowErrorBox(LPCSTR lpcMessage, BOOLEAN bExit){
    /* Shows an Error Box to the user
     */
    MessageBox(NULL, lpcMessage, "ERROR!", MB_OK | MB_ICONERROR);
    if (bExit){exit(EXIT_STATUS_FAILURE);}
}

int MinuteToMiliSec(int nMinutes){
    return 60000 * nMinutes;
}