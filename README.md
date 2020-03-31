![Brailleplot](Brailleplot.png)

A simple colour graphics library for the Linux terminal using Braille dots as
pixels.

## Examples

Compiling the project with `make`creates two executables: `pam2braille` and
`bplot`.

### pam2braille

This program converts a portable anymap image file (pbm, pgm or ppm) into
black and white Braille art. For example, running

> `./pam2braille examples/B.pbm`

should produce the image of a large letter **B**:

![Image of a large B](examples/B.png)

### bplot

`bplot` is a very simple plotting program. It accepts ASCII data files (or
pipes) formatted by rows. Run `./bplot` for a usage message.

## Testing your terminal's colour modes

Run `make test` to create three executable programs to test the three colour
modes available in `brailleplot.h`. If the colour modes work properly on your
terminal, you should get the following outputs.

### colour_test_C16

![16 colours test screen](examples/colour_test_C16.png)

### colour_test_C56

![256 colours test screen](examples/colour_test_C256.png)

### colour_test_CTRUE

![True colour test screen](examples/colour_test_CTRUE.png)
