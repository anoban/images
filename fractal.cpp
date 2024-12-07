#include <canvas>
#include <chrono>
#include <cmaps>
#include <random>

static constexpr RGBQUAD            FRACTAL_FOREGROUND { .rgbBlue = 0x00, .rgbGreen = 0x00, .rgbRed = 0xFF, .rgbReserved = 0xFF };
// BELOW IS ABSOLUTELY CRITICAL NOT TO RUN INTO BUFFER OVERRUNS
static constexpr unsigned long long FRACTAL_MAX_ITERATIONS { colourmaps::CMAPSIZE };
static constexpr double             FRACTAL_EXPLODE_THRESHOLD { 4.0000 };

[[maybe_unused]] static void __cdecl waves(_Inout_ canvas& pane) noexcept {
    double red {}, green {}, blue {}; // NOLINT(readability-isolate-declaration)

    for (long row = 0; row < pane.height(); ++row) {
        for (long col = 0; col < pane.width(); ++col) {
            // minimum and maximum values returned by cos() are -1.000 and 1.000
            // if cos() returned -1.000, red will be 1.000, if cos() returned 1.000, red will be 255.000, so all cool
            red                                     = 128.0000 + 127.0000 * ::cos(0.02153 * row * col);
            green                                   = 128.0000 + 127.0000 * ::sin(0.01297 * row * col) - ::cos(0.1354 * col) / 2.0000;
            blue                                    = 128.0000 + 127.0000 * ::cos(0.1248 * row) + ::sin(0.1573 * row * col) / 2.0000;

            pane[row * pane.width() + col].rgbBlue  = static_cast<unsigned char>(blue);
            pane[row * pane.width() + col].rgbGreen = static_cast<unsigned char>(green);
            pane[row * pane.width() + col].rgbRed   = static_cast<unsigned char>(red);
        }
    }
}

// most of Wikipdia's fractal implementations have a lot of show stopping bugs, even though these are all inspired from Wikipedia's implementations,
// these contain numerous refactors to fix those bugs
// these fractal functions behave as if the scanlines in the pixel buffer are ordered top-down, since these are fractals, this is not much of an issue here

// look up https://en.wikipedia.org/wiki/Tricorn_(mathematics)
[[maybe_unused]] static void __cdecl tricorn(_Inout_ canvas& pane, _In_ const colourmaps::colourmap& cmap) noexcept {
    // NOLINTNEXTLINE(readability-isolate-declaration)
    double             scaled_x_coordinate {} /* (-2.5, 1) */, scaled_y_coordinate {} /* (-1, 1) */; // of the pixel
    ::complex<double>  scaled_coordinates {};

    double             xtemp {}, xsq {}, ysq {}; // NOLINT(readability-isolate-declaration)
    unsigned long long iterations {};

    // for each pixel in the image
    for (long row = 0; row < pane.height(); ++row) {
        for (long col = 0; col < pane.width(); ++col) {
            // dividing col by width gives a value (0, 1), multiplying that by 3.500 upscales to to (0, 3.5), then subtracting 2.5 gives us (-2.5, 1)
            scaled_x_coordinate    = col / static_cast<double>(pane.width()) * 3.50000 - 2.50000;
            // dividing row by height will give a value (0, 1), by multiplying by 2 we could stretch it to (0, 2), then by subtracting 1, we could shift the range to (-1, 1)
            scaled_y_coordinate    = row / static_cast<double>(pane.height()) * 2.00000 - 1.00000;

            scaled_coordinates.x() = scaled_x_coordinate;
            scaled_coordinates.y() = scaled_y_coordinate;

            iterations             = 0;

            for (xsq = scaled_coordinates.x() * scaled_coordinates.x(), ysq = scaled_coordinates.y() * scaled_coordinates.y();
                 xsq + ysq < 4.000 && iterations < FRACTAL_MAX_ITERATIONS;
                 ++iterations) {
                xtemp                  = xsq - ysq + scaled_x_coordinate;
                scaled_coordinates.y() = -2.000 * scaled_coordinates.x() * scaled_coordinates.y() + scaled_y_coordinate;
                scaled_coordinates.x() = xtemp;
            }

            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            pane[row * pane.width() + col] = (iterations == FRACTAL_MAX_ITERATIONS) ? FRACTAL_FOREGROUND : cmap.at(iterations);
        }
    }
}

