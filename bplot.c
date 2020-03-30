# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <math.h>
# include "brailleplot.h"

# define VERSION "0.1" /* Program version */

/* Default brailleplot canvas width and height */
# define WIDTH 80
# define HEIGHT 25
/* Default colour mode */
# define BRAILLEPLOT_COLOUR_MODE CTRUE
# define DEFAULT_FOREGROUND_COLOUR 13158600

/* Colour a line */
void brailleplot_cline(brailleplot_canvas * canvas, int x1, int y1, int x2, int y2, int bgcolour, int fgcolour);
/* Colour a circle */
void brailleplot_ccircle(brailleplot_canvas * canvas, int x, int y, int radius, int bgcolour, int fgcolour);

int main(int argc, char * argv[])
{
  FILE * plotdata = NULL; /* Pointer to data file */
  brailleplot_canvas canvas; /* Brailleplot canvas for pixels and characters */
  int canvaswidth = WIDTH, canvasheight = HEIGHT; /* Width and height of canvas */
  double xrange = 0, yrange = 0; /* Width and height of plot */

  char buffer[250]; /* Text buffer to store data read from file */
  double x, x2, xmin = 0, xmax = 0, y, y2, ymin = 0, ymax = 0; /* Data coordinates and maximum and minimum coordinates */
  int nread = 0; /* Number of items read */
  char keypress;

  int px, py, px2, py2; /* Pixel coordinates */
  int colour = DEFAULT_FOREGROUND_COLOUR; /* Current foreground colour */

  /* Read command line arguments */
  if(argc < 2 && isatty(0)) { /* Use help message */
    printf("--- bplot (version " VERSION ") ---\n");
    printf("Display plot data in ASCII files.\n"
           "Data should be formatted by rows:\n"
           " <n>                 Single integer RGB value (colour)\n"
           " <x> <y>             Pixel coordinates.\n"
           " <x> <y> <R>         Circle centre coordinates and radius.\n"
           " <x1> <y1> <x2> <y2> Line from (x1, y1) to (x2, y2).\n"
           "                     Empty rows separate frames.\n\n");
    printf("Usage: %s [options] <Data file>\n", argv[0]);
    printf("Options:\n"
           "  -x <xmin> <xmax>   Minimum and maximum x coordinate values.\n"
           "  -y <ymin> <ymax>   Minimum and maximum y coordinate values.\n"
           "  -w <width>         Canvas width.\n"
           "  -h <height>        Canvas height.\n");
    printf("Interaction keys:\n"
           "  +, -             Zoom in, out.\n"
           "  w, s             Up, down.\n"
           "  a, d             Left, right.\n"
           "  q                Quit program.\n");
    return 0;
  }
  else {
    int i; /* Command line argument index */
    for(i = 1; i < argc; i++) {
      /* Open md data file */
      if(argv[i][0] != '-')
        plotdata = fopen(argv[i], "r");
      /* Other options */
      else if(argv[i][1] == 'w') { /* Canvas width */
        canvaswidth = atoi(argv[i + 1]);
        i++;
      }
      else if(argv[i][1] == 'h') { /* Canvas height */
        canvasheight = atoi(argv[i + 1]);
        i++;
      }
      else if(argv[i][1] == 'x') { /* x range */
        xmin = atof(argv[i + 1]);
        xmax = atof(argv[i + 2]);
        xrange = xmax - xmin;
        i += 2;
      }
      else if(argv[i][1] == 'y') { /* y range */
        ymin = atof(argv[i + 1]);
        ymax = atof(argv[i + 2]);
        yrange = ymax - ymin;
        i += 2;
      }
    }
  }

  if(!isatty(0)) { /* Open stdin */
    plotdata = stdin;
  }

  if(plotdata == NULL) { /* File error */
    printf("Error: File not found or error opening file.\n");
    return -1;
  }

  /* If the plot width and height were not set, set them using the first line in the first frame */
  if(xrange <= 0 || yrange <= 0) {
    /* Skip initial whitespace and comments */
    while(nread < 2) {
      if(!fgets(buffer, 250, plotdata)) {
        printf("Error: No data found in file.\n");
        return -1;
      }
      nread = sscanf(buffer, "%lf %lf", &x, &y);
      xmin = xmax = x; ymin = ymax = y;
    }

    /* Read data from file and change x or y limits if necessary */
    while(nread > 1) {
      if(!fgets(buffer, 250, plotdata)) { rewind(plotdata); break; }
      nread = sscanf(buffer, "%lf %lf", &x, &y);
      if(nread > 1) {
        if(xrange <= 0) { if(xmin > x) xmin = x; if(xmax < x) xmax = x; }
        if(yrange <= 0) { if(ymin > y) ymin = y; if(ymax < y) ymax = y; }
      }
    }

    /* Resize x and y ranges */
    if(xrange <= 0) xrange = xmax - xmin;
    if(yrange <= 0) yrange = ymax - ymin;

    /* Return to the top of the file */
    rewind(plotdata);
  }


  /* Initialise brailleplot */
  brailleplot_init(&canvas, canvaswidth, canvasheight, BRAILLEPLOT_COLOUR_MODE);

  /* Clear screen */
  brailleplot_cls(&canvas);

  while(1) { /* Loop forever */

    if(!fgets(buffer, 250, plotdata)) { /* Read a line from file */
      rewind(plotdata); /* Rewind when you reach the end of the file */
    }

    /* Read data from file and plot */
    nread = sscanf(buffer, "%lf %lf %lf %lf", &x, &y, &x2, &y2);
    if(nread > 0) { /* Plot data point */
      px = (int) round(2*(x - xmin)*(canvaswidth - 1)/xrange) + 1;
      py = (int) round(4*(y - ymin)*(canvasheight - 1)/yrange) + 1;
      if(nread == 1) { /* Select colour */
        colour = (int) x;
      }
      else if(nread == 2) { /* Draw pixel */
        brailleplot_setpixel(&canvas, px, py, 1);
        brailleplot_setcolour(&canvas, canvasheight - py/4 - 1, px/2, 0, colour);
      }
      else if(nread == 3) { /* Draw circle */
        brailleplot_circle(&canvas, px, py, (int) round(2*x2*(canvaswidth - 1)/xrange) + 1, 1);
        brailleplot_ccircle(&canvas, px, py, (int) round(2*x2*(canvaswidth - 1)/xrange) + 1, 0, colour);
      }
      else if(nread == 4) { /* Draw line */
        px2 = (int) round(2*(x2 - xmin)*(canvaswidth - 1)/xrange) + 1;
        py2 = (int) round(4*(y2 - ymin)*(canvasheight - 1)/yrange) + 1;
        brailleplot_line(&canvas, px, py, px2, py2, 1);
        brailleplot_cline(&canvas, px, py, px2, py2, 0, colour);
      }
    }
    else { /* Otherwise, draw the frame */
      brailleplot_refresh(&canvas); /* Flush to screen */
      usleep(5000); /* Sleep briefly */
      brailleplot_cls(&canvas); /* Clear canvas */
    }

    /* Interactive commands */
    if(kbhit()) {
      keypress = getchar();
      switch(keypress) {
        case 'w':
        case 'W': /* Move up */
          ymax += yrange/10;
          ymin += yrange/10;
          break;
        case 's':
        case 'S': /* Move down */
          ymax -= yrange/10;
          ymin -= yrange/10;
          break;
        case 'a':
        case 'A': /* Move left */
          xmax -= xrange/10;
          xmin -= xrange/10;
          break;
        case 'd':
        case 'D': /* Move right */
          xmax += xrange/10;
          xmin += xrange/10;
          break;
        case '+': /* Zoom in */
          xrange *= 0.9; yrange *= 0.9;
          xmin = 0.5*(xmax + xmin - xrange);
          xmax = xmin + xrange;
          ymin = 0.5*(ymax + ymin - yrange);
          ymax = ymin + yrange;
          break;
        case '-': /* Zoom out */
          xrange /= 0.9; yrange /= 0.9;
          xmin = 0.5*(xmax + xmin - xrange);
          xmax = xmin + xrange;
          ymin = 0.5*(ymax + ymin - yrange);
          ymax = ymin + yrange;
          break;
        case 'q':  /* Exit program */
        case 'Q':

          /* Flush to screen */
          brailleplot_refresh(&canvas);

          /* Clean up */
          brailleplot_close(&canvas);
          fclose(plotdata);

          return 0;
      }
    }
  }
}

