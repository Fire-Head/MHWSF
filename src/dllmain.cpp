#include <Windows.h>
#include "CPatch.h"
#include <stdio.h>
#include <cmath>

#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

//-------------------------------------------------------------------------------------------------------

class CVector2D
{
public:
	float x, y;
	CVector2D(float x, float y) : x(x), y(y) {  }
};

struct RwCamera;
struct RwRaster;

#define RwRasterGetWidth(r)  (*(int *)(DWORD(r) + 12))
#define RwRasterGetHeight(r) (*(int *)(DWORD(r) + 16))

struct SCREEN
{
	float     fWidth;
	float     fHeight;
	float     fInvWidth;
	float     fInvHeight;
	float     fWidthScale;
	float     fHeightScale;
	float     HudStretch;
	int       nDepth;
	RwCamera *pCamera;
	RwRaster *pFrameBuffer;
	RwRaster *pZBuffer;
char _unk0[24];
};

struct IMMEDIATEMODE
{
char _unk0[8];
	float     fAngle;
char _unk1[74];
	float     fX1;
	float     fY1;
	float     fX2;
	float     fY2;
	float     fX3;
	float     fY3;
	float     fX4;
	float     fY4;
	CVector2D vCenteredPos;
char _unk2[20];
	CVector2D vHalfScale;
char _unk3[32];
	float     fScreenWidth;
	float     fScreenHeight;
char _unk4[4];
};

class CRenderer
{
public:
	static IMMEDIATEMODE &ms_im;

	static void RenderStateSetBlendDefault()
	{
		((void (__cdecl *)())0x5F5CC0)();
	}

	static void DrawQuad2dBegin(RwRaster *raster)
	{
		((void (__cdecl *)(RwRaster *))0x5F83F0)(raster);
	}

	static void DrawQuad2dSet(float posx, float posy, float scalex, float scaley, int R, int G, int B, int A)
	{
		((void (__cdecl *)(float, float, float, float, int, int, int, int))0x5F8470)(posx, posy, scalex, scaley, R, G, B, A);
	}

	static void DrawQuad2dEnd()
	{
		((void (__cdecl *)())0x5F96A0)();
	}

	static void DrawQuad2d(float posx, float posy, float scalex, float scaley, long R, long G, long B, long A, RwRaster *raster)
	{
		((void (__cdecl *)(float, float, float, float, long, long, long, long, RwRaster *))0x5F96F0)(posx, posy, scalex, scaley, R, G, B, A, raster);
	}
};

IMMEDIATEMODE &CRenderer::ms_im = *(IMMEDIATEMODE*)0x7D35A4;

class CFrontend
{
public:
	static SCREEN &ms_scrn;

	static void Print16_(const wchar_t *text, float x, float y, float scalex, float scaley, float a7, int font)
	{
		((void (__cdecl *)(const wchar_t *, float, float, float, float, float, int))0x5E5980)(text, x, y, scalex, scaley, a7, font);
	}
};

SCREEN &CFrontend::ms_scrn = *(SCREEN*)0x7D3440;

class CFrontendMenu
{
public:
	static void SetPrintColor(int r, int g, int b, int a)
	{
		((void (__cdecl *)(int, int, int, int))0x5D7AA0)(r, g, b, a);
	}
};

class CScene
{
public:
	static CVector2D &ms_viewWinScale;
	static CVector2D &m_viewWindowOriginal;
};

CVector2D &CScene::ms_viewWinScale      = *(CVector2D*)0x715CDC;
CVector2D &CScene::m_viewWindowOriginal = *(CVector2D*)0x715C98;

class CCamera
{
public:
	static float &m_aspectRatio;
	static float &m_viewWindow;
};

float &CCamera::m_aspectRatio = *(float *)0x7A164C;
float &CCamera::m_viewWindow  = *(float *)0x7A1650;

//-------------------------------------------------------------------------------------------------------

#define PI float(3.1415927)
#define DEGTORAD(x) ((x) * PI / 180.0f)
#define RADTODEG(x) ((x) * 180.0f / PI)

#define DEFAULT_SCREEN_WIDTH  ((float)(640))
#define DEFAULT_SCREEN_HEIGHT ((float)(480))
#define DEFAULT_ASPECT_RATIO  (4.0f/3.0f)
#define DEFAULT_VIEWWINDOW    (0.7f)
#define SCREEN_WIDTH          (CFrontend::ms_scrn.fWidth)
#define SCREEN_HEIGHT         (CFrontend::ms_scrn.fHeight)
#define SCREEN_ASPECT_RATIO   GetScreenAspectRatio()
#define SCREEN_VIEWWINDOW     (tan(DEGTORAD(70.0f * 0.5f)))

#define SCREEN_SCALE_AR(a)    ((a) * DEFAULT_ASPECT_RATIO / SCREEN_ASPECT_RATIO)
#define SCREEN_STRETCH_X(a)   ((a) * CFrontend::ms_scrn.fWidthScale)
#define SCREEN_STRETCH_Y(a)   ((a) * CFrontend::ms_scrn.fHeightScale)
#define SCREEN_SCALE_X(a)     SCREEN_SCALE_AR(SCREEN_STRETCH_X(a))
#define SCREEN_SCALE_Y(a)     SCREEN_STRETCH_Y(a)

//

#define SCLX(x) ( SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH * (x)) / SCREEN_WIDTH )

#define SCREEN_WIDTH_4_3 SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH)

#define FC(x)       ( (SCREEN_WIDTH/2 - SCREEN_WIDTH_4_3/2) + SCREEN_SCALE_X((x)) )
#define FR(x)       ( (SCREEN_WIDTH   - SCREEN_WIDTH_4_3  ) + SCREEN_SCALE_X((x)) )
#define FL(x)       (                                         SCREEN_SCALE_X((x)) )
#define FRIGHT(x)   ( FR(DEFAULT_SCREEN_WIDTH * (x)) / SCREEN_WIDTH )
#define FLEFT(x)    ( FL(DEFAULT_SCREEN_WIDTH * (x)) / SCREEN_WIDTH )
#define FCENTER(x)  ( FC(DEFAULT_SCREEN_WIDTH * (x)) / SCREEN_WIDTH )


//

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

class CConfig
{
	static char m_szPath[MAX_PATH];

	static int GetInt(char *section, char *key, int defaultValue)
	{
		return GetPrivateProfileInt(section,  key, defaultValue, m_szPath);
	}
	static void SetInt(char *section, char *key, int value)
	{
		char temp[32];
		_snprintf_s(temp, sizeof(temp), "%d", value);
		WritePrivateProfileString(section, key, temp, m_szPath);
	}
public:

	static int   bWideScreen;
	static int   bPS2Subs;
	static int   bXboxHud;
	static int   bLockedCursor;
	static int   bForceEquilateralLockOnCrosshair;

	static void Read()
	{
		GetModuleFileName((HINSTANCE)&__ImageBase, m_szPath, MAX_PATH);
		PathRenameExtension(m_szPath, ".ini");

		bWideScreen                      = GetInt  ("Main", "bWideScreen", bWideScreen != 0) != 0;
		bPS2Subs                         = GetInt  ("Main", "bPS2Subs", bPS2Subs != 0) != 0;
		bXboxHud                         = GetInt  ("Main", "bXboxHud", bXboxHud != 0) != 0;
		bLockedCursor                    = GetInt  ("Main", "bLockedCursor", bLockedCursor != 0) != 0;
		bForceEquilateralLockOnCrosshair = GetInt  ("Main", "bForceEquilateralLockOnCrosshair", bForceEquilateralLockOnCrosshair != 0) !=0;
	}

	static void Write()
	{
		GetModuleFileName((HINSTANCE)&__ImageBase, m_szPath, MAX_PATH);
		PathRenameExtension(m_szPath, ".ini");

		SetInt  ("Main", "bWideScreen", bWideScreen != 0);
		SetInt  ("Main", "bPS2Subs", bPS2Subs != 0);
		SetInt  ("Main", "bXboxHud", bXboxHud != 0);
		SetInt  ("Main", "bLockedCursor", bLockedCursor != 0);
		//SetInt  ("Main", "bForceEquilateralLockOnCrosshair", bForceEquilateralLockOnCrosshair != 0);
	}
};

char  CConfig::m_szPath[MAX_PATH];

int   CConfig::bWideScreen = 1;
int   CConfig::bPS2Subs = 0;
int   CConfig::bXboxHud = 0;
int   CConfig::bLockedCursor = 1;
int   CConfig::bForceEquilateralLockOnCrosshair = 1;


float GetScreenAspectRatio()
{
	return CConfig::bWideScreen ? float(SCREEN_WIDTH / SCREEN_HEIGHT) : float(DEFAULT_ASPECT_RATIO);
}


//-------------------------------------------------------------------------------------------------------

