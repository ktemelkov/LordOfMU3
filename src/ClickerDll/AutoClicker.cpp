// AutoClicker.cpp : Implementation of DLL Exports.


#include "stdafx.h"
#include "resource.h"
#include "AutoClicker.h"
#include "MuWindow.h"
#include "MuUtil.h"
#include "..\_Shared\LoaderInterface.h"
#include "Gui.h"
#include "event/MouseEvent.h"
#include "UI/layout/BorderLayout.h"
#include "UI/component/Panel.h"
#include "UI/component/Button.h"
#include "UI/component/Dialog.h"
#include "UI/component/ScrollPane.h"
#include "BitmapFontFactory.h"

TCHAR g_szRoot[_MAX_PATH + 1] = {0};

CAutoClickerModule _AtlModule;

typedef BOOL (WINAPI* wglSwapBuffersPtr)(HDC hdc);

using namespace ui;
using namespace ui::layout;
using namespace ui::event;


/**  
 * \brief 
 */
struct 
{
	wglSwapBuffersPtr wglSwapBuffers;
} gPatchAddr = {0};




ui::TitleBar* pTitleBar;

class CMyGuiFrame : public ui::Frame
{
public:
	std::string myTitle;

	CMyGuiFrame(int x, int y, int width, int height)
			: ui::Frame(x,y,width,height)
	{
		myTitle = "Test Window";
		pTitleBar = new ui::TitleBar(myTitle);

		pTitleBar->setTitle(myTitle);
		pTitleBar->setTransparency(1.0f);

		setTitleBar(pTitleBar);
		this->setEnabled(true);


		button1 = new Button("Button 1");
		button2 = new Button("Button 2");
		button3 = new Button("Button 3");
		button4 = new Button("Button 4");
		button5 = new Button("Button 5");

		contentPane = new Panel(&layout);
		contentPane->add(button1);
		contentPane->add(button2);
		contentPane->add(button3);
		contentPane->add(button4);
		contentPane->add(button5);

		scrollPane = new ScrollPane(contentPane);
		setContentPane(scrollPane);
		pack();
	}


	virtual ~CMyGuiFrame()
	{
		delete button1;
		delete button2;
		delete button3;
		delete button4;
		delete button5;

		delete contentPane;
		delete scrollPane;
	}


private:
	ui::Panel *contentPane;
	ui::ScrollPane *scrollPane;
	ui::layout::FlowLayout layout;

	ui::Button *button1;
	ui::Button *button2;
	ui::Button *button3;
	ui::Button *button4;
	ui::Button *button5;
};


DWORD gdwLastTickCount = 0;
bool gfInitialized = false;
ui::Gui* gpGui = 0;
USHORT gsLastLBtnState = 0x0000;


/**  
 * \brief 
 */
void GuiInitialize()
{
	gdwLastTickCount = GetTickCount();

	gpGui = new ui::Gui();
	BitmapFontFactory* bFactory = new BitmapFontFactory();

	gpGui->setFontFactory(bFactory);

	CMyGuiFrame* frame = new CMyGuiFrame(0, 0, 800, 600);
	gpGui->addFrame(frame);

	frame->setTransparency(0.8f);
	frame->setEnabled(true);
	frame->setFocus(true);
	frame->show();
}



/**  
 * \brief 
 */
BOOL WINAPI MyWglSwapBuffers(HDC hDC)
{
	return gPatchAddr.wglSwapBuffers(hDC);
}


/*
BOOL WINAPI MyWglSwapBuffers(HDC hDC)
{
	if (!gfInitialized)
	{
		gfInitialized = true;
		GuiInitialize();
	}

//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//	glLoadIdentity();


	DWORD thisFrameIndex = GetTickCount();
	float deltaTime = ((float)(thisFrameIndex-gdwLastTickCount))/1000.0f;

	gdwLastTickCount = thisFrameIndex;


	HWND hMuWnd = WindowFromDC(hDC);


	POINT ptCursor;
	GetCursorPos(&ptCursor);
//	ScreenToClient(hMuWnd, &ptCursor);

	RECT rc;
	GetWindowRect(hMuWnd, &rc);

	char szBuff[256];
	sprintf(szBuff, "x: %d, y: %d, left: %d, top: %d", ptCursor.x, ptCursor.y, rc.left, rc.top);

	gpGui->importMouseMotion(ptCursor.x - rc.left, 600 - (ptCursor.y - rc.top));

pTitleBar->setTitle(szBuff);

	USHORT usLBtn = (USHORT)GetAsyncKeyState(VK_LBUTTON);

	if ((usLBtn & 0x8000) != gsLastLBtnState)
	{
		if ((usLBtn & 0x8000) != 0)
		{
			gpGui->importMousePressed(1);
		}
		else
			gpGui->importMouseReleased(1);

		gsLastLBtnState = usLBtn;
	}

	gpGui->importUpdate(deltaTime);


	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glViewport(0, 0, 800, 600);

	glLoadIdentity();
	glOrtho(0, 800, 600, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	GLboolean fTex = glIsEnabled(GL_TEXTURE_2D);
	GLboolean fDepth = glIsEnabled(GL_DEPTH_TEST);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);


	// render ui
	gpGui->paint();
	//frame->paint();

	if (fDepth)
		glEnable(GL_DEPTH_TEST);

	if (fTex)
		glEnable(GL_TEXTURE_2D);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	return gPatchAddr.wglSwapBuffers(hDC);
}

*/

