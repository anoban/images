pub mod bitmap {
    /*

    unsigned char*   buffer; // this will point to the original file buffer, this is the one that needs deallocation!
            RGBQUAD*         pixels; // this points to the start of pixels in the file buffer i.e offset buffer + 54
            BITMAPFILEHEADER file_header;
            BITMAPINFOHEADER info_header;
            unsigned long    file_size;   // REDUNDANT BECAUSE BITMAPFILEHEADER::bfSize STORES THE SAME INFO BUT NECESSARY
            unsigned long    buffer_size;

     */

    #[repr(C)]
    struct RgbQuad {
        /*
        typedef struct tagRGBQUAD {
          BYTE rgbBlue;
          BYTE rgbGreen;
          BYTE rgbRed;
          BYTE rgbReserved;
        } RGBQUAD;
        */
        blue: u8,
        green: u8,
        red: u8,
        reserved: u8,
    }

    #[repr(C)]
    struct BitmapFileHeader {
        /*
        typedef struct tagBITMAPFILEHEADER {
          WORD  bfType;
          DWORD bfSize;
          WORD  bfReserved1;
          WORD  bfReserved2;
          DWORD bfOffBits;
        } BITMAPFILEHEADER, *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;
        */
    }

    pub struct BitMap {}
}