static float &_fOverlayX = *(float*)0x724840;
static float &_fOverlayY = *(float*)0x72483C;
static float &fSubX = *(float*)0x7E9D54;
static float &fSubBoxW = *(float*)0x7E9D40;
static float &_fTIBoxY = *(float*)0x7A1494;
static float _fTITextY;
static float &_fRecIconX = *(float*)0x7A14AC;
static float _fRecTextX;
static float &_fTCRX = *(float*)0x7A1498;
static float &_TCRW1 = *(float*)0x7A148C;
static float _TCRW2;
static float &_fMENUCNTTEXT1X = *(float*)0x7C9078;
static float &_fMENUCNTTEXT2X = *(float*)0x7C907C;
static float &_fMENUCNTTEXT3X = *(float*)0x7C9080;
static float _fMENUCNTTEXT4X;
static float _fMENUCNTTEXT5X;
static float _fMENUCNTTEXT6X;
static float &_fMENUCNTX = *(float*)0x7C9074;
static float &_fMENUCNTSCLX = *(float*)0x7C906C;
static float &fVideoFxTC1 = *(float*)0x7CEDD0;
static float fVideoFxTC2Min;
static float fVideoFxTC2MaxMinusMin;
static float &fVideoFxSize = *(float*)0x7CEDD4;
static float fVideoFxSizeHalf;
static float fFuzzStepX;
static float fConsoleSCLX;
static float fConsoleSCLY;
static float fFrameCounterX;
static float fShadowCameraX;
static float fMSX;
static float fRSSETX;
static float fRSGETX;
static float fMEMUSEDX;
static float fDebugX;
static float fDebugW;
static float fDebugTextX1;
static float fDebugTextX2;
static float fDebugTextX3;
static float fDebugTextX4;
static float &fManX = *(float*)0x7CE254;
static float &fManY = *(float*)0x7CE250;
static float fHudBaseX;
static float fHudBaseY;
static float &fBarOffX = *(float*)0x7CE258;
static float fOffX;
static float &fDamageDirX = *(float*)0x7C96C4;
static float &fDamageDir2X = *(float*)0x7C96D0;
static float fDamageMidY;
static float &fWeaponSwapBarX = *(float*)0x7C9DB0;
static float fWeaponBaseY;
static float &fWeaponIconX = *(float*)0x7C9DB8;
static float &fWeaponIconY = *(float*)0x7C9DB4;
static float &fWeaponSwapWeapon2X = *(float*)0x7C9DBC;
static float &fWeaponSwapIconX = *(float*)0x7C9DCC;
static float &fWeaponSwapIconY = *(float*)0x7C9DC8;
static float &fWeponTextOffX = *(float*)0x7C9DEC;
static float &fWeponTextOff2X = *(float*)0x7C9E04;
static float &fWeponTextX = *(float*)0x7C9DE8;
static float &fWeponText2X = *(float*)0x7C9DF0;
static float &fWeaponSwapItem2Y = *(float*)0x7C9DFC;
static float fInvConOffX;
static float _fTimerX;
static float &_fMapPosX = *(float*)0x7CA56C;
static float &_fMapPosY = *(float*)0x7CA578;
static float &_fMapPos3Y = *(float*)0x7CA580;
static float _fMapPos2X;
static float _fMapPos2Y;
static float &fTextBoxX = *(float*)0x7CE280;
static float &fTextBoxW = *(float*)0x7CE27C;
static float fTextBoxOffX;
static float fTextBoxOffX2;
static float fTextBoxOffX3;
static float fTextBoxOffX4;
static float fPrintInfoX1;
static float fPrintInfoX2;
static float fPrintInfoPadSizeX1;
static float fPrintInfoPadSizeX2;
static float &fLevelCompX1 = *(float *)0x7C9848;
static float fLevelCompX2;
static float fLevelCompX3;
static float &fMemcardSaveX1 = *(float *)0x7C9704;
static float &fMemcardSaveX2 = *(float *)0x7C98AC;
static float &fMemcardSaveX3 = *(float *)0x7C9910;
static float &fMemcardSaveX4 = *(float *)0x7C9700;
static float &fMemcardSaveX5 = *(float *)0x7C96CC;
static float &fOptionX = *(float*)0x7C8718;
static float &fOption2X = *(float*)0x7D6410;
static float &fLOGOX = *(float*)0x7D6408;
static float &fSelectionBoxX = *(float*)0x7C872C;
static float fMHLogo;
static float &fCasetteX = *(float*)0x7CE13C;
static float &fInfoTextX1 = *(float*)0x7C90A0;
static float &fInfoTextX2 = *(float*)0x7C90A4;
static float &fSliderW = *(float *)0x7C90B0;
static float &fSlider2W = *(float *)0x7C90C0;
static float &fLoadGameX = *(float*)0x7D628C;
static float &fLoadGame2X = *(float*)0x7D6294;
static float &fLoadGameW = *(float*)0x7D6290;
static float &fSaveGameX = *(float*)0x7D649C;
static float &fSaveGame2X = *(float*)0x7D64A4;
static float &fSaveGameW = *(float*)0x7D64A0;
static float &fSlotGame1X = *(float*)0x7C95A4;
static float &fSlotGame2X = *(float*)0x7C95A8;
static float &fSlotGame3X = *(float*)0x7C95AC;
static float &fSceneSelX = *(float*)0x7D65B8;
static float fSceneSel1X;
static float &fSceneSelPageX = *(float *)0x7D65D0;
static float &fSceneSelPage1X = *(float *)0x7D6554;
static float &fSceneSelPage2X = *(float *)0x7D6558;
static float &fSceneSelPage3X = *(float *)0x7D655C;
static float &fSceneSelPage4X = *(float *)0x7D6560;
static float &fSceneSelPage5X = *(float *)0x7D6564;
static float &fSceneSelPage1W = *(float *)0x7D65D8;
static float &fSceneSelPage2W = *(float *)0x7D65DC;
static float &fSceneSelPage3W = *(float *)0x7D65E0;
static float &fSceneSelPage4W = *(float *)0x7D65E4;
static float &fSceneSelPage5W = *(float *)0x7D65E8;
static float fAudioX;
static float &fAudio2X = *(float *)0x7D67AC;
static float &fAudio3X = *(float *)0x7D67B0;
static float &fAudio4X = *(float *)0x7D67BC;
static float &fVideo1X = *(float *)0x7D68BC;
static float &fVideo2X = *(float *)0x7D68C0;
static float &fBinusSelX = *(float*)0x7D6A70;
static float fBonusSel1X;
static float &fBonusSelPage1X = *(float *)0x7D6A1C;
static float &fBonusSelPage2X = *(float *)0x7D6A20;
static float &fBonusSelPage3X = *(float *)0x7D6A24;
static float &fBonusSelPage4X = *(float *)0x7D6A28;
static float &fBonusSelPage5X = *(float *)0x7D6A2C;
static float &fBonusSelPage1W = *(float *)0x7D6A80;
static float &fBonusSelPage2W = *(float *)0x7D6A84;
static float &fBonusSelPage3W = *(float *)0x7D6A88;
static float &fBonusSelPage4W = *(float *)0x7D6A8C;
static float &fTestPicX = *(float*)0x7D69A0;
static float &fTestPic2X = *(float*)0x7D69B8;
static float &fTestPicTextX = *(float*)0x7D69BC;
static float &fControlsX = *(float*)0x7D5F64;
static float &fControls2X = *(float*)0x7D5F68;
static float &fDefOffX = *(float*)0x7D6678;
static float &fQuitOffX = *(float*)0x7E9CD0;
static float &fRemap1X = *(float*)0x7E99AC;
static float &fRemap2X = *(float*)0x7E99B0;
static float &fRemap3X = *(float*)0x7E99B4;
static float &fRemap4X = *(float*)0x7E99B8;
static float fRemap5X;
static float &fRemap6X = *(float*)0x7E99BC;
static float &fRemap7X = *(float*)0x7E99C0;
static float &fRemap8X = *(float*)0x7E99CC;
static float fRemap9X;
static float fGoalW;

//-------------------------------------------------------------------------------------------------------

void Calc2D();

int __cdecl SCGetAspectRatio()
{
	return CConfig::bWideScreen != 0;
}

void __cdecl SetViewWindowOriginal()
{
	CScene::m_viewWindowOriginal.x = CCamera::m_viewWindow;
    CScene::m_viewWindowOriginal.y = CCamera::m_viewWindow / CCamera::m_aspectRatio;
}

void __cdecl SetViewWindowDefault()
{
	Calc2D();

	CScene::ms_viewWinScale.x = 0.0f;
	CScene::ms_viewWinScale.y = 0.0f;
}

void __cdecl SetViewWindowWidescreen()
{
	Calc2D();

	CVector2D orig(CCamera::m_viewWindow, CCamera::m_viewWindow / CCamera::m_aspectRatio);

	CScene::ms_viewWinScale.x = (orig.x*(SCREEN_ASPECT_RATIO / DEFAULT_ASPECT_RATIO))-orig.x;
	CScene::ms_viewWinScale.y = (orig.y)-orig.y;
}

static float fSCL1;

int __cdecl CeilScaleHor(int x)
{
	return (int)ceil(1.0f/(fSCL1*(1.0f/float(x))));
}

int __cdecl CeilToPow2(int x)
{
	return (int)pow(2, ceil(log(float(x)) / log(2.0f)));
}

float __cdecl ScaleX(float x, float w)
{
	float left  = 1.0f/3.0f;
	float right = 1.0f-(1.0f/3.0f);
	float cpos  = x + w/2.0f;

	if      ( cpos < left )                 return FLEFT(x);
	else if ( cpos > left && cpos < right ) return FCENTER(x);
	else /*if ( cpos > right )*/            return FRIGHT(x);
}

