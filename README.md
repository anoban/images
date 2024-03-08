# ___BMP___

__Original__

![Original](./images/Elsa/elsa.bmp)

```C
ToBWhite(&image, LUMINOSITY, false);
```
![Elsa](./images/Elsa/elsa_lum.bmp)

```C
ToBWhite(&image, AVERAGE, false);
```
![Elsa](./images/Elsa/elsa_average.bmp)

```C
ToBWhite(&image, WEIGHTED_AVERAGE, false);
```
![Elsa](./images/Elsa/elsa_waverage.bmp)

```C
ToBWhite(&image, BINARY, false);
```
![Elsa](./images/Elsa/elsa_bin.bmp)


```C
RemoveColour(&image, RED, false);
```
![Elsa](./images/Elsa/elsa_bg.bmp)

```C
RemoveColour(&image, BLUE, false);
```
![Elsa](./images/Elsa/elsa_rg.bmp)

```C
RemoveColour(&image, GREEN, false);
```
![Elsa](./images/Elsa/elsa_br.bmp)

```C
RemoveColour(&image, GREENBLUE, false);
```
![Elsa](./images/Elsa/elsa_r.bmp)

```C
RemoveColour(&image, REDBLUE, false);
```
![Elsa](./images/Elsa/elsa_g.bmp)

```C
RemoveColour(&image, REDGREEN, false);
```
![Elsa](./images/Elsa/elsa_b.bmp)

----------------
___References:___

- `Compressed Image File Formats: JPEG, PNG, GIF, XBM, BMP - John Miano (1999) ACM Press/Addison-Wesley Publishing Co`.

- `PNG: The Definitive Guide. - Greg Roelofs and Richard Koman. (1999) O'Reilly Associates, Inc., USA`.

- `The Data Compression Book, Second Edition, Mark Nelson and Jean-Loop Gailly. 1995. M&T Books, New York, NY`

- https://koushtav.me/jpeg/tutorial/2017/11/25/lets-write-a-simple-jpeg-library-part-1/

- https://koushtav.me/jpeg/tutorial/c++/decoder/2019/03/02/lets-write-a-simple-jpeg-library-part-2/

----------------
