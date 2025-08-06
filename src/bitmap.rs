use std::{
    ops::{Index, IndexMut},
    vec::Vec,
};

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

// that's 'M' followed by a 'B' (LE), wingdi's BITMAPFILEHEADER uses an unsigned short for SOI instead of two chars
const SOI: u16 = ('B' as u16) | (('M' as u16) << 8); // Start Of Image

#[repr(C)]
#[allow(non_snake_case)]
pub struct RgbTriple {
    // typedef struct tagRGBTRIPLE RGBTRIPLE;
    rgbBlue: u8,  // BYTE rgbBlue;
    rgbGreen: u8, // BYTE rgbGreen;
    rgbRed: u8,   // BYTE rgbRed;
}

#[repr(C)]
#[allow(non_snake_case)]
pub struct RgbQuad {
    // typedef struct tagRGBQUAD RGBQUAD;
    pub rgbBlue: u8,     // BYTE rgbBlue;
    pub rgbGreen: u8,    // BYTE rgbGreen;
    pub rgbRed: u8,      // BYTE rgbRed;
    pub rgbReserved: u8, // BYTE rgbReserved;
}

#[repr(C)]
#[allow(non_snake_case)]
struct BitmapFileHeader {
    // typedef struct tagBITMAPFILEHEADER BITMAPFILEHEADER;
    bfType: u16,      // WORD  bfType;
    bfSize: u32,      // DWORD bfSize;
    bfReserved1: u16, // WORD  bfReserved1;
    bfReserved2: u16, // WORD  bfReserved2;
    bfOffBits: u32,   // DWORD bfOffBits;
}

#[repr(C)]
#[allow(non_snake_case)]
struct BitmapInfoHeader {
    // typedef struct tagBITMAPINFOHEADER BITMAPINFOHEADER;
    biSize: u32,          // DWORD biSize;
    biWidth: i32,         // LONG  biWidth;
    biHeight: i32,        // LONG  biHeight;
    biPlanes: u16,        // WORD  biPlanes;
    biBitCount: u16,      // WORD  biBitCount;
    biCompression: u32,   // DWORD biCompression;
    biSizeImage: u32,     // DWORD biSizeImage;
    biXPelsPerMeter: i32, // LONG  biXPelsPerMeter;
    biYPelsPerMeter: i32, // LONG  biYPelsPerMeter;
    biClrUsed: u32,       // DWORD biClrUsed;
    biClrImportant: u32,  // DWORD biClrImportant;
}

impl BitmapInfoHeader {
    #[inline(always)]
    const fn get_scanline_order(&self) -> ScanlineOrder {
        return if self.biHeight >= 0 {
            ScanlineOrder::BOTTOMUP // BitmapInfoHeader::height is usually an unsigned value, indicating the customary bottom up order
        } else {
            ScanlineOrder::TOPDOWN // a negative value indicates that the scanlines are ordered top down
        };
    }
}

#[repr(C)]
pub struct Bitmap {
    pixels: Vec<RgbQuad>,
    file_header: BitmapFileHeader,
    info_header: BitmapInfoHeader,
    buffersize: usize,
    filesize: usize,
}

impl Bitmap {
    fn new(height: i32, width: i32) -> Bitmap {
        Bitmap {
            pixels: Vec::<RgbQuad>::with_capacity(height as usize * width as usize),
            file_header: BitmapFileHeader {
                bfType: (),
                bfSize: (),
                bfReserved1: (),
                bfReserved2: (),
                bfOffBits: (),
            },
            info_header: BitmapInfoHeader {
                size: (),
                width: (),
                height: (),
                planes: (),
                bitcount: (),
                compression: (),
                image_size: (),
                pixels_per_meter_x: (),
                pixels_per_meter_y: (),
                colours_used: (),
                colours_important: (),
            },
            buffersize: width as usize * height as usize * size_of::<RgbQuad>(),
            filesize: 0,
        }
    }

    fn from_file(path: &str) -> Bitmap {}

    fn height(&self) -> i32 {
        self.info_header.height
    }

    fn width(&self) -> i32 {
        self.info_header.width
    }
}

impl Index<usize> for Bitmap {
    type Output = RgbQuad;
    fn index(&self, idx: usize) -> &Self::Output {
        &self.pixels[idx]
    }
}

impl IndexMut<usize> for Bitmap {
    fn index_mut(&mut self, idx: usize) -> &mut Self::Output {
        &mut self.pixels[idx]
    }
}

impl Iterator for Bitmap {}

mod tests {}
