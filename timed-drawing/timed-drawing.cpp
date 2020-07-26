#include "framework.h"
#include "timed-drawing.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

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

	int titleLoaded = LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	int appLoaded = LoadStringW(hInstance, IDC_TIMEDDRAWING, szWindowClass, MAX_LOADSTRING);
	ATOM registerCls = MyRegisterClass(hInstance);

	DWORD dw = GetLastError();

	if (!registerCls)
	{
		return FALSE;
	}

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TIMEDDRAWING));

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

	return (int)msg.wParam;
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

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TIMEDDRAWING));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_TIMEDDRAWING);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

RECT rcCurrent = { 0,0,20,20 };
POINT aptStar[6] = { 10,1, 1,19, 19,6, 1,6, 19,19, 10,1 };
int X = 2, Y = -1, idTimer = -1;
BOOL fVisible = FALSE;
HDC hdc;

LRESULT APIENTRY WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	RECT rc;

	switch (message)
	{
	case WM_CREATE:

		// Calculate the starting point.  

		GetClientRect(hwnd, &rc);
		OffsetRect(&rcCurrent, rc.right / 2, rc.bottom / 2);

		// Initialize the private DC.  

		hdc = GetDC(hwnd);
		SetViewportOrgEx(hdc, rcCurrent.left,
			rcCurrent.top, NULL);
		SetROP2(hdc, R2_NOT);

		// Start the timer.  

		SetTimer(hwnd, idTimer = 1, 10, NULL);
		return 0L;

	case WM_DESTROY:
		KillTimer(hwnd, 1);
		PostQuitMessage(0);
		return 0L;

	case WM_SIZE:
		switch (wParam)
		{
		case SIZE_MINIMIZED:

			// Stop the timer if the window is minimized.

				KillTimer(hwnd, 1);
			idTimer = -1;
			break;

		case SIZE_RESTORED:

			// Move the star back into the client area  
			// if necessary.  

			if (rcCurrent.right > (int)LOWORD(lParam))
			{
				rcCurrent.left =
					(rcCurrent.right =
					(int)LOWORD(lParam)) - 20;
			}
			if (rcCurrent.bottom > (int)HIWORD(lParam))
			{
				rcCurrent.top =
					(rcCurrent.bottom =
					(int)HIWORD(lParam)) - 20;
			}

			// Fall through to the next case.  

		case SIZE_MAXIMIZED:

			// Start the timer if it had been stopped.  

			if (idTimer == -1)
				SetTimer(hwnd, idTimer = 1, 10, NULL);
			break;
		}
		return 0L;

	case WM_TIMER:

		// Hide the star if it is visible.  

		if (fVisible)
			Polyline(hdc, aptStar, 6);

		// Bounce the star off a side if necessary.  

		GetClientRect(hwnd, &rc);
		if (rcCurrent.left + X < rc.left ||
			rcCurrent.right + X > rc.right)
			X = -X;
		if (rcCurrent.top + Y < rc.top ||
			rcCurrent.bottom + Y > rc.bottom)
			Y = -Y;

		// Show the star in its new position.  

		OffsetRect(&rcCurrent, X, Y);
		SetViewportOrgEx(hdc, rcCurrent.left,
			rcCurrent.top, NULL);
		fVisible = Polyline(hdc, aptStar, 6);

		return 0L;

	case WM_ERASEBKGND:

		// Erase the star.  

		fVisible = FALSE;
		return DefWindowProc(hwnd, message, wParam, lParam);

	case WM_PAINT:

		// Show the star if it is not visible. Use BeginPaint  
		// to clear the update region.  

		BeginPaint(hwnd, &ps);
		if (!fVisible)
			fVisible = Polyline(hdc, aptStar, 6);
		EndPaint(hwnd, &ps);
		return 0L;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
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
