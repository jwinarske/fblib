/* fbgrad: draw gradient using framebuffer.
   run in console, X11 would overwrite everything immediatelly.

   (c) Lev, 2018, MIT licence
*/

#include "fblib.h"

#include <stdio.h>

#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <time.h>

#define fbdev "/dev/fb0"

typedef struct {
    uint_fast8_t    r, g, b, a;
} Color;

#define Die(Msg, ...) { \
    fprintf (stderr, "fbgrad: " Msg ".\n", __VA_ARGS__); \
    exit(1); \
}\

#define Assumption(Cond, Msg) \
    if (!(Cond)) { \
        fprintf (stderr, "fbgrad: failed assumption: %s\n", Msg);\
        exit(2);\
    }

int main (int argc, char **argv) {

    int fbfd = open (fbdev, O_RDWR);
    if (fbfd < 0)
        Die ("cannot open \"%s\"", fbdev);

    struct fb_var_screeninfo vinf;
    struct fb_fix_screeninfo finf;

    if (ioctl (fbfd, FBIOGET_FSCREENINFO, &finf) == -1)
        Die ("cannot open fixed screen info for \"%s\"", fbdev);

    if (ioctl (fbfd, FBIOGET_VSCREENINFO, &vinf) == -1)
        Die ("cannot open variable screen info for \"%s\"", fbdev);

    printf("%d x %d\n", vinf.xres, vinf.yres);
    printf("%d bpp\n", vinf.bits_per_pixel);
    printf("virtual resolution: %dx%d\n", vinf.xres_virtual, vinf.yres_virtual);
    printf("offset from virtual to visible: %dx%d\n", vinf.xoffset, vinf.yoffset);
    printf("grayscale: %dx%d\n", vinf.grayscale);
    printf("red: %d\n", vinf.red);
    printf("green: %d\n", vinf.green);
    printf("blue: %d\n", vinf.blue);
    printf("transp: %d\n", vinf.transp);
    printf("nonstd: %d\n", vinf.nonstd);
    printf("height: %d mm\n", vinf.height);
    printf("width: %d mm\n", vinf.width);
    printf("pixclock: %d ps\n", vinf.pixclock);
    printf("left_margin: %d ps\n", vinf.left_margin);
    printf("right_margin: %d ps\n", vinf.right_margin);
    printf("upper_margin: %d ps\n", vinf.upper_margin);
    printf("lower_margin: %d ps\n", vinf.lower_margin);
    printf("hsync_len: %d\n", vinf.hsync_len);
    printf("vsync_len: %d\n", vinf.vsync_len);
    printf("sync: %d\n", vinf.sync);
    printf("vmode: %d\n", vinf.vmode);
    printf("rotate: %d\n", vinf.rotate);
    printf("colorspace: %d\n", vinf.colorspace);

#if 0
    Assumption ((vinf.red.offset%8) == 0 && (vinf.red.length == 8) &&
                (vinf.green.offset%8) == 0 && (vinf.green.length == 8) &&
                (vinf.blue.offset%8) == 0 && (vinf.blue.length == 8) &&
                (vinf.transp.offset) == 0 && (vinf.transp.length == 0) &&
                vinf.xoffset == 0 && vinf.yoffset == 0 &&
                vinf.red.msb_right == 0 &&
                vinf.green.msb_right == 0 &&
                vinf.blue.msb_right == 0,
                "Color masks are 8bit, byte aligned, little endian, no transparency."
    );
#endif

    Screen s = {
        .size            = finf.line_length * vinf.yres,
        .bytes_per_pixel = vinf.bits_per_pixel / 8,
        .bytes_per_line  = finf.line_length,
        .red             = vinf.red.offset/8,
        .green           = vinf.green.offset/8,
        .blue            = vinf.blue.offset/8,
        .width           = vinf.xres,
        .height          = vinf.yres
    };

    s.buffer = mmap (0, s.size, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);

    if (s.buffer == MAP_FAILED)
        Die ("cannot map frame buffer \"%s\"", fbdev);

    int time_start = time (NULL);

    for (uint t = 0; t < 255; t++) {
        for (uint y = 0; y < vinf.yres; y++) {
            for (uint x = 0; x < vinf.xres; x++) {
                uint pix_offset = x * s.bytes_per_pixel + y * s.bytes_per_line;
                s.buffer[pix_offset + s.red] = x * 255 / s.width;
                s.buffer[pix_offset + s.green] = y * 255 / s.height;
                s.buffer[pix_offset + s.blue] = t;
            }
        }
    }

    int time_end = time(NULL);

    munmap (s.buffer, s.size);

    close (fbfd);

    printf ("FPS: %.2f.\n", 255.0 / (time_end - time_start));

    return EXIT_SUCCESS;
}
