////////////////////////////////////////////////////////////////
// CFlatToolBar 1997 Microsoft Systems Journal.
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
// This code compiles with Visual C++ 5.0 on Windows 95
//

#ifndef FLATBAR_H__
#define FLATBAR_H__

#ifndef TBSTYLE_FLAT
#define TBSTYLE_FLAT 0x0800	// (in case you don't have the new commctrl.h)
#endif

//////////////////
// "Flat" style tool bar. Use instead of CToolBar in your CMainFrame
// or other window to create a tool bar with the flat look.
//
// CFlatToolBar fixes the display bug described in the article. It also has
// overridden load functions that modify the style to TBSTYLE_FLAT. If you
// don't create your toolbar by loading it from a resource, you should call
// ModifyStyle(0, TBSTYLE_FLAT) yourself.
//
class CFlatToolBar : public wxToolBar //CToolBar
{
public:
    bool LoadToolBar(const char *resourceName);

    bool LoadToolBar(UINT resourceId)
    {
        //return LoadToolBar(MAKEINTRESOURCE(resourceId));
        return false;
    }

protected:
    virtual void OnUpdateCmdUI(wxFrame* target, bool disableIfNoHndler);

    //afx_msg void OnWindowPosChanging(LPWINDOWPOS lpWndPos);
    //afx_msg void OnWindowPosChanged(LPWINDOWPOS lpWndPos);
};

#endif /* FLATBAR_H__ */
