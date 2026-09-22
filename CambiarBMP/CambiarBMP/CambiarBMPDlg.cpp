// CambiarBMPDlg.cpp
#include "CambiarBMPDlg.h"
#include <cstdlib>

namespace {
    constexpr int32_t ZOOM_MINIMO = 1;
    constexpr int32_t ZOOM_MAXIMO = 40;
}

BEGIN_MESSAGE_MAP(CCambiarBMPDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_BN_CLICKED(IDC_BTN_ABRIR, &CCambiarBMPDlg::OnBnClickedBtnAbrir)
    ON_BN_CLICKED(IDC_BTN_GUARDAR, &CCambiarBMPDlg::OnBnClickedBtnGuardar)
    ON_BN_CLICKED(IDC_BTN_ELEGIR_COLOR, &CCambiarBMPDlg::OnBnClickedBtnElegirColor)
    ON_BN_CLICKED(IDC_BTN_APLICAR, &CCambiarBMPDlg::OnBnClickedBtnAplicar)
END_MESSAGE_MAP()

CCambiarBMPDlg::CCambiarBMPDlg(CWnd* pParent)
    : CDialogEx(IDD_CAMBIARBMP_DIALOG, pParent),
      m_buffer(nullptr), m_tamanio(0), m_fileHeader(nullptr), m_infoHeader(nullptr),
      m_pixelX(-1), m_pixelY(-1), m_haySeleccion(false),
      m_zoom(ZOOM_MINIMO), m_rcCanvas(0, 0, 0, 0) {
}

CCambiarBMPDlg::~CCambiarBMPDlg() {
    LiberarImagenActual();
}

void CCambiarBMPDlg::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
}

BOOL CCambiarBMPDlg::OnInitDialog() {
    CDialogEx::OnInitDialog();

    // El area del canvas se calcula a partir de los controles vecinos ya creados,
    // en vez de adivinar coordenadas: asi no depende de la conversion DLU->pixeles.
    CRect rcLabel, rcGroup, rcClient;
    GetDlgItem(IDC_STATIC_CANVAS_LABEL)->GetWindowRect(&rcLabel);
    ScreenToClient(&rcLabel);
    GetDlgItem(IDC_GROUP_PIXEL)->GetWindowRect(&rcGroup);
    ScreenToClient(&rcGroup);
    GetClientRect(&rcClient);

    m_rcCanvasArea = CRect(rcLabel.left, rcLabel.bottom + 6, rcGroup.left - 10, rcClient.bottom - 10);

    HabilitarPanelPixel(FALSE);

    return TRUE;
}

void CCambiarBMPDlg::LiberarImagenActual() {
    if (m_buffer != nullptr) {
        free(m_buffer);
        m_buffer = nullptr;
    }
    m_fileHeader = nullptr;
    m_infoHeader = nullptr;
    m_tamanio = 0;
}

void CCambiarBMPDlg::RecalcularZoom() {
    if (m_buffer == nullptr) {
        m_zoom = ZOOM_MINIMO;
        m_rcCanvas = CRect(m_rcCanvasArea.left, m_rcCanvasArea.top, m_rcCanvasArea.left, m_rcCanvasArea.top);
        return;
    }

    int32_t ancho = m_infoHeader->biWidth;
    int32_t alto = m_infoHeader->biHeight < 0 ? -m_infoHeader->biHeight : m_infoHeader->biHeight;

    int32_t zoomAncho = ancho > 0 ? m_rcCanvasArea.Width() / ancho : ZOOM_MINIMO;
    int32_t zoomAlto = alto > 0 ? m_rcCanvasArea.Height() / alto : ZOOM_MINIMO;
    m_zoom = zoomAncho < zoomAlto ? zoomAncho : zoomAlto;
    if (m_zoom < ZOOM_MINIMO) m_zoom = ZOOM_MINIMO;
    if (m_zoom > ZOOM_MAXIMO) m_zoom = ZOOM_MAXIMO;

    m_rcCanvas = CRect(m_rcCanvasArea.left, m_rcCanvasArea.top,
                        m_rcCanvasArea.left + ancho * m_zoom, m_rcCanvasArea.top + alto * m_zoom);
}

CRect CCambiarBMPDlg::RectCanvasDePixel(int32_t x, int32_t y) const {
    int32_t left = m_rcCanvas.left + x * m_zoom;
    int32_t top = m_rcCanvas.top + y * m_zoom;
    return CRect(left, top, left + m_zoom, top + m_zoom);
}

