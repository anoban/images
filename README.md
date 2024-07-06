# _Bitmaps_
---------

```C
const bmp_t image = BmpRead(L"./images/Elsa/elsa.bmp");
```
<img src="./images/Elsa/elsa.jpeg" width=60%>

### _Black and white transformations_

```C
ToBWhite(&image, LUMINOSITY, false);             ToBWhite(&image, AVERAGE, false);
```
<div>
<img src="./images/Elsa/elsa_lum.jpeg" width=45%>
<img src="./images/Elsa/elsa_average.jpeg" width=45%>
</div>

```C
ToBWhite(&image, WEIGHTED_AVERAGE, false);       ToBWhite(&image, BINARY, false);
```
<div>
<img src="./images/Elsa/elsa_waverage.jpeg" width=45%>
<img src="./images/Elsa/elsa_bin.jpeg" width=45%>
</div>

### _Colour removals_

```C
RemoveColour(&image, RED, false);                RemoveColour(&image, BLUE, false);
```
<div>
<img src="./images/Elsa/elsa_bg.jpeg" width=45%>
<img src="./images/Elsa/elsa_rg.jpeg" width=45%>
</div>

```C
RemoveColour(&image, GREEN, false);              RemoveColour(&image, GREENBLUE, false);
```
<div>
<img src="./images/Elsa/elsa_br.jpeg" width=45%>
<img src="./images/Elsa/elsa_r.jpeg" width=45%>
</div>

```C
RemoveColour(&image, REDBLUE, false);           RemoveColour(&image, REDGREEN, false);
```
<div>
<img src="./images/Elsa/elsa_g.jpeg" width=45%>
<img src="./images/Elsa/elsa_b.jpeg" width=45%>
</div>

### _Negative_

```C
ToNegative(&image, true);
```
<img src="./images/Elsa/elsa_neg.jpeg" width=60%>

----------------

### _References:_

- `Compressed Image File Formats: JPEG, PNG, GIF, XBM, BMP - John Miano (1999) ACM Press/Addison-Wesley Publishing Co`.

- `PNG: The Definitive Guide. - Greg Roelofs and Richard Koman. (1999) O'Reilly Associates, Inc., USA`.

- `The Data Compression Book, Second Edition, Mark Nelson and Jean-Loop Gailly. 1995. M&T Books, New York, NY`
