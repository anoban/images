int wmain(_In_ int argc, _In_ wchar_t* argv[]) {
    if (argc < 2) {
        fputs("Error: no files passed, bmp.exe takes/needs one argument: [file name]", stderr);
        exit(1);
    }

    BMP image  = new_bmp(argv[1]);

    BMP bw_ave = to_blacknwhite(&image, AVERAGE, false);
    BMP bw_wav = to_blacknwhite(&image, WEIGHTED_AVERAGE, false);
    BMP bw_lum = to_blacknwhite(&image, LUMINOSITY, false);
    BMP bw_bin = to_blacknwhite(&image, BINARY, false);

    serialize_bmp(&bw_ave, L"./sydney_bw_ave.bmp");
    serialize_bmp(&bw_wav, L"./sydney_bw_wav.bmp");
    serialize_bmp(&bw_lum, L"./sydney_bw_lum.bmp");
    serialize_bmp(&bw_bin, L"./sydney_bw_bin.bmp");

    close_bmp(bw_ave);
    close_bmp(bw_wav);
    close_bmp(bw_lum);
    close_bmp(bw_bin);

    BMP nored   = remove_color(&image, REMRED, false);
    BMP nogreen = remove_color(&image, REMGREEN, false);
    BMP noblue  = remove_color(&image, REMBLUE, false);
    BMP norg    = remove_color(&image, REMRG, false);
    BMP norb    = remove_color(&image, REMRB, false);
    BMP nogb    = remove_color(&image, REMGB, false);

    serialize_bmp(&nored, L"./sydney_GB.bmp");
    serialize_bmp(&noblue, L"./sydney_RG.bmp");
    serialize_bmp(&nogreen, L"./sydney_RB.bmp");
    serialize_bmp(&nogb, L"./sydney_R.bmp");
    serialize_bmp(&norb, L"./sydney_G.bmp");
    serialize_bmp(&norg, L"./sydney_B.bmp");

    close_bmp(image);
    close_bmp(nored);
    close_bmp(noblue);
    close_bmp(nogreen);
    close_bmp(nogb);
    close_bmp(norb);
    close_bmp(norg);

    return EXIT_SUCCESS;
}