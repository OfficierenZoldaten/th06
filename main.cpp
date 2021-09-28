#include <windows.h>
#include <cstdio>
#include <tchar.h>

#include <d3d8.h>

class __declspec(align(4)) CGameWindow {
public:
	HWND m_hWnd;
	DWORD bClosing;
	DWORD bActive;
	DWORD bUnfocused;
	BYTE byte0;
};

class __declspec(align(4)) CApp {
public:
	int LoadSettings(char* szPath)
	{
		// no-op, TODO
		return 0;
	}

	HINSTANCE m_hInstance;
	BYTE gap0[234];
	DWORD dwFlags;
	BYTE gap1[84];
	DWORD dwFullScreen;
};

class __declspec(align(4)) CTexture {
  BYTE gap0[4];
  DWORD dword4;
  int nTextureID;
  signed int uWidth;
  signed int uHeight;
  DWORD format;
  DWORD dword18;
  DWORD dword1C;
  DWORD dword20;
  DWORD dword24;
};

class Class_424127 {
public:
	DWORD dwMagic0;
	float floatMagic0;
	DWORD dwMagic2;
};

class __declspec(align(4)) CTouhou {
public:
	BYTE span0[2048][56];
	Class_424127 gameState;
	BYTE span1[1316];
	PBYTE specialTextures[256];
	int *pMagic0;
	BYTE span2[16384];
	CTexture *textures[480];
	int dwMagic0;
	int *pMagic1;
	BYTE byteMagic0;
	BYTE byteMagic1;
	BYTE byteMagic2;
	BYTE byteMagic3;
	BYTE span3[4];
	int *pMagic2;
	int nCount;
	BYTE span4[76];
	int dwMagic2;
	BYTE span5[16];
};

class __declspec(align(4)) CLog
{
public:
	LPTSTR Push(LPTSTR message, ...)
	{
		size_t len;
		TCHAR chTest;
		LPTSTR pStart;
		LPTSTR pDst;
		TCHAR pSrc[512];
		va_list va;

		va_start(va, message);
		/* Wtf is this? VA compiler optimization probably*/
		/* v8 = retaddr ^ dword_47A630; */
		vsprintf(pSrc, message, va);
		len = _tcslen(pSrc);

		if (start[len] < buf[2047])
		{
			pDst = pSrc;
			pStart = start;

			do {
				chTest = *pDst;
				*pStart++ = *pDst++;
			}
			while (chTest);
			start += len;
			*start = 0;
		}
		bPushed = 1;

		return message;
	}

	LPTSTR PushSilent(LPTSTR message, ...)
	{
		size_t len;
		TCHAR chTest;
		LPTSTR pStart;
		LPTSTR pDst;
		TCHAR pSrc[512];
		va_list va;

		va_start(va, message);
		_vstprintf(pSrc, message, va);
		len = _tcslen(pSrc);
		if (&start[len] < &buf[2047])
		{
			pDst = pSrc;
			pStart = start;

			do {
				chTest = *pDst;
				*pStart++ = *pDst++;
			}
			while (chTest);
			start += len;
			*start = 0;
		}

		return message;
	}

	CLog *Dump()
	{
		if (start != &buf[0])
		{
			FILE *pFile;
			PushSilent(_T("---------------------------------------------------------- \n"));
			if (bPushed)
				MessageBox(NULL, buf, _T("log"), MB_ICONERROR);

			pFile = fopen("./log.txt", "wt");
			_fputts(buf, pFile);
			return (CLog *)fclose(pFile);
		}
		return this;
	}

	TCHAR buf[2048];
	TCHAR *start;
	bool bPushed;
};

CTouhou *g_touhou;
IDirect3DDevice8 *g_pD3DDevice;
IDirect3D8 *g_pD3D;
HANDLE g_hAppMutex;
CLog g_log;
CApp g_app;
CGameWindow g_window;
BOOL g_bScreensaverActive;
BOOL g_bLowPowerActive;
BOOL g_bPowerOffActive;
BOOL g_bFullScreen;
HWND g_hWnd;

int CheckInstance()
{
	g_hAppMutex = CreateMutex(0, 1, _T("Touhou Koumakyou App"));
	if (!g_hAppMutex)
		return -1;
	if (GetLastError() != ERROR_ALREADY_EXISTS)
		return 0;

	g_log.Push(_T("二つは起動できません\n"));
	return -1;
}

int Direct3DInitialize()
{
	g_pD3D = (IDirect3D8 *)Direct3DCreate8(120); // Version
	if (g_pD3D)
		return 0;

	g_log.Push(_T("Direct3D オブジェクトは何故か作成出来なかった\n"));
	return 1;
}

