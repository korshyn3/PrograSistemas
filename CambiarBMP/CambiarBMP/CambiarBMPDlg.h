// CambiarBMPDlg.h
#pragma once

#ifndef __AFXWIN_H__
#include <afxwin.h>
#endif
#include <afxdialogex.h>
#include <afxdlgs.h>

#include "Resource.h"
#include "BmpUtils.h"

class CCambiarBMPDlg : public CDialogEx {
public:
    explicit CCambiarBMPDlg(CWnd* pParent = nullptr);
    virtual ~CCambiarBMPDlg();

    enum { IDD = IDD_CAMBIARBMP_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnBnClickedBtnAbrir();
    afx_msg void OnBnClickedBtnGuardar();
    afx_msg void OnBnClickedBtnElegirColor();
    afx_msg void OnBnClickedBtnAplicar();
    DECLARE_MESSAGE_MAP()

private:
    void CargarImagen(const CString& ruta);
    void LiberarImagenActual();
    void SeleccionarPixel(int32_t x, int32_t y);
    void ActualizarPanelPixel();
    void HabilitarPanelPixel(BOOL habilitar);
    void RecalcularZoom();
    CRect RectCanvasDePixel(int32_t x, int32_t y) const;

    // Datos de la imagen cargada (NULL si no hay ninguna).
    uint8_t* m_buffer;
    int64_t m_tamanio;
    CabeceraArchivoBMP* m_fileHeader;
    CabeceraInfoBMP* m_infoHeader;
    CString m_rutaArchivo;

    // Seleccion actual.
    int32_t m_pixelX;
    int32_t m_pixelY;
    bool m_haySeleccion;

    // Presentacion del canvas: m_rcCanvasArea es el espacio disponible (fijo, calculado
    // una vez a partir de los controles vecinos); m_rcCanvas es el rectangulo real que
    // ocupa la cuadricula de la imagen cargada (ancho*zoom x alto*zoom) dentro de esa area.
    CRect m_rcCanvasArea;
    CRect m_rcCanvas;
    int32_t m_zoom;
};