/* Draw coloured line */
void brailleplot_cline(brailleplot_canvas * canvas, int x1, int y1, int x2, int y2, int bgcolour, int fgcolour) {
  int i; /* Counter */
  int sign; /* Sign of increment */

  if(x1 == x2 && y1 == y2) { /* Zero length line */
    brailleplot_setcolour(canvas, canvas->height - y1/4 - 1, x1/2, bgcolour, fgcolour);
  }
  else if(abs(y2 - y1) < abs(x2 - x1)) { /* Consider slope of the line */
    sign = (x2 - x1)/abs(x2 - x1); /* Sign of increment */
    for(i = 0; i < abs(x2 - x1); i++)
      /* Draw pixel */
      brailleplot_setcolour(canvas, canvas->height - (int) (y1 + sign*i*(y2 - y1)/(1.0f*(x2 - x1)) + 0.5)/4 - 1, (x1 + sign*i)/2, bgcolour, fgcolour);
  }
  else
  {
    sign = (y2 - y1)/abs(y2 - y1); /* Sign of increment */
    for(i = 0; i < abs(y2 - y1); i++)
      /* Draw pixel */
      brailleplot_setcolour(canvas, canvas->height - (y1 + sign*i)/4 - 1, (int) (x1 + sign*i*(x2 - x1)/(1.0f*(y2 - y1)) + 0.5)/2, bgcolour, fgcolour);
  }

  return;
}

