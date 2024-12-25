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

float background_element_speed = 0;
dirs background_element_dir = dirs::down;

bool need_left_field = false;
bool need_right_field = false;
bool need_left_background = false;
bool need_right_background = false;

bool hero_killed = false;
float RIP_x = 0;
float RIP_y = 0;

int number_of_crystals_collected = 0;

//////////////////////////////////////////////////

ID2D1Factory* iFactory = nullptr;
ID2D1HwndRenderTarget* Draw = nullptr;

ID2D1RadialGradientBrush* bckgBrush = nullptr;
ID2D1SolidColorBrush* txtBrush = nullptr;
ID2D1SolidColorBrush* hgltBrush = nullptr;
ID2D1SolidColorBrush* inactBrush = nullptr;

ID2D1SolidColorBrush* statTxtBrush = nullptr;

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

std::vector<dll::PROTON> vBackgrounds;
std::vector<dll::PROTON> vFields;
std::vector<dll::PROTON> vOnePlatforms;
std::vector<dll::PROTON> vTwoPlatforms;
std::vector<dll::PROTON> vThreePlatforms;
std::vector<dll::PROTON> vFourPlatforms;

dll::Creature Hero = nullptr;

std::vector<dll::Creature> vEvils;

std::vector<dll::SHOT> vShots;

