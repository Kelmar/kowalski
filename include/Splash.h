#ifndef SPLASH_WND_H__
#define SPLASH_WND_H__

// Splash.h : header file
//

/////////////////////////////////////////////////////////////////////////////
//   Splash Screen class

class CSplashWnd : public wxWindow
{
protected:
    /* constructor */ CSplashWnd();

    // Attributes:
public:
    wxBitmap m_bitmap;
    //HANDLE m_hdib;

    // Operations
public:
    static void EnableSplashScreen(bool bEnable = true);
    static void ShowSplashScreen(wxWindow* pParentWnd = nullptr);
    //static void PreTranslateAppMessage(MSG* pMsg);

public:
    virtual ~CSplashWnd();
    //virtual void PostNcDestroy();

protected:
    bool Create(wxWindow* pParentWnd = nullptr);
    void HideSplashScreen();

    static bool s_showSplash;
    static CSplashWnd* s_splashWnd;

protected:
    //afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnPaint();
    afx_msg void OnTimer(UINT nIDEvent);
};

#endif /* SPLASH_WND_H__ */
