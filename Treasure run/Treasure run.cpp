#include "framework.h"
#include "Treasure run.h"
#include <mmsystem.h>
#include <d2d1.h>
#include <dwrite.h>
#include "ErrH.h"
#include "FCheck.h"
#include "D2BMPLOADER.h"
#include "gifresizer.h"
#include "runengine.h"
#include "chrono"

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "errh.lib")
#pragma comment(lib, "fcheck.lib")
#pragma comment(lib, "d2bmploader.lib")
#pragma comment(lib, "gifresizer.lib")
#pragma comment(lib, "runengine.lib")

constexpr wchar_t bWinClassName[]{ L"MyNewPlatform" };

constexpr char tmp_file[]{ ".\\res\\data\\temp.dat" };
constexpr wchar_t Ltmp_file[]{ L".\\res\\data\\temp.dat" };
constexpr wchar_t snd_file[]{ L".\\res\\snd\\main.wav" };
constexpr wchar_t record_file[]{ L".\\res\\data\\record.dat" };
constexpr wchar_t save_file[]{ L".\\res\\data\\save.dat" };
constexpr wchar_t help_file[]{ L".\\res\\data\\help.dat" };

constexpr int mNew{ 1001 };
constexpr int mTurbo{ 1002 };
constexpr int mExit{ 1003 };
constexpr int mSave{ 1004 };
constexpr int mLoad{ 1005 };
constexpr int mHoF{ 1006 };

constexpr int record{ 2001 };
constexpr int no_record{ 2002 };
constexpr int first_record{ 2003 };

/////////////////////////////////////////////////////////////

WNDCLASS bWin{};
HINSTANCE bIns = nullptr;
HWND bHwnd = nullptr;
HMENU bBar = nullptr;
HMENU bMain = nullptr;
HMENU bStore = nullptr;
HCURSOR mainCur = nullptr;
HCURSOR outCur = nullptr;
HICON mainIcon = nullptr;
HDC paintDC = nullptr;
PAINTSTRUCT bPaint{};
POINT cur_pos{};
MSG bMsg{};
BOOL bRet = 0;

D2D1_RECT_F b1Rect{ 10.0f, 5.0f, scr_width / 3 - 50.0f, 50.0f };
D2D1_RECT_F b2Rect{ scr_width / 3 + 10.0f, 5.0f, scr_width * 2 / 3 - 50.0f, 50.0f };
D2D1_RECT_F b3Rect{ scr_width * 2 / 3 + 10.0f, 5.0f, scr_width, 50.0f };

wchar_t current_player[16] = L"ONE RUNNER";

dll::RANDiT RandEngine{};

bool pause = false;
bool show_help = false;
bool in_client = true;
bool sound = true;
bool name_set = false;
bool b1Hglt = false;
bool b2Hglt = false;
bool b3Hglt = false;

int level = 1;
int score = 0;

int background_frame = 0;
int background_delay = 0;

//////////////////////////////////////////////////

ID2D1Factory* iFactory = nullptr;
ID2D1HwndRenderTarget* Draw = nullptr;

ID2D1RadialGradientBrush* bckgBrush = nullptr;
ID2D1RadialGradientBrush* shootBrush = nullptr;
ID2D1SolidColorBrush* txtBrush = nullptr;
ID2D1SolidColorBrush* hgltBrush = nullptr;
ID2D1SolidColorBrush* inactBrush = nullptr;

IDWriteFactory* iWriteFactory = nullptr;
IDWriteTextFormat* txtFormat = nullptr;
IDWriteTextFormat* midFormat = nullptr;
IDWriteTextFormat* bigFormat = nullptr;

ID2D1Bitmap* bmpBaseField = nullptr;
ID2D1Bitmap* bmpPlatform1 = nullptr;
ID2D1Bitmap* bmpPlatform2 = nullptr;
ID2D1Bitmap* bmpPlatform3 = nullptr;
ID2D1Bitmap* bmpPlatform4 = nullptr;
ID2D1Bitmap* bmpRIP = nullptr;

ID2D1Bitmap* bmpCrystal = nullptr;
ID2D1Bitmap* bmpGold = nullptr;
ID2D1Bitmap* bmpPotion = nullptr;

