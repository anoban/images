#include "bmp.hpp"

int wmain(_In_opt_ int32_t argc, _In_opt_count_(argc) wchar_t* argv[]) {
    const auto basenames { bmp::remove_ext(argv, argc, L".bmp") };
    size_t     bnamepos {};
    for (size_t i = 1; i < argc; ++i) {
        bnamepos = i - 1;
        try {
            auto img { bmp::bmp(argv[i]) };

            img.tobwhite(bmp::TOBWKIND::AVERAGE).value().serialize(basenames.at(bnamepos) + L"_BW_AVG.bmp");
            img.tobwhite(bmp::TOBWKIND::WEIGHTED_AVERAGE).value().serialize(basenames.at(bnamepos) + L"_BW_WAVG.bmp");
            img.tobwhite(bmp::TOBWKIND::LUMINOSITY).value().serialize(basenames.at(bnamepos) + L"_BW_LUM.bmp");
            img.tobwhite(bmp::TOBWKIND::BINARY).value().serialize(basenames.at(bnamepos) + L"_BW_BIN.bmp");

            img.tonegative().value().serialize(basenames.at(bnamepos) + L"_NEG.bmp");

            img.remove_clr(bmp::RGBCOMB::BLUE).value().serialize(basenames.at(bnamepos) + L"_GR.bmp");
            img.remove_clr(bmp::RGBCOMB::GREEN).value().serialize(basenames.at(bnamepos) + L"_BR.bmp");
            img.remove_clr(bmp::RGBCOMB::RED).value().serialize(basenames.at(bnamepos) + L"_GB.bmp");
            img.remove_clr(bmp::RGBCOMB::REDBLUE).value().serialize(basenames.at(bnamepos) + L"_G.bmp");
            img.remove_clr(bmp::RGBCOMB::REDGREEN).value().serialize(basenames.at(bnamepos) + L"_B.bmp");
            img.remove_clr(bmp::RGBCOMB::GREENBLUE).value().serialize(basenames.at(bnamepos) + L"_R.bmp");

        } catch (const std::exception& excpt) { ::fwprintf_s(stderr, L"%s: %S\n", argv[i], excpt.what()); }
    }
    return EXIT_SUCCESS;
}