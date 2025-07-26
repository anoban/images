pub mod utilities {
    pub mod wingdi {
        // Wingdi data structures

        // order of pixels in the BMP buffer.
        #[repr(u8)]
        enum ScanlineOrder {
            TOPDOWN,
            BOTTOMUP,
        }

        // types of compressions used in BMP files.
        #[repr(u8)]
        enum CompressionKind {
            RGB,
            RLE8,
            RLE4,
            BITFIELDS,
            UNKNOWN,
        }

        // that's 'M' followed by a 'B' (LE), wingdi's BITMAPFILEHEADER uses a  unsigned short for SOI instead of two chars
        const SOI: u16 = ('B' as u16) | (('M' as u16) << 8); // Start Of Image

        #[repr(C)]
        pub struct RgbTriple {
            // typedef struct tagRGBTRIPLE RGBTRIPLE;
            blue: u8,  // BYTE rgbBlue;
            green: u8, // BYTE rgbGreen;
            red: u8,   // BYTE rgbRed;
        }

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
            // typedef struct tagBITMAPFILEHEADER BITMAPFILEHEADER;
            r#type: u16,      // WORD  bfType;
            size: u32,        // DWORD bfSize;
            reserved_00: u16, // WORD  bfReserved1;
            reserved_01: u16, // WORD  bfReserved2;
            offset_bits: u32, // DWORD bfOffBits;
        }

        #[repr(C)]
        struct BitmapInfoHeader {
            // typedef struct tagBITMAPINFOHEADER BITMAPINFOHEADER;
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

        impl BitmapInfoHeader {
            const fn get_scanline_order(&self) -> ScanlineOrder {
                // BitmapInfoHeader::height is usually an unsigned value, a negative value indicates that the scanlines are ordered top down instead of the customary bottom up order
                return if self.height >= 0 {
                    ScanlineOrder::BOTTOMUP
                } else {
                    ScanlineOrder::TOPDOWN
                };
            }
        }
    }
}