ID2D1Bitmap* bmpBackground[20] = { nullptr };
ID2D1Bitmap* bmpIntro[8] = { nullptr };

ID2D1Bitmap* bmpHeroL[4] = { nullptr };
ID2D1Bitmap* bmpHeroR[4] = { nullptr };

ID2D1Bitmap* bmpEvil1L[16] = { nullptr };
ID2D1Bitmap* bmpEvil1R[16] = { nullptr };

ID2D1Bitmap* bmpEvil2[37] = { nullptr };

ID2D1Bitmap* bmpEvil3L[32] = { nullptr };
ID2D1Bitmap* bmpEvil3R[32] = { nullptr };

ID2D1Bitmap* bmpEvil4[29] = { nullptr };

ID2D1Bitmap* bmpEvil5[36] = { nullptr };

//////////////////////////////////////////////////




//////////////////////////////////////////////////

template<typename T> concept HasRelease = requires(T check_var)
{
    check_var.Release();
};
template<HasRelease T> bool ClearHeap(T** var)
{
    if (*var)
    {
        (*var)->Release();
        (*var) = nullptr;
        return true;
    }
    return false;
}
void LogError(LPCWSTR what)
{
    std::wofstream err(L".\\res\\data\\error.log", std::ios::app);
    err << what << L" Time stamp: " << std::chrono::system_clock::now() << std::endl << std::endl;
    err.close();
}
void ReleaseResources()
{
    if (!ClearHeap(&iFactory))LogError(L"Error releasing iFactory !");
    if (!ClearHeap(&Draw))LogError(L"Error releasing Draw !");
    if (!ClearHeap(&bckgBrush))LogError(L"Error releasing bckgBrush !");
    if (!ClearHeap(&shootBrush))LogError(L"Error releasing shootBrush !");
    if (!ClearHeap(&txtBrush))LogError(L"Error releasing txtBrush !");
    if (!ClearHeap(&hgltBrush))LogError(L"Error releasing hgltBrush !");
    if (!ClearHeap(&inactBrush))LogError(L"Error releasing inactBrush !");

    if (!ClearHeap(&iWriteFactory))LogError(L"Error releasing iWriteFactory !");
    if (!ClearHeap(&txtFormat))LogError(L"Error releasing txtFormat !");
    if (!ClearHeap(&midFormat))LogError(L"Error releasing midFormat !");
    if (!ClearHeap(&bigFormat))LogError(L"Error releasing bigFormat !");

    if (!ClearHeap(&bmpBaseField))LogError(L"Error releasing bmpBaseField !");
    if (!ClearHeap(&bmpPlatform1))LogError(L"Error releasing bmpPlatform1 !");
    if (!ClearHeap(&bmpPlatform2))LogError(L"Error releasing bmpPlatform2 !");
    if (!ClearHeap(&bmpPlatform3))LogError(L"Error releasing bmpPlatform3 !");
    if (!ClearHeap(&bmpPlatform4))LogError(L"Error releasing bmpPlatform4 !");

    if (!ClearHeap(&bmpRIP))LogError(L"Error releasing bmpRIP !");
    if (!ClearHeap(&bmpCrystal))LogError(L"Error releasing bmpCrystal !");
    if (!ClearHeap(&bmpGold))LogError(L"Error releasing bmpGold !");
    if (!ClearHeap(&bmpPotion))LogError(L"Error releasing bmpPotion !");

    for (int i = 0; i < 20; i++)if (!ClearHeap(&bmpBackground[i]))LogError(L"Error releasing bmpBackground !");
    for (int i = 0; i < 8; i++)if (!ClearHeap(&bmpBackground[i]))LogError(L"Error releasing bmpIntro !");

    for (int i = 0; i < 4; i++)if (!ClearHeap(&bmpHeroL[i]))LogError(L"Error releasing bmpHeroL !");
    for (int i = 0; i < 4; i++)if (!ClearHeap(&bmpHeroR[i]))LogError(L"Error releasing bmpHeroR !");

    for (int i = 0; i < 16; i++)if (!ClearHeap(&bmpEvil1L[i]))LogError(L"Error releasing bmpEvil1L !");
    for (int i = 0; i < 16; i++)if (!ClearHeap(&bmpEvil1R[i]))LogError(L"Error releasing bmpEvil1R !");

    for (int i = 0; i < 37; i++)if (!ClearHeap(&bmpEvil2[i]))LogError(L"Error releasing bmpEvil2 !");

    for (int i = 0; i < 32; i++)if (!ClearHeap(&bmpEvil3L[i]))LogError(L"Error releasing bmpEvil3L !");
    for (int i = 0; i < 32; i++)if (!ClearHeap(&bmpEvil3R[i]))LogError(L"Error releasing bmpEvil3R !");

    for (int i = 0; i < 29; i++)if (!ClearHeap(&bmpEvil4[i]))LogError(L"Error releasing bmpEvil4 !");

    for (int i = 0; i < 36; i++)if (!ClearHeap(&bmpEvil5[i]))LogError(L"Error releasing bmpEvil5 !");
}
void ErrExit(int what)
{
    MessageBeep(MB_ICONERROR);
    MessageBox(NULL, ErrHandle(what), L"КРИТИЧНА ГРЕШКА !", MB_OK | MB_APPLMODAL | MB_ICONERROR);

    std::remove(tmp_file);
    ReleaseResources();
    exit(1);
}