void __cdecl Calc2D()
{
	fSCL1 = SCLX(1.0f);

	float stretch = (CFrontend::ms_scrn.fHeight / CFrontend::ms_scrn.fWidth) / (CFrontend::ms_scrn.fHeightScale / CFrontend::ms_scrn.fWidthScale);
	CFrontend::ms_scrn.HudStretch = SCREEN_SCALE_AR(stretch);

	//-------------------------------------------------------------------------------------------------------

	#define XBOXT(x)  (CConfig::bXboxHud ? ((x) - 0.037f) : (x))
	#define XBOXX(x)  (CConfig::bXboxHud ? ((x) - 0.043f) : (x))
	#define XBOXY(x)  (CConfig::bXboxHud ? ((x) - 0.052f) : (x))
	#define XBOXWX(x) (CConfig::bXboxHud ? ((x) - 0.05f)  : (x))
	#define XBOXWY(x) (CConfig::bXboxHud ? ((x) - 0.052f) : (x))
	#define XBOXRX(x) (CConfig::bXboxHud ? ((x) + 0.04f)  : (x))
	#define XBOXRY(x) (CConfig::bXboxHud ? ((x) - 0.044f) : (x))
	#define XBOXTI(x) (CConfig::bXboxHud ? ((x) - 0.84f)  : (x))

	// BinkFrame-> DrawSubtitles
	fSubX = FCENTER(0.029f);
	fSubBoxW = SCLX(0.987f);

	// CScene::Render __DrawTapeInfo
	_fTIBoxY = XBOXTI(0.89f);
	_fTITextY = XBOXTI(0.9f);
	_fRecIconX = FLEFT(0.06f);
	_fRecTextX = FLEFT(0.1f);
	_fTCRX = FCENTER(0.4f);
	_TCRW1 = SCLX(0.0075f);
	_TCRW2 = SCLX(0.02f);

	// DrawCameraCounter
	_fMENUCNTSCLX = SCLX(0.28f);
	_fMENUCNTX = 0.5f - _fMENUCNTSCLX / 2;
	_fMENUCNTTEXT1X = FCENTER(0.38f);
	_fMENUCNTTEXT2X = SCLX(0.046f);
	_fMENUCNTTEXT3X = SCLX(0.057f);
	_fMENUCNTTEXT4X = SCLX(0.28f);
	_fMENUCNTTEXT5X = FCENTER(0.38f);
	_fMENUCNTTEXT6X = FCENTER(0.368f);

	// CFrontend::DrawVideoFX
	{
		float maxSide = max(CFrontend::ms_scrn.fWidth, CFrontend::ms_scrn.fHeight);
		float minSide = min(CFrontend::ms_scrn.fWidth, CFrontend::ms_scrn.fHeight);

		fVideoFxSize = (maxSide/minSide)*1.2f;
		fVideoFxSizeHalf = fVideoFxSize/2;

		float sizeMul = fVideoFxSize/1.6f;

		fVideoFxTC1 = 11.0f * sizeMul;
		fVideoFxTC2Min = 10.0f * sizeMul;
		fVideoFxTC2MaxMinusMin = 8.0f * sizeMul;
	}

	// RenderFuzz
	fFuzzStepX = SCLX(0.0625f);

	// console
	fConsoleSCLX = 1.0f / DEFAULT_SCREEN_WIDTH * SCLX(1.0f);
	fConsoleSCLY = 1.0f / DEFAULT_SCREEN_HEIGHT;

	// CFrontend::Render2d

	// dbg
	fFrameCounterX  = FCENTER(0.488f);
	fShadowCameraX  = FRIGHT(0.61f);
	fMSX  = FLEFT(0.02f);
	fRSSETX = FLEFT(0.02f);
	fRSGETX = FLEFT(0.02f);
	fMEMUSEDX = FCENTER(0.3f);

	// debug menu
	fDebugX = FLEFT(0.032f);
	fDebugW = SCLX(0.4f);
	fDebugTextX1 = FLEFT(0.05f);
	fDebugTextX2 = FLEFT(0.28f);
	fDebugTextX3 = FLEFT(0.412f);
	fDebugTextX4 = FLEFT(0.395f);

	// hud
	fManX = FRIGHT(XBOXX(0.84f));
	fManY = XBOXY(0.768f);
	fHudBaseX = FRIGHT(XBOXX(0.929f));
	fHudBaseY = XBOXY(0.762f);
	fBarOffX = SCLX(0.015625f);
	fOffX = SCLX(0.01f);

	// CGameInfo::RenderDamageDirections
	fDamageDirX = FCENTER(0.65f-0.07f);
	fDamageDir2X = FCENTER(0.75f-0.07f);
	fDamageMidY = 0.5f - 0.4f * 0.5f;

	// CGameInventory::InventoryControl
	fWeaponSwapBarX = FRIGHT(XBOXWX(0.72157812f));
	fWeaponBaseY = XBOXWY(0.613f);
	fWeaponIconX = FRIGHT(XBOXWX(0.8805f));
	fWeaponIconY = XBOXWY(0.4488f);
	fWeaponSwapWeapon2X = FRIGHT(XBOXWX(0.75354689f));
	fWeaponSwapIconX = FRIGHT(XBOXWX(0.84339064f));
	fWeaponSwapIconY = XBOXWY(0.48395625f);
	fWeponTextOffX = SCLX(0.14f);
	fWeponTextOff2X = SCLX(-0.005f);
	fWeponTextX = FRIGHT(XBOXWX(0.829f));
	fWeponText2X = FRIGHT(XBOXWX(0.969f));
	fWeaponSwapItem2Y = XBOXWY(0.5988f);
	fInvConOffX = SCLX(0.01f);

	// CFrontend::TimerDraw
	_fTimerX = FRIGHT(XBOXT(0.956f));

	// CGameMap::UpdateAndRender
	_fMapPos2X = _fMapPosX = FLEFT(XBOXRX(0.04f));
	_fMapPos2Y = XBOXRY(0.721f);
	_fMapPosY = XBOXRY(0.8365f);
	_fMapPos3Y = XBOXRY(0.716f);

	// CFrontend::TextBoxDraw
	fTextBoxX = FCENTER(0.12f);
	fTextBoxW = SCLX(0.75f);
	fTextBoxOffX = SCLX(0.01f);
	fTextBoxOffX2 = SCLX(0.005f);
	fTextBoxOffX3 = SCLX(0.03f);
	fTextBoxOffX4 = SCLX(0.06f);

	// CFrontend::PrintInfoDraw
	fPrintInfoX1 = FLEFT(0.042f);
	fPrintInfoX2 = FRIGHT(0.954f);
	fPrintInfoPadSizeX1 = SCLX(0.01f);
	fPrintInfoPadSizeX2 = SCLX(0.02f);

	// CGameInfo::Render
	fLevelCompX1 = FCENTER(0.19f);
	fLevelCompX2 = FCENTER(0.8f);
	fLevelCompX3 = SCLX(0.02f);
	fMemcardSaveX1 = FCENTER(0.33f);
	fMemcardSaveX2 = FCENTER(0.31f);
	fMemcardSaveX3 = FCENTER(0.23f);
	fMemcardSaveX4 = FCENTER(0.1f);
	fMemcardSaveX5 = FCENTER(0.3f);

	// CFrontend::LoadProgressTexDictLoad
	fMHLogo = FCENTER(0.1f);

	// CFrontend::RenderProgressStep
	fCasetteX = FCENTER(0.46f);

	// CFrontendMenu::PrintInfo
	fInfoTextX1 = FCENTER(0.074f);
	fInfoTextX2 = FCENTER(0.926f);

	// CFrontendMenu__DrawSlider
	fSliderW = SCLX(0.3f);
	fSlider2W = SCLX(0.03f);

	// CFEP_Main
	fOptionX = FCENTER(0.15f);
	fOption2X = FCENTER(0.58f);
	fLOGOX = FCENTER(0.1f);
	fSelectionBoxX = SCLX(0.01f);

	// CFEP_LoadGame
	fLoadGameX = FCENTER(0.15f);
	fLoadGame2X = FCENTER(0.3f);
	fLoadGameW = SCLX(0.69f);

	// CFEP_SaveGame
	fSaveGameX = FCENTER(0.15f);
	fSaveGame2X = FCENTER(0.3f);
	fSaveGameW = SCLX(0.69f);

	// Slot
	fSlotGame1X = FCENTER(0.05f);
	fSlotGame2X = FCENTER(0.3f);
	fSlotGame3X = FCENTER(0.6f);

	// CFEP_SceneSelection
	fSceneSelX = FCENTER(0.8f);
	fSceneSel1X = FCENTER(0.2f);
	fSceneSelPageX = SCLX(0.027f);
	fSceneSelPage1X = FCENTER(0.48f);
	fSceneSelPage2X = FCENTER(0.436f);
	fSceneSelPage3X = FCENTER(0.389f);
	fSceneSelPage4X = FCENTER(0.343f);
	fSceneSelPage5X = FCENTER(0.294f);
	fSceneSelPage1W = SCLX(0.074f);
	fSceneSelPage2W = SCLX(0.08f);
	fSceneSelPage3W = SCLX(0.085f);
	fSceneSelPage4W = SCLX(0.09f);
	fSceneSelPage5W = SCLX(0.1f);

	// CFEP_SettingsAudio
	fAudioX = SCLX(0.08f);
	fAudio2X = SCLX(0.46f);
	fAudio3X = SCLX(0.0f);
	fAudio4X = SCLX(0.09f);

	// CFEP_SettingsVideo
	fVideo1X = SCLX(0.3f);
	fVideo2X = SCLX(0.09f);

	// CFEP_BonusFeatures
	fBinusSelX = FCENTER(0.8f);
	fBonusSel1X = FCENTER(0.2f);
	fBonusSelPage1X = FCENTER(0.48f);
	fBonusSelPage2X = FCENTER(0.436f);
	fBonusSelPage3X = FCENTER(0.389f);
	fBonusSelPage4X = FCENTER(0.343f);
	fBonusSelPage5X = FCENTER(0.294f);
	fBonusSelPage1W = SCLX(0.074f);
	fBonusSelPage2W = SCLX(0.08f);
	fBonusSelPage3W = SCLX(0.085f);
	fBonusSelPage4W = SCLX(0.09f);

	// CFEP_TestPicture
	fTestPicX = FCENTER(0.58f);
	fTestPic2X = FCENTER(0.774f);
	fTestPicTextX = FCENTER(0.1f);

	// CFEP_Controls
	fControlsX = SCLX(0.36f);
	fControls2X = SCLX(0.09f);

	// CFEP_SetDefaults
	fDefOffX = SCLX(0.2f);

	// CFEP_Quit
	fQuitOffX = SCLX(0.2f);

	// CFEP_RemapCtrl
	fRemap1X = FCENTER(0.025f);
	fRemap5X = SCLX(0.025f);
	fRemap2X = FCENTER(0.205f);
	fRemap3X = FCENTER(0.365f);
	fRemap4X = SCLX(0.4f);
	fRemap6X = SCLX(0.18f);
	fRemap7X = SCLX(0.34f);
	fRemap8X = FCENTER(0.6f);
	fRemap9X = FCENTER(0.4f);

	// CFEP_InGame, CFrontendMenu::LevelGoalsDraw
	fGoalW = SCLX(0.9f);


	#undef XBOXT
	#undef XBOXX
	#undef XBOXY
	#undef XBOXWX
	#undef XBOXWY
	#undef XBOXRX
	#undef XBOXRY
	#undef XBOXTI
}