dll::PROTON* Potion{ nullptr };
dll::PROTON* Crystal{ nullptr };
dll::PROTON* Gold{ nullptr };

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
    if (!ClearHeap(&txtBrush))LogError(L"Error releasing txtBrush !");
    if (!ClearHeap(&hgltBrush))LogError(L"Error releasing hgltBrush !");
    if (!ClearHeap(&inactBrush))LogError(L"Error releasing inactBrush !");
    if (!ClearHeap(&statTxtBrush))LogError(L"Error releasing hgltBrush !");

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
    for (int i = 0; i < 8; i++)if (!ClearHeap(&bmpIntro[i]))LogError(L"Error releasing bmpIntro !");

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

    number_of_crystals_collected = 0;

    background_element_speed = 0;
    background_element_dir = dirs::down;

    vBackgrounds.clear();
    for (float i = -scr_width; i < 2 * scr_width; i += scr_width)
        vBackgrounds.push_back(dll::PROTON(i, 50.0f, scr_width, ground));
    vFields.clear();
    for (float i = -scr_width; i < 2 * scr_width; i += scr_width)
        vFields.push_back(dll::PROTON(i, ground, scr_width, 50.0f));

    vOnePlatforms.clear();
    vTwoPlatforms.clear();
    vThreePlatforms.clear();
    vFourPlatforms.clear();

    need_left_field = false;
    need_right_field = false;
    need_left_background = false;
    need_right_background = false;
    
    ClearHeap(&Hero);
    Hero = dll::CreatureFactory(hero_type, scr_width / 2 - 100.0f, ground - 50.0f);

    if (!vEvils.empty())
        for (int i = 0; i < vEvils.size(); ++i)ClearHeap(&vEvils[i]);
    vEvils.clear();

    vShots.clear();

    if (Potion)
    {
        delete Potion;
        Potion = nullptr;
    }
    if (Gold)
    {
        delete Gold;
        Gold = nullptr;
    }
    if (Crystal)
    {
        delete Crystal;
        Crystal = nullptr;
    }

}
void LevelUp()
{
    ++level;

    number_of_crystals_collected = 0;

    background_element_speed = 0;
    background_element_dir = dirs::down;

    vBackgrounds.clear();
    for (float i = -scr_width; i < 2 * scr_width; i += scr_width)
        vBackgrounds.push_back(dll::PROTON(i, 50.0f, scr_width, ground));
    vFields.clear();
    for (float i = -scr_width; i < 2 * scr_width; i += scr_width)
        vFields.push_back(dll::PROTON(i, ground, scr_width, 50.0f));

    vOnePlatforms.clear();
    vTwoPlatforms.clear();
    vThreePlatforms.clear();
    vFourPlatforms.clear();

    need_left_field = false;
    need_right_field = false;
    need_left_background = false;
    need_right_background = false;

    ClearHeap(&Hero);
    Hero = dll::CreatureFactory(hero_type, scr_width / 2 - 100.0f, ground - 50.0f);

    if (!vEvils.empty())
        for (int i = 0; i < vEvils.size(); ++i)ClearHeap(&vEvils[i]);
    vEvils.clear();

    vShots.clear();

    if (Potion)
    {
        delete Potion;
        Potion = nullptr;
    }
    if (Gold)
    {
        delete Gold;
        Gold = nullptr;
    }
    if (Crystal)
    {
        delete Crystal;
        Crystal = nullptr;
    }

    if (txtBrush && bigFormat)
    {
        Draw->BeginDraw();
        Draw->Clear(D2D1::ColorF(D2D1::ColorF::DarkSlateGray));
        Draw->DrawTextW(L"СЛЕДВАЩО НИВО !", 16, bigFormat, D2D1::RectF(100.0f, 100.0f, scr_width, scr_height), txtBrush);
        Draw->EndDraw();
    }

    if (sound)
    {
        PlaySound(NULL, NULL, NULL);
        PlaySound(L".\\res\\snd\\levelup.wav", NULL, SND_SYNC);
        PlaySound(snd_file, NULL, SND_LOOP | SND_ASYNC);
    }
    else Sleep(3000);
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

    case WM_KEYDOWN:
        if (Hero)
        {
            switch (wParam)
            {
            case VK_LEFT:
                Hero->SetMoveFlag(run_flag);
                Hero->dir = dirs::left;
                break;

            case VK_RIGHT:
                Hero->SetMoveFlag(run_flag);
                Hero->dir = dirs::right;
                break;

            case VK_UP:
                if (!Hero->CheckMoveFlag(fall_flag) && !vFields.empty())
                {
                    size_t all_platforms_number = vFields.size() + vOnePlatforms.size() + vTwoPlatforms.size()
                        + vThreePlatforms.size() + vFourPlatforms.size();

                    dll::PROT_MESH AllPlatforms(all_platforms_number);
                    for (int i = 0; i < vFields.size(); ++i)
                    {
                        dll::PROTON OnePlatform(vFields[i].x, vFields[i].y,
                            vFields[i].GetWidth(), vFields[i].GetHeight());
                        AllPlatforms.push_back(OnePlatform);
                    }
                    if (!vOnePlatforms.empty())
                        for (int i = 0; i < vOnePlatforms.size(); ++i)
                        {
                            dll::PROTON OnePlatform(vOnePlatforms[i].x, vOnePlatforms[i].y,
                                vOnePlatforms[i].GetWidth(), vOnePlatforms[i].GetHeight());
                            AllPlatforms.push_back(OnePlatform);
                        }
                    if (!vTwoPlatforms.empty())
                        for (int i = 0; i < vTwoPlatforms.size(); ++i)
                        {
                            dll::PROTON OnePlatform(vTwoPlatforms[i].x, vTwoPlatforms[i].y,
                                vTwoPlatforms[i].GetWidth(), vTwoPlatforms[i].GetHeight());
                            AllPlatforms.push_back(OnePlatform);
                        }
                    if (!vThreePlatforms.empty())
                        for (int i = 0; i < vThreePlatforms.size(); ++i)
                        {
                            dll::PROTON OnePlatform(vThreePlatforms[i].x, vThreePlatforms[i].y,
                                vThreePlatforms[i].GetWidth(), vThreePlatforms[i].GetHeight());
                            AllPlatforms.push_back(OnePlatform);
                        }
                    if (!vFourPlatforms.empty())
                        for (int i = 0; i < vFourPlatforms.size(); ++i)
                        {
                            dll::PROTON OnePlatform(vFourPlatforms[i].x, vFourPlatforms[i].y,
                                vFourPlatforms[i].GetWidth(), vFourPlatforms[i].GetHeight());
                            AllPlatforms.push_back(OnePlatform);
                        }
                    
                    Hero->Jump((float)(level), AllPlatforms);
                }
                break;

            case VK_DOWN:
                if (Hero)
                    if (Hero->CheckMoveFlag(run_flag))Hero->SetMoveFlag(stop_flag);
                break;
            }
        }
        break;

    case WM_LBUTTONDOWN:
        if (Hero)
        {
            vShots.push_back(dll::SHOT(Hero->x + Hero->GetWidth() / 2, Hero->y + Hero->GetHeight() / 2,
                LOWORD(lParam), HIWORD(lParam)));
            if (sound)mciSendString(L"play .\\res\\snd\\shot.wav", NULL, NULL, NULL);
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
            hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkBlue), &statTxtBrush);

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

    if (Draw && statTxtBrush && bigFormat)
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
                    scr_height), statTxtBrush);
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
            scr_height), statTxtBrush);
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

        if (Hero)
        {
            char current_action = Hero->GetMoveFlag();

            switch (current_action)
            {
            case run_flag:
                if (!vFields.empty())
                {
                    size_t all_platforms_number = vFields.size() + vOnePlatforms.size() + vTwoPlatforms.size()
                        + vThreePlatforms.size() + vFourPlatforms.size();

                    dll::PROT_MESH AllPlatforms(all_platforms_number);
                    for (int i = 0; i < vFields.size(); ++i)
                    {
                        dll::PROTON OnePlatform(vFields[i].x, vFields[i].y,
                            vFields[i].GetWidth(), vFields[i].GetHeight());
                        AllPlatforms.push_back(OnePlatform);
                    }
                    if(!vOnePlatforms.empty())
                        for (int i = 0; i < vOnePlatforms.size(); ++i)
                        {
                            dll::PROTON OnePlatform(vOnePlatforms[i].x, vOnePlatforms[i].y,
                                vOnePlatforms[i].GetWidth(), vOnePlatforms[i].GetHeight());
                            AllPlatforms.push_back(OnePlatform);
                        }
                    if (!vTwoPlatforms.empty())
                        for (int i = 0; i < vTwoPlatforms.size(); ++i)
                        {
                            dll::PROTON OnePlatform(vTwoPlatforms[i].x, vTwoPlatforms[i].y,
                                vTwoPlatforms[i].GetWidth(), vTwoPlatforms[i].GetHeight());
                            AllPlatforms.push_back(OnePlatform);
                        }
                    if (!vThreePlatforms.empty())
                        for (int i = 0; i < vThreePlatforms.size(); ++i)
                        {
                            dll::PROTON OnePlatform(vThreePlatforms[i].x, vThreePlatforms[i].y,
                                vThreePlatforms[i].GetWidth(), vThreePlatforms[i].GetHeight());
                            AllPlatforms.push_back(OnePlatform);
                        }
                    if (!vFourPlatforms.empty())
                        for (int i = 0; i < vFourPlatforms.size(); ++i)
                        {
                            dll::PROTON OnePlatform(vFourPlatforms[i].x, vFourPlatforms[i].y,
                                vFourPlatforms[i].GetWidth(), vFourPlatforms[i].GetHeight());
                            AllPlatforms.push_back(OnePlatform);
                        }

                    if (Hero->dir == dirs::right)
                    {
                        
                        Hero->Move((float)(level), scr_width, Hero->y, AllPlatforms);
                        background_element_speed = 1.0f + (float)(level / 10);
                        background_element_dir = dirs::left;
                    }
                    else if (Hero->dir == dirs::left)
                    {
                        Hero->Move((float)(level), 0, Hero->y, AllPlatforms);
                        background_element_speed = 1.0f + (float)(level / 10);
                        background_element_dir = dirs::right;
                    }
                }
                break;

            case jump_up_flag:
                if (!vFields.empty())
                {
                    size_t all_platforms_number = vFields.size() + vOnePlatforms.size() + vTwoPlatforms.size()
                        + vThreePlatforms.size() + vFourPlatforms.size();

                    dll::PROT_MESH AllPlatforms(all_platforms_number);
                    for (int i = 0; i < vFields.size(); ++i)
                    {
                        dll::PROTON OnePlatform(vFields[i].x, vFields[i].y,
                            vFields[i].GetWidth(), vFields[i].GetHeight());
                        AllPlatforms.push_back(OnePlatform);
                    }
                    if (!vOnePlatforms.empty())
                        for (int i = 0; i < vOnePlatforms.size(); ++i)
                        {
                            dll::PROTON OnePlatform(vOnePlatforms[i].x, vOnePlatforms[i].y,
                                vOnePlatforms[i].GetWidth(), vOnePlatforms[i].GetHeight());
                            AllPlatforms.push_back(OnePlatform);
                        }
                    if (!vTwoPlatforms.empty())
                        for (int i = 0; i < vTwoPlatforms.size(); ++i)
                        {
                            dll::PROTON OnePlatform(vTwoPlatforms[i].x, vTwoPlatforms[i].y,
                                vTwoPlatforms[i].GetWidth(), vTwoPlatforms[i].GetHeight());
                            AllPlatforms.push_back(OnePlatform);
                        }
                    if (!vThreePlatforms.empty())
                        for (int i = 0; i < vThreePlatforms.size(); ++i)
                        {
                            dll::PROTON OnePlatform(vThreePlatforms[i].x, vThreePlatforms[i].y,
                                vThreePlatforms[i].GetWidth(), vThreePlatforms[i].GetHeight());
                            AllPlatforms.push_back(OnePlatform);
                        }
                    if (!vFourPlatforms.empty())
                        for (int i = 0; i < vFourPlatforms.size(); ++i)
                        {
                            dll::PROTON OnePlatform(vFourPlatforms[i].x, vFourPlatforms[i].y,
                                vFourPlatforms[i].GetWidth(), vFourPlatforms[i].GetHeight());
                            AllPlatforms.push_back(OnePlatform);
                        }

                    Hero->Jump((float)(level), AllPlatforms);
                    if (Hero->dir == dirs::right)
                    {
                        background_element_speed = 1.0f + (float)(level / 10);
                        background_element_dir = dirs::left;
                    }
                    else if (Hero->dir == dirs::left)
                    {
                        background_element_speed = 1.0f + (float)(level / 10);
                        background_element_dir = dirs::right;
                    }
                }
                break;

            case jump_down_flag:
                if (!vFields.empty())
                {
                    size_t all_platforms_number = vFields.size() + vOnePlatforms.size() + vTwoPlatforms.size()
                        + vThreePlatforms.size() + vFourPlatforms.size();

                    dll::PROT_MESH AllPlatforms(all_platforms_number);
                    for (int i = 0; i < vFields.size(); ++i)
                    {
                        dll::PROTON OnePlatform(vFields[i].x, vFields[i].y,
                            vFields[i].GetWidth(), vFields[i].GetHeight());
                        AllPlatforms.push_back(OnePlatform);
                    }
                    if (!vOnePlatforms.empty())
                        for (int i = 0; i < vOnePlatforms.size(); ++i)
                        {
                            dll::PROTON OnePlatform(vOnePlatforms[i].x, vOnePlatforms[i].y,
                                vOnePlatforms[i].GetWidth(), vOnePlatforms[i].GetHeight());
                            AllPlatforms.push_back(OnePlatform);
                        }
                    if (!vTwoPlatforms.empty())
                        for (int i = 0; i < vTwoPlatforms.size(); ++i)
                        {
                            dll::PROTON OnePlatform(vTwoPlatforms[i].x, vTwoPlatforms[i].y,
                                vTwoPlatforms[i].GetWidth(), vTwoPlatforms[i].GetHeight());
                            AllPlatforms.push_back(OnePlatform);
                        }
                    if (!vThreePlatforms.empty())
                        for (int i = 0; i < vThreePlatforms.size(); ++i)
                        {
                            dll::PROTON OnePlatform(vThreePlatforms[i].x, vThreePlatforms[i].y,
                                vThreePlatforms[i].GetWidth(), vThreePlatforms[i].GetHeight());
                            AllPlatforms.push_back(OnePlatform);
                        }
                    if (!vFourPlatforms.empty())
                        for (int i = 0; i < vFourPlatforms.size(); ++i)
                        {
                            dll::PROTON OnePlatform(vFourPlatforms[i].x, vFourPlatforms[i].y,
                                vFourPlatforms[i].GetWidth(), vFourPlatforms[i].GetHeight());
                            AllPlatforms.push_back(OnePlatform);
                        }

                    Hero->Jump((float)(level), AllPlatforms);
                    if (Hero->dir == dirs::right)
                    {
                        background_element_speed = 1.0f + (float)(level / 10);
                        background_element_dir = dirs::left;
                    }
                    else if (Hero->dir == dirs::left)
                    {
                        background_element_speed = 1.0f + (float)(level / 10);
                        background_element_dir = dirs::right;
                    }
                }
                break;

            case fall_flag:
                if (!vFields.empty())
                {
                    size_t all_platforms_number = vFields.size() + vOnePlatforms.size() + vTwoPlatforms.size()
                        + vThreePlatforms.size() + vFourPlatforms.size();

                    dll::PROT_MESH AllPlatforms(all_platforms_number);
                    for (int i = 0; i < vFields.size(); ++i)
                    {
                        dll::PROTON OnePlatform(vFields[i].x, vFields[i].y,
                            vFields[i].GetWidth(), vFields[i].GetHeight());
                        AllPlatforms.push_back(OnePlatform);
                    }
                    if (!vOnePlatforms.empty())
                        for (int i = 0; i < vOnePlatforms.size(); ++i)
                        {
                            dll::PROTON OnePlatform(vOnePlatforms[i].x, vOnePlatforms[i].y,
                                vOnePlatforms[i].GetWidth(), vOnePlatforms[i].GetHeight());
                            AllPlatforms.push_back(OnePlatform);
                        }
                    if (!vTwoPlatforms.empty())
                        for (int i = 0; i < vTwoPlatforms.size(); ++i)
                        {
                            dll::PROTON OnePlatform(vTwoPlatforms[i].x, vTwoPlatforms[i].y,
                                vTwoPlatforms[i].GetWidth(), vTwoPlatforms[i].GetHeight());
                            AllPlatforms.push_back(OnePlatform);
                        }
                    if (!vThreePlatforms.empty())
                        for (int i = 0; i < vThreePlatforms.size(); ++i)
                        {
                            dll::PROTON OnePlatform(vThreePlatforms[i].x, vThreePlatforms[i].y,
                                vThreePlatforms[i].GetWidth(), vThreePlatforms[i].GetHeight());
                            AllPlatforms.push_back(OnePlatform);
                        }
                    if (!vFourPlatforms.empty())
                        for (int i = 0; i < vFourPlatforms.size(); ++i)
                        {
                            dll::PROTON OnePlatform(vFourPlatforms[i].x, vFourPlatforms[i].y,
                                vFourPlatforms[i].GetWidth(), vFourPlatforms[i].GetHeight());
                            AllPlatforms.push_back(OnePlatform);
                        }

                    Hero->Fall((float)(level), AllPlatforms);
                    background_element_speed = 0;
                    background_element_dir = dirs::down;
                }
                break;

            case stop_flag:
                background_element_speed = 0;
                background_element_dir = dirs::down;
                break;
            }
        }

        if (!vFields.empty() && Hero)
        {
            char stopped = Hero->GetMoveFlag();
            if (stopped != stop_flag)
            {
                for (int i = 0; i < vFields.size(); i++)
                {
                    if (background_element_dir == dirs::left)
                    {
                        vFields[i].x -= background_element_speed;
                        vFields[i].SetEdges();
                        if (vFields[i].ex <= -scr_width)
                        {
                            vFields.erase(vFields.begin() + i);
                            need_right_field = true;
                            break;
                        }
                    }
                    else if (background_element_dir == dirs::right)
                    {
                        vFields[i].x += background_element_speed;
                        vFields[i].SetEdges();
                        if (vFields[i].x >= 2 * scr_width)
                        {
                            vFields.erase(vFields.begin() + i);
                            need_left_field = true;
                            break;
                        }
                    }
                }
            }
        }
        if (need_left_field)
        {
            need_left_field = false;
            vFields.push_back(dll::PROTON(-scr_width, ground, scr_width, 50.0f));
        }
        if (need_right_field)
        {
            need_right_field = false;
            vFields.push_back(dll::PROTON(scr_width, ground, scr_width, 50.0f));
        }

        if (!vBackgrounds.empty() && Hero)
        {
            char stopped = Hero->GetMoveFlag();
            if (stopped != stop_flag)
            {
                for (int i = 0; i < vBackgrounds.size(); i++)
                {
                    vBackgrounds[i].x += Hero->GetXAxisMove((float)(level));
                    vBackgrounds[i].SetEdges();
                    if (vBackgrounds[i].ex <= -scr_height)
                    {
                        need_right_background = true;
                        vBackgrounds.erase(vBackgrounds.begin() + i);
                        break;
                    }
                    if (vBackgrounds[i].x >= 2 * scr_height)
                    {
                        need_left_background = true;
                        vBackgrounds.erase(vBackgrounds.begin() + i);
                        break;
                    }
                }
            }
        }
        if (need_left_background)
        {
            need_left_background = false;
            vBackgrounds.push_back(dll::PROTON(-scr_width, 50.0f, scr_width, ground));
        }
        if (need_right_background)
        {
            need_right_background = false;
            vBackgrounds.push_back(dll::PROTON(scr_width, 50.0f, scr_width, ground));
        }

        // CREATE PLATFORMS ***********************

        if (vOnePlatforms.size() < 2 && RandEngine(0, 1500) == 32)
        {
            bool can_be_pushed = true;

            if (!vOnePlatforms.empty())
                if (vOnePlatforms.back().ex >= scr_width)can_be_pushed = false;
            if (!vTwoPlatforms.empty())
                if (vTwoPlatforms.back().ex >= scr_width)can_be_pushed = false;
            if (!vThreePlatforms.empty())
                if (vThreePlatforms.back().ex >= scr_width)can_be_pushed = false;
            if (!vFourPlatforms.empty())
                if (vFourPlatforms.back().ex >= scr_width)can_be_pushed = false;

            if (can_be_pushed)
                vOnePlatforms.push_back(dll::PROTON(scr_width + (float)(RandEngine(140, 200)), ground - 80.0f, 140.0f, 80.0f));
        }
        if (!vOnePlatforms.empty() && Hero)
        {
            char stopped = Hero->GetMoveFlag();
            if(stopped != stop_flag)
            for (std::vector<dll::PROTON>::iterator plat = vOnePlatforms.begin(); plat < vOnePlatforms.end(); ++plat)
            {
                plat->x += Hero->GetXAxisMove((float)(level));
                plat->SetEdges();
                if (plat->ex <= -scr_width || plat->x >= 2 * scr_width)
                {
                    vOnePlatforms.erase(plat);
                    break;
                }
            }
        }
        
        if (vTwoPlatforms.size() < 2 && RandEngine(0, 4000) == 118)
        {
            bool can_be_pushed = true;

            if (!vOnePlatforms.empty())
                if (vOnePlatforms.back().ex >= scr_width)can_be_pushed = false;
            if (!vTwoPlatforms.empty())
                if (vTwoPlatforms.back().ex >= scr_width)can_be_pushed = false;
            if (!vThreePlatforms.empty())
                if (vThreePlatforms.back().ex >= scr_width)can_be_pushed = false;
            if (!vFourPlatforms.empty())
                if (vFourPlatforms.back().ex >= scr_width)can_be_pushed = false;

            if (can_be_pushed)
                vTwoPlatforms.push_back(dll::PROTON(scr_width + (float)(RandEngine(140, 200)), ground - 94.0f, 150.0f, 94.0f));
        }
        if (!vTwoPlatforms.empty() && Hero)
        {
            char stopped = Hero->GetMoveFlag();
            if (stopped != stop_flag)
            for (std::vector<dll::PROTON>::iterator plat = vTwoPlatforms.begin(); plat < vTwoPlatforms.end(); ++plat)
            {
                plat->x += Hero->GetXAxisMove((float)(level));
                plat->SetEdges();
                if (plat->ex <= -scr_width || plat->x >= 2 * scr_width)
                {
                    vTwoPlatforms.erase(plat);
                    break;
                }
            }
        }
        
        if (vThreePlatforms.size() < 2 && RandEngine(0, 500) == 66)
        {
            bool can_be_pushed = true;

            if (!vOnePlatforms.empty())
                if (vOnePlatforms.back().ex >= scr_width)can_be_pushed = false;
            if (!vTwoPlatforms.empty())
                if (vTwoPlatforms.back().ex >= scr_width)can_be_pushed = false;
            if (!vThreePlatforms.empty())
                if (vThreePlatforms.back().ex >= scr_width)can_be_pushed = false;
            if (!vFourPlatforms.empty())
                if (vFourPlatforms.back().ex >= scr_width)can_be_pushed = false;

            if (can_be_pushed)
                vThreePlatforms.push_back(dll::PROTON(scr_width + (float)(RandEngine(140, 200)), ground - 90.0f, 110.0f, 90.0f));
        }
        if (!vThreePlatforms.empty() && Hero)
        {
            char stopped = Hero->GetMoveFlag();
            if (stopped != stop_flag)
            for (std::vector<dll::PROTON>::iterator plat = vThreePlatforms.begin(); plat < vThreePlatforms.end(); ++plat)
            {
                plat->x += Hero->GetXAxisMove((float)(level));
                plat->SetEdges();
                if (plat->ex <= -scr_width || plat->x >= 2 * scr_width)
                {
                    vThreePlatforms.erase(plat);
                    break;
                }
            }
        }

        if (vFourPlatforms.size() < 2 && RandEngine(0, 1000) == 83)
        {
            bool can_be_pushed = true;

            if (!vOnePlatforms.empty())
                if (vOnePlatforms.back().ex >= scr_width)can_be_pushed = false;
            if (!vTwoPlatforms.empty())
                if (vTwoPlatforms.back().ex >= scr_width)can_be_pushed = false;
            if (!vThreePlatforms.empty())
                if (vThreePlatforms.back().ex >= scr_width)can_be_pushed = false;
            if (!vFourPlatforms.empty())
                if (vFourPlatforms.back().ex >= scr_width)can_be_pushed = false;

            if (can_be_pushed)
                vFourPlatforms.push_back(dll::PROTON(scr_width + (float)(RandEngine(140, 200)), ground - 100.0f, 85.0f, 100.0f));
        }
        if (!vFourPlatforms.empty() && Hero)
        {
            char stopped = Hero->GetMoveFlag();
            if (stopped != stop_flag)
            for (std::vector<dll::PROTON>::iterator plat = vFourPlatforms.begin(); plat < vFourPlatforms.end(); ++plat)
            {
                plat->x += Hero->GetXAxisMove((float)(level));
                plat->SetEdges();
                if (plat->ex <= -scr_width || plat->x >= 2 * scr_width)
                {
                    vFourPlatforms.erase(plat);
                    break;
                }
            }
        }
        /////////////////////////////////////////////////////////////////////
        
        //CREATE AND MOVE EVILS *****************************

        if (vEvils.size() < 4 + level && RandEngine(0, 350) == 333)
        {
            int atype = RandEngine(0, 4);
            
            switch (atype)
            {
            case 0:
                vEvils.push_back(dll::CreatureFactory(evil1_type, scr_width + (float)(RandEngine(0, 50)), ground - 43.0f));
                break;

            case 1:
                vEvils.push_back(dll::CreatureFactory(evil2_type, scr_width + (float)(RandEngine(0, 50)), ground - 40.0f));
                break;

            case 2:
                vEvils.push_back(dll::CreatureFactory(evil3_type, scr_width + (float)(RandEngine(00, 50)), ground - 45.0f));
                break;

            case 3:
                vEvils.push_back(dll::CreatureFactory(evil4_type, scr_width + (float)(RandEngine(0, 50)), ground - 30.0f));
                break;

            case 4:
                vEvils.push_back(dll::CreatureFactory(evil5_type, scr_width + (float)(RandEngine(0, 50)), ground - 50.0f));
                break;

            }
        }

        if (!vEvils.empty() && Hero)
        {
            for (std::vector<dll::Creature>::iterator evil = vEvils.begin(); evil < vEvils.end(); ++evil)
            {
                size_t obst_num = vFields.size() + vOnePlatforms.size() + vTwoPlatforms.size()
                    + vThreePlatforms.size() + vFourPlatforms.size();
                
                dll::PROT_MESH AllObstacles(obst_num);

                for (int i = 0; i < vFields.size(); ++i)
                {
                    dll::PROTON obstacle(dll::PROTON(vFields[i].x, vFields[i].y, scr_width, 50.0f));
                    AllObstacles.push_back(obstacle);
                }
                if(!vOnePlatforms.empty())
                    for (int i = 0; i < vOnePlatforms.size(); ++i)
                    {
                        dll::PROTON obstacle(dll::PROTON(vOnePlatforms[i].x, vOnePlatforms[i].y, 140.0f, 80.0f));
                        AllObstacles.push_back(obstacle);
                    }
                if (!vTwoPlatforms.empty())
                    for (int i = 0; i < vTwoPlatforms.size(); ++i)
                    {
                        dll::PROTON obstacle(dll::PROTON(vTwoPlatforms[i].x, vTwoPlatforms[i].y, 150.0f, 94.0f));
                        AllObstacles.push_back(obstacle);
                    }
                if (!vThreePlatforms.empty())
                    for (int i = 0; i < vThreePlatforms.size(); ++i)
                    {
                        dll::PROTON obstacle(dll::PROTON(vThreePlatforms[i].x, vThreePlatforms[i].y, 110.0f, 90.0f));
                        AllObstacles.push_back(obstacle);
                    }
                if (!vFourPlatforms.empty())
                    for (int i = 0; i < vFourPlatforms.size(); ++i)
                    {
                        dll::PROTON obstacle(dll::PROTON(vFourPlatforms[i].x, vFourPlatforms[i].y, 85.0f, 100.0f));
                        AllObstacles.push_back(obstacle);
                    }

                char action = (*evil)->GetMoveFlag();

                if (Hero->ey < (*evil)->y && RandEngine(0, 100) == 66 && action != jump_up_flag && action != jump_down_flag)
                    (*evil)->Jump((float)(level), AllObstacles);
                    
                
                if (action == run_flag)
                {
                    if (background_element_speed)
                        (*evil)->Move((float)(level) * 10.0f, Hero->x, (*evil)->y, AllObstacles);
                    else
                        (*evil)->Move((float)(level), Hero->x, (*evil)->y, AllObstacles);
                }
                else if (action == fall_flag)(*evil)->Fall((float)(level), AllObstacles);
                else if (action == jump_up_flag || action == jump_down_flag) (*evil)->Jump((float)(level), AllObstacles);
            }
        }

        if (!vShots.empty())
        {
            for (std::vector<dll::SHOT>::iterator shot = vShots.begin(); shot < vShots.end(); ++shot)
            {
                if (!shot->Move(float(level)))
                {
                    vShots.erase(shot);
                    break;
                }
            }
        }

        if (!vEvils.empty() && !vShots.empty())
        {
            bool killed = false;

            for (std::vector<dll::Creature>::iterator evil = vEvils.begin(); evil < vEvils.end(); evil++)
            {
                for (std::vector<dll::SHOT>::iterator shot = vShots.begin(); shot < vShots.end(); shot++)
                {
                    float a = (float)(pow((abs(((*evil)->x + (*evil)->GetWidth() / 2) - (shot->x + shot->GetWidth() / 2))), 2));
                    float b = (float)(pow((abs(((*evil)->y + (*evil)->GetHeight() / 2) - (shot->y + shot->GetHeight() / 2))), 2));
                    float distance = sqrt(a + b);

                    if ((((*evil)->GetWidth() / 2 + shot->GetWidth() / 2) > distance))
                    {
                        (*evil)->lifes -= 20;
                        vShots.erase(shot);
                        if ((*evil)->lifes <= 0)
                        {
                            (*evil)->Release();
                            vEvils.erase(evil);
                            score += 10 + level;
                            if (sound)mciSendString(L"play .\\res\\snd\\evilkilled.wav", NULL, NULL, NULL);
                            killed = true;
                        }
                        break;
                    }
                }
                if (killed)break;
            }
        }

        if (!vEvils.empty() && Hero)
        {
            for (std::vector<dll::Creature>::iterator evil = vEvils.begin(); evil < vEvils.end(); ++evil)
            {
                float a = (float)(pow((abs(((*evil)->x + (*evil)->GetWidth() / 2) - (Hero->x + Hero->GetWidth() / 2))), 2));
                float b = (float)(pow((abs(((*evil)->y + (*evil)->GetHeight() / 2) - (Hero->y + Hero->GetHeight() / 2))), 2));
                float distance = sqrt(a + b);

                if (distance <= ((*evil)->GetWidth() / 2 + Hero->GetWidth() / 2))
                {
                    Hero->lifes -= (*evil)->Attack();
                    if (Hero->lifes <= 0)
                    {
                        RIP_x = Hero->x;
                        RIP_y = Hero->y;
                        hero_killed = true;
                        ClearHeap(&Hero);
                        break;
                    }
                }
            }
        }
        
        ///////////////////////////////////////////
        
        // ASSETS *********************************
        
        if (!Potion && RandEngine(0, 1800) == 888)
            Potion = new dll::PROTON{ scr_width + RandEngine(0,100),ground - 32.0f,32.0f,32.0f };
        if (Potion && Hero)
        {
            char stopped = Hero->GetMoveFlag();
            if (stopped != stop_flag)
            {
                Potion->x += Hero->GetXAxisMove((float)(level));
                Potion->SetEdges();
                if (Potion->ex <= -scr_width || Potion->x >= 2 * scr_width)
                {
                    delete Potion;
                    Potion = nullptr;
                }
            }
            if(Potion)
            {
                if (!(Hero->x > Potion->ex || Hero->ex < Potion->x || Hero->y > Potion->ey || Hero->ey < Potion->y))
                {
                    delete Potion;
                    Potion = nullptr;
                    if (Hero->lifes + 20 <= 100)Hero->lifes += 20;
                    else score += 20;
                    if (sound)mciSendString(L"play .\\res\\snd\\takeasset.wav", NULL, NULL, NULL);
                }
            }
        }

        if (!Gold && RandEngine(0, 1500) == 556)
            Gold = new dll::PROTON{ scr_width + RandEngine(0,100),ground - 32.0f,32.0f,32.0f };
        if (Gold && Hero)
        {
            char stopped = Hero->GetMoveFlag();
            if (stopped != stop_flag)
            {
                Gold->x += Hero->GetXAxisMove((float)(level));
                Gold->SetEdges();
                if (Gold->ex <= -scr_width || Gold->x >= 2 * scr_width)
                {
                    delete Gold;
                    Gold = nullptr;
                }
            }
            if(Gold)
            {
                if (!(Hero->x > Gold->ex || Hero->ex < Gold->x || Hero->y > Gold->ey || Hero->ey < Gold->y))
                {
                    delete Gold;
                    Gold = nullptr;
                    score += 50;
                    if (sound)mciSendString(L"play .\\res\\snd\\takeasset.wav", NULL, NULL, NULL);
                }
            }
        }

        if (!Crystal && RandEngine(0, 2000) == 333)
            Crystal = new dll::PROTON{ scr_width + RandEngine(0,100),ground - 32.0f,32.0f,32.0f };
        if (Crystal && Hero)
        {
            char stopped = Hero->GetMoveFlag();
            if (stopped != stop_flag)
            {
                Crystal->x += Hero->GetXAxisMove((float)(level));
                Crystal->SetEdges();
                if (Crystal->ex <= -scr_width || Crystal->x >= 2 * scr_width)
                {
                    delete Crystal;
                    Crystal = nullptr;
                }
               
            }
            if(Crystal)
            {
                if (!(Hero->x > Crystal->ex || Hero->ex < Crystal->x || Hero->y > Crystal->ey || Hero->ey < Crystal->y))
                {
                    delete Crystal;
                    Crystal = nullptr;
                    ++number_of_crystals_collected;
                    if (sound)mciSendString(L"play .\\res\\snd\\crystal.wav", NULL, NULL, NULL);
                    if (number_of_crystals_collected > 8 + level)
                    {
                        Draw->EndDraw();
                        LevelUp();
                    }
                }
            }
        }
        


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

        if(!vBackgrounds.empty())
        for(int i=0;i<vBackgrounds.size();i++)
            Draw->DrawBitmap(bmpBackground[background_frame], D2D1::RectF(vBackgrounds[i].x,vBackgrounds[i].y,
                vBackgrounds[i].ex, vBackgrounds[i].ey));
        if (!vFields.empty())
            for (int i = 0; i < vFields.size(); i++)
                Draw->DrawBitmap(bmpBaseField, D2D1::RectF(vFields[i].x, vFields[i].y, vFields[i].ex, vFields[i].ey));

        if (!vOnePlatforms.empty())
            for (int i = 0; i < vOnePlatforms.size(); ++i)
                Draw->DrawBitmap(bmpPlatform1, D2D1::RectF(vOnePlatforms[i].x, vOnePlatforms[i].y,
                    vOnePlatforms[i].ex, vOnePlatforms[i].ey));
        if (!vTwoPlatforms.empty())
            for (int i = 0; i < vTwoPlatforms.size(); ++i)
                Draw->DrawBitmap(bmpPlatform2, D2D1::RectF(vTwoPlatforms[i].x, vTwoPlatforms[i].y,
                    vTwoPlatforms[i].ex, vTwoPlatforms[i].ey));
        if (!vThreePlatforms.empty())
            for (int i = 0; i < vThreePlatforms.size(); ++i)
                Draw->DrawBitmap(bmpPlatform3, D2D1::RectF(vThreePlatforms[i].x, vThreePlatforms[i].y,
                    vThreePlatforms[i].ex, vThreePlatforms[i].ey));
        if (!vFourPlatforms.empty())
            for (int i = 0; i < vFourPlatforms.size(); ++i)
                Draw->DrawBitmap(bmpPlatform4, D2D1::RectF(vFourPlatforms[i].x, vFourPlatforms[i].y,
                    vFourPlatforms[i].ex, vFourPlatforms[i].ey));

        if (Hero)
        {
            switch (Hero->dir)
            {
            case dirs::left:
                Draw->DrawBitmap(bmpHeroL[Hero->GetFrame()], Resizer(bmpHeroL[Hero->GetFrame()], Hero->x, Hero->y));
                break;

            case dirs::right:
                Draw->DrawBitmap(bmpHeroR[Hero->GetFrame()], Resizer(bmpHeroR[Hero->GetFrame()], Hero->x, Hero->y));
                break;
            }

            if (inactBrush)
                Draw->DrawLine(D2D1::Point2F(Hero->x, Hero->ey + 8.0f),
                    D2D1::Point2F(Hero->x + (float)(Hero->lifes) / 2.5f, Hero->ey + 8.0f), inactBrush, 7.0f);
        }

        if (!vEvils.empty())
        {
            for (int i = 0; i < vEvils.size(); i++)
            {
                char type = vEvils[i]->GetTypeFlag();

                switch (type)
                {
                case evil1_type:
                    if (vEvils[i]->dir == dirs::left)
                        Draw->DrawBitmap(bmpEvil1L[vEvils[i]->GetFrame()], Resizer(bmpEvil1L[vEvils[i]->GetFrame()],
                            vEvils[i]->x, vEvils[i]->y));
                    else if (vEvils[i]->dir == dirs::right)
                        Draw->DrawBitmap(bmpEvil1R[vEvils[i]->GetFrame()], Resizer(bmpEvil1R[vEvils[i]->GetFrame()],
                            vEvils[i]->x, vEvils[i]->y));
                    break;

                case evil2_type:
                    Draw->DrawBitmap(bmpEvil2[vEvils[i]->GetFrame()], Resizer(bmpEvil2[vEvils[i]->GetFrame()],
                            vEvils[i]->x, vEvils[i]->y));
                    break;

                case evil3_type:
                    if (vEvils[i]->dir == dirs::left)
                        Draw->DrawBitmap(bmpEvil3L[vEvils[i]->GetFrame()], Resizer(bmpEvil3L[vEvils[i]->GetFrame()],
                            vEvils[i]->x, vEvils[i]->y));
                    else if (vEvils[i]->dir == dirs::right)
                        Draw->DrawBitmap(bmpEvil3R[vEvils[i]->GetFrame()], Resizer(bmpEvil3R[vEvils[i]->GetFrame()],
                            vEvils[i]->x, vEvils[i]->y));
                    break;

                case evil4_type:
                    Draw->DrawBitmap(bmpEvil4[vEvils[i]->GetFrame()], Resizer(bmpEvil4[vEvils[i]->GetFrame()],
                        vEvils[i]->x, vEvils[i]->y));
                    break;

                case evil5_type:
                    Draw->DrawBitmap(bmpEvil5[vEvils[i]->GetFrame()], Resizer(bmpEvil5[vEvils[i]->GetFrame()],
                        vEvils[i]->x, vEvils[i]->y));
                    break;
                }

                if (inactBrush)
                    Draw->DrawLine(D2D1::Point2F(vEvils[i]->x, vEvils[i]->ey + 8.0f),
                        D2D1::Point2F(vEvils[i]->x + (float)(vEvils[i]->lifes) / 1.5f, vEvils[i]->ey + 8.0f), inactBrush, 7.0f);
            }
        }

        if (!vShots.empty())
        {
            for (std::vector<dll::SHOT>::iterator shot = vShots.begin(); shot < vShots.end(); shot++)
            {
                ID2D1RadialGradientBrush* shootBrush = nullptr;
                D2D1_GRADIENT_STOP gStops[2]{};
                ID2D1GradientStopCollection* stColl = nullptr;

                gStops[0].position = 0;
                gStops[0].color = D2D1::ColorF(D2D1::ColorF::Violet);
                gStops[1].position = 1.0f;
                gStops[1].color = D2D1::ColorF(D2D1::ColorF::Orange);

                Draw->CreateGradientStopCollection(gStops, 2, &stColl);
                if (stColl)
                {
                    Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(shot->x, shot->y),
                        D2D1::Point2F(0, 0), shot->GetWidth() / 2, shot->GetHeight() / 2), stColl, &shootBrush);
                    if (shootBrush)
                        Draw->FillEllipse(D2D1::Ellipse(D2D1::Point2F(shot->x, shot->y),
                            shot->GetWidth() / 2, shot->GetHeight() / 2), shootBrush);
                    ClearHeap(&shootBrush);
                    ClearHeap(&stColl);
                }
            }
        }

        if (Potion) Draw->DrawBitmap(bmpPotion, D2D1::RectF(Potion->x, Potion->y, Potion->ex, Potion->ey));
        if (Gold) Draw->DrawBitmap(bmpGold, D2D1::RectF(Gold->x, Gold->y, Gold->ex, Gold->ey));
        if (Crystal) Draw->DrawBitmap(bmpCrystal, D2D1::RectF(Crystal->x, Crystal->y, Crystal->ex, Crystal->ey));
        
        if (hero_killed)
        {
            Draw->DrawBitmap(bmpRIP, D2D1::RectF(RIP_x, RIP_y, RIP_x + 43.0f, RIP_y + 50.0f));
            Draw->EndDraw();
            if (sound)
            {
                PlaySound(NULL, NULL, NULL);
                PlaySound(L".\\res\\snd\\killed.wav", NULL, SND_SYNC);
            }
            else Sleep(3000);
            GameOver();
        }

        // STATUS TEXT ************************

        Draw->DrawBitmap(bmpCrystal, D2D1::RectF(scr_width / 2 - 50.0f, 70.0f, scr_width / 2, 120.0f));
        
        wchar_t status_txt[150] = L"\0";
        wchar_t add[5] = L"\0";
        int txt_size = 0;

        wcscpy_s(status_txt, L" : ");
        wsprintf(add, L"%d", 9 + level - number_of_crystals_collected);
        wcscat_s(status_txt, add);

        for (int i = 0; i < 150; ++i)
        {
            if (status_txt[i] != '\0')++txt_size;
            else break;
        }

        if (midFormat && statTxtBrush)
        {
            Draw->DrawTextW(status_txt, txt_size, midFormat,
                D2D1::RectF(scr_width / 2, 70.0f, scr_width, 150.0f), statTxtBrush);
            txt_size = 0;
        
            wcscpy_s(status_txt, current_player);

            wcscat_s(status_txt, L", ниво: ");
            wsprintf(add, L"%d", level);
            wcscat_s(status_txt, add);

            wcscat_s(status_txt, L", резултат: ");
            wsprintf(add, L"%d", score);
            wcscat_s(status_txt, add);
        
            for (int i = 0; i < 150; ++i)
            {
                if (status_txt[i] != '\0')++txt_size;
                else break;
            }

            Draw->DrawTextW(status_txt, txt_size, midFormat,
                D2D1::RectF(20.0f, ground + 10.0f, scr_width, scr_height), statTxtBrush);

        }



        //////////////////////////////////////
        Draw->EndDraw();



    }

    std::remove(tmp_file);
    ReleaseResources();
    return (int) bMsg.wParam;
}