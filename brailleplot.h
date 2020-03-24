# ifndef __BRAILLEPLOT_H__
# define __BRAILLEPLOT_H__
# include <stdio.h>
# include <stdlib.h>
# include <locale.h>
# include <wchar.h>
# include <math.h>

/* Brailleplot mode (black and white, or different colour schemes) */
typedef enum {DEFAULT=0, C16, C256, CTRUE} brailleplot_mode;
# define red(color) ((color)>>16)&255
# define green(color) ((color)>>8)&255
# define blue(color) (color)&255

/* Get the closest match to an RGB colour from the palette */
int rgb_C256(int r, int g, int b) /* C256 mode */
{
  /* 0  -   7:  standard colors (as in \033 [ 30–37 m)
     8  -  15:  high intensity colors (as in \033 [ 90–97 m)
     16 - 231:  6 × 6 × 6 cube (216 colors): 16 + 36 × r + 6 × g + b (0 ≤ r, g, b ≤ 5)
     232- 255:  grayscale from black to white in 24 steps */
  if(r == g && r == b)
    return (int) (23.0*r/255) + 232;
  else
    return (int) round((5.0*r)/255)*36 + (int) round((5.0*g)/255)*6 + (int) round((5.0*b)/255) + 16;
}

int rgb(int r, int g, int b) /* CTRUE mode */
{
  return (r<<16) + (g<<8) + b;
}

/* Canvas structure */
typedef struct brailleplot_canvas {
  int width, height; /* Screen width and height in characters */
  int * screen; /* Raster pixels */
  int * text; /* Text buffer */
  int * colour; /* Foreground and background colours for every text character */
  brailleplot_mode mode; /* Graphics mode */
  int default_background; /* Default background colour */
  int default_foreground; /* Default foreground colour */
  int magicnum[8]; /* Eight magic numbers for the translation from pixels to Braille */
} brailleplot_canvas;

/* Bitmap structure */
struct brailleplot_bitmap {
  int width, height;
  int * pixels;
};

/* Initialise the drawing canvas */
void brailleplot_init(brailleplot_canvas * canvas, int width, int height, brailleplot_mode mode) {
  /* Initialise magic numbers */
  canvas->magicnum[7] = 6*width; canvas->magicnum[4] = 6*width + 1;
  canvas->magicnum[6] = 4*width; canvas->magicnum[3] = 4*width + 1;
  canvas->magicnum[5] = 2*width; canvas->magicnum[2] = 2*width + 1;
  canvas->magicnum[1] = 0;       canvas->magicnum[0] = 1;

  /* We will be using unicode Braille characters */
  setlocale(LC_ALL, "");

  /* Set the width and height of the screen and text buffers */
  canvas->width = width;
  canvas->height = height;

  /* Allocate memory to the buffers */
  canvas->screen = (int *) malloc(8*width*height*sizeof(int));
  canvas->text = (int *) malloc(((width + 1)*height + 1)*sizeof(int));

  /* Set colour mode */
  canvas->mode = mode;

  /* Allocate memory to the colour buffer */
  if(mode > 0) {
    canvas->colour = (int *) malloc(2*((width + 1)*height + 1)*sizeof(int));
    switch(mode) {
      case CTRUE:
        canvas->default_background = 0;
        canvas->default_foreground = 13158600;
        break;
      case C256:
        canvas->default_background = 16;
        canvas->default_foreground = 7;
        break;
      default:
        canvas->default_background = 9;
        canvas->default_foreground = 7;
    }
  }
}

/* Clear the screen and text buffers */
void brailleplot_cls(brailleplot_canvas * canvas) {
  int x, i;
  for(x = 0; x < 8*canvas->width*canvas->height; x++) canvas->screen[x] = 0;
  for(i = 0; i < (canvas->width + 1)*canvas->height; i++) canvas->text[i] = 0;
  if(canvas->mode > 0)
    for(i = 0; i < 2*(canvas->width + 1)*canvas->height; i += 2) {
      canvas->colour[i] = canvas->default_background;
      canvas->colour[i + 1] = canvas->default_foreground;
    }
}

