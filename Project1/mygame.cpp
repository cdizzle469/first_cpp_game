#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <deque>
#include <string>
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

// Global variables
HINSTANCE hInst;
LPCWSTR szWindowClass = L"win32app";
LPCWSTR szTitle = L"PNG Background Example";
HBITMAP hBitmap = nullptr;

// Forward declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

Bitmap* LoadPNG(LPCWSTR szFilename) {

    Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromFile(szFilename);
    if (bitmap && !bitmap->GetLastStatus()) {
        return bitmap;
    }
    else {
        MessageBox(NULL, L"Failed to load PNG file.", L"Error", MB_OK | MB_ICONERROR);
        delete bitmap;
    }
    return nullptr;
}

Bitmap* bg;
Bitmap* enemy;
Bitmap* player;
Bitmap* playtext;
Bitmap* titletext;

UINT_PTR frames;
UINT_PTR enemyspawn;
UINT_PTR seconds;

int playerwidth;
int playerheight;
int enemywidth;
int enemyheight;

RECT clientRect;


int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
    
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    bg = LoadPNG(L"gamebg.png");
    enemy = LoadPNG(L"meteor.png");
    player = LoadPNG(L"myimage.png");
    playtext = LoadPNG(L"playtext.png");
    titletext = LoadPNG(L"titletext.png");

    playerwidth = player->GetWidth();
    playerheight = player->GetHeight();
    enemywidth = enemy->GetWidth();
    enemyheight = enemy->GetHeight();

    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    RegisterClassExW(&wcex);

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
    GetClientRect(hWnd, &clientRect);
    if (!hWnd) {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    delete bg;
    delete enemy;
    delete player;
    delete playtext;
    Gdiplus::GdiplusShutdown(gdiplusToken);

    return (int)msg.wParam;
}

int playerx = 0;
int playery = 0;

int score = 0;
int spawncount = 1;

bool up = false;
bool down = false;
bool right = false;
bool left = false;

bool popping = false;

std::deque<std::vector<int>> enemycoords;

bool menu = true;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        if (menu) {
            HDC compdc = CreateCompatibleDC(hdc);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
            HBITMAP hbmOld = (HBITMAP)SelectObject(compdc, hbmMem);
            Graphics graphics(compdc);
            graphics.Clear(Color(150, 41, 92));
            int width = playtext->GetWidth();
            int height = playtext->GetHeight();
            Pen pen(Color(0, 0, 0));
            SolidBrush brush(Color(255, 0, 0));
            Rect rect((clientRect.right / 2) - 160, (clientRect.bottom / 2) - 50, 320, 100);
            graphics.DrawRectangle(&pen, rect);
            graphics.FillRectangle(&brush, rect);
            graphics.DrawImage(playtext, (clientRect.right / 2) - (width / 2), (clientRect.bottom / 2) - (height/2), width, height);
            graphics.DrawImage(titletext, (clientRect.right / 2) - 325, 0, 750,200);
            BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, compdc, 0, 0, SRCCOPY);
            SelectObject(compdc, hbmOld);
            DeleteObject(hbmMem);
            DeleteDC(compdc);
            EndPaint(hWnd, &ps);
            break;
        }
        HDC hdcMem = CreateCompatibleDC(hdc);
        
        HBITMAP hbmMem = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
        HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);
        Graphics graphics(hdcMem);

        graphics.Clear(Color(255,255,255));
        if (bg) {
            graphics.DrawImage(bg, 0, 0, clientRect.right, clientRect.bottom);
        }
        if (player) {
            graphics.DrawImage(player, playerx, playery, playerheight/2, playerheight/2);
        }
        if (enemy) {
            for (auto& i : enemycoords) {
                if (i[0] < 0) {
                    popping = true;
                    score++;
                }
                if (i[0] - (enemywidth / 8) < playerx && i[0]+(enemywidth/8)>playerx && i[1]-(enemyheight/8) < playery && i[1]+(enemyheight/8)>playery) {
                    menu = true;
                }
                i[0] -= 2;
                graphics.DrawImage(enemy, i[0], i[1], enemywidth/4, enemyheight/4);
                
            }
        }
        if (popping) {
            enemycoords.pop_front();
            popping = false;
        }
        
        BitBlt(hdc,0,0,clientRect.right,clientRect.bottom, hdcMem, 0,0, SRCCOPY);
        
        SelectObject(hdcMem, hbmOld);
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);
        EndPaint(hWnd, &ps);
        if (menu) {
            wchar_t buffer[1024];
            swprintf(buffer, 1024, L"Score: %d", score);
            MessageBox(NULL, L"death", buffer, MB_OK);
            score = 0;
            enemycoords.clear();
            right = false;
            left = false;
            up = false;
            down = false;
            KillTimer(hWnd, enemyspawn);
            KillTimer(hWnd, frames);
            KillTimer(hWnd, seconds);
            spawncount = 1;
        }
    }
                 break;
    case WM_LBUTTONUP:
        if (!menu) {
            break;
        }
        POINT mousepos;
        if (GetCursorPos(&mousepos)) {
            ScreenToClient(hWnd, &mousepos);

            int x = mousepos.x;
            int y = mousepos.y;
            int rectLeft = (clientRect.right / 2) - 160;
            int rectRight = (clientRect.right / 2) + 160;
            int rectTop = (clientRect.bottom / 2) - 50;
            int rectBottom = (clientRect.bottom / 2) + 50;

            if (rectLeft < x && x < rectRight && rectTop < y && y < rectBottom) {
                frames = SetTimer(hWnd, 1, 30, NULL);
                enemyspawn = SetTimer(hWnd, 2, 1500, NULL);
                seconds = SetTimer(hWnd, 3, 10000, NULL);
                playerx = 0;
                playery = 0;
                menu = false;
            }

        }

        break;
    case WM_KEYDOWN: 
        switch (wParam) {
        case VK_RIGHT:
            right = true;
            break;
        case VK_LEFT:
            left = true;
            break;

        case VK_UP:
            up = true;
            break;
        case VK_DOWN:
            down = true;
            break;
        }
        break;
    case WM_KEYUP:
        switch (wParam) {
        case VK_RIGHT:
            right = false;
            break;
        case VK_LEFT:
            left = false;
            break;

        case VK_UP:
            up = false;
            break;
        case VK_DOWN:
            down = false;
            break;
        
        }
    case WM_TIMER:
        switch (wParam) {
        case 1:
        {
            if (right && playerx < 1000) playerx += 3;
            if (left && playerx > 0) playerx -= 3;
            if (up && playery > 0) playery -= 3;
            if (down && playery < 500) playery += 3;
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }
        case 2:
        {
            for (int i = 0; i < spawncount; i++) {
                int randy = rand() % 500;
                std::vector<int> coord = { 1000, randy };
                enemycoords.push_back(coord);
            }
            break;
        }
        case 3:
        {
            spawncount ++;
            break;
        }
        }
        

        break;
    case WM_DESTROY:
        KillTimer(hWnd, enemyspawn);
        KillTimer(hWnd, frames);
        KillTimer(hWnd, seconds);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
