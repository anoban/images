# _Bitmaps_
---------

```C
const bmp_t image = BmpRead(L"./images/Elsa/elsa.bmp");
```
 <img src="./images/Elsa/elsa.jpeg">

## _Black and white transformations_
-----------

```C
ToBWhite(&image, LUMINOSITY, false);                                   ToBWhite(&image, AVERAGE, false);
```

<div>
<img src="./images/Elsa/elsa_lum.jpeg" width=45%>
<img src="./images/Elsa/elsa_average.jpeg" width=45%>
</div>

```C
ToBWhite(&image, WEIGHTED_AVERAGE, false);                             ToBWhite(&image, BINARY, false);
```

<div>
<img src="./images/Elsa/elsa_waverage.jpeg" width=45%>
<img src="./images/Elsa/elsa_bin.jpeg" width=45%>
</div>


```C
RemoveColour(&image, RED, false);
```
![Elsa](./images/Elsa/elsa_bg.jpeg)

```C
RemoveColour(&image, BLUE, false);
```
![Elsa](./images/Elsa/elsa_rg.jpeg)

```C
RemoveColour(&image, GREEN, false);
```
![Elsa](./images/Elsa/elsa_br.jpeg)

```C
RemoveColour(&image, GREENBLUE, false);
```
![Elsa](./images/Elsa/elsa_r.jpeg)

```C
RemoveColour(&image, REDBLUE, false);
```
![Elsa](./images/Elsa/elsa_g.jpeg)

```C
RemoveColour(&image, REDGREEN, false);
```
![Elsa](./images/Elsa/elsa_b.jpeg)

```C
ToNegative(&image, true);
```
![Elsa](./images/Elsa/elsa_neg.jpeg)

----------------
___References:___

- `Compressed Image File Formats: JPEG, PNG, GIF, XBM, BMP - John Miano (1999) ACM Press/Addison-Wesley Publishing Co`.

- `PNG: The Definitive Guide. - Greg Roelofs and Richard Koman. (1999) O'Reilly Associates, Inc., USA`.

- `The Data Compression Book, Second Edition, Mark Nelson and Jean-Loop Gailly. 1995. M&T Books, New York, NY`

- https://koushtav.me/jpeg/tutorial/2017/11/25/lets-write-a-simple-jpeg-library-part-1/

- https://koushtav.me/jpeg/tutorial/c++/decoder/2019/03/02/lets-write-a-simple-jpeg-library-part-2/

----------------
