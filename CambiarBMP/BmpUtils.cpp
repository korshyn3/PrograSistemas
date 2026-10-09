// BmpUtils.cpp
#include "BmpUtils.h"
#include <cstdio>
#include <cstdlib>

ResultadoCargaBMP CargarArchivoBMP(const wchar_t* ruta, uint8_t** bufferSalida, int64_t* tamanioSalida,
                                   CabeceraArchivoBMP** fileHeaderSalida, CabeceraInfoBMP** infoHeaderSalida) {
    *bufferSalida = nullptr;
    *tamanioSalida = 0;
    *fileHeaderSalida = nullptr;
    *infoHeaderSalida = nullptr;

    FILE* f = _wfopen(ruta, L"rb");
    if (f == nullptr) return ResultadoCargaBMP::NoSePudoAbrir;

    fseek(f, 0, SEEK_END);
    int64_t tamanio = _ftelli64(f);
    fseek(f, 0, SEEK_SET);

    if ((size_t)tamanio < sizeof(CabeceraArchivoBMP) + sizeof(CabeceraInfoBMP)) {
        fclose(f);
        return ResultadoCargaBMP::ArchivoDemasiadoPequenio;
    }

    uint8_t* buffer = (uint8_t*)malloc((size_t)tamanio);
    if (buffer == nullptr) {
        fclose(f);
        return ResultadoCargaBMP::NoSePudoAbrir;
    }

    fread(buffer, 1, (size_t)tamanio, f);
    fclose(f);

    CabeceraArchivoBMP* fileHeader = (CabeceraArchivoBMP*)buffer;
    CabeceraInfoBMP* infoHeader = (CabeceraInfoBMP*)(buffer + sizeof(CabeceraArchivoBMP));

    if (fileHeader->bfType != 0x4D42) { // "BM"
        free(buffer);
        return ResultadoCargaBMP::FirmaInvalida;
    }
    if (infoHeader->biBitCount != 24) {
        free(buffer);
        return ResultadoCargaBMP::BitsPorPixelNoSoportado;
    }
    if (infoHeader->biCompression != 0) {
        free(buffer);
        return ResultadoCargaBMP::ComprimidoNoSoportado;
    }

    *bufferSalida = buffer;
    *tamanioSalida = tamanio;
    *fileHeaderSalida = fileHeader;
    *infoHeaderSalida = infoHeader;
    return ResultadoCargaBMP::Ok;
}

bool GuardarArchivoBMP(const wchar_t* ruta, uint8_t* buffer, int64_t tamanio) {
    FILE* f = _wfopen(ruta, L"wb");
    if (f == nullptr) return false;
    fwrite(buffer, 1, (size_t)tamanio, f);
    fclose(f);
    return true;
}

int32_t BytesPorFilaBMP(const CabeceraInfoBMP* info) {
    return ((info->biWidth * info->biBitCount + 31) / 32) * 4;
}

uint8_t* ObtenerPixelBMP(uint8_t* buffer, const CabeceraInfoBMP* info, uint32_t offBits, int32_t x, int32_t y) {
    int32_t alturaAbs = info->biHeight < 0 ? -info->biHeight : info->biHeight;
    int32_t topDown = info->biHeight < 0;
    int32_t filaArchivo = topDown ? y : (alturaAbs - 1 - y);
    int32_t fila = BytesPorFilaBMP(info);
    return buffer + offBits + (int64_t)filaArchivo * fila + (int64_t)x * 3;
}

void CambiarPixelBMP(uint8_t* buffer, const CabeceraInfoBMP* info, uint32_t offBits,
                      int32_t x, int32_t y, uint8_t r, uint8_t g, uint8_t b) {
    uint8_t* p = ObtenerPixelBMP(buffer, info, offBits, x, y);
    p[0] = b;
    p[1] = g;
    p[2] = r;
}