void GameOver()
{
    PlaySound(NULL, NULL, NULL);

    bMsg.message = WM_QUIT;
    bMsg.wParam = 0;
}
void InitGame()
{
    level = 1;
    score = 0;
    wcscpy_s(current_player, L"ONE RUNNER");
    name_set = false;


}

INT_PTR CALLBACK bDlgProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
    switch (ReceivedMsg)
    {
    case WM_INITDIALOG:
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)(mainIcon));
        return true;

    case WM_CLOSE:
        EndDialog(hwnd, IDCANCEL);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;

        case IDOK:
            if (GetDlgItemText(hwnd, IDC_NAME, current_player, 16) < 1)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
                MessageBox(bHwnd, L"Ха, ха, хаааа !\n\nЗабрави си името !", L"Забраватор !", MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
                EndDialog(hwnd, IDCANCEL);
                break;
            }
            EndDialog(hwnd, IDOK);
            break;
        }
        break;
    }

    return (INT_PTR)(FALSE);
}
LRESULT CALLBACK bWinProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
    switch (ReceivedMsg)
    {
    case WM_CREATE:
        bBar = CreateMenu();
        bMain = CreateMenu();
        bStore = CreateMenu();
        AppendMenu(bBar, MF_POPUP, (UINT_PTR)(bMain), L"Основно меню");
        AppendMenu(bBar, MF_POPUP, (UINT_PTR)(bStore), L"Меню за данни");
        AppendMenu(bMain, MF_STRING, mNew, L"Нова игра");
        AppendMenu(bMain, MF_STRING, mTurbo, L"Включи турбото");
        AppendMenu(bMain, MF_SEPARATOR, NULL, NULL);
        AppendMenu(bMain, MF_STRING, mExit, L"Изход");
        AppendMenu(bStore, MF_STRING, mSave, L"Запази игра");
        AppendMenu(bStore, MF_STRING, mLoad, L"Зареди игра");
        AppendMenu(bStore, MF_SEPARATOR, NULL, NULL);
        AppendMenu(bStore, MF_STRING, mHoF, L"Зала на славата");
        SetMenu(hwnd, bBar);
        InitGame();
        break;

    case WM_CLOSE:
        pause = true;
        if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
        if (MessageBox(hwnd, L"Ако излезеш, ще загубиш тази игра !\n\nНаистина ли излизаш ?",
            L"Изход", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
        {
            pause = false;
            break;
        }
        GameOver();
        break;

    case WM_PAINT:
        paintDC = BeginPaint(hwnd, &bPaint);
        FillRect(paintDC, &bPaint.rcPaint, CreateSolidBrush(RGB(100, 100, 100)));
        EndPaint(hwnd, &bPaint);
        break;

    case WM_SETCURSOR:
        GetCursorPos(&cur_pos);
        ScreenToClient(hwnd, &cur_pos);
        if (LOWORD(lParam) == HTCLIENT)
        {
            if (!in_client)
            {
                in_client = true;
                pause = false;
            }

            if (cur_pos.y <= 50)
            {
                if (cur_pos.x >= b1Rect.left && cur_pos.x <= b1Rect.right)
                {
                    if (!b1Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = true;
                        b2Hglt = false;
                        b3Hglt = false;
                    }
                }
                if (cur_pos.x >= b2Rect.left && cur_pos.x <= b2Rect.right)
                {
                    if (!b2Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = false;
                        b2Hglt = true;
                        b3Hglt = false;
                    }
                }
                if (cur_pos.x >= b3Rect.left && cur_pos.x <= b3Rect.right)
                {
                    if (!b3Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = false;
                        b2Hglt = false;
                        b3Hglt = true;
                    }
                }
                SetCursor(outCur);
                return true;
            }
            else if (b1Hglt || b2Hglt || b3Hglt)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                b1Hglt = false;
                b2Hglt = false;
                b3Hglt = false;
            }

            SetCursor(mainCur);
            return true;
        }
        else
        {
            if (in_client)
            {
                in_client = false;
                pause = true;
            }

            if (b1Hglt || b2Hglt || b3Hglt)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                b1Hglt = false;
                b2Hglt = false;
                b3Hglt = false;
            }

            SetCursor(LoadCursor(NULL, IDC_ARROW));
            return true;
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case mNew:
            pause = true;
            if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
            if (MessageBox(hwnd, L"Ако рестартираш, ще загубиш тази игра !\n\nНаистина ли рестартираш ?",
                L"Рестарт", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
            {
                pause = false;
                break;
            }
            InitGame();
            break;

        case mTurbo:
            pause = true;
            if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
            if (MessageBox(hwnd, L"Наистина ли пускаш турбото ?",
                L"Турбо", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
            {
                pause = false;
                break;
            }
            ++level;
            break;

        case mExit:
            SendMessage(hwnd, WM_CLOSE, NULL, NULL);
            break;

        }
        break;


    default: return DefWindowProc(hwnd, ReceivedMsg, wParam, lParam);
    }

    return (LRESULT)(FALSE);
}

void CreateResources()
{
    int result = 0;
    CheckFile(Ltmp_file, &result);
    if (result == FILE_EXIST)ErrExit(eStarted);
    else
    {
        std::wofstream started(Ltmp_file);
        started << L"Game started at: " << std::chrono::system_clock::now();
        started.close();
    }

    int win_x = GetSystemMetrics(SM_CXSCREEN) / 2 - (int)(scr_width / 2);
    int win_y = 100;
    if (GetSystemMetrics(SM_CXSCREEN) < win_x + (int)(scr_width) || GetSystemMetrics(SM_CYSCREEN) < scr_height + 100)
        ErrExit(eScreen);

    mainIcon = (HICON)(LoadImage(NULL, L".\\res\\main.ico", IMAGE_ICON, 255, 255, LR_LOADFROMFILE));
    if (!mainIcon)ErrExit(eIcon);

    mainCur = LoadCursorFromFile(L".\\res\\main.ani");
    outCur = LoadCursorFromFile(L".\\res\\out.ani");
    if (!mainCur || !outCur)ErrExit(eCursor);

    bWin.lpszClassName = bWinClassName;
    bWin.hInstance = bIns;
    bWin.lpfnWndProc = &bWinProc;
    bWin.hbrBackground = CreateSolidBrush(RGB(100, 100, 100));
    bWin.hIcon = mainIcon;
    bWin.hCursor = mainCur;
    bWin.style = CS_DROPSHADOW;

    if (!RegisterClass(&bWin))ErrExit(eClass);

    bHwnd = CreateWindow(bWinClassName, L"ЛОВ ЗА СЪКРОВИЩА !", WS_CAPTION | WS_SYSMENU, win_x, win_y, (int)(scr_width),
        (int)(scr_height), NULL, NULL, bIns, NULL);
    if (!bHwnd)ErrExit(eWindow);
    else
    {
        ShowWindow(bHwnd, SW_SHOWDEFAULT);

        HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &iFactory);
        if (hr != S_OK)
        {
            LogError(L"Error creating D2D1 Factory !");
            ErrExit(eD2D);
        }

        if (iFactory)
        {
            hr = iFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(bHwnd,
                D2D1::SizeU((UINT32)(scr_width), (UINT32)(scr_height))), &Draw);

            if (hr != S_OK)
            {
                LogError(L"Error creating D2D1 Hwnd Render Target !");
                ErrExit(eD2D);
            }
        }
        
        if (Draw)
        {
            D2D1_GRADIENT_STOP gStops[2]{};
            ID2D1GradientStopCollection* Coll = nullptr;

            gStops[0].position = 0;
            gStops[0].color = D2D1::ColorF(D2D1::ColorF::Orange);
            gStops[1].position = 1.0f;
            gStops[1].color = D2D1::ColorF(D2D1::ColorF::Brown);

            hr = Draw->CreateGradientStopCollection(gStops, 2, &Coll);
            if (hr != S_OK)
            {
                LogError(L"Error creating D2D1 Gradient Stop Collection for background brush !");
                ErrExit(eD2D);
            }
            if (Coll)
            {
                hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(scr_width / 2, 25.0f),
                    D2D1::Point2F(0, 0), scr_width / 2, 25.0f), Coll, &bckgBrush);
                if (hr != S_OK)
                {
                    LogError(L"Error creating D2D1 background brush !");
                    ErrExit(eD2D);
                }
                ClearHeap(&Coll);
            }

            hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green), &txtBrush);
            hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Violet), &hgltBrush);
            hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkSlateBlue), &inactBrush);

            if (hr != S_OK)
            {
                LogError(L"Error creating D2D1 text brushes !");
                ErrExit(eD2D);
            }

            bmpBaseField = Load(L".\\res\\img\\field\\field.png", Draw);
            if (!bmpBaseField)
            {
                LogError(L"Error loading bmpBaseField");
                ErrExit(eD2D);
            }
            bmpPlatform1 = Load(L".\\res\\img\\field\\platform1.png", Draw);
            if (!bmpPlatform1)
            {
                LogError(L"Error loading bmpPlatform1");
                ErrExit(eD2D);
            }
            bmpPlatform2 = Load(L".\\res\\img\\field\\platform2.png", Draw);
            if (!bmpPlatform2)
            {
                LogError(L"Error loading bmpPlatform2");
                ErrExit(eD2D);
            }
            bmpPlatform3 = Load(L".\\res\\img\\field\\platform3.png", Draw);
            if (!bmpPlatform3)
            {
                LogError(L"Error loading bmpPlatform3");
                ErrExit(eD2D);
            }
            bmpPlatform4 = Load(L".\\res\\img\\field\\platform4.png", Draw);
            if (!bmpPlatform4)
            {
                LogError(L"Error loading bmpPlatform4");
                ErrExit(eD2D);
            }
            bmpRIP = Load(L".\\res\\img\\field\\RIP.png", Draw);
            if (!bmpRIP)
            {
                LogError(L"Error loading bmpRIP");
                ErrExit(eD2D);
            }
            bmpCrystal = Load(L".\\res\\img\\assets\\crystal.png", Draw);
            if (!bmpCrystal)
            {
                LogError(L"Error loading bmpCrystal");
                ErrExit(eD2D);
            }
            bmpGold = Load(L".\\res\\img\\assets\\gold.png", Draw);
            if (!bmpGold)
            {
                LogError(L"Error loading bmpGold");
                ErrExit(eD2D);
            }
            bmpPotion = Load(L".\\res\\img\\assets\\potion.png", Draw);
            if (!bmpPotion)
            {
                LogError(L"Error loading bmpPotion");
                ErrExit(eD2D);
            }

            for (int i = 0; i < 20; ++i)
            {
                wchar_t name[75] = L".\\res\\img\\field\\background\\";
                wchar_t add[4] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpBackground[i] = Load(name, Draw);
                if (!bmpBackground[i])
                {
                    LogError(L"Error loading bmpBackground");
                    ErrExit(eD2D);
                }
            }

            for (int i = 0; i < 8; ++i)
            {
                wchar_t name[75] = L".\\res\\img\\intro\\";
                wchar_t add[4] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpIntro[i] = Load(name, Draw);
                if (!bmpIntro[i])
                {
                    LogError(L"Error loading bmpIntro");
                    ErrExit(eD2D);
                }
            }
            
            for (int i = 0; i < 4; ++i)
            {
                wchar_t name[75] = L".\\res\\img\\hero\\l\\";
                wchar_t add[4] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpHeroL[i] = Load(name, Draw);
                if (!bmpHeroL[i])
                {
                    LogError(L"Error loading bmpHeroL");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 4; ++i)
            {
                wchar_t name[75] = L".\\res\\img\\hero\\r\\";
                wchar_t add[4] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpHeroR[i] = Load(name, Draw);
                if (!bmpHeroR[i])
                {
                    LogError(L"Error loading bmpHeroR");
                    ErrExit(eD2D);
                }
            }

            for (int i = 0; i < 16; ++i)
            {
                wchar_t name[75] = L".\\res\\img\\evil1\\l\\";
                wchar_t add[4] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpEvil1L[i] = Load(name, Draw);
                if (!bmpEvil1L[i])
                {
                    LogError(L"Error loading bmpEvil1L");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 16; ++i)
            {
                wchar_t name[75] = L".\\res\\img\\evil1\\r\\";
                wchar_t add[4] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpEvil1R[i] = Load(name, Draw);
                if (!bmpEvil1R[i])
                {
                    LogError(L"Error loading bmpEvil1R");
                    ErrExit(eD2D);
                }
            }

            for (int i = 0; i < 37; ++i)
            {
                wchar_t name[75] = L".\\res\\img\\evil2\\";
                wchar_t add[4] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpEvil2[i] = Load(name, Draw);
                if (!bmpEvil2[i])
                {
                    LogError(L"Error loading bmpEvil2");
                    ErrExit(eD2D);
                }
            }

            for (int i = 0; i < 32; ++i)
            {
                wchar_t name[75] = L".\\res\\img\\evil3\\l\\";
                wchar_t add[4] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpEvil3L[i] = Load(name, Draw);
                if (!bmpEvil3L[i])
                {
                    LogError(L"Error loading bmpEvil3L");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 32; ++i)
            {
                wchar_t name[75] = L".\\res\\img\\evil3\\r\\";
                wchar_t add[4] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpEvil3R[i] = Load(name, Draw);
                if (!bmpEvil3R[i])
                {
                    LogError(L"Error loading bmpEvil3R");
                    ErrExit(eD2D);
                }
            }

            for (int i = 0; i < 29; ++i)
            {
                wchar_t name[75] = L".\\res\\img\\evil4\\";
                wchar_t add[4] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpEvil4[i] = Load(name, Draw);
                if (!bmpEvil4[i])
                {
                    LogError(L"Error loading bmpEvil4");
                    ErrExit(eD2D);
                }
            }

            for (int i = 0; i < 36; ++i)
            {
                wchar_t name[75] = L".\\res\\img\\evil5\\";
                wchar_t add[4] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpEvil5[i] = Load(name, Draw);
                if (!bmpEvil5[i])
                {
                    LogError(L"Error loading bmpEvil5");
                    ErrExit(eD2D);
                }
            }
        }

        hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&iWriteFactory));
        if (hr != S_OK)
        {
            LogError(L"Error creating D2D1 WriteFactory !");
            ErrExit(eD2D);
        }
        if (iWriteFactory)
        {
            hr = iWriteFactory->CreateTextFormat(L"Segoe Print", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK, DWRITE_FONT_STYLE_OBLIQUE,
                DWRITE_FONT_STRETCH_NORMAL, 18, L"", &txtFormat);
            hr = iWriteFactory->CreateTextFormat(L"Segoe Print", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK, DWRITE_FONT_STYLE_OBLIQUE,
                DWRITE_FONT_STRETCH_NORMAL, 28, L"", &midFormat);
            hr = iWriteFactory->CreateTextFormat(L"Segoe Print", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK, DWRITE_FONT_STYLE_OBLIQUE,
                DWRITE_FONT_STRETCH_NORMAL, 72, L"", &bigFormat);

            if (hr != S_OK)
            {
                LogError(L"Error creating iWriteFactory Text Formats !");
                ErrExit(eD2D);
            }
        }
    }

    if (Draw && txtBrush && bigFormat)
    {
        int intro_frame = 0;

        for (int i = 0; i <= 75; i++)
        {
            Draw->BeginDraw();
            Draw->DrawBitmap(bmpIntro[intro_frame], D2D1::RectF(0, 0, scr_width, scr_height));
            ++intro_frame;
            if (intro_frame > 7)intro_frame = 0;
            if (RandEngine(0, 4) == 1)
            {
                Draw->DrawTextW(L"ЛОВ ЗА СЪКРОВИЩА !\n\n   dev. Daniel", 35, bigFormat, D2D1::RectF(30.0f, 80.0f, scr_width,
                    scr_height), txtBrush);
                mciSendString(L"play .\\res\\snd\\buzz.wav", NULL, NULL, NULL);
                Draw->EndDraw();
                Sleep(60);
            }
            else
            {
                Draw->EndDraw();
                Sleep(60);
            }
        }

        Draw->BeginDraw();
        Draw->DrawBitmap(bmpIntro[0], D2D1::RectF(0, 0, scr_width, scr_height));
        Draw->DrawTextW(L"ЛОВ ЗА СЪКРОВИЩА !\n\n   dev. Daniel", 35, bigFormat, D2D1::RectF(30.0f, 80.0f, scr_width,
            scr_height), txtBrush);
        Draw->EndDraw();
        PlaySound(L".\\res\\snd\\boom.wav", NULL, SND_SYNC);
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    bIns = hInstance;
    if (!bIns)
    {
        LogError(L"Error in WINDOWS hInstance !");
        ErrExit(eClass);
    }

    CreateResources();

    while (bMsg.message != WM_QUIT)
    {
        if ((bRet = PeekMessage(&bMsg, bHwnd, NULL, NULL, PM_REMOVE)) != 0)
        {
            if (bRet == -1)ErrExit(eMsg);
            TranslateMessage(&bMsg);
            DispatchMessageW(&bMsg);
        }

        if (pause)
        {
            if (show_help)continue;
            if (Draw && bigFormat && txtBrush)
            {
                Draw->BeginDraw();
                Draw->Clear(D2D1::ColorF(D2D1::ColorF::BlanchedAlmond));
                Draw->DrawTextW(L"ПАУЗА", 6, bigFormat, D2D1::RectF(scr_width / 2 - 100.0f, scr_height / 2 - 80.0f, scr_width,
                    scr_height), txtBrush);
                Draw->EndDraw();
                continue;
            }
        }

        ////////////////////////////////




        // DRAW THINGS ******************

        Draw->BeginDraw();

        if (txtFormat && hgltBrush && txtBrush && inactBrush && bckgBrush)
        {
            Draw->FillRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(0, 0, scr_width, 50.0f), 25.0f, 25.0f), bckgBrush);
            if (name_set)Draw->DrawText(L"ИМЕ НА ИГРАЧ", 13, txtFormat, b1Rect, inactBrush);
            else
            {
                if (b1Hglt)Draw->DrawText(L"ИМЕ НА ИГРАЧ", 13, txtFormat, b1Rect, hgltBrush);
                else Draw->DrawText(L"ИМЕ НА ИГРАЧ", 13, txtFormat, b1Rect, txtBrush);
            }
            if (b2Hglt)Draw->DrawText(L"ЗВУЦИ ON / OFF", 15, txtFormat, b2Rect, hgltBrush);
            else Draw->DrawText(L"ЗВУЦИ ON / OFF", 15, txtFormat, b2Rect, txtBrush);
            if (b3Hglt)Draw->DrawText(L"ПОМОЩ ЗА ИГРАТА", 16, txtFormat, b3Rect, hgltBrush);
            else Draw->DrawText(L"ПОМОЩ ЗА ИГРАТА", 16, txtFormat, b3Rect, txtBrush);
        }

        ++background_delay;
        if (background_delay > 3)
        {
            background_delay = 0;
            ++background_frame;
            if(background_frame>19)background_frame = 0;
        }
        Draw->DrawBitmap(bmpBackground[background_frame], D2D1::RectF(0, 50, scr_width, scr_height));



        //////////////////////////////////////
        Draw->EndDraw();



    }

    std::remove(tmp_file);
    ReleaseResources();
    return (int) bMsg.wParam;
}