WORD InitializeJoystick()
{
	WORD result;
	JOYINFOEX jiex = {0};

	jiex.dwSize = sizeof(JOYINFOEX);
	jiex.dwFlags = 255; // Wtf is this
	if (joyGetPosEx(0, &jiex))
	{
		g_log.PushSilent(_T("使えるパッドが存在しないようです、残念\n"));
		result = 1;
	}
	else {
		
		// joyGetDevCaps(0, &jiex, 0x194u); // Wtf is this
		result = 0;
	}

	return result;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message > WM_SETCURSOR)
	{
		/*
		if (message == MM_MOM_DONE && dword_6C6EC8)
			sub_422560((void *)dword_6C6EC8, (LPIDIHDR)lParam);
			*/

		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	if (message != WM_SETCURSOR)
	{
		switch (message)
		{
		case WM_CLOSE:
			g_window.bClosing = TRUE;
			return 1;
		case WM_ACTIVATEAPP:
			g_window.bActive = wParam;
			g_window.bUnfocused = wParam == 0;
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	if (g_bFullScreen || g_window.bUnfocused)
	{
		HCURSOR hCursor;

		hCursor = LoadCursor(0, IDC_ARROW);
		SetCursor(hCursor);
		ShowCursor(TRUE);
	}
	else {
		ShowCursor(FALSE);
		SetCursor(NULL);
	}

	return 1;
}

HWND InitializeWindow(HINSTANCE hInstance)
{
	HWND hWnd;
	WNDCLASS wc;
	int nWidth;
	int nHeight;

	ZeroMemory(&wc, sizeof(WNDCLASS));
	wc.hbrBackground = (HBRUSH)GetStockObject(0);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	
	g_window.bActive = FALSE;
	g_window.bUnfocused = FALSE;
	wc.lpszClassName = _T("BASE");
	RegisterClass(&wc);
	if (g_bFullScreen)
	{
		int v2;
		nWidth = 2 * GetSystemMetrics(SM_CXDLGFRAME) + 640;
		v2 = GetSystemMetrics(SM_CYDLGFRAME);
		nHeight = GetSystemMetrics(SM_CYCAPTION) + 2 * v2 + 480;
		hWnd = CreateWindowEx(
			0, _T("BASE"), _T("Touhou koumakyou ~ the Embodiment of Scarlet Devil"),
			0x100A0000u,
			CW_USEDEFAULT,
			0,
			nWidth,
			nHeight,
			NULL,
			NULL,
			hInstance,
			NULL);
	}
	else {
		nWidth = 640;
		nHeight = 480;
		hWnd = CreateWindowEx(
			0, _T("BASE"), _T("Touhou koumakyou ~ the Embodiment of Scarlet Devil"),
			WS_OVERLAPPEDWINDOW,
			0,
			0,
			640,
			480,
			NULL, NULL, hInstance, NULL);
	}
	g_window.m_hWnd = hWnd;
	g_hWnd = hWnd;
	return hWnd;
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	MSG msg = {0};

	if (CheckInstance() ||
		g_app.LoadSettings(_T("th06e.cfg")))
	{
		g_log.Dump();
		return -1;
	}
	else
	{
		if (Direct3DInitialize())
		{
			g_log.Dump();
			return 1;
		}

		SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, &g_bScreensaverActive, 0);
		SystemParametersInfo(SPI_GETLOWPOWERACTIVE, 0, &g_bLowPowerActive, 0);
		SystemParametersInfo(SPI_GETPOWEROFFACTIVE, 0, &g_bPowerOffActive, 0);
		SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, 0, 0, SPIF_SENDCHANGE);
		SystemParametersInfo(SPI_SETLOWPOWERACTIVE, 0, 0, SPIF_SENDCHANGE);
		SystemParametersInfo(SPI_SETPOWEROFFACTIVE, 0, 0, SPIF_SENDCHANGE);

		while (TRUE)
		{
			InitializeWindow(hInstance);
			if (InitializeScene())
			{
				g_log.Dump();
				return 1;
			}

			// Class_430510::Create(&unk_6D3F50, g_window.m_hWnd);
			InitializeJoystick();
			ReleaseKeyboardKeys();
			
			CTouhou* touhou = new CTouhou();
			g_touhou = touhou;

			if (!sub_423868())
			{
				if (!g_bFullScreen)
				{
					ShowCursor(0);
				}
				g_window.byte0 = 0;
				while(!g_window.bClosing)
				{
					if (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
					else {
						HRESULT hResult;
						hResult = g_pD3DDevice->TestCooperativeLevel();
					
						// SUCCEEDED() possibly...
						if (hResult)
						{
							if (hResult == D3DERR_DEVICENOTRESET)
							{
								g_touhou->FreeObjects();
								if (g_pD3DDevice->Reset())
									break;

								sub_421420();
								dword_6C6EB0 = 3;
							}
						}
						else {
							if (g_window->Tick)
							break;
						}
					}
				}
			}
			g_treeLike->sub41CD10();
			unk_6D3F50->sub430510();

			if (g_touhou)
			{
				delete g_touhou;
			}
			g_touhou = NULL;

			if (g_pD3DDevice)
			{
				g_pD3DDevice->Release();
				g_pD3DDevice = NULL;
			}

			ShowWindow(g_window.m_hWnd, SW_HIDE);
			MoveWindow(g_window.m_hWnd, 0, 0, 0, 0, FALSE);
			DestroyWindow(g_window.m_hWnd);

			if (v10 != 2)
				break;

			g_log.start = &g_log;
			g_log.buf[0] = 0;
			g_log.PushSilent(_T("再起動を要するオプションが変更されたので再起動します\n");
			if (!g_bFullScreen)
				ShowCursor(TRUE);
		}
		WriteDataToFile(_T("th06e.cfg", &g_settings, 0x38);
		SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, g_bScreensaveActive, 0, SPIF_SENDCHANGE);
		SystemParametersInfo(SPI_SETLOWPOWERACTIVE, g_bLowPowerActive, 0, SPIF_SENDCHANGE);
		SystemParametersInof(SPI_SETPOWEROFFACTIVE, g_bPoserOffActive, 0, SPIF_SENDCHANGE);

		if (g_pD3D)
		{
			g_pD3D->Release();
			g_pD3D = NULL;
		}
		ShowCursor(1);
		g_log.Dump();
		return 0;
	}
	return 0;
}