void __declspec(naked) patch_Initialise()
{
	__asm
	{
		call    Calc2D

		mov     dword ptr ds :[7D2A98h], 0

		retn
	}
}

void __cdecl CRenderer_DrawQuad2d_RotFix()
{
	if ( !CConfig::bWideScreen ) return;

	IMMEDIATEMODE &im = CRenderer::ms_im;

	float cx = im.vCenteredPos.x * im.fScreenWidth;
	float cy = im.vCenteredPos.y * im.fScreenHeight;
	float hw = im.vHalfScale.x   * im.fScreenWidth;
	float hh = im.vHalfScale.y   * im.fScreenHeight;

	float angle = DEGTORAD(im.fAngle);
	float c = cosf(angle);
	float s = sinf(angle);

	float cw = c * hw;
	float ch = c * hh;
	float sw = s * hw;
	float sh = s * hh;

	im.fX1 = (cx + cw - sh) / im.fScreenWidth;
	im.fY1 = (cy + ch + sw) / im.fScreenHeight;

	im.fX2 = (cx - cw - sh) / im.fScreenWidth;
	im.fY2 = (cy + ch - sw) / im.fScreenHeight;

	im.fX3 = (cx - cw + sh) / im.fScreenWidth;
	im.fY3 = (cy - ch - sw) / im.fScreenHeight;

	im.fX4 = (cx + cw + sh) / im.fScreenWidth;
	im.fY4 = (cy - ch + sw) / im.fScreenHeight;
}

void __declspec(naked) patch_DrawQuad2dSet()
{
	__asm
	{
		call  CRenderer_DrawQuad2d_RotFix

		mov eax, [7D35FCh]
		fld [eax]

		mov eax, 5F8AA8h
		jmp eax
	}
}

void __declspec(naked) patch_DrawQuad2dSetAll()
{
	__asm
	{
		call  CRenderer_DrawQuad2d_RotFix

		mov eax, [7D35FCh]
		fld [eax]

		mov eax, 5F93C8h
		jmp eax
	}
}

void __cdecl CRenderer_DrawTri2d_FixNoFX(float x1, float y1, float x2, float y2, float x3, float y3, float u1, float v1, float u2, float v2, float u3, float v3, int r1, int g1, int b1, int a1, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3, RwRaster *raster)
{
	CRenderer::DrawQuad2d(x1, y1, 1.0f + 0.0f, 1.0f + ((y3-(y1+2.0f))/2.0f) + 0.0f, r1, g1, b1, a1, raster);
}

void __cdecl CRenderer_DrawQuad2d_Menu(float posx, float posy, float scalex, float scaley, long R, long G, long B, long A, RwRaster *raster)
{
	if ( raster != NULL )
	{
		if ( RwRasterGetWidth(raster) != RwRasterGetHeight(raster) )
		{
			float ar = float(RwRasterGetWidth(raster))/float(RwRasterGetHeight(raster));
			float sclx = ar / SCREEN_ASPECT_RATIO * scalex;
			float px   =((SCREEN_ASPECT_RATIO/2.0f) - (ar/2.0f)) / SCREEN_ASPECT_RATIO;

			CRenderer::DrawQuad2d(px+SCLX(posx), posy, sclx, scaley, R, G, B, A, raster);
		}
		else
			CRenderer::DrawQuad2d(FCENTER(posx), posy, SCLX(scalex), scaley, R, G, B, A, raster);
	}
	else
		CRenderer::DrawQuad2d(posx, posy, scalex, scaley, R, G, B, A, raster);
}

void __cdecl CRenderer_DrawQuad2d_Menu_Black(float posx, float posy, float scalex, float scaley, long R, long G, long B, long A, RwRaster *raster)
{
	CRenderer::DrawQuad2d(0.0f, 0.0f, 1.0f, 1.0f, 0, 0, 0, 255, NULL);
	CRenderer_DrawQuad2d_Menu(posx, posy, scalex, scaley, R, G, B, A, raster);
}

void __cdecl CRenderer_DrawRasterLineFX(int red, int green, int blue, int alpha)
{
	CRenderer::RenderStateSetBlendDefault();

	float size = 2.0f / DEFAULT_SCREEN_HEIGHT;

	int isize = max(1, (int)floor(size*CFrontend::ms_scrn.fHeight));
	int h = (int)CFrontend::ms_scrn.fHeight;
	int istep = isize*2;

	CRenderer::DrawQuad2dBegin(NULL);
	for ( int y = 0; y < h; y += istep )
	{
		float Y = float(y)/CFrontend::ms_scrn.fHeight;
		float H = float(isize)/CFrontend::ms_scrn.fHeight;
		CRenderer::DrawQuad2dSet(0.0f, Y, 1.0f, H, red, green, blue, alpha);
	}
	CRenderer::DrawQuad2dEnd();
}

void __cdecl DrawSubtitles()
{
	static wchar_t *& SubLine1 = *(wchar_t **)0x7E9D30;
	static wchar_t *& SubLine2 = *(wchar_t **)0x7E9D34;

	if ( SubLine1 || SubLine2 )
	{
		float fX = FCENTER(55.0f / DEFAULT_SCREEN_WIDTH);
		float fY =        380.0f / DEFAULT_SCREEN_HEIGHT;
		float fW =   SCLX(530.0f / DEFAULT_SCREEN_WIDTH);
		float fH =         54.0f / DEFAULT_SCREEN_HEIGHT;

		CRenderer::DrawQuad2d(fX, fY, fW, fH, 32, 32, 32, 196, NULL);
	}

	{
		float fX  = FCENTER(512.0f * 0.129f / DEFAULT_SCREEN_WIDTH);
		float fY  =         448.0f * 0.86f  / DEFAULT_SCREEN_HEIGHT;
		float fY2 =         448.0f * 0.905f / DEFAULT_SCREEN_HEIGHT;
		float fW  =         512.0f * 1.0f   / DEFAULT_SCREEN_WIDTH;
		float fH  =         448.0f * 0.8f   / DEFAULT_SCREEN_HEIGHT;

		float fShadX = SCLX(2.0f / DEFAULT_SCREEN_WIDTH);
		float fShadY =      2.0f / DEFAULT_SCREEN_HEIGHT;

		if ( SubLine1 )
		{
			CFrontendMenu::SetPrintColor(0, 0, 0, 128);
			CFrontend::Print16_(SubLine1, fX-fShadX, fY+fShadY, fW, fH, 0.0f, 1);

			CFrontendMenu::SetPrintColor(200, 200, 200, 255);
			CFrontend::Print16_(SubLine1, fX,        fY,        fW, fH, 0.0f, 1);
		}

		if ( SubLine2 )
		{
			CFrontendMenu::SetPrintColor(0, 0, 0, 128);
			CFrontend::Print16_(SubLine2, fX-fShadX, fY2+fShadY, fW, fH, 0.0f, 1);

			CFrontendMenu::SetPrintColor(200, 200, 200, 255);
			CFrontend::Print16_(SubLine2, fX,        fY2,        fW, fH, 0.0f, 1);
		}
	}
}

void __declspec(naked) patch_DrawSubtitles()
{
	__asm
	{
		cmp     CConfig::bPS2Subs, 0
		jz      PCSubs

		call    DrawSubtitles

		mov     eax, 60FE8Ah
		jmp     eax

PCSubs:
		cmp     dword ptr [esp+0Ch-4], 0
		jz      loc_60FCC3

		mov     eax, 60FB1Fh
		jmp     eax

loc_60FCC3:
		mov     eax, 60FCC3h
		jmp     eax
	}
}

void __declspec(naked) patch___DrawTapeInfo_1()
{
	__asm
	{
		fld     dword ptr [ebp+364h]
		fmul    fSCL1

		retn
	}
}

void __declspec(naked) patch___DrawTapeInfo_2()
{
	__asm
	{
		fld     dword ptr [ebp+368h]
		fmul    fSCL1

		retn
	}
}

