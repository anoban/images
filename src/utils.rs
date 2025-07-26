mod utils {
    mod wingdi {
        // Wingdi data structures

        #[repr(C)]
        pub struct RgbQuad {
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

        #[repr(C)]
        struct BitmapInfoHeader {
            // typedef struct tagBITMAPINFOHEADER BITMAPINFOHEADER, *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;
            size: u32,               // DWORD biSize;
            width: i32,              // LONG  biWidth;
            height: i32,             // LONG  biHeight;
            planes: u16,             // WORD  biPlanes;
            bitcount: u16,           // WORD  biBitCount;
            compression: u32,        // DWORD biCompression;
            image_size: u32,         // DWORD biSizeImage;
            pixels_per_meter_x: i32, // LONG  biXPelsPerMeter;
            pixels_per_meter_y: i32, // LONG  biYPelsPerMeter;
            colours_used: u32,       // DWORD biClrUsed;
            colours_important: u32,  // DWORD biClrImportant;
        }
    }
}