void CCambiarBMPDlg::OnPaint() {
    CPaintDC dc(this);
    if (m_buffer == nullptr) return;

    int32_t ancho = m_infoHeader->biWidth;
    int32_t alto = m_infoHeader->biHeight < 0 ? -m_infoHeader->biHeight : m_infoHeader->biHeight;

    for (int32_t y = 0; y < alto; y++) {
        for (int32_t x = 0; x < ancho; x++) {
            uint8_t* p = ObtenerPixelBMP(m_buffer, m_infoHeader, m_fileHeader->bfOffBits, x, y);
            COLORREF color = RGB(p[2], p[1], p[0]); // el BMP guarda B,G,R
            dc.FillSolidRect(RectCanvasDePixel(x, y), color);
        }
    }

    if (m_haySeleccion) {
        CRect rcSel = RectCanvasDePixel(m_pixelX, m_pixelY);
        CBrush brNegro(RGB(0, 0, 0));
        dc.FrameRect(rcSel, &brNegro);
        rcSel.InflateRect(1, 1);
        CBrush brBlanco(RGB(255, 255, 255));
        dc.FrameRect(rcSel, &brBlanco);
    }
}

void CCambiarBMPDlg::OnLButtonDown(UINT nFlags, CPoint point) {
    if (m_buffer != nullptr && m_rcCanvas.PtInRect(point)) {
        int32_t x = (point.x - m_rcCanvas.left) / m_zoom;
        int32_t y = (point.y - m_rcCanvas.top) / m_zoom;
        int32_t ancho = m_infoHeader->biWidth;
        int32_t alto = m_infoHeader->biHeight < 0 ? -m_infoHeader->biHeight : m_infoHeader->biHeight;
        if (x >= 0 && x < ancho && y >= 0 && y < alto) {
            SeleccionarPixel(x, y);
        }
    }
    CDialogEx::OnLButtonDown(nFlags, point);
}

void CCambiarBMPDlg::SeleccionarPixel(int32_t x, int32_t y) {
    m_pixelX = x;
    m_pixelY = y;
    m_haySeleccion = true;
    HabilitarPanelPixel(TRUE);
    ActualizarPanelPixel();
    Invalidate();
}

void CCambiarBMPDlg::ActualizarPanelPixel() {
    if (!m_haySeleccion || m_buffer == nullptr) return;

    uint8_t* p = ObtenerPixelBMP(m_buffer, m_infoHeader, m_fileHeader->bfOffBits, m_pixelX, m_pixelY);
    uint8_t b = p[0], g = p[1], r = p[2];

    CString texto;
    texto.Format(_T("Pixel: (%d, %d)"), m_pixelX, m_pixelY);
    SetDlgItemText(IDC_STATIC_COORD, texto);

    CString colorTexto;
    colorTexto.Format(_T("Color actual:\r\nR=%d  G=%d  B=%d"), r, g, b);
    SetDlgItemText(IDC_STATIC_COLOR_ACTUAL, colorTexto);

    SetDlgItemInt(IDC_EDIT_R, r, FALSE);
    SetDlgItemInt(IDC_EDIT_G, g, FALSE);
    SetDlgItemInt(IDC_EDIT_B, b, FALSE);
}

void CCambiarBMPDlg::HabilitarPanelPixel(BOOL habilitar) {
    GetDlgItem(IDC_EDIT_R)->EnableWindow(habilitar);
    GetDlgItem(IDC_EDIT_G)->EnableWindow(habilitar);
    GetDlgItem(IDC_EDIT_B)->EnableWindow(habilitar);
    GetDlgItem(IDC_BTN_ELEGIR_COLOR)->EnableWindow(habilitar);
    GetDlgItem(IDC_BTN_APLICAR)->EnableWindow(habilitar);
}

