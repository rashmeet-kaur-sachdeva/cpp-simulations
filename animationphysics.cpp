#define _USE_MATH_DEFINES
#include <windows.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>

const int WIDTH = 1000;
const int VIRTUAL_HEIGHT = 1550;
int current_scroll_y = 0;        
int current_n = 1;
DWORD startTime = 0;

struct Color {
    float r, g, b; 
};

std::vector<unsigned char> frameBuffer(WIDTH * VIRTUAL_HEIGHT * 3);
HDC hdcMem = NULL;
HBITMAP hbmMem = NULL;
HBITMAP hbmOld = NULL;

void setPixel(int x, int y, Color col, float intensity = 1.0f) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= VIRTUAL_HEIGHT) return;
    int idx = (y * WIDTH + x) * 3;
    
    float currentR = frameBuffer[idx + 0] / 255.0f;
    float currentG = frameBuffer[idx + 1] / 255.0f;
    float currentB = frameBuffer[idx + 2] / 255.0f;

    float r = std::min(1.0f, currentR + col.r * intensity);
    float g = std::min(1.0f, currentG + col.g * intensity);
    float b = std::min(1.0f, currentB + col.b * intensity);

    frameBuffer[idx + 0] = static_cast<unsigned char>(r * 255);
    frameBuffer[idx + 1] = static_cast<unsigned char>(g * 255);
    frameBuffer[idx + 2] = static_cast<unsigned char>(b * 255);
}