/**
 * \brief 
 */
BOOL CAutoClickerModule::InitClicker()
{
	if (m_hMuWindow != 0)
		return TRUE;

	int iInstanceNumber = 0;
	HWND hMuWindow = 0;

	if (m_pLoader)
		m_pLoader->SendCommand(_MODULE_LOADER_COMMAND_GET_MUWND, _MODULE_LOADER_TARGET_SELF, (void*)&hMuWindow, (void**)&iInstanceNumber);

	if (0 == hMuWindow)
		return FALSE;

	m_hMuWindow = hMuWindow;
	m_dwMainThreadId = GetWindowThreadProcessId(hMuWindow, 0);
	return CMuWindow::Init(m_hMuWindow, iInstanceNumber);
}


/**
 * \brief 
 */
void CAutoClickerModule::TermClicker()
{
	CMuWindow::Term();
}


/**  
 * \brief 
 */
bool CAutoClickerModule::SendCommand(int iCommand, void* pParam, void** pResult)
{
	if (_CLICKER_MODULE_COMMAND_SET_VERSION == iCommand)
	{
		ULONG ulGameVersion = *((ULONG*)pParam);

		if (m_hMuWindow)
			PostMessage(m_hMuWindow, WM_SET_GAME_VERSION, 0, (LPARAM)ulGameVersion);

		return true;
	}
	else if (_CLICKER_MODULE_COMMAND_REG_MU_WND == iCommand)
	{
		InitClicker();
	}
	else if (_CLICKER_MODULE_COMMAND_LOGIN == iCommand)
	{
		CMuWindow::DoLogin((const char*)pParam);
	}
	else if (_CLICKER_MODULE_COMMAND_SET_FEATURES == iCommand)
	{
		DWORD dwFeatures = *((ULONG*)pParam);
		CMuWindow::SetFeatures(dwFeatures);

		return true;
	}
	else if (_CLICKER_MODULE_COMMAND_CONNECT == iCommand)
	{
		CMuWindow::SetConnected();
	}
	
	return false;
}



/**
 * \brief DLL Entry Point
 */
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (!_AtlModule.DllMain(dwReason, 0))
		return FALSE;
	
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hInstance);

		LoaderInitStruct* pInit = (LoaderInitStruct*)lpReserved;

		if (pInit)
		{
			char szRoot[_MAX_PATH+1] = {0};
			pInit->pLoader->GetRootDir(szRoot, _MAX_PATH);

			_tcscpy_s(g_szRoot, _MAX_PATH, CA2CT(szRoot));

			_AtlModule.m_pLoader = pInit->pLoader;
			_AtlModule.m_dwCookie = pInit->dwCookie;

			if (pInit->ppMoudle)
				*pInit->ppMoudle = (IClickerModule*)&_AtlModule;

			if (_AtlModule.m_pLoader)
			{
				InstallerServiceTable* pServices = 0;
				_AtlModule.m_pLoader->SendCommand(_MODULE_LOADER_COMMAND_GET_INSTALLER_SERVICES, _MODULE_LOADER_TARGET_SELF, 0, (void**)&pServices);

				if (pServices)
				{
					pServices->SetApiHook(_API_HOOK_WGLSWAPBUFFERS, (PVOID)&MyWglSwapBuffers, (PVOID*)&gPatchAddr.wglSwapBuffers);
				}
			}
		}
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		if (_AtlModule.m_pLoader)
		{
			InstallerServiceTable* pServices = 0;
			_AtlModule.m_pLoader->SendCommand(_MODULE_LOADER_COMMAND_GET_INSTALLER_SERVICES, _MODULE_LOADER_TARGET_SELF, 0, (void**)&pServices);

			if (pServices)
			{
				pServices->ClearApiHook(_API_HOOK_WGLSWAPBUFFERS);
			}
		}

		_AtlModule.TermClicker();
	}

    return TRUE; 
}