/* Redraw the screen */
void brailleplot_refresh(brailleplot_canvas * canvas) {
  int i, j, k, x, y, c;
  int background, foreground;

  /* Translate image to text */
  for(i = 0; i < canvas->width; i++) {
    for(j = 0; j < canvas->height; j++) {
      x = 2*i;
      y = 4*(canvas->height - 1 - j);
      c = canvas->text[(canvas->width + 1)*j + i];
      if(c < 0 || c > 255) continue;
      for(k = 0; k < 8; k++)
          canvas->text[(canvas->width + 1)*j + i] = (canvas->text[(canvas->width + 1)*j + i] << 1)|
                                                   canvas->screen[2*canvas->width*y + x + canvas->magicnum[k]];
    }
  }

  /* Get the correct Braille codes in unicode */
  for(j = 0; j < canvas->height; j++) {
    for(i = 0; i < canvas->width; i++) {
       canvas->text[(canvas->width + 1)*j + i] += 0x2800;
    }
  }

  /* Insert newlines at the end of the lines in the text buffer */
  for(i = 0; i < canvas->height; i++)
    canvas->text[i*(canvas->width + 1) + canvas->width] = 10;

  /* NULL character at the end of the text buffer */
  canvas->text[(canvas->width + 1)*canvas->height] = 0;

  printf("\x1b[2J\x1b[1;1H"); /* Clear text */

  /* Print the characters */
  switch(canvas->mode) {
    case C16:
			for(j = 0; j < canvas->height; j++) {
   			for(i = 0; i < canvas->width; i++) {
          background = canvas->colour[2*((canvas->width + 1)*j + i)];
          foreground = canvas->colour[2*((canvas->width + 1)*j + i) + 1];
     			printf("\033[%d;4%dm\033[%d;3%dm%lc", background/8, background%8,
                                                foreground/8, foreground%8,
                                                (wint_t) canvas->text[(canvas->width + 1)*j + i]);
   			}
   			printf("\n");
 			}
 			break;
    case C256:
 			for(j = 0; j < canvas->height; j++) {
   			for(i = 0; i < canvas->width; i++) {
     			printf("\033[48;5;%dm\033[38;5;%dm%lc", canvas->colour[2*((canvas->width + 1)*j + i)],
                           			                  canvas->colour[2*((canvas->width + 1)*j + i) + 1],
                                             			(wint_t) canvas->text[(canvas->width + 1)*j + i]);
   			}
   			printf("\n");
 			}
 			break;
    case CTRUE:
 			for(j = 0; j < canvas->height; j++) {
   			for(i = 0; i < canvas->width; i++) {
          background = canvas->colour[2*((canvas->width + 1)*j + i)];
          foreground = canvas->colour[2*((canvas->width + 1)*j + i) + 1];
     			printf("\033[48;2;%d;%d;%dm\033[38;2;%d;%d;%dm%lc",
     			       red(background), green(background), blue(background),
     			       red(foreground), green(foreground), blue(foreground),
                 (wint_t) canvas->text[(canvas->width + 1)*j + i]);
   			}
   			printf("\n");
 			}
 			break;
  	  default:
 	      printf("%ls", canvas->text);
    }
}

/* Set a pixel at a given location to the value c */
void brailleplot_setpixel(brailleplot_canvas * canvas, int x, int y, int c) {
  if(x >= 0 && x < 2*canvas->width && y >= 0 && y < 4*canvas->height)
    canvas->screen[2*canvas->width*y + x] = c;

  return;
}

/* Get the value of a pixel at a given location */
int brailleplot_getpixel(brailleplot_canvas * canvas, int x, int y) {
  if(x >= 0 && x < 2*canvas->width && y >= 0 && y < 4*canvas->height)
    return canvas->screen[2*canvas->width*y + x];
  else
    return 0;
}

/* Draw line (colour c) */
void brailleplot_line(brailleplot_canvas * canvas, int x1, int y1, int x2, int y2, int c) {
  int i; /* Counter */
  int sign; /* Sign of increment */

  if(x1 == x2 && y1 == y2) { /* Zero length line */
    brailleplot_setpixel(canvas, x1, y1, c);
  }
  else if(abs(y2 - y1) < abs(x2 - x1)) { /* Consider slope of the line */
    sign = (x2 - x1)/abs(x2 - x1); /* Sign of increment */
    for(i = 0; i < abs(x2 - x1); i++)
      /* Draw pixel */
      brailleplot_setpixel(canvas, x1 + sign*i, (int) (y1 + sign*i*(y2 - y1)/(1.0f*(x2 - x1)) + 0.5), c);
  }
  else
  {
    sign = (y2 - y1)/abs(y2 - y1); /* Sign of increment */
    for(i = 0; i < abs(y2 - y1); i++)
      /* Draw pixel */
      brailleplot_setpixel(canvas, (int) (x1 + sign*i*(x2 - x1)/(1.0f*(y2 - y1)) + 0.5), y1 + sign*i, c);
  }

  return;
}

