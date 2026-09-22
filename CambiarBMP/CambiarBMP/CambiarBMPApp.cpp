// CambiarBMPApp.cpp
#include "CambiarBMPApp.h"
#include "CambiarBMPDlg.h"

CCambiarBMPApp theApp;

BOOL CCambiarBMPApp::InitInstance() {
    CWinApp::InitInstance();

    CCambiarBMPDlg dlg;
    m_pMainWnd = &dlg;
    dlg.DoModal();

    // La app termina al cerrar el dialogo (no hay ventana principal que seguir bombeando).
    return FALSE;
}