void CCambiarBMPDlg::CargarImagen(const CString& ruta) {
    uint8_t* nuevoBuffer;
    int64_t nuevoTamanio;
    CabeceraArchivoBMP* nuevoFileHeader;
    CabeceraInfoBMP* nuevoInfoHeader;

    ResultadoCargaBMP resultado = CargarArchivoBMP(ruta.GetString(), &nuevoBuffer, &nuevoTamanio,
                                                    &nuevoFileHeader, &nuevoInfoHeader);
    if (resultado != ResultadoCargaBMP::Ok) {
        CString mensaje;
        switch (resultado) {
        case ResultadoCargaBMP::NoSePudoAbrir:
            mensaje = _T("No se pudo abrir el archivo.");
            break;
        case ResultadoCargaBMP::ArchivoDemasiadoPequenio:
            mensaje = _T("El archivo es demasiado pequenio para ser un BMP valido.");
            break;
        case ResultadoCargaBMP::FirmaInvalida:
            mensaje = _T("El archivo no tiene la firma BMP valida.");
            break;
        case ResultadoCargaBMP::BitsPorPixelNoSoportado:
            mensaje = _T("Este programa solo soporta BMP de 24 bits por pixel.");
            break;
        case ResultadoCargaBMP::ComprimidoNoSoportado:
            mensaje = _T("Este programa solo soporta BMP sin compresion.");
            break;
        default:
            mensaje = _T("No se pudo cargar el archivo.");
            break;
        }
        AfxMessageBox(mensaje, MB_ICONERROR);
        return;
    }

    LiberarImagenActual();
    m_buffer = nuevoBuffer;
    m_tamanio = nuevoTamanio;
    m_fileHeader = nuevoFileHeader;
    m_infoHeader = nuevoInfoHeader;
    m_rutaArchivo = ruta;

    m_haySeleccion = false;
    HabilitarPanelPixel(FALSE);
    SetDlgItemText(IDC_STATIC_COORD, _T("Ningun pixel seleccionado"));
    SetDlgItemText(IDC_STATIC_COLOR_ACTUAL, _T("Color actual: -"));
    SetDlgItemText(IDC_EDIT_R, _T(""));
    SetDlgItemText(IDC_EDIT_G, _T(""));
    SetDlgItemText(IDC_EDIT_B, _T(""));

    RecalcularZoom();

    GetDlgItem(IDC_BTN_GUARDAR)->EnableWindow(TRUE);
    SetDlgItemText(IDC_STATIC_ARCHIVO, ruta.Mid(ruta.ReverseFind(_T('\\')) + 1));

    Invalidate();
}

void CCambiarBMPDlg::OnBnClickedBtnAbrir() {
    CFileDialog dlg(TRUE, _T("bmp"), nullptr, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
                     _T("Archivos BMP (*.bmp)|*.bmp|Todos los archivos (*.*)|*.*||"), this);
    if (dlg.DoModal() == IDOK) {
        CargarImagen(dlg.GetPathName());
    }
}

void CCambiarBMPDlg::OnBnClickedBtnGuardar() {
    if (m_buffer == nullptr) return;

    if (GuardarArchivoBMP(m_rutaArchivo.GetString(), m_buffer, m_tamanio)) {
        AfxMessageBox(_T("Cambios guardados."), MB_ICONINFORMATION);
    } else {
        AfxMessageBox(_T("No se pudo guardar el archivo."), MB_ICONERROR);
    }
}

void CCambiarBMPDlg::OnBnClickedBtnElegirColor() {
    if (!m_haySeleccion) return;

    int r = GetDlgItemInt(IDC_EDIT_R);
    int g = GetDlgItemInt(IDC_EDIT_G);
    int b = GetDlgItemInt(IDC_EDIT_B);

    CColorDialog dlgColor(RGB(r, g, b), CC_FULLOPEN, this);
    if (dlgColor.DoModal() == IDOK) {
        COLORREF c = dlgColor.GetColor();
        SetDlgItemInt(IDC_EDIT_R, GetRValue(c), FALSE);
        SetDlgItemInt(IDC_EDIT_G, GetGValue(c), FALSE);
        SetDlgItemInt(IDC_EDIT_B, GetBValue(c), FALSE);
    }
}

void CCambiarBMPDlg::OnBnClickedBtnAplicar() {
    if (!m_haySeleccion || m_buffer == nullptr) return;

    int r = GetDlgItemInt(IDC_EDIT_R);
    int g = GetDlgItemInt(IDC_EDIT_G);
    int b = GetDlgItemInt(IDC_EDIT_B);
    if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
        AfxMessageBox(_T("Los valores de color deben estar entre 0 y 255."), MB_ICONWARNING);
        return;
    }

    CambiarPixelBMP(m_buffer, m_infoHeader, m_fileHeader->bfOffBits, m_pixelX, m_pixelY,
                     (uint8_t)r, (uint8_t)g, (uint8_t)b);
    ActualizarPanelPixel();
    Invalidate();
}