void __declspec(naked) patch___DrawTapeInfo_3()
{
	__asm
	{
		fld     dword ptr [ebp+368h]
		fmul    fSCL1
		faddp   st(1), st

		retn
	}
}

void __declspec(naked) patch___DrawTapeInfo_4()
{
	__asm
	{
		push    dword ptr [ebp+340h]
		call    CeilScaleHor
		add     esp, 4

		push    eax
		fild    dword ptr [esp]
		pop     eax

		fadd    st, st
		fdivp   st(1), st

		retn
	}
}

void __declspec(naked) patch___DrawTapeInfo_5()
{
	__asm
	{
		mov     ebx, eax

		push    dword ptr [ebp+340h]
		call    CeilScaleHor
		add     esp, 4

		push    eax
		call    CeilToPow2
		add     esp, 4

		add     eax, eax
		dec     eax

		and     ebx, eax
		mov     eax, ebx

		retn
	}
}

void __declspec(naked) patch___DrawTapeInfo_6()
{
	__asm
	{
		push    dword ptr [ebp+340h]
		call    CeilScaleHor
		add     esp, 4

		push    eax
		call    CeilToPow2
		add     esp, 4

		cmp     eax, 0
		jle     loc_58EA6B

		mov     eax, 58E846h
		jmp     eax

loc_58EA6B:
		mov     eax, 58EA6Bh
		jmp     eax
	}
}

void __declspec(naked) patch___DrawTapeInfo_7()
{
	__asm
	{
		push    dword ptr [ebp+340h]
		call    CeilScaleHor
		add     esp, 4

		push    eax
		call    CeilToPow2
		add     esp, 4

		cmp     esi, eax
		jl      loc_58E846

		mov     eax, 58EA6Bh
		jmp     eax

loc_58E846:
		mov     eax, 58E846h
		jmp     eax
	}
}

void __declspec(naked) patch_DrawCameraCounter_1()
{
	__asm
	{
		fld     _fMENUCNTTEXT4X
		fstp    [esp+34h+4-20h]

		retn
	}
}

void __declspec(naked) patch_DrawCameraCounter_2()
{
	__asm
	{
		fld     _fMENUCNTTEXT5X
		fstp    [esp+34h+4-1Ch]
		fld     _fMENUCNTTEXT6X
		fstp    [esp+34h+4-34h]

		retn
	}
}

void __declspec(naked) patch_RenderFuzz()
{
	__asm
	{
		cmp     CConfig::bWideScreen, 0
		jnz     FuzzFix

		fadd    dword ptr ds :[7A1580h] // flt_7A1580
		fmul    dword ptr ds :[7D3458h] // HudStretch

		retn

FuzzFix:

		fadd    dword ptr ds :[7A1580h] // flt_7A1580

		fld     dword ptr [esp+44h+4-40h]

		fmul    st, st
		fld     st(1)
		fmulp   st(2), st
		faddp   st(1), st
		fstp    dword ptr [esp-4]
		push    dword ptr [esp-4]
		call    sqrtf
		add     esp, 4
		mov     dword ptr [esp-4], 3F34FDF4h // 0.707f
		fmul    dword ptr [esp-4]

		fst     dword ptr [esp+44h+4-40h]

		fmul    dword ptr ds :[7D3458h] // HudStretch

		retn
	}
}

void __declspec(naked) patch_BloodFire1()
{
	__asm
	{
		fmul    fSCL1
		fadd    dword ptr [ebx+0CC4h]

		retn
	}
}

void __declspec(naked) patch_BloodFire2()
{
	__asm
	{
		fdiv    dword ptr [ebx+0D10h]
		fmul    fSCL1

		retn
	}
}

void __declspec(naked) patch_BloodFire3()
{
	__asm
	{
		fmul    [esp+0BCh+4-0A8h]
		fmul    fSCL1
		fadd    [esp+0BCh+4-0A4h]

		retn
	}
}

void __declspec(naked) patch_FXNightvision_1()
{
	__asm
	{
		mov     [esp+104h+4-0B0h], edi
		mov     ecx, [esp+104h+4+0ACh]


		fld     dword ptr [esp+104h+4-3Ch]
		fmul    dword ptr ds :[7D3458h] // HudStretch
		fstp    dword ptr [esp+104h+4-3Ch]

		retn
	}
}

void __declspec(naked) patch_FXNightvision_2()
{
	__asm
	{
		fld     dword ptr ds :[7D3458h] // HudStretch

		fmul    st, st(1)
		fadd    dword ptr ss : [esp+104h+4-3Ch]
		fstp    [esp+104h+4-3Ch]

		retn
	}
}

void __declspec(naked) patch_FXArtFilter_1()
{
	__asm
	{
		mov     ecx, [esp+104h+4-0ACh]
		mov     edx, [esp+104h+4-88h]


		fld     dword ptr [esp+104h+4-3Ch]
		fmul    dword ptr ds :[7D3458h] // HudStretch
		fstp    dword ptr [esp+104h+4-3Ch]

		retn
	}
}

void __declspec(naked) patch_FXArtFilter_2()
{
	__asm
	{
		fld     dword ptr ds :[7D3458h] // HudStretch

		fmul    st, st(3)
		fadd    dword ptr ss : [esp+104h+4-3Ch]
		fstp    [esp+104h+4-3Ch]

		retn
	}
}

void __declspec(naked) patch_FXDrug_1()
{
	__asm
	{
		mov     [esp+104h+4-0B0h], edx
		mov     ecx, [esp+104h+4-0ACh]


		fld     dword ptr [esp+104h+4-3Ch]
		fmul    dword ptr ds :[7D3458h] // HudStretch
		fstp    dword ptr [esp+104h+4-3Ch]

		retn
	}
}

void __declspec(naked) patch_FXDrug_2()
{
	__asm
	{
		fld     dword ptr ds :[7D3458h] // HudStretch

		fmul    st, st(5)
		fadd    dword ptr ss : [esp+104h+4-3Ch]
		fstp    [esp+104h+4-3Ch]

		retn
	}
}

void __declspec(naked) patch_FXHit1_1()
{
	__asm
	{
		mov     [esp+104h+4-84h], edx

		fld     dword ptr [esp+104h+4-3Ch]
		fmul    dword ptr ds :[7D3458h] // HudStretch
		fstp    dword ptr [esp+104h+4-3Ch]

		retn
	}
}

void __declspec(naked) patch_FXHit1_2()
{
	__asm
	{
		fld     dword ptr ds :[7D3458h] // HudStretch

		fmul    st, st(3)
		fadd    dword ptr ss : [esp+104h+4-3Ch]
		fstp    [esp+104h+4-3Ch]

		retn
	}
}

void __declspec(naked) patch_FXHit2_1()
{
	__asm
	{
		mov     ecx, [esp+104h+4-3Ch]
		mov     [esp+104h+4-88h], ecx

		fld     dword ptr [esp+104h+4-3Ch]
		fmul    dword ptr ds :[7D3458h] // HudStretch
		fstp    dword ptr [esp+104h+4-3Ch]

		retn
	}
}

void __declspec(naked) patch_FXHit2_2()
{
	__asm
	{
		fld     dword ptr ds :[7D3458h] // HudStretch

		fmul    st, st(3)
		fadd    dword ptr ss : [esp+104h+4-3Ch]
		fstp    [esp+104h+4-3Ch]

		retn
	}
}

void __declspec(naked) patch_FXHit3()
{
	__asm
	{
		fdiv    dword ptr ds :[7D3458h] // HudStretch
		fmul    dword ptr ds :[7CD138h]

		retn
	}
}

void __declspec(naked) patch_CTextOverlay_Render1()
{
	__asm
	{
		fld     dword ptr [ebp+24h]
		fmul    fSCL1

		fstp    dword ptr [esp+3Ch+4-38h]

		retn
	}
}

void __declspec(naked) patch_CTextOverlay_Render2()
{
	__asm
	{
		push    dword ptr [ebp+24h]
		push    dword ptr [ebp+1Ch]
		call    ScaleX
		add     esp, 8
		fld     dword ptr [ebp+38h]
		fmul    fSCL1
		faddp   st(1), st(0)

		fstp    [esp+3Ch+4-2Ch]

		retn
	}
}

void __declspec(naked) patch_CTextOverlay_Render3()
{
	__asm
	{
		push    dword ptr [ebp+24h]
		push    dword ptr [ebp+1Ch]
		call    ScaleX
		add     esp, 8
		fld     dword ptr [ebp+38h]
		fmul    fSCL1
		faddp   st(1), st(0)

		fstp    st(1)

		retn
	}
}

void __declspec(naked) patch_CTextOverlay_CheckTextWidth()
{
	__asm
	{
		fld     dword ptr [ecx+24h]
		fmul    fSCL1
		fstp    dword ptr [esp-4]

		fcomp   dword ptr [esp-4]
		fnstsw  ax

		retn
	}
}

void __declspec(naked) patch_CTextOverlay_SplitAddWord()
{
	__asm
	{
		fld     dword ptr [ebp+38h]
		fmul    fSCL1
		fadd    st, st
		fld     dword ptr [ebp+24h]
		fmul    fSCL1
		fsub    st(0), st(1)
		fstp    dword ptr [esp-4]
		fstp    st

		fcomp   dword ptr [esp-4]
		fnstsw  ax

		retn
	}
}

void __declspec(naked) patch_CTextOverlay_RightJustify()
{
	__asm
	{
		fld     dword ptr [eax+24h]
		fmul    fSCL1
		fsub    [esp+184h+4-180h]

		retn
	}
}

