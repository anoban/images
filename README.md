### _Usage:_
--------

```C++
    #include <canvas> // a class that uses the bitmap class as base
    // below we read in the bitmap from disk as a canvas object
    canvas image {LR"(./image.bmp)"};
```

<img src="./images/image.jpg" width=300 height=300>

---------

```C++
    // .copy() returns a deep copy so that the original stays as is during the transformation

    image.copy(). // specify a preferred RGB to black and white mapping method
    to_blacknwhite<rgb::BW_TRANSFORMATION::AVERAGE>().
    to_file(LR"(./average.bmp)");
```

<img src="./images/average.jpg" width=300 height=300>

```C++
    image.copy().to_blacknwhite<rgb::BW_TRANSFORMATION::WEIGHTED_AVERAGE>().
    to_file(LR"(./weighted_average.bmp)");
```

<img src="./images/weighted_average.jpg" width=300 height=300>

```C++
    image.copy().to_blacknwhite<rgb::BW_TRANSFORMATION::BINARY>().
    to_file(LR"(./binary.bmp)");
```

<img src="./images/binary.jpg" width=300 height=300>

```C++
    image.copy().to_blacknwhite<rgb::BW_TRANSFORMATION::LUMINOSITY>().
    to_file(LR"(./luminosity.bmp)");
```

<img src="./images/luminosity.jpg" width=300 height=300>

---------

```C++
    image.copy().remove_colour<rgb::RGB_TAG::BLUE>().to_file(LR"(.\redgreen.bmp)"); // remove blue
```

<img src="./images/redgreen.jpg" width=300 height=300>

```C++
    image.copy().remove_colour<rgb::RGB_TAG::RED>().to_file(LR"(.\bluegreen.bmp)"); // remove red
```

<img src="./images/bluegreen.jpg" width=300 height=300>

```C++
    image.copy().remove_colour<rgb::RGB_TAG::GREEN>().to_file(LR"(.\redblue.bmp)"); // remove green
```

<img src="./images/redblue.jpg" width=300 height=300>

```C++
    image.copy().remove_colour<rgb::RGB_TAG::GREENBLUE>(). // remove green & blue
    to_file(LR"(.\red.bmp)");
```

<img src="./images/red.jpg" width=300 height=300>

```C++
    image.copy().remove_colour<rgb::RGB_TAG::REDGREEN>(). // remove red & green
    to_file(LR"(.\blue.bmp)");
```

<img src="./images/blue.jpg" width=300 height=300>

```C++
    image.copy().remove_colour<rgb::RGB_TAG::REDBLUE>(). // remove red & blue
    to_file(LR"(.\green.bmp)");
```

<img src="./images/green.jpg" width=300 height=300>

### _Warning:_
--------

Owing to the non-opt-in use of `SSSE3`, `AVX2` and `AVX512` compiler (`MSVC` & `LLVM`) intrinsics, If compiles, will probably raise an illegal instruction hardware exception at runtime on unsupported CPU architectures (anything other than `AMD64`). Unfortunately my expertise is very Windows centric hence, I have no desire to accommodate the `linux/g++` toolchain in this project.


### _Reference:_
--------

Compressed Image File Formats: JPEG, PNG, GIF, XBM, BMP - John Miano (1999) ACM Press/Addison-Wesley Publishing Co.