// look up https://en.wikipedia.org/wiki/Julia_set
[[maybe_unused]] static void __cdecl julia(
    _Inout_ canvas& pane,
    _In_ const colourmaps::colourmap& cmap,
    _In_ const double& escape_radius // choose escape_radius > 0 such that escape_radius**2 - escape_radius >= sqrt(cx**2 + cy**2)
) noexcept {
    ::complex<double> scaled_coordinates {}; // scaled between (-escape_radius, escape_radius)

    const double      escrsq { escape_radius * escape_radius };

    unsigned long     iterations {};
    double            xtemp {};

    // for each pixel in the image
    for (long row = 0; row < pane.height(); ++row) {
        for (long col = 0; col < pane.width(); ++col) {
            // dividing col by width will scale the value (0, 1), multiplication by 2 shifts the scale to (0, 2)
            // subtracting 1 shifts to (-1, 1) and the ultimate multiplication by escape_radius will shift it to (-escape_radius, escape_radius)
            // same goes for the col too
            scaled_coordinates.x() = (col / static_cast<double>(pane.width()) * 2.000 - 1.0000) * escape_radius;
            scaled_coordinates.y() = (row / static_cast<double>(pane.height()) * 2.000 - 1.0000) * escape_radius;

            iterations             = 0;

            while (scaled_coordinates.x() * scaled_coordinates.x() + scaled_coordinates.y() * scaled_coordinates.y() < escrsq &&
                   ++iterations < FRACTAL_MAX_ITERATIONS) {
                //
                xtemp              = scaled_coordinates.x() * scaled_coordinates.x() - scaled_coordinates.y() * scaled_coordinates.y();
                scaled_coordinates = {};
            }
        }
    }
}

// lookup https://en.wikipedia.org/wiki/Julia_set
[[maybe_unused]] void __cdecl multijulia(
    _Inout_ canvas& pane,
    _In_ const colourmaps::colourmap& cmap,
    _In_ const double& escape_radius // choose escape_radius > 0 such that escape_radius**2 - escape_radius >= sqrt(cx**2 + cy**2)
) noexcept {
    ::complex<double> scaled_coordinates {}; // scaled between (-escape_radius, escape_radius)

    const auto        escaperadsq { escape_radius * escape_radius };

    unsigned long     iterations {};
    double            xtemp {}, xsq {}, ysq {}; // NOLINT(readability-isolate-declaration)

    // for each pixel in the image
    for (long row = 0; row < pane.height(); ++row) {
        for (long col = 0; col < pane.width(); ++col) {
            // dividing col by width will scale the value (0, 1), multiplication by 2 shifts the scale to (0, 2)
            // subtracting 1 shifts to (-1, 1) and the ultimate multiplication by escape_radius will shift it to (-escape_radius, escape_radius)
            // same goes for the col too
            scaled_coordinates = { (col / static_cast<double>(pane.width()) * 2.000 - 1.0000) * escape_radius,
                                   (row / static_cast<double>(pane.height()) * 2.000 - 1.0000) * escape_radius };

            iterations         = 0;
            xsq                = scaled_coordinates.x() * scaled_coordinates.x();
            ysq                = scaled_coordinates.y() * scaled_coordinates.y();

            while (xsq + ysq < escaperadsq && ++iterations < FRACTAL_MAX_ITERATIONS) {
                //
                xtemp              = xsq - ysq;
                scaled_coordinates = {};
            }
        }
    }
}

