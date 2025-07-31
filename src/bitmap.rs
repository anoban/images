pub mod bitmap {
    use std::vec::Vec;

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
        pub blue: u8,     // BYTE rgbBlue;
        pub green: u8,    // BYTE rgbGreen;
        pub red: u8,      // BYTE rgbRed;
        pub reserved: u8, // BYTE rgbReserved;
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
        #[inline(always)]
        const fn get_scanline_order(&self) -> ScanlineOrder {
            return if self.height >= 0 {
                ScanlineOrder::BOTTOMUP // BitmapInfoHeader::height is usually an unsigned value, indicating the customary bottom up order
            } else {
                ScanlineOrder::TOPDOWN // a negative value indicates that the scanlines are ordered top down
            };
        }
    }

    pub struct Bitmap {
        /*

        unsigned char*   buffer; // this will point to the original file buffer, this is the one that needs deallocation!
                RGBQUAD*         pixels; // this points to the start of pixels in the file buffer i.e offset buffer + 54
                BITMAPFILEHEADER file_header;
                BITMAPINFOHEADER info_header;
                unsigned long    file_size;   // REDUNDANT BECAUSE BITMAPFILEHEADER::bfSize STORES THE SAME INFO BUT NECESSARY
                unsigned long    buffer_size;

         */
        pixels: Vec<RgbQuad>,
        file_header: BitmapFileHeader,
        info_header: BitmapInfoHeader,
        filesize: usize,
        buffersize: usize,
    }

    impl Bitmap {
        fn new(&mut self, height: i32, width: i32) {}

        fn height(&self) -> i32 {
            self.info_header.height
        }

        fn width(&self) -> i32 {
            self.info_header.width
        }
    }
}