/* Draw circle (colour c) */
void brailleplot_circle(brailleplot_canvas * canvas, int x, int y, int radius, int c) {
  float p = 1.25 - radius; /* Decision parameter */
  int Dx = 0, Dy = radius; /* Coordinates relative to (x, y) */

  /* Draw first four points */
  brailleplot_setpixel(canvas, x + radius, y, c);
  brailleplot_setpixel(canvas, x - radius, y, c);
  brailleplot_setpixel(canvas, x, y + radius, c);
  brailleplot_setpixel(canvas, x, y - radius, c);

  while(Dx < Dy) {
    /* Calculate the position of the next pixel */
    Dx++;
    if(p < 0)
      p += 2*Dx + 1;
    else {
      Dy--;
      p += 2*(Dx - Dy) + 1;
    }

    /* Draw points in the eight octants */
    brailleplot_setpixel(canvas, x + Dx, y + Dy, c);
    brailleplot_setpixel(canvas, x - Dx, y + Dy, c);
    brailleplot_setpixel(canvas, x + Dx, y - Dy, c);
    brailleplot_setpixel(canvas, x - Dx, y - Dy, c);
    brailleplot_setpixel(canvas, x + Dy, y + Dx, c);
    brailleplot_setpixel(canvas, x - Dy, y + Dx, c);
    brailleplot_setpixel(canvas, x + Dy, y - Dx, c);
    brailleplot_setpixel(canvas, x - Dy, y - Dx, c);
  }

  return;
}

/* Write a char at a given location */
void brailleplot_setchar(brailleplot_canvas * canvas, int i, int j, char c) {
  if(i >= 0 && i < canvas->width && j >= 0 && j < canvas->height)
    canvas->text[(canvas->width + 1)*j + i] = ((int) c) - 0x2800;
  return;
}

/* Write a string at a given position */
void brailleplot_setstring(brailleplot_canvas * canvas, int i, int j, char * str) {
  int k = 0;
  while(str[k] != '\0') {
    brailleplot_setchar(canvas, i + k, j, str[k]);
    k++;
  }
  return;
}

/* Copy bitmap to screen */
void brailleplot_bitmap(brailleplot_canvas * canvas, int x0, int y0,
                        struct brailleplot_bitmap bitmap) {
  int x, y; /* Screen coordinates */
  int c; /* Bit value */

  /* Loop over the pixels in the bitmap */
  for(y = 0; y < bitmap.height; y++) {
    for(x = 0; x < bitmap.width; x++) {
      if(bitmap.pixels[y*bitmap.width + x] > 0) c = 1;
      else c = 0;

      brailleplot_setpixel(canvas, x0 + x, y0 + y, c);
    }
  }

  return;
}

/* Superimpose a bitmap at screen coordinates (x0, y0) */
void brailleplot_xorput(brailleplot_canvas * canvas, int x0, int y0,
                        struct brailleplot_bitmap bitmap) {
  int x, y; /* Screen coordinates */

  /* Loop over the pixels in the bitmap */
  for(y = 0; y < bitmap.height; y++)
    for(x = 0; x < bitmap.width; x++)
      if(bitmap.pixels[y*bitmap.width + x] > 0)
        brailleplot_setpixel(canvas, x0 + x, y0 + y, 1);

  return;
}

/* Get bitmap from screen */
void brailleplot_get(brailleplot_canvas * canvas, int x1, int y1, int x2, int y2,
                     struct brailleplot_bitmap * bitmap) {
  int x, y; /* Screen coordinates */

  /* Calculate bitmap width and height */
  bitmap->width = x2 - x1 + 1;
  bitmap->height = y2 - y1 + 1;

  /* Loop over the pixels in the rectangle (x1, y1) --> (x2, y2) */
  for(y = y1; y < y2 + 1; y++) {
    for(x = x1; x < x2 + 1; x++){
      /* Copy the value of the pixel to the bitmap */
      bitmap->pixels[(y - y1)*bitmap->width + (x - x1)] = brailleplot_getpixel(canvas, x, y);
    }
  }

  return;
}

/* Set the colour of a given character on the screen */
void brailleplot_setcolour(brailleplot_canvas * canvas, int row, int column, int background, int foreground)
{
  if(column >= 0 && column < canvas->width && row >= 0 && row < canvas->height) {
    canvas->colour[2*((canvas->width + 1)*row + column)] = background;
    canvas->colour[2*((canvas->width + 1)*row + column) + 1] = foreground;
  }

  return;
}

/* Get the foreground colour for a location on the text buffer */
int brailleplot_getbackground(brailleplot_canvas * canvas, int row, int column)
{
  return canvas->colour[2*((canvas->width + 1)*row + column)];
}

/* Get the foreground colour for a location on the text buffer */
int brailleplot_getforeground(brailleplot_canvas * canvas, int row, int column)
{
  return canvas->colour[2*((canvas->width + 1)*row + column) + 1];
}

/* Clean up */
void brailleplot_close(brailleplot_canvas * canvas) {
  /* Free memory */
  free(canvas->screen);
  free(canvas->text);

  if(canvas->mode > 0) {
    free(canvas->colour);
    printf("\033[0;49m\033[0;39m");  /* Return console text to default colours */
  }

  return;
}
# endif
