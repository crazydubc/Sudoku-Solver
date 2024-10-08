// Sudoku.cpp : Defines the entry point for the application.
//

#include "framework.h"

#include "Sudoku.h"
#include "CPuzzleEdit.h"
#include <string>
#include "globals.h"
#include "solver.h"
#include "DancingLinks.h"

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND hEdit[GRID_SIZE][GRID_SIZE]; // Handle to the edit control
HWND hSolve;
WNDPROC wpOrigEditProc; // Original window procedure of the edit control

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SUDOKU, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SUDOKU));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SUDOKU));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SUDOKU);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, dwStyle,
      CW_USEDEFAULT, 0, CLIENT_WIDTH, CLIENT_HEIGHT, nullptr, nullptr, hInstance, nullptr);


   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

static LRESULT CALLBACK EditSubclassProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CHAR:
    {
        wchar_t chChar = (wchar_t)wParam;
        if ((chChar < L'1' || chChar > L'9') && chChar != VK_BACK)
        {
            // Discard the character
            return 0;
        }
        return CallWindowProc(wpOrigEditProc, hWnd, message, wParam, lParam);
    }
    break;

    case WM_PASTE:
    {
        // Handle paste operation
        if (OpenClipboard(hWnd))
        {
            HANDLE hData = GetClipboardData(CF_UNICODETEXT);
            if (hData)
            {
                wchar_t* pszText = (wchar_t*)GlobalLock(hData);
                if (pszText)
                {
                    // Filter the text
                    std::wstring strText(pszText);
                    std::wstring strFiltered;
                    for (size_t i = 0; i < strText.length(); ++i)
                    {
                        wchar_t ch = strText[i];
                        if (ch >= L'1' && ch <= L'9')
                        {
                            strFiltered += ch;
                        }
                    }
                    // Insert the filtered text
                    // Replace the current selection with filtered text
                    SendMessage(hWnd, EM_REPLACESEL, TRUE, (LPARAM)strFiltered.c_str());
                    GlobalUnlock(hData);
                }
            }
            CloseClipboard();
        }
        return 0; // Indicate that we've handled the message
    }
    break;

    default:
        return CallWindowProc(wpOrigEditProc, hWnd, message, wParam, lParam);
    }
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE:
            {
                CONST INT BUFFER = CELL_SIZE + CELL_SPACING;
                for (int row = 0; row < GRID_SIZE; row++)
                {
                    for (int col = 0; col < GRID_SIZE; col++)
                    {
                        const int x = X_OFFSET + (col * BUFFER);
                        const int y = Y_OFFSET + (row * BUFFER);

                        hEdit[row][col] = CreateWindowEx(
                            WS_EX_CLIENTEDGE,
                            L"EDIT",
                            NULL,
                            WS_CHILD | WS_VISIBLE | ES_CENTER,
                            x, y, CELL_SIZE, CELL_SIZE,
                            hWnd,
                            (HMENU)(IDC_PUZZLE_EDIT+ row * GRID_SIZE + col),
                            hInst,
                            NULL
                        );

                        if (hEdit[row][col] == NULL)
                        {
                            MessageBox(hWnd, L"Could not create edit box.", L"Error", MB_OK | MB_ICONERROR);
                        }
                        else
                        {
                            // Subclass each edit control
                            wpOrigEditProc = (WNDPROC)SetWindowLongPtr(hEdit[row][col], GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);
                            // Set the maximum text length to 1 character
                            SendMessage(hEdit[row][col], EM_SETLIMITTEXT, (WPARAM)1, 0);
                        }
                    }
                }
                hSolve = CreateWindowEx(WS_EX_CLIENTEDGE, L"BUTTON", NULL, WS_CHILD | WS_VISIBLE | ES_CENTER, X_OFFSET, Y_OFFSET + (10 * BUFFER),
                    50, 20, hWnd, (HMENU)IDM_SOLVE, hInst, NULL);
                SetWindowTextW(hSolve, L"Solve");
            }
        case WM_COMMAND:
            {
                const int wmId = LOWORD(wParam);
                // Parse the menu selections:
                switch (wmId)
                {
                    case IDM_SOLVE:
                    {
                        DancingLinks solver;
                        PUZZLE_GRID puzzle;

                        TCHAR buffer[2]; // Buffer to hold the digit and null terminator
                        // Retrieve values from the edit controls into the puzzle grid
                        for (int row = 0; row < GRID_SIZE; row++) {
                            for (int col = 0; col < GRID_SIZE; col++) {
                                int length = GetWindowTextW(hEdit[row][col], buffer, 2);
                                if (length == 0) {
                                    puzzle[row][col] = 0; // Empty cell
                                }
                                else {
                                    puzzle[row][col] = _ttoi(buffer); // Convert text to integer
                                }
                            }
                        }

                        // Solve the puzzle
                        if (solver.Solve(puzzle)) {
                            // Set the new values to the edit controls
                            for (int row = 0; row < GRID_SIZE; row++) {
                                for (int col = 0; col < GRID_SIZE; col++) {
                                    if (puzzle[row][col] == 0) {
                                        SetWindowTextW(hEdit[row][col], L""); // Clear the edit control
                                    }
                                    else {
                                        _itow_s(puzzle[row][col], buffer, 2, 10); // Convert integer to string
                                        SetWindowTextW(hEdit[row][col], buffer); // Set text in the edit control
                                    }
                                }
                            }
                        }
                        else {
                            // Handle the case where the puzzle cannot be solved
                            MessageBox(hWnd, L"The puzzle cannot be solved.", L"Error", MB_OK | MB_ICONERROR);
                        }

                        break;
                    }
                    case IDM_ABOUT:
                    {
                        DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                        break;
                    }
                    case IDM_EXIT:
                        DestroyWindow(hWnd);
                        break;
                    default:
                        return DefWindowProc(hWnd, message, wParam, lParam);
                }
            }
            break;
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hWnd, &ps);
                // TODO: Add any drawing code that uses hdc here...
                EndPaint(hWnd, &ps);
            }
            break;
        case WM_DESTROY:
        {
            
            PostQuitMessage(0);
            break;
        }
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
