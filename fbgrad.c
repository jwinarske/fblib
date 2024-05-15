/* fbgrad: draw gradient using framebuffer.

   (c) Lev, 2018, MIT licence
   (c) Joel Winarske, 2024 All Rights Reserved
*/

#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <time.h>

#define fbdev "/dev/fb0"


typedef struct {
    char    *buffer;
    size_t  size;
    size_t  bytes_per_pixel, bytes_per_line;
    size_t  width, height;
    uint_fast16_t red, green, blue;
} Screen;

typedef struct {
    uint_fast8_t    r, g, b, a;
} Color;

#define Die(Msg, ...) { \
    fprintf (stderr, "fbgrad: " Msg ".\n", __VA_ARGS__); \
    exit(1); \
}\

int main (int argc, char **argv) {

    int fbfd = open (fbdev, O_RDWR);
    if (fbfd < 0)
        Die ("cannot open \"%s\"", fbdev);

    struct fb_var_screeninfo vinf;
    if (ioctl (fbfd, FBIOGET_VSCREENINFO, &vinf) == -1)
        Die ("cannot open variable screen info for \"%s\"", fbdev);

    printf("width .................................. %d\n", vinf.xres);
    printf("height ................................. %d\n", vinf.yres);
    printf("virtual width .......................... %dx%d\n", vinf.xres_virtual);
    printf("virtual height ......................... %dx%d\n", vinf.yres_virtual);
    printf("offset from virtual to visible ......... %dx%d\n", vinf.xoffset, vinf.yoffset);
    printf("grayscale .............................. %dx%d\n", vinf.grayscale);
    printf("bpp .................................... %d\n", vinf.bits_per_pixel);
    printf("red .................................... offset: %d, length: %d, msb_right: %d\n", vinf.red.offset, vinf.red.length, vinf.red.msb_right);
    printf("green .................................. offset: %d, length: %d, msb_right: %d\n", vinf.green.offset, vinf.green.length, vinf.green.msb_right);
    printf("blue ................................... offset: %d, length: %d, msb_right: %d\n", vinf.blue.offset, vinf.blue.length, vinf.blue.msb_right);
    printf("transparent ............................ offset: %d, length: %d, msb_right: %d\n", vinf.transp.offset, vinf.transp.length, vinf.transp.msb_right);
    printf("nonstd ................................. %d\n", vinf.nonstd);
    printf("height ................................. %d mm\n", vinf.height);
    printf("width .................................. %d mm\n", vinf.width);
    printf("pixclock ............................... %d ps\n", vinf.pixclock);
    printf("left_margin ............................ %d ps\n", vinf.left_margin);
    printf("right_margin ........................... %d ps\n", vinf.right_margin);
    printf("upper_margin ........................... %d ps\n", vinf.upper_margin);
    printf("lower_margin ........................... %d ps\n", vinf.lower_margin);
    printf("hsync_len .............................. %d\n", vinf.hsync_len);
    printf("vsync_len .............................. %d\n", vinf.vsync_len);
    printf("sync ................................... %d\n", vinf.sync);
    printf("vmode .................................. %d\n", vinf.vmode);
    printf("rotate ................................. %d\n", vinf.rotate);
    printf("colorspace ............................. %d\n", vinf.colorspace);


    struct fb_fix_screeninfo finf;
    if (ioctl (fbfd, FBIOGET_FSCREENINFO, &finf) == -1)
        Die ("cannot open fixed screen info for \"%s\"", fbdev);

    printf("id ..................................... %s\n", finf.id);
    printf("Start of frame buffer mem (physical) ... 0x%08X\n", finf.smem_start);
    printf("Length of frame buffer mem ............. 0x%08X\n", finf.smem_len);
    switch(finf.type) {
        case FB_TYPE_PACKED_PIXELS:
            printf("type ................................... Packed Pixels\n");
            break;
        case FB_TYPE_PLANES:
            printf("type ................................... Non interleaved planes\n");
            break;
        case FB_TYPE_INTERLEAVED_PLANES:
            printf("type ................................... interleaved planes\n");
            break;
        case FB_TYPE_TEXT:
            printf("type ................................... Text/attributes\n");
            break;
        case FB_TYPE_VGA_PLANES:
            printf("type ................................... EGA/VGA planes\n");
            break;
        case FB_TYPE_FOURCC:
            printf("type ................................... Type identified by a V4L2 FOURCC\n");
            break;
        default:
            printf("type ................................... unknown\n");
            break;
    }

    printf("type_aux ............................... Interleave for interleaved Planes: %d\n", finf.type_aux);

    switch(finf.visual) {
        case FB_VISUAL_MONO01:
            printf("visual ................................. Monochr. 1=Black 0=White\n");
            break;
        case FB_VISUAL_MONO10:
            printf("visual ................................. Monochr. 1=White 0=Black\n");
            break;
        case FB_VISUAL_TRUECOLOR:
            printf("visual ................................. True color\n");
            break;
        case FB_VISUAL_PSEUDOCOLOR:
            printf("visual ................................. Pseudo color (like atari)\n");
            break;
        case FB_VISUAL_DIRECTCOLOR:
            printf("visual ................................. Direct color\n");
            break;
        case FB_VISUAL_STATIC_PSEUDOCOLOR:
            printf("visual ................................. Pseudo color readonly\n");
            break;
        case FB_VISUAL_FOURCC:
            printf("visual ................................. identified by a V4L2 FOURCC\n");
            break;
        default:
            printf("visual ................................. unknown\n");
            break;
    }

    printf("xpanstep (hw panning) .................. %d\n", finf.xpanstep);
    printf("ypanstep (hw panning) .................. %d\n", finf.ypanstep);
    printf("ywrapstep (hw ywrap) ................... %d\n", finf.ywrapstep);
    printf("length of a line in bytes .............. %d\n", finf.line_length);
    printf("Start of Memory Mapped I/O (physical) .. 0x%08X\n", finf.mmio_start);
    printf("Length of Memory Mapped I/O ............ 0x%08X\n", finf.mmio_len);

    printf("specific chip/card ..................... ");
    switch(finf.accel) {
        case FB_ACCEL_NONE:
            printf("no hardware accelerator\n");
            break;
        case FB_ACCEL_ATARIBLITT:
            printf("Atari Blitter\n");
            break;
        case FB_ACCEL_AMIGABLITT:
            printf("Amiga Blitter\n");
            break;
        case FB_ACCEL_S3_TRIO64:
            printf("Cybervision64 (S3 Trio64)\n");
            break;
        case FB_ACCEL_NCR_77C32BLT:
            printf("RetinaZ3 (NCR 77C32BLT)\n");
            break;
        case FB_ACCEL_S3_VIRGE:
            printf("Cybervision64/3D (S3 ViRGE)\n");
            break;
        case FB_ACCEL_ATI_MACH64GX:
            printf("ATI Mach 64GX family\n");
            break;
        case FB_ACCEL_DEC_TGA:
            printf("DEC 21030 TGA\n");
            break;
        case FB_ACCEL_ATI_MACH64CT:
            printf("ATI Mach 64CT family\n");
            break;
        case FB_ACCEL_ATI_MACH64VT:
            printf("ATI Mach 64CT family VT class\n");
            break;
        case FB_ACCEL_ATI_MACH64GT:
            printf("ATI Mach 64CT family GT class\n");
            break;
        case FB_ACCEL_SUN_CREATOR:
            printf("Sun Creator/Creator3D\n");
            break;
        case FB_ACCEL_SUN_CGSIX:
            printf("Sun cg6\n");
            break;
        case FB_ACCEL_SUN_LEO:
            printf("Sun leo/zx\n");
            break;
        case FB_ACCEL_IMS_TWINTURBO:
            printf("IMS Twin Turbo\n");
            break;
        case FB_ACCEL_3DLABS_PERMEDIA2:
            printf("3Dlabs Permedia 2\n");
            break;
        case FB_ACCEL_MATROX_MGA2064W:
            printf("Matrox MGA2064W (Millenium)\n");
            break;
        case FB_ACCEL_MATROX_MGA1064SG:
            printf("Matrox MGA1064SG (Mystique)\n");
            break;
        case FB_ACCEL_MATROX_MGA2164W:
            printf("Matrox MGA2164W (Millenium II)\n");
            break;
        case FB_ACCEL_MATROX_MGA2164W_AGP:
            printf("Matrox MGA2164W (Millenium II)\n");
            break;
        case FB_ACCEL_MATROX_MGAG100:
            printf("Matrox G100 (Productiva G100)\n");
            break;
        case FB_ACCEL_MATROX_MGAG200:
            printf("Matrox G200 (Myst, Mill, ...)\n");
            break;
        case FB_ACCEL_SUN_CG14:
            printf("Sun cgfourteen\n");
            break;
        case FB_ACCEL_SUN_BWTWO:
            printf("Sun bwtwo\n");
            break;
        case FB_ACCEL_SUN_CGTHREE:
            printf("Sun cgthree\n");
            break;
        case FB_ACCEL_SUN_TCX:
            printf("Sun tcx\n");
            break;
        case FB_ACCEL_MATROX_MGAG400:
            printf("Matrox G400\n");
            break;
        case FB_ACCEL_NV3:
            printf("nVidia RIVA 128\n");
            break;
        case FB_ACCEL_NV4:
            printf("nVidia RIVA TNT\n");
            break;
        case FB_ACCEL_NV5:
            printf("nVidia RIVA TNT2\n");
            break;
        case FB_ACCEL_CT_6555x:
            printf("C&T 6555x\n");
            break;
        case FB_ACCEL_3DFX_BANSHEE:
            printf("3Dfx Banshee\n");
            break;
        case FB_ACCEL_ATI_RAGE128:
            printf("ATI Rage128 family\n");
            break;
        case FB_ACCEL_IGS_CYBER2000:
            printf("CyberPro 2000\n");
            break;
        case FB_ACCEL_IGS_CYBER2010:
            printf("CyberPro 2010\n");
            break;
        case FB_ACCEL_IGS_CYBER5000:
            printf("CyberPro 5000\n");
            break;
        case FB_ACCEL_SIS_GLAMOUR:
            printf("SiS 300/630/540\n");
            break;
        case FB_ACCEL_3DLABS_PERMEDIA3:
            printf("3Dlabs Permedia 3\n");
            break;
        case FB_ACCEL_ATI_RADEON:
            printf("ATI Radeon family\n");
            break;
        case FB_ACCEL_I810:
            printf("Intel 810/815\n");
            break;
        case FB_ACCEL_SIS_GLAMOUR_2:
            printf("SiS 315, 650, 740\n");
            break;
        case FB_ACCEL_SIS_XABRE:
            printf("SiS 330 (\"Xabre\")\n");
            break;
        case FB_ACCEL_I830:
            printf("Intel 830M/845G/85x/865G\n");
            break;
        case FB_ACCEL_NV_10:
            printf("nVidia Arch 10\n");
            break;
        case FB_ACCEL_NV_20:
            printf("nVidia Arch 20\n");
            break;
        case FB_ACCEL_NV_30:
            printf("nVidia Arch 30\n");
            break;
        case FB_ACCEL_NV_40:
            printf("nVidia Arch 40\n");
            break;
        case FB_ACCEL_XGI_VOLARI_V:
            printf("XGI Volari V3XT, V5, V8\n");
            break;
        case FB_ACCEL_XGI_VOLARI_Z:
            printf("XGI Volari Z7\n");
            break;
        case FB_ACCEL_OMAP1610:
            printf("TI OMAP16xx\n");
            break;
        case FB_ACCEL_TRIDENT_TGUI:
            printf("Trident TGUI\n");
            break;
        case FB_ACCEL_TRIDENT_3DIMAGE:
            printf("Trident 3DImage\n");
            break;
        case FB_ACCEL_TRIDENT_BLADE3D:
            printf("Trident Blade3D\n");
            break;
        case FB_ACCEL_CIRRUS_ALPINE:
            printf("Cirrus Logic 543x/544x/5480\n");
            break;
        case FB_ACCEL_NEOMAGIC_NM2070:
            printf("NeoMagic NM2070\n");
            break;
        case FB_ACCEL_NEOMAGIC_NM2090:
            printf("NeoMagic NM2090\n");
            break;
        case FB_ACCEL_NEOMAGIC_NM2093:
            printf("NeoMagic NM2093\n");
            break;
        case FB_ACCEL_NEOMAGIC_NM2097:
            printf("NeoMagic NM2097\n");
            break;
        case FB_ACCEL_NEOMAGIC_NM2160:
            printf("NeoMagic NM2160\n");
            break;
        case FB_ACCEL_NEOMAGIC_NM2200:
            printf("NeoMagic NM2200\n");
            break;
        case FB_ACCEL_NEOMAGIC_NM2230:
            printf("NeoMagic NM2230\n");
            break;
        case FB_ACCEL_NEOMAGIC_NM2360:
            printf("NeoMagic NM2360\n");
            break;
        case FB_ACCEL_NEOMAGIC_NM2380:
            printf("NeoMagic NM2380\n");
            break;
        case FB_ACCEL_PXA3XX:
            printf("PXA3xx\n");
            break;
        case FB_ACCEL_SAVAGE4:
            printf("S3 Savage4\n");
            break;
        case FB_ACCEL_SAVAGE3D:
            printf("S3 Savage3D\n");
            break;
        case FB_ACCEL_SAVAGE3D_MV:
            printf("S3 Savage3D-MV\n");
            break;
        case FB_ACCEL_SAVAGE2000:
            printf("S3 Savage2000\n");
            break;
        case FB_ACCEL_SAVAGE_MX_MV:
            printf("S3 Savage/MX-MV\n");
            break;
        case FB_ACCEL_SAVAGE_MX:
            printf("S3 Savage/MX\n");
            break;
        case FB_ACCEL_SAVAGE_IX_MV:
            printf("S3 Savage/IX-MV\n");
            break;
        case FB_ACCEL_SAVAGE_IX:
            printf("S3 Savage/IX\n");
            break;
        case FB_ACCEL_PROSAVAGE_PM:
            printf("S3 ProSavage PM133\n");
            break;
        case FB_ACCEL_PROSAVAGE_KM:
            printf("S3 ProSavage KM133\n");
            break;
        case FB_ACCEL_S3TWISTER_P:
            printf("S3 Twister\n");
            break;
        case FB_ACCEL_S3TWISTER_K:
            printf("S3 TwisterK\n");
            break;
        case FB_ACCEL_SUPERSAVAGE:
            printf("S3 Supersavage\n");
            break;
        case FB_ACCEL_PROSAVAGE_DDR:
            printf("S3 ProSavage DDR\n");
            break;
        case FB_ACCEL_PROSAVAGE_DDRK:
            printf("S3 ProSavage DDR-K\n");
            break;
        case FB_ACCEL_PUV3_UNIGFX:
            printf("PKUnity-v3 Unigfx\n");
            break;
    }

    if (finf.capabilities & FB_CAP_FOURCC == FB_CAP_FOURCC) {
        printf("caps ................................... Device supports FOURCC-based formats\n");
    }

    else if (finf.capabilities != 0) {
        printf("caps ................................... Unknown: 0x%04X\n", finf.capabilities);
    }


    Screen s = {
        .size            = finf.line_length * vinf.yres,
        .bytes_per_pixel = vinf.bits_per_pixel / 8,
        .bytes_per_line  = finf.line_length,
        .red             = vinf.red.offset / 8,
        .green           = vinf.green.offset / 8,
        .blue            = vinf.blue.offset / 8,
        .width           = vinf.xres,
        .height          = vinf.yres
    };

    s.buffer = mmap (0, s.size, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);

    if (s.buffer == MAP_FAILED)
        Die ("cannot map frame buffer \"%s\"", fbdev);

    printf("line_length (bytes) .................... %d\n", finf.line_length);
    printf("yres (height) .......................... %d\n", vinf.yres);
    printf("size = (line_length * yres) ............ %d\n", s.size);

    int time_start = time (NULL);

    switch(s.bytes_per_pixel) {

        case 1:
            for (uint_fast16_t t = 0; t < 255; t++) {
                for (uint_fast16_t y = 0; y < vinf.yres; y++) {
                    for (uint_fast16_t x = 0; x < vinf.xres; x++) {
                        uint_fast16_t pix_offset = x * s.bytes_per_pixel + y * s.bytes_per_line;

                        s.buffer[pix_offset + s.red] = x * 255 / s.width;
                        s.buffer[pix_offset + s.green] = y * 255 / s.height;
                        s.buffer[pix_offset + s.blue] = t;
                    }
                }
            }
            break;

        case 2:

            if (vinf.red.offset == 11 && vinf.green.offset == 5 && vinf.blue.offset == 0) {
                for (uint_fast16_t t = 0; t < 255; t++) {
                    for (uint_fast16_t y = 0; y < vinf.yres; y++) {
                        for (uint_fast16_t x = 0; x < vinf.xres; x++) {
                            uint_fast16_t pix_offset = x * s.bytes_per_pixel + y * s.bytes_per_line;

                            uint_fast16_t r = x * 255 / s.width;
                            uint_fast16_t g = y * 255 / s.height;
                            uint_fast16_t b = t;

                            uint_fast16_t *buff = (uint_fast16_t *)&s.buffer[pix_offset];
                            
                            *buff = ((r>>3) << 11) | ((g>>2) << 5) | b >> 3;
                        }
                    }
                }
            }
            else {
                printf("Error: two byte format not supported");
            }
            break;

        default:
            printf("Error: %d byte format not supported", s.bytes_per_pixel);
            break;
    }
    
    int time_end = time(NULL);

    munmap (s.buffer, s.size);

    close (fbfd);

    printf ("FPS: %.2f.\n", 255.0 / (time_end - time_start));

    return EXIT_SUCCESS;
}
