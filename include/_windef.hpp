#pragma once

extern "C" {

    // https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-rgbquad
    struct RGBQUAD {
            unsigned char rgbBlue;
            unsigned char rgbGreen;
            unsigned char rgbRed;
            unsigned char rgbReserved;
    };

    // https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader
    struct BITMAPINFOHEADER {
            unsigned       biSize;
            int            biWidth;
            int            biHeight;
            unsigned short biPlanes;
            unsigned short biBitCount;
            unsigned       biCompression;
            unsigned       biSizeImage;
            int            biXPelsPerMeter;
            int            biYPelsPerMeter;
            unsigned       biClrUsed;
            unsigned       biClrImportant;
    };

    // https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapfileheader

#pragma pack(push, 2) // this is critical here as this struct is supposed to be 14 bytes in size

    struct BITMAPFILEHEADER {
            unsigned short bfType;
            unsigned       bfSize;
            unsigned short bfReserved1;
            unsigned short bfReserved2;
            unsigned       bfOffBits;
    };

#pragma pack(pop)

    enum class Compression : unsigned char {
        BI_RGB       = 0x1,
        BI_RLE8      = 0x2,
        BI_RLE4      = 0x3,
        BI_BITFIELDS = 0x4,
        BI_JPEG      = 0x5,
        BI_PNG       = 0x6,
        BI_CMYK      = 0xB,
        BI_CMYKRLE8  = 0xC,
        BI_CMYKRLE4  = 0xD,
    };
}
