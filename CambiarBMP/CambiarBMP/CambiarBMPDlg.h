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
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnBnClickedBtnAbrir();
    afx_msg void OnBnClickedBtnGuardar();
    afx_msg void OnBnClickedBtnElegirColor();
    afx_msg void OnBnClickedBtnAplicar();
    DECLARE_MESSAGE_MAP()

private:
    void CargarImagen(const CString& ruta);
    void LiberarImagenActual();
    void ActualizarSeleccion(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
    bool PixelDesdePunto(CPoint point, int32_t& x, int32_t& y) const;
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

    // Seleccion actual: rectangulo de pixeles [m_selX1..m_selX2] x [m_selY1..m_selY2]
    // (ambos extremos incluidos, m_selX1<=m_selX2, m_selY1<=m_selY2). Un solo pixel
    // clicado es un rectangulo de 1x1.
    int32_t m_selX1, m_selY1, m_selX2, m_selY2;
    bool m_haySeleccion;

    // Arrastre del mouse en curso (para seleccionar un rango).
    bool m_arrastrando;
    int32_t m_arrastreOrigenX, m_arrastreOrigenY;

    // Presentacion del canvas: m_rcCanvasArea es el espacio disponible (fijo, calculado
    // una vez a partir de los controles vecinos); m_rcCanvas es el rectangulo real que
    // ocupa la cuadricula de la imagen cargada (ancho*zoom x alto*zoom) dentro de esa area.
    CRect m_rcCanvasArea;
    CRect m_rcCanvas;
    int32_t m_zoom;
};
