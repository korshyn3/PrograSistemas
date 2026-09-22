// BmpUtils.h
// Lectura, escritura y acceso a pixeles de archivos BMP de 24 bits por pixel,
// sin compresion. Logica independiente de la interfaz (consola o MFC).

#pragma once

#include <cstdint>

#pragma pack(push, 1)
typedef struct {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} CabeceraArchivoBMP;

typedef struct {
    uint32_t biSize;
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} CabeceraInfoBMP;
#pragma pack(pop)

// Resultado de intentar cargar un BMP: por que fallo, si fallo.
enum class ResultadoCargaBMP {
    Ok,
    NoSePudoAbrir,
    ArchivoDemasiadoPequenio,
    FirmaInvalida,
    BitsPorPixelNoSoportado,
    ComprimidoNoSoportado
};

// Lee un archivo BMP completo a memoria y valida que sea de 24 bpp sin compresion.
// Si el resultado no es Ok, buffer queda en NULL y no hay que liberar nada.
ResultadoCargaBMP CargarArchivoBMP(const wchar_t* ruta, uint8_t** bufferSalida, int64_t* tamanioSalida,
                                   CabeceraArchivoBMP** fileHeaderSalida, CabeceraInfoBMP** infoHeaderSalida);

// Escribe el buffer completo (ya modificado en memoria) de vuelta al archivo.
bool GuardarArchivoBMP(const wchar_t* ruta, uint8_t* buffer, int64_t tamanio);

// Numero de bytes por fila en el archivo (cada fila BMP se rellena a multiplo de 4).
int32_t BytesPorFilaBMP(const CabeceraInfoBMP* info);

// Puntero a los 3 bytes (B, G, R) del pixel (x, y). (0,0) = esquina superior izquierda.
uint8_t* ObtenerPixelBMP(uint8_t* buffer, const CabeceraInfoBMP* info, uint32_t offBits, int32_t x, int32_t y);

// Cambia el color de un pixel en el buffer en memoria (no escribe a disco).
void CambiarPixelBMP(uint8_t* buffer, const CabeceraInfoBMP* info, uint32_t offBits,
                      int32_t x, int32_t y, uint8_t r, uint8_t g, uint8_t b);