void drawLine(int x0, int y0, int x1, int y1, Color col, float intensity = 1.0f) {
    int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    while (true) {
        setPixel(x0, y0, col, intensity);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void drawArrowHeadUp(int x, int y, Color col) {
    for (int i = 0; i < 7; ++i) {
        drawLine(x - i, y + i, x, y, col, 1.0f);
        drawLine(x + i, y + i, x, y, col, 1.0f);
    }
}

void drawArrowHeadRight(int x, int y, Color col) {
    for (int i = 0; i < 7; ++i) {
        drawLine(x - i, y - i, x, y, col, 1.0f);
        drawLine(x - i, y + i, x, y, col, 1.0f);
    }
}

void drawGlowParticle(double cx, double cy, double radius, Color col) {
    int xmin = std::max(0, static_cast<int>(cx - radius * 3.0));
    int xmax = std::min(WIDTH - 1, static_cast<int>(cx + radius * 3.0));
    int ymin = std::max(0, static_cast<int>(cy - radius * 3.0));
    int ymax = std::min(VIRTUAL_HEIGHT - 1, static_cast<int>(cy + radius * 3.0));

    for (int y = ymin; y <= ymax; ++y) {
        for (int x = xmin; x <= xmax; ++x) {
            double dx = x - cx;
            double dy = y - cy;
            double distSq = dx * dx + dy * dy;
            double radSq = radius * radius;
            
            if (distSq <= radSq * 9.0) {
                double dist = std::sqrt(distSq);
                float intensity = static_cast<float>(std::exp(-dist / (radius * 0.35)));
                setPixel(x, y, col, intensity);
            }
        }
    }
}

void RenderScene() {
    double t = static_cast<double>(GetTickCount() - startTime) / 1000.0;

    Color bg = {0.03f, 0.04f, 0.08f};         
    Color wallColor = {0.9f, 0.95f, 1.0f};    
    Color axisColor = {0.35f, 0.45f, 0.65f};  
    Color psiColor = {0.2f, 0.85f, 1.0f};     
    Color activePsiColor = {1.0f, 0.3f, 0.6f};
    Color energyColor = {1.0f, 0.85f, 0.2f};  
    Color particleColor = {1.0f, 0.2f, 0.4f}; 
    Color boxBg = {0.06f, 0.08f, 0.12f};      
    Color redArrowColor = {0.9f, 0.2f, 0.2f};

    // Reset background
    for (int i = 0; i < WIDTH * VIRTUAL_HEIGHT; ++i) {
        frameBuffer[i * 3 + 0] = static_cast<unsigned char>(bg.r * 255);
        frameBuffer[i * 3 + 1] = static_cast<unsigned char>(bg.g * 255);
        frameBuffer[i * 3 + 2] = static_cast<unsigned char>(bg.b * 255);
    }

    // Panels Background
    for (int y = 70; y <= 450; ++y)
        for (int x = 60; x <= WIDTH - 60; ++x) setPixel(x, y, boxBg, 0.4f);

    for (int y = 490; y <= 980; ++y)
        for (int x = 60; x <= WIDTH - 60; ++x) setPixel(x, y, boxBg, 0.4f);

    for (int y = 1020; y <= 1500; ++y)
        for (int x = 60; x <= WIDTH - 60; ++x) setPixel(x, y, boxBg, 0.4f);

    // Diagram 1: Potential Well
    double wellLeft = 280.0, wellRight = 720.0;
    double wellTop = 130.0, wellBottom = 380.0;

    drawLine((int)wellLeft, (int)wellTop, (int)wellLeft, (int)wellBottom, wallColor, 1.5f);
    drawLine((int)wellRight, (int)wellTop, (int)wellRight, (int)wellBottom, wallColor, 1.5f);
    drawLine((int)wellLeft, (int)wellBottom, (int)wellRight, (int)wellBottom, wallColor, 1.5f);

    double particlePosNorm = 0.5 + 0.4 * std::sin(t * 3.0 * current_n);
    double particleX = wellLeft + particlePosNorm * (wellRight - wellLeft);
    double particleY = (wellTop + wellBottom) / 2.0;
    drawGlowParticle(particleX, particleY, 10.0, particleColor);

    drawLine((int)wellLeft, (int)wellBottom + 30, (int)wellRight, (int)wellBottom + 30, redArrowColor, 1.2f);
    drawArrowHeadRight((int)wellRight, (int)wellBottom + 30, redArrowColor);
    drawArrowHeadRight((int)wellLeft, (int)wellBottom + 30, redArrowColor);

    // Diagram 2: Wavefunctions
    double psiAxisY = 920.0;
    double psiLeftAxisX = 160.0;
    double psiRightWallX = 840.0;
    double psiInnerWidth = psiRightWallX - psiLeftAxisX;

    drawLine((int)psiLeftAxisX, 550, (int)psiLeftAxisX, (int)psiAxisY, redArrowColor, 1.2f);
    drawArrowHeadUp((int)psiLeftAxisX, 550, redArrowColor);

    drawLine((int)psiLeftAxisX - 30, (int)psiAxisY, (int)psiRightWallX + 50, (int)psiAxisY, redArrowColor, 1.2f);
    drawArrowHeadRight((int)psiRightWallX + 50, (int)psiAxisY, redArrowColor);

    drawLine((int)psiLeftAxisX, 580, (int)psiLeftAxisX, (int)psiAxisY, wallColor, 1.5f);
    drawLine((int)psiRightWallX, 580, (int)psiRightWallX, (int)psiAxisY, wallColor, 1.5f);

    double stateHeights[4] = { psiAxisY - 60, psiAxisY - 150, psiAxisY - 240, psiAxisY - 330 };

    for (int n = 1; n <= 4; ++n) {
        double baseY = stateHeights[n - 1];
        drawLine((int)psiLeftAxisX - 15, (int)baseY, (int)psiRightWallX + 30, (int)baseY, axisColor, 0.4f);

        Color curColor = (n == current_n) ? activePsiColor : psiColor;
        double phase = (n == current_n) ? t * (n * n * 0.8) : 0.0;

        int prevY = (int)baseY;
        for (int x = (int)psiLeftAxisX; x <= (int)psiRightWallX; ++x) {
            double normX = (double)(x - psiLeftAxisX) / psiInnerWidth;
            double spatial = std::sin(n * M_PI * normX);
            double psiVal = spatial * std::cos(phase);
            int y = static_cast<int>(baseY - psiVal * 30.0);

            if (x > (int)psiLeftAxisX) {
                drawLine(x - 1, prevY, x, y, curColor, (n == current_n) ? 1.2f : 0.7f);
            }
            prevY = y;
        }
    }

    // Diagram 3: Energy Levels
    double eAxisY = 1440.0;
    double eAxisX = 160.0;
    double eAxisRight = 840.0;

    drawLine((int)eAxisX, 1080, (int)eAxisX, (int)eAxisY, redArrowColor, 1.2f);
    drawArrowHeadUp((int)eAxisX, 1080, redArrowColor);

    drawLine((int)eAxisX - 20, (int)eAxisY, (int)eAxisRight + 50, (int)eAxisY, redArrowColor, 1.2f);
    drawArrowHeadRight((int)eAxisRight + 50, (int)eAxisY, redArrowColor);

    double colX[4] = { eAxisX + 80, eAxisX + 240, eAxisX + 400, eAxisX + 560 };
    double colWidth = 80.0;

    for (int m_idx = 0; m_idx < 4; ++m_idx) {
        double massFactor = 1.0 + m_idx * 0.25;
        for (int n = 1; n <= 5; ++n) {
            double energyVal = (n * n) / massFactor;
            int levelY = static_cast<int>(eAxisY - energyVal * 22.0);

            if (levelY >= 1110) {
                Color col = (n == current_n) ? energyColor : axisColor;
                float intensity = (n == current_n) ? 1.2f : 0.7f;
                drawLine((int)colX[m_idx], levelY, (int)(colX[m_idx] + colWidth), levelY, col, intensity);
            }
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CREATE:
            SetTimer(hwnd, 1, 16, NULL); // ~60 FPS smooth timer update
            break;
        case WM_TIMER:
            InvalidateRect(hwnd, NULL, FALSE); // Triggers flicker-free redraw
            break;
        case WM_KEYDOWN:
            if (wp >= '1' && wp <= '4') {
                current_n = wp - '0';
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        case WM_VSCROLL: {
            SCROLLINFO si = { sizeof(SCROLLINFO) };
            si.fMask = SIF_ALL;
            GetScrollInfo(hwnd, SB_VERT, &si);
            int yOldPos = si.nPos;
            
            switch (LOWORD(wp)) {
                case SB_LINEUP:   si.nPos -= 25; break;
                case SB_LINEDOWN: si.nPos += 25; break;
                case SB_PAGEUP:   si.nPos -= si.nPage; break;
                case SB_PAGEDOWN: si.nPos += si.nPage; break;
                case SB_THUMBTRACK: si.nPos = si.nTrackPos; break;
            }
            si.fMask = SIF_POS;
            SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
            GetScrollInfo(hwnd, SB_VERT, &si);
            
            if (si.nPos != yOldPos) {
                current_scroll_y = si.nPos;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RenderScene();

            void* pBits = NULL;
            BITMAPINFO bmi = {0};
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = WIDTH;
            bmi.bmiHeader.biHeight = -VIRTUAL_HEIGHT;
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 24;
            bmi.bmiHeader.biCompression = BI_RGB;

            HDC hdcMemLocal = CreateCompatibleDC(hdc);
            HBITMAP hbmMemLocal = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
            HBITMAP hOldBmp = (HBITMAP)SelectObject(hdcMemLocal, hbmMemLocal);

            memcpy(pBits, frameBuffer.data(), WIDTH * VIRTUAL_HEIGHT * 3);

            // Draw texts onto memory DC
            SetBkMode(hdcMemLocal, TRANSPARENT);
            
            HFONT hTitleFont = CreateFontW(32, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial");
            SelectObject(hdcMemLocal, hTitleFont);
            SetTextColor(hdcMemLocal, RGB(220, 50, 50));
            TextOutW(hdcMemLocal, 230, 15, L"Particle in one dimensional box", 31);

            HFONT hTextFont = CreateFontW(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Times New Roman");
            SelectObject(hdcMemLocal, hTextFont);
            SetTextColor(hdcMemLocal, RGB(255, 255, 255));

            // Diagram 1 Labels
            SetTextColor(hdcMemLocal, RGB(200, 220, 255));
            double wellLeft = 280.0, wellRight = 720.0, wellTop = 130.0, wellBottom = 380.0;
            TextOutW(hdcMemLocal, (int)wellLeft - 50, (int)wellTop + 45, L"V = \u221E", 5);
            TextOutW(hdcMemLocal, (int)wellRight + 20, (int)wellTop + 45, L"V = \u221E", 5);
            TextOutW(hdcMemLocal, (int)wellLeft + 45, (int)wellTop + 15, L"V = \u221E", 5);
            TextOutW(hdcMemLocal, (int)wellRight - 80, (int)wellTop + 15, L"\u03C8 = 0", 5);
            TextOutW(hdcMemLocal, (int)wellRight - 80, (int)wellTop + 35, L"V = \u221E", 5);
            TextOutW(hdcMemLocal, (int)wellLeft + 80, (int)wellTop + 75, L"V = 0", 5);
            TextOutW(hdcMemLocal, (int)wellLeft + 90, (int)wellTop + 110, L"m", 1);
            TextOutW(hdcMemLocal, (int)wellLeft - 10, (int)wellBottom + 10, L"x = 0", 5);
            TextOutW(hdcMemLocal, (int)wellRight - 20, (int)wellBottom + 10, L"x = a", 5);
            TextOutW(hdcMemLocal, (int)(wellLeft + wellRight) / 2 - 5, (int)wellBottom + 35, L"a", 1);

            // Diagram 2 Labels
            SetTextColor(hdcMemLocal, RGB(255, 255, 255));
            TextOutW(hdcMemLocal, 330, 510, L"\u03C8_n = \u221A(2/a) Sin(n \u03C0 x / a)", 28);
            
            double psiLeftAxisX = 160.0, psiRightWallX = 840.0, psiAxisY = 920.0, psiInnerWidth = psiRightWallX - psiLeftAxisX;
            double stateHeights[4] = { psiAxisY - 60, psiAxisY - 150, psiAxisY - 240, psiAxisY - 330 };

            TextOutW(hdcMemLocal, (int)psiLeftAxisX - 50, 620, L"\u03C8 = 0", 5);
            TextOutW(hdcMemLocal, (int)psiLeftAxisX - 50, 645, L"V = \u221E", 5);
            TextOutW(hdcMemLocal, (int)psiRightWallX + 20, 620, L"\u03C8 = 0", 5);
            TextOutW(hdcMemLocal, (int)psiRightWallX + 20, 645, L"V = \u221E", 5);

            for (int n = 1; n <= 4; ++n) {
                if (n == current_n) SetTextColor(hdcMemLocal, RGB(255, 100, 150));
                else SetTextColor(hdcMemLocal, RGB(255, 255, 255));
                
                std::wstring nStr = L"n = " + std::to_wstring(n);
                TextOutW(hdcMemLocal, (int)psiLeftAxisX - 50, (int)stateHeights[n - 1] - 12, nStr.c_str(), 5);
            }
            SetTextColor(hdcMemLocal, RGB(255, 255, 255));

            TextOutW(hdcMemLocal, (int)psiLeftAxisX - 30, 560, L"\u03C8_n", 3);
            TextOutW(hdcMemLocal, (int)psiRightWallX + 30, (int)psiAxisY + 15, L"x", 1);

            TextOutW(hdcMemLocal, (int)psiLeftAxisX - 5, (int)psiAxisY + 12, L"0", 1);
            TextOutW(hdcMemLocal, (int)psiLeftAxisX + psiInnerWidth * 0.25 - 10, (int)psiAxisY + 12, L"a/4", 3);
            TextOutW(hdcMemLocal, (int)psiLeftAxisX + psiInnerWidth * 0.5 - 10, (int)psiAxisY + 12, L"a/2", 3);
            TextOutW(hdcMemLocal, (int)psiLeftAxisX + psiInnerWidth * 0.75 - 10, (int)psiAxisY + 12, L"3a/4", 4);
            TextOutW(hdcMemLocal, (int)psiRightWallX - 5, (int)psiAxisY + 12, L"a", 1);

            // Diagram 3 Labels
            TextOutW(hdcMemLocal, 330, 1045, L"E_n = (n\u00B2 h\u00B2) / (8 m a\u00B2)", 24);
            
            double eAxisX = 160.0, eAxisY = 1440.0, eAxisRight = 840.0;
            TextOutW(hdcMemLocal, (int)eAxisX - 12, 1095, L"E", 1);
            TextOutW(hdcMemLocal, (int)eAxisRight + 30, (int)eAxisY + 15, L"m", 1);

            double colX[4] = { eAxisX + 80, eAxisX + 240, eAxisX + 400, eAxisX + 560 };
            TextOutW(hdcMemLocal, (int)colX[0] + 25, (int)eAxisY + 12, L"m1", 2);
            TextOutW(hdcMemLocal, (int)colX[1] + 25, (int)eAxisY + 12, L"m2", 2);
            TextOutW(hdcMemLocal, (int)colX[2] + 25, (int)eAxisY + 12, L"m3", 2);
            TextOutW(hdcMemLocal, (int)colX[3] + 25, (int)eAxisY + 12, L"m4", 2);

            DeleteObject(hTitleFont);
            DeleteObject(hTextFont);

            // Blit from memory DC to screen DC with scroll offset (completely flicker-free)
            BitBlt(hdc, 0, 0, WIDTH, 700, hdcMemLocal, 0, current_scroll_y, SRCCOPY);

            SelectObject(hdcMemLocal, hOldBmp);
            DeleteObject(hbmMemLocal);
            DeleteDC(hdcMemLocal);

            EndPaint(hwnd, &ps);
            break;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_DESTROY:
            KillTimer(hwnd, 1);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    startTime = GetTickCount();

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "QuantumFlickerFreeClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(0, "QuantumFlickerFreeClass", "Particle in One Dimensional Box - Smooth & Flicker Free",
                                WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_VSCROLL,
                                CW_USEDEFAULT, CW_USEDEFAULT, WIDTH + 16, 750,
                                NULL, NULL, hInstance, NULL);

    SCROLLINFO si = { sizeof(SCROLLINFO) };
    si.fMask = SIF_RANGE | SIF_PAGE;
    si.nMin = 0;
    si.nMax = VIRTUAL_HEIGHT;
    si.nPage = 700;
    SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}