pub mod bitmap {


    struct RgbQuad{
        rgbBlue:u8;
        rgbGreen:u8;
        rgbRed:u8;
        rgbReserved:u8;
    };

    struct Bitmap {
        buffer:u8; // this will point to the original file buffer, this is the one that needs deallocation!
        RGBQUAD*         pixels; // this points to the start of pixels in the file buffer i.e offset buffer + 54
        BITMAPFILEHEADER file_header;
        BITMAPINFOHEADER info_header;
        unsigned long    file_size;   // REDUNDANT BECAUSE BITMAPFILEHEADER::bfSize STORES THE SAME INFO BUT NECESSARY
        unsigned long    buffer_size; // length of the buffer, may include trailing unused bytes if construction involved a buffer reuse

    }
}