void __declspec(naked) patch_CLightFX_Render()
{
	__asm
	{
		fld     dword ptr [ebx+40h]
		fmul    dword ptr ds :[7D3458h] // HudStretch
		fstp    st(1)
		fld     dword ptr [ebx+44h]
		fmul    dword ptr ds :[7D3458h] // HudStretch
		fstp    st(4)

		retn
	}
}

void __declspec(naked) patch_Render2d_dbg1()
{
	__asm
	{
		fld     fFrameCounterX
		fstp    dword ptr ds:7D4EB0h

		retn
	}
}

void __declspec(naked) patch_Render2d_dbg2()
{
	__asm
	{
		fld     fShadowCameraX
		fstp    dword ptr ds:7D4EB0h

		retn
	}
}

void __declspec(naked) patch_Render2d_dbg3()
{
	__asm
	{
		fld     fMSX
		fstp    dword ptr ds:7D4EB0h

		retn
	}
}

void __declspec(naked) patch_Render2d_dbg4()
{
	__asm
	{
		fld     fRSSETX
		fstp    dword ptr ds:7D4EB0h

		retn
	}
}

void __declspec(naked) patch_Render2d_dbg5()
{
	__asm
	{
		fld     fRSGETX
		fstp    dword ptr ds:7D4EB0h

		retn
	}
}

void __declspec(naked) patch_Render2d_dbg6()
{
	__asm
	{
		fld     fMEMUSEDX
		fstp    dword ptr ds:7D4EB0h

		retn
	}
}

void __declspec(naked) patch_Render2d_dbgmenu1()
{
	__asm
	{
		fld     fDebugTextX1
		fstp    dword ptr ds:7D4EB0h

		retn
	}
}

void __declspec(naked) patch_Render2d_dbgmenu2()
{
	__asm
	{
		fld     fDebugTextX2
		fstp    dword ptr ds:7D4EB0h

		retn
	}
}

void __declspec(naked) patch_Render2d_dbgmenu3()
{
	__asm
	{
		fld     fDebugTextX3
		fstp    dword ptr ds:7D4EB0h

		retn
	}
}

void __declspec(naked) patch_Render2d_dbgmenu4()
{
	__asm
	{
		fld     fDebugTextX4
		fstp    dword ptr ds:7D4EB0h

		retn
	}
}

void __declspec(naked) patch_Render2d_hudx()
{
	__asm
	{
		fld     fHudBaseX
		fstp    [esp+1BBCh+4-130h]

		retn
	}
}

void __declspec(naked) patch_Render2d_hudy()
{
	__asm
	{
		fld     fHudBaseY
		fstp    [esp+1BBCh+4-13Ch]

		retn
	}
}

void __declspec(naked) patch_RenderLockOnDisplayForPed()
{
	__asm
	{
		mov     eax, dword ptr [esi+8]
		mov     [esp+0B8h+4-8Ch], eax

		cmp     CConfig::bWideScreen, 0
		jz      NonEquilateral

		cmp     CConfig::bForceEquilateralLockOnCrosshair, 0
		jz      NonEquilateral

		fld     dword ptr [esi+4]
		fld     dword ptr [esi+8]

		fmul    st, st
		fld     st(1)
		fmulp   st(2), st
		faddp   st(1), st
		fstp    dword ptr [esp-4]
		push    dword ptr [esp-4]
		call    sqrtf
		add     esp, 4
		mov     dword ptr [esp-4], 3F34FDF4h // 0.707f
		fmul    dword ptr [esp-4]

		fst     dword ptr [esp+0B8h+4-8Ch]

		fmul    dword ptr ds :[7D3458h] // HudStretch
		fstp    dword ptr [esp+0B8h+4-3Ch]

NonEquilateral:
		retn
	}
}

void __declspec(naked) patch_InventoryControl()
{
	__asm
	{
		fld     fWeaponBaseY
		fstp    [esp+0C8h+4-14h]

		retn
	}
}

void __declspec(naked) patch_TimerDraw()
{
	__asm
	{
		fld     _fTimerX
		fstp    [esp+1250h+4-228h]

		retn
	}
}

void __declspec(naked) patch_GameMap()
{
	__asm
	{
		fld     _fMapPos2X
		fstp    dword ptr ds:7C9E40h

		fld     _fMapPos2Y
		fstp    dword ptr ds:7C9E44h

		retn
	}
}

void __declspec(naked) patch_PrintInfoDraw_1()
{
	__asm
	{
		fld     fPrintInfoX1
		fstp    [esp+14h+4-14h]

		retn
	}
}

void __declspec(naked) patch_PrintInfoDraw_2()
{
	__asm
	{
		fld     fPrintInfoX2
		fstp    [esp+14h+4-14h]

		retn
	}
}

void __declspec(naked) patch_LoadProgressTexDictLoad()
{
	__asm
	{
		fld     fMHLogo
		fstp    dword ptr ds:7D4EB0h

		retn
	}
}

void __declspec(naked) patch_CFEP_SceneSelection__Draw()
{
	__asm
	{
		fld     fSceneSel1X
		fstp    [esp+98h+4-80h]

		retn
	}
}

void __declspec(naked) patch_CFEP_BonusFeatures__Draw()
{
	__asm
	{
		fld     fBonusSel1X
		fstp    [esp+0C4h+4-88h]

		retn
	}
}

void __declspec(naked) patch_CFrontendMenu__MouseDraw()
{
	__asm
	{
		fld     dword ptr ds :[7C8FC8h]
		fmul    dword ptr ds :[7D3458h] // HudStretch
		fstp    dword ptr [esp-4]
		sub     esp, 4

		mov     eax, 5DB0B8h
		jmp     eax
	}
}

void __declspec(naked) patch_CFrontendMenu__MouseUpdate()
{
	__asm
	{
		fld     dword ptr ds :[7C8FC8h]
		fmul    dword ptr ds :[7D3458h] // HudStretch
		fsubp   st(1), st(0)

		retn
	}
}

void __declspec(naked) patch_CFEP_SettingsVideo__Update()
{
	__asm
	{
		mov     ebx, 2
		lea     esi, dword ptr [CConfig::bWideScreen]

		retn
	}
}

void __declspec(naked) patch_CFEP_SettingsVideo__Draw_1()
{
	__asm
	{
		cmp     CConfig::bWideScreen, 0
		jnz     loc_6037F0
		mov     eax, 6037EBh
		jmp     eax

loc_6037F0:
		mov     eax, 6037F0h
		jmp     eax
	}
}

void __declspec(naked) patch_CFEP_SettingsVideo__Draw_2()
{
	__asm
	{
		cmp     CConfig::bWideScreen, 1
		jnz     loc_603850
		mov     eax, 603849h
		jmp     eax

loc_603850:
		mov     eax, 603850h
		jmp     eax
	}
}

