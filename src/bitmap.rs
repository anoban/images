pub mod bitmap {

    pub struct Bitmap {
        /*

        unsigned char*   buffer; // this will point to the original file buffer, this is the one that needs deallocation!
                RGBQUAD*         pixels; // this points to the start of pixels in the file buffer i.e offset buffer + 54
                BITMAPFILEHEADER file_header;
                BITMAPINFOHEADER info_header;
                unsigned long    file_size;   // REDUNDANT BECAUSE BITMAPFILEHEADER::bfSize STORES THE SAME INFO BUT NECESSARY
                unsigned long    buffer_size;

         */
    }

    impl Bitmap {
        fn new(&mut self, height: i32, width: i32) {}
    }
}