static void mandelbrot_wikipedia(_Inout_ canvas& pane, _In_ const colourmaps::colourmap& cmap) noexcept {
    ::complex<long double> scaled {};
    unsigned long long     niterations {};
    long double            x {}, xtemp {}, y {};

    for (long row = 0; row < pane.height(); ++row) {
        for (long col = 0; col < pane.width(); ++col) {
            scaled.x() = col / static_cast<long double>(pane.width()) * 2.4700 - 2.0000;
            scaled.y() = (row / static_cast<long double>(pane.height()) - 0.5000) * 2.2400;

            x = y       = 0.0000;
            niterations = 0;

            while (niterations < FRACTAL_MAX_ITERATIONS - 1 && (x * x + y * y) <= 4.0000) {
                xtemp = x * x - y * y + scaled.x();
                y     = 2 * x * y + scaled.y();
                x     = xtemp;
                niterations++;
            }

            // ::wprintf_s(L"%10llu\n", niterations);
            pane[row * pane.width() + col] = cmap.at(niterations);
        }
    }
}

// look up https://en.wikipedia.org/wiki/Mandelbrot_set
[[maybe_unused]] static void __cdecl mandelbrot(_Inout_ canvas& pane, _In_ const colourmaps::colourmap& cmap) noexcept {
    ::complex<double>  scaled_coords {}, coords {}; // NOLINT(readability-isolate-declaration)
    double             xtemp {};                    // NOLINT(readability-isolate-declaration)
    unsigned long long niterations {};

    // for each pixel in the image
    for (long row = 0; row < pane.height(); ++row) {
        for (long col = 0; col < pane.width(); ++col) {
            // Mandelbrot space is defined by a horizontal range (-2.00, 0.47) and a vertical range (-1.12, 1.12)
            // dividing col by width gives a value (0, 1), multiplying that by 2.4700 upscales to to (0, 2.4700), then subtracting 2.000 gives us (-2.000, 0.4700)
            scaled_coords.x() = col / static_cast<double>(pane.width()) * 2.47000 - 2.0000;
            // dividing row by height will give a value (0, 1), subtracting 0.500 shifts it to (-0.5, 0.5) then by multiplying by 2.2400 we could stretch it to (-1.1200, 1.1200)
            scaled_coords.y() = (row / static_cast<double>(pane.height()) - 0.5000) * 2.24000;

            niterations       = 0;
            coords.x() = coords.y() = 0.0000;

            // Mandelbrot equation :: Z = Z^2 + C
            // throughout the loop Z is iteratively updated while C is updated only once before this loop begins
            // at some point in the loop Z may undergo an explosion, which is defined as the sums of squares of Z's x and y coordinates surpassing a predefined threshold value
            // the selection of colour down the road depends on whcih happens first, whether the loop reaching maximum number of iterations or the explosion
            for (// BEWARE OF POTENTIAL SHORT CIRCUITS BELOW, REORDERING THE SUBEXPRESSIONS MAY LEAD TO UNWANTED RUNTIME BEHAVIOURS
                ; niterations < FRACTAL_MAX_ITERATIONS - 1 &&
                   coords.x() * coords.x() + coords.y() * coords.y() /* magnitude of Z */ < FRACTAL_EXPLODE_THRESHOLD;
                 ++niterations) {
                xtemp      = coords.x() * coords.x() - coords.y() * coords.y() + scaled_coords.x();
                // IT IS ABSOULUTELY CRITICAL THAT WHEN THE IMAGINARY PART (y) IS UPDATED, THE USED REAL PART (x) SHOULD BE IN THE ORIGINAL (PREVIOUS) STATE
                coords.y() = 2.000 * coords.x() * coords.y() + scaled_coords.y();
                coords.x() = xtemp; // THUS UPDATING THE REAL PART (x) AFTER THE UPDATE OF IMAGINARY PART (y)
            }

            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            pane[row * pane.width() + col] = cmap.at(niterations);
            // if the loop was exited because of an explosion
            // if the loop was exited because we reached the maximum iterations
        }
    }
}

int main() {
    std::mt19937_64 rand_engine { static_cast<unsigned long long>(std::chrono::high_resolution_clock::now().time_since_epoch().count()) };
    std::uniform_int_distribution<long> runif { 5000, 8000 };
    canvas                              board { runif(rand_engine), runif(rand_engine) };

    ::mandelbrot(board, colourmaps::JET);
    board.to_file(LR"(./mandelbrot_jet.bmp)");

    return EXIT_SUCCESS;
}