/* Draw coloured circle */
void brailleplot_ccircle(brailleplot_canvas * canvas, int x, int y, int radius, int bgcolour, int fgcolour) {
  float p = 1.25 - radius; /* Decision parameter */
  int Dx = 0, Dy = radius; /* Coordinates relative to (x, y) */

  /* Draw first four points */
  brailleplot_setcolour(canvas, canvas->height - y/4 - 1, (x + radius)/2, bgcolour, fgcolour);
  brailleplot_setcolour(canvas, canvas->height - y/4 - 1, (x - radius)/2, bgcolour, fgcolour);
  brailleplot_setcolour(canvas, canvas->height - (y + radius)/4 - 1, x/2, bgcolour, fgcolour);
  brailleplot_setcolour(canvas, canvas->height - (y - radius)/4 - 1, x/2, bgcolour, fgcolour);

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
    brailleplot_setcolour(canvas, canvas->height - (y + Dy)/4 - 1, (x + Dx)/2, bgcolour, fgcolour);
    brailleplot_setcolour(canvas, canvas->height - (y + Dy)/4 - 1, (x - Dx)/2, bgcolour, fgcolour);
    brailleplot_setcolour(canvas, canvas->height - (y - Dy)/4 - 1, (x + Dx)/2, bgcolour, fgcolour);
    brailleplot_setcolour(canvas, canvas->height - (y - Dy)/4 - 1, (x - Dx)/2, bgcolour, fgcolour);
    brailleplot_setcolour(canvas, canvas->height - (y + Dx)/4 - 1, (x + Dy)/2, bgcolour, fgcolour);
    brailleplot_setcolour(canvas, canvas->height - (y + Dx)/4 - 1, (x - Dy)/2, bgcolour, fgcolour);
    brailleplot_setcolour(canvas, canvas->height - (y - Dx)/4 - 1, (x + Dy)/2, bgcolour, fgcolour);
    brailleplot_setcolour(canvas, canvas->height - (y - Dx)/4 - 1, (x - Dy)/2, bgcolour, fgcolour);
  }

  return;
}