void __declspec(naked) patch_CPCLoadSave__SaveSettings()
{
	__asm
	{
		jmp     CConfig::Write

		retn
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
	if(reason == DLL_PROCESS_ATTACH)
	{
		CConfig::Read();

		static float fOne = 1.0f;

		if ( CConfig::bLockedCursor )
			CPatch::SetChar(0x4C184A, 0x75); // jz -> jnz

		CCamera::m_aspectRatio = DEFAULT_ASPECT_RATIO;

		CPatch::Nop(0x475BF5, 0x475C09-0x475BF5);
		CPatch::RedirectCall(0x475BF5, SetViewWindowOriginal);
		CPatch::RedirectJump(0x476A80, SetViewWindowDefault);
		CPatch::RedirectJump(0x476AA0, SetViewWindowWidescreen);

		CPatch::Nop(0x5E2536, 0x5E2540-0x5E2536);
		CPatch::RedirectCall(0x5E2536, patch_Initialise);

		CPatch::RedirectJump(0x604F20, SCGetAspectRatio);

		CPatch::RedirectJump(0x60625A, patch_CPCLoadSave__SaveSettings);

		// 2d
		CPatch::RedirectJump(0x5F8AA2, patch_DrawQuad2dSet);
		CPatch::RedirectJump(0x5F93C2, patch_DrawQuad2dSetAll);

		// BinkFrame
		// take resolution from bink file
		unsigned char push_width_ebx[]  =  { 0xFF, 0x73, 0x00 };   // push    dword ptr [ebx+0]
		unsigned char push_height_ebx[] =  { 0xFF, 0x73, 0x04 };   // push    dword ptr [ebx+4]
		unsigned char push_width_ebp[]  =  { 0xFF, 0x75, 0x00 };   // push    dword ptr [ebp+0]
		unsigned char push_height_ebp[] =  { 0xFF, 0x75, 0x04 };   // push    dword ptr [ebp+4]
		CPatch::Nop(0x4C0D2F, 5);
		CPatch::Set(0x4C0D2F, push_width_ebx, sizeof(push_width_ebx));
		CPatch::Nop(0x4C0D2A, 5);
		CPatch::Set(0x4C0D2A, push_height_ebx, sizeof(push_height_ebx));
		CPatch::Nop(0x4C0ECA, 5);
		CPatch::Set(0x4C0ECA, push_width_ebp, sizeof(push_width_ebp));
		CPatch::Nop(0x4C0EC5, 5);
		CPatch::Set(0x4C0EC5, push_height_ebp, sizeof(push_height_ebp));
		CPatch::Nop(0x4C0ED7, 5);
		CPatch::Set(0x4C0ED7, push_height_ebp, sizeof(push_height_ebp));
		//
		CPatch::RedirectCall(0x4C0F31, CRenderer_DrawQuad2d_Menu_Black);
		CPatch::RedirectJump(0x60FB14, patch_DrawSubtitles);

		// __DrawTapeInfo
		CPatch::SetPointer(0x58DE85 + 2, &_fTITextY);
		CPatch::SetPointer(0x58DECF + 2, &_fTITextY);
		CPatch::SetPointer(0x58DF08 + 2, &_fTITextY);
		CPatch::SetPointer(0x58DF47 + 2, &_fTITextY);

		CPatch::SetPointer(0x58E08B + 2, &_fRecTextX);
		CPatch::SetPointer(0x58DE47 + 2, &_TCRW2);
		CPatch::Nop(0x58DED8, 0x58DEDE-0x58DED8);
		CPatch::RedirectCall(0x58DED8, patch___DrawTapeInfo_1);
		CPatch::Nop(0x58DF0E, 0x58DF14-0x58DF0E);
		CPatch::RedirectCall(0x58DF0E, patch___DrawTapeInfo_1);
		CPatch::Nop(0x58DF4D, 0x58DF53-0x58DF4D);
		CPatch::RedirectCall(0x58DF4D, patch___DrawTapeInfo_1);
		CPatch::Nop(0x58DF5C, 0x58DF62-0x58DF5C);
		CPatch::RedirectCall(0x58DF5C, patch___DrawTapeInfo_2);
		CPatch::Nop(0x58DF21, 0x58DF27-0x58DF21);
		CPatch::RedirectCall(0x58DF21, patch___DrawTapeInfo_3);
		CPatch::Nop(0x58E93B, 0x58E941-0x58E93B);
		CPatch::RedirectCall(0x58E93B, patch___DrawTapeInfo_4);
		CPatch::Nop(0x58E94F, 0x58E955-0x58E94F);
		CPatch::RedirectCall(0x58E94F, patch___DrawTapeInfo_4);
		CPatch::RedirectCall(0x58E88C, patch___DrawTapeInfo_5);
		CPatch::RedirectJump(0x58E839, patch___DrawTapeInfo_6);
		CPatch::RedirectJump(0x58EA5F, patch___DrawTapeInfo_7);

		// DrawCameraCounter
		CPatch::Nop(0x5D575F, 0x5D5767-0x5D575F);
		CPatch::RedirectCall(0x5D575F, patch_DrawCameraCounter_1);
		CPatch::Nop(0x5D57F3, 0x5D5802-0x5D57F3);
		CPatch::RedirectCall(0x5D57F3, patch_DrawCameraCounter_2);

		// CFrontend::DrawVideoFX
		CPatch::SetPointer(0x5F7B0C + 2, &fOne);
		CPatch::SetPointer(0x5F7D1B + 2, &fOne);
		CPatch::SetPointer(0x5F7D11 + 2, &fVideoFxTC2Min);
		CPatch::SetPointer(0x5F7D0B + 2, &fVideoFxTC2MaxMinusMin);
		CPatch::SetPointer(0x5F7A75 + 2, &fVideoFxSizeHalf);
		CPatch::SetPointer(0x5F7C58 + 2, &fVideoFxSizeHalf);
		CPatch::SetPointer(0x5F7E4A + 2, &fVideoFxSizeHalf);

		// RenderFuzz
		CPatch::SetPointer(0x592456 + 2, &fFuzzStepX);
		CPatch::Nop(0x592417, 0x59241D-0x592417);
		CPatch::RedirectCall(0x592417, patch_RenderFuzz);

		// Scanlines
		CPatch::RedirectJump(0x5FA6F0, CRenderer_DrawRasterLineFX);

		// CBloodDropManager::BloodFire
		CPatch::Nop(0x592CFA, 0x592D00-0x592CFA);
		CPatch::RedirectCall(0x592CFA, patch_BloodFire1);
		CPatch::Nop(0x592DBE, 0x592DC4-0x592DBE);
		CPatch::RedirectCall(0x592DBE, patch_BloodFire2);
		CPatch::Nop(0x593023, 0x59302B-0x593023);
		CPatch::RedirectCall(0x593023, patch_BloodFire3);

		// FX
		CPatch::Nop(0x5F3A6A, 0x5F3A72-0x5F3A6A);
		CPatch::RedirectCall(0x5F3A6A, patch_FXNightvision_1);
		CPatch::Nop(0x5F3AAA, 0x5F3ABA-0x5F3AAA);
		CPatch::RedirectCall(0x5F3AAA, patch_FXNightvision_2);
		CPatch::Nop(0x5F41B0, 0x5F41B8-0x5F41B0);
		CPatch::RedirectCall(0x5F41B0, patch_FXArtFilter_1);
		CPatch::Nop(0x5F41EC, 0x5F41FC-0x5F41EC);
		CPatch::RedirectCall(0x5F41EC, patch_FXArtFilter_2);
		CPatch::Nop(0x5F450F, 0x5F4517-0x5F450F);
		CPatch::RedirectCall(0x5F450F, patch_FXDrug_1);
		CPatch::Nop(0x5F454F, 0x5F455F-0x5F454F);
		CPatch::RedirectCall(0x5F454F, patch_FXDrug_2);
		CPatch::Nop(0x5F4980, 0x5F4987-0x5F4980);
		CPatch::RedirectCall(0x5F4980, patch_FXHit1_1);
		CPatch::Nop(0x5F49A9, 0x5F49B9-0x5F49A9);
		CPatch::RedirectCall(0x5F49A9, patch_FXHit1_2);
		CPatch::Nop(0x5F5067, 0x5F5072-0x5F5067);
		CPatch::RedirectCall(0x5F5067, patch_FXHit2_1);
		CPatch::Nop(0x5F5090, 0x5F50A0-0x5F5090);
		CPatch::RedirectCall(0x5F5090, patch_FXHit2_2);
		CPatch::SetChar(0x5F54C2 + 3, 0x2C);	// var_88 -> var_F8
		CPatch::SetChar(0x5F5577 + 3, 0x2C);	// var_88 -> var_F8
		CPatch::Nop(0x5F5484, 0x5F548A-0x5F5484);
		CPatch::RedirectCall(0x5F5484, patch_FXHit3);
		CPatch::RedirectCall(0x5F567C, CRenderer_DrawTri2d_FixNoFX);

		// CLightFx, fleshlight
		CPatch::SetPointer(0x59C005 + 2, &fOne);
		CPatch::Nop(0x59C7CF, 0x59C7D9-0x59C7CF);
		CPatch::RedirectCall(0x59C7CF, patch_CLightFX_Render);

		// CTextOverlay
		if ( CConfig::bXboxHud )
		{
			_fOverlayX = 0.075f;
			_fOverlayY = 0.075f;
		}

		CPatch::Nop(0x48AA44, 0x48AA4B-0x48AA44);
		CPatch::RedirectCall(0x48AA44, patch_CTextOverlay_Render1);
		CPatch::Nop(0x48AA8F, 0x48AA96-0x48AA8F);
		CPatch::RedirectCall(0x48AA8F, patch_CTextOverlay_Render2);
		CPatch::Nop(0x48AC43, 0x48AC49-0x48AC43);
		CPatch::RedirectCall(0x48AC43, patch_CTextOverlay_Render3);
		CPatch::RedirectCall(0x48B0AB, patch_CTextOverlay_SplitAddWord);
		CPatch::RedirectCall(0x48B1D4, patch_CTextOverlay_CheckTextWidth);
		CPatch::RedirectCall(0x48B2A4, patch_CTextOverlay_SplitAddWord);
		CPatch::Nop(0x48B421, 0x48B428-0x48B421);
		CPatch::RedirectCall(0x48B421, patch_CTextOverlay_RightJustify);

		// console
		CPatch::SetPointer(0x4BD801 + 2, &fConsoleSCLY);
		CPatch::SetPointer(0x4BD8A2 + 2, &fConsoleSCLY);
		CPatch::SetPointer(0x4BD81D + 2, &fConsoleSCLX);
		CPatch::SetPointer(0x4BD8C8 + 2, &fConsoleSCLX);

		//CFrontend::Render2d

		// dbg
		CPatch::Nop(0x5EFB3E, 0x5EFB48-0x5EFB3E);
		CPatch::RedirectCall(0x5EFB3E, patch_Render2d_dbg1);
		CPatch::Nop(0x5EFC5C, 0x5EFC66-0x5EFC5C);
		CPatch::RedirectCall(0x5EFC5C, patch_Render2d_dbg2);
		CPatch::Nop(0x5EFDBF, 0x5EFDC9-0x5EFDBF);
		CPatch::RedirectCall(0x5EFDBF, patch_Render2d_dbg3);
		CPatch::Nop(0x5F1A9F, 0x5F1AA9-0x5F1A9F);
		CPatch::RedirectCall(0x5F1A9F, patch_Render2d_dbg4);
		CPatch::Nop(0x5F1BCB, 0x5F1BD5-0x5F1BCB);
		CPatch::RedirectCall(0x5F1BCB, patch_Render2d_dbg5);
		CPatch::Nop(0x5F1CFE, 0x5F1D08-0x5F1CFE);
		CPatch::RedirectCall(0x5F1CFE, patch_Render2d_dbg6);
		CPatch::SetPointer(0x5F1E2B + 2, &fDebugX);
		CPatch::SetPointer(0x5F1E1F + 2, &fDebugW);
		CPatch::Nop(0x5F22DF, 0x5F22E9-0x5F22DF);
		CPatch::RedirectCall(0x5F22DF, patch_Render2d_dbgmenu1);
		CPatch::Nop(0x5F24C1, 0x5F24CB-0x5F24C1);
		CPatch::RedirectCall(0x5F24C1, patch_Render2d_dbgmenu2);
		CPatch::Nop(0x5F25C6, 0x5F25D0-0x5F25C6);
		CPatch::RedirectCall(0x5F25C6, patch_Render2d_dbgmenu3);
		CPatch::Nop(0x5F2638, 0x5F2642-0x5F2638);
		CPatch::RedirectCall(0x5F2638, patch_Render2d_dbgmenu3);
		CPatch::Nop(0x5F2695, 0x5F269F-0x5F2695);
		CPatch::RedirectCall(0x5F2695, patch_Render2d_dbgmenu3);
		CPatch::Nop(0x5F270E, 0x5F2718-0x5F270E);
		CPatch::RedirectCall(0x5F270E, patch_Render2d_dbgmenu3);
		CPatch::Nop(0x5F2765, 0x5F276F-0x5F2765);
		CPatch::RedirectCall(0x5F2765, patch_Render2d_dbgmenu4);
		CPatch::Nop(0x5F279E, 0x5F27A8-0x5F279E);
		CPatch::RedirectCall(0x5F279E, patch_Render2d_dbgmenu3);

		// hud
		CPatch::Nop(0x5F02AE, 0x5F02B9-0x5F02AE);
		CPatch::RedirectCall(0x5F02AE, patch_Render2d_hudx);
		CPatch::Nop(0x5F02C8, 0x5F02D3-0x5F02C8);
		CPatch::RedirectCall(0x5F02C8, patch_Render2d_hudy);
		CPatch::SetPointer(0x5F0C90 + 2, &fOffX);
		CPatch::SetPointer(0x5F0E0C + 2, &fOffX);

		// RenderLockOnDisplayForPed
		CPatch::Nop(0x4AAB67, 0x4AAB6E-0x4AAB67);
		CPatch::RedirectCall(0x4AAB67, patch_RenderLockOnDisplayForPed);

		// CGameInfo::RenderDamageDirections
		CPatch::SetPointer(0x5DB814 + 2, &fDamageMidY);
		CPatch::SetPointer(0x5DBC16 + 2, &fDamageMidY);

		// CGameInventory::InventoryControl
		CPatch::Nop(0x5DE913, 0x5DE91E-0x5DE913);
		CPatch::RedirectCall(0x5DE913, patch_InventoryControl);
		CPatch::SetPointer(0x5DF8F2 + 2, &fInvConOffX);
		//

		// CFrontend::TimerDraw
		CPatch::Nop(0x5E7941, 0x5E794C-0x5E7941);
		CPatch::RedirectCall(0x5E7941, patch_TimerDraw);
		CPatch::Nop(0x5E7DF6, 0x5E7E01-0x5E7DF6);
		CPatch::RedirectCall(0x5E7DF6, patch_TimerDraw);

		// CGameMap::UpdateAndRender
		CPatch::Nop(0x5E0082, 0x5E0096-0x5E0082);
		CPatch::RedirectCall(0x5E0082, patch_GameMap);

		// CFrontend::TextBoxDraw
		CPatch::SetPointer(0x5F6E30 + 2, &fTextBoxOffX);
		CPatch::SetPointer(0x5F7158 + 2, &fTextBoxOffX);
		CPatch::SetPointer(0x5F7289 + 2, &fTextBoxOffX);
		CPatch::SetPointer(0x5F72B3 + 2, &fTextBoxOffX);
		CPatch::SetPointer(0x5F6F79 + 2, &fTextBoxOffX2);
		CPatch::SetPointer(0x5F6F8F + 2, &fTextBoxOffX2);
		CPatch::SetPointer(0x5F6FD4 + 2, &fTextBoxOffX2);
		CPatch::SetPointer(0x5F74EE + 2, &fTextBoxOffX3);
		CPatch::SetPointer(0x5F76AD + 2, &fTextBoxOffX4);

		// CFrontend::PrintInfoDraw
		CPatch::Nop(0x5E523A, 0x5E5241-0x5E523A);
		CPatch::RedirectCall(0x5E523A, patch_PrintInfoDraw_1);
		CPatch::Nop(0x5E5243, 0x5E524A-0x5E5243);
		CPatch::RedirectCall(0x5E5243, patch_PrintInfoDraw_2);
		CPatch::SetPointer(0x5E533E + 2, &fPrintInfoPadSizeX1);
		CPatch::SetPointer(0x5E53A3 + 2, &fPrintInfoPadSizeX1);
		CPatch::SetPointer(0x5E532A + 2, &fPrintInfoPadSizeX2);
		CPatch::SetPointer(0x5E5392 + 2, &fPrintInfoPadSizeX2);

		// CGameInfo::Render
		CPatch::SetPointer(0x5DC8B3 + 2, &fLevelCompX2);
		CPatch::SetPointer(0x5DC980 + 2, &fLevelCompX2);
		CPatch::SetPointer(0x5DCA49 + 2, &fLevelCompX2);
		CPatch::SetPointer(0x5DCAFD + 2, &fLevelCompX2);
		CPatch::SetPointer(0x5DCE71 + 2, &fLevelCompX2);
		CPatch::SetPointer(0x5DD011 + 2, &fLevelCompX2);
		CPatch::SetPointer(0x5DCC32 + 2, &fLevelCompX3);

		// CFrontend::Initialise
		CPatch::RedirectCall(0x5E2652, CRenderer_DrawQuad2d_Menu_Black);
		CPatch::RedirectCall(0x5E26DC, CRenderer_DrawQuad2d_Menu_Black);

		// RenderMenu
		CPatch::RedirectCall(0x5D7471, CRenderer_DrawQuad2d_Menu);
		CPatch::RedirectCall(0x5D7369, CRenderer_DrawQuad2d_Menu);
		CPatch::RedirectCall(0x5D7504, CRenderer_DrawQuad2d_Menu);
		CPatch::RedirectCall(0x5D74A8, CRenderer_DrawQuad2d_Menu);

		// CPeripherals::Render_Standard
		CPatch::RedirectCall(0x608841, CRenderer_DrawQuad2d_Menu_Black);

		// CGameInfo::DrawBlackBox
		CPatch::RedirectCall(0x5DDBE4, CRenderer_DrawQuad2d_Menu);

		// CFrontend::LoadProgressTexDictLoad
		CPatch::SetPointer(0x5EECD9+2, &fMHLogo);
		CPatch::SetPointer(0x5EF026+2, &fMHLogo);
		CPatch::Nop(0x5EEDD6, 0x5EEDE0-0x5EEDD6);
		CPatch::RedirectCall(0x5EEDD6, patch_LoadProgressTexDictLoad);
		CPatch::Nop(0x5EEF9C, 0x5EEFA6-0x5EEF9C);
		CPatch::RedirectCall(0x5EEF9C, patch_LoadProgressTexDictLoad);
		CPatch::RedirectCall(0x5EEBBF, CRenderer_DrawQuad2d_Menu);

		// CFEP_SceneSelection
		CPatch::Nop(0x601AAF, 0x601AB7-0x601AAF);
		CPatch::RedirectCall(0x601AAF, patch_CFEP_SceneSelection__Draw);
		CPatch::Nop(0x601A6D, 0x601A75-0x601A6D);
		CPatch::RedirectCall(0x601A6D, patch_CFEP_SceneSelection__Draw);

		// CFEP_SettingsAudio
		CPatch::SetPointer(0x602E86+2, &fAudioX);

		// CFEP_SettingsVideo
		CPatch::SetPointer(0x5DAA20 + 2, &CConfig::bWideScreen);
		CPatch::SetPointer(0x5DAA30 + 2, &CConfig::bWideScreen);
		CPatch::Nop(0x6032C6, 0x6032CE-0x6032C6);
		CPatch::RedirectCall(0x6032C6, patch_CFEP_SettingsVideo__Update);
		CPatch::RedirectJump(0x6037E5, patch_CFEP_SettingsVideo__Draw_1);
		CPatch::RedirectJump(0x603843, patch_CFEP_SettingsVideo__Draw_2);

		// CFEP_BonusFeatures
		CPatch::Nop(0x60440A, 0x604412-0x60440A);
		CPatch::RedirectCall(0x60440A, patch_CFEP_BonusFeatures__Draw);
		CPatch::Nop(0x60444F, 0x604457-0x60444F);
		CPatch::RedirectCall(0x60444F, patch_CFEP_BonusFeatures__Draw);

		// CFEP_BonusPicture__Draw
		CPatch::RedirectCall(0x5FE993, CRenderer_DrawQuad2d_Menu_Black);

		// CFEP_RemapCtrl
		CPatch::SetPointer(0x60AD9C+2, &fRemap5X);
		CPatch::SetPointer(0x60B962+2, &fRemap9X);

		// CFEP_InGame, CFrontendMenu::LevelGoalsDraw
		CPatch::SetPointer(0x5D9152+2, &fGoalW);

		// CFrontendMenu, Mouse
		CPatch::SetPointer(0x5DADB6 + 2, &fOne);
		CPatch::RedirectJump(0x5DB0B2, patch_CFrontendMenu__MouseDraw);
		CPatch::Nop(0x5DB02A, 0x5DB030-0x5DB02A);
		CPatch::RedirectCall(0x5DB02A, patch_CFrontendMenu__MouseUpdate);
	}

	return TRUE;
}