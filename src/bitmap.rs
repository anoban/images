mod wingdi {
    // Wingdi data structures

    #[repr(C)]
    struct RgbQuad {
        // typedef struct tagRGBQUAD RGBQUAD;
        blue: u8,     // BYTE rgbBlue;
        green: u8,    // BYTE rgbGreen;
        red: u8,      // BYTE rgbRed;
        reserved: u8, // BYTE rgbReserved;
    }

    #[repr(C)]
    struct BitmapFileHeader {
        // typedef struct tagBITMAPFILEHEADER BITMAPFILEHEADER, *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;
        r#type: u16,      // WORD  bfType;
        size: u32,        // DWORD bfSize;
        reserved_00: u16, // WORD  bfReserved1;
        reserved_01: u16, // WORD  bfReserved2;
        offset_bits: u32, // DWORD bfOffBits;
    }
}

pub mod bitmap {
    pub struct BitMap {
        /*

        unsigned char*   buffer; // this will point to the original file buffer, this is the one that needs deallocation!
                RGBQUAD*         pixels; // this points to the start of pixels in the file buffer i.e offset buffer + 54
                BITMAPFILEHEADER file_header;
                BITMAPINFOHEADER info_header;
                unsigned long    file_size;   // REDUNDANT BECAUSE BITMAPFILEHEADER::bfSize STORES THE SAME INFO BUT NECESSARY
                unsigned long    buffer_size;

         */
    }
}
