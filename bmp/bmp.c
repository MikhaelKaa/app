#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "fatfs.h"
#include "bmp.h"

#define ENDL "\r\n"
#define MAX_BMP_DRAW_WIDTH 320

void (*BMP_draw_pixel)(uint16_t x, uint16_t y, uint16_t pixel) = 0;
uint16_t BMP_scr_heigth = 0;
uint16_t BMP_scr_width = 0;

#pragma pack(push, 1)
typedef struct {              // Total: 54 bytes
  uint16_t  type;             // Magic identifier: 0x4d42
  uint32_t  size;             // File size in bytes
  uint16_t  reserved1;        // Not used
  uint16_t  reserved2;        // Not used
  uint32_t  offset;           // Offset to image data in bytes from beginning of file (54 bytes)
  uint32_t  dib_header_size;  // DIB Header size in bytes (40 bytes)
  int32_t   width_px;         // Width of the image
  int32_t   height_px;        // Height of image
  uint16_t  num_planes;       // Number of color planes
  uint16_t  bits_per_pixel;   // Bits per pixel
  uint32_t  compression;      // Compression type
  uint32_t  image_size_bytes; // Image size in bytes
  int32_t   x_resolution_ppm; // Pixels per meter
  int32_t   y_resolution_ppm; // Pixels per meter
  uint32_t  num_colors;       // Number of colors  
  uint32_t  important_colors; // Important colors 
} BMPHeader_t;
#pragma pack(pop)

// ucmd handler for bmp.
int ucmd_bmp(int argc, char *argv[]) {
  int test_parameter = 42;

  switch (argc) {
    case 1:
      printf("bmp usage: \r\n");
      return -EINVAL;
      break;
    
    case 3:
      // Display BMP header
      if (strcmp(argv[1], "head") == 0) {
        FIL fil;
        FRESULT res = f_open(&fil, argv[2], FA_READ);
        if (res != FR_OK) {
          printf("Open error: %d" ENDL, res);
          return -EIO;
        }
        
        BMPHeader_t header;
        UINT bytes_read;
        res = f_read(&fil, &header, sizeof(BMPHeader_t), &bytes_read);
        if (res != FR_OK || bytes_read != sizeof(BMPHeader_t)) {
          f_close(&fil);
          printf("Read error: %d or size mismatch (%d/%d)" ENDL, 
                 res, bytes_read, sizeof(BMPHeader_t));
          return -EIO;
        }
        
        printf("BMP header:" ENDL);
        printf("Type:                0x%04x (must be 0x4D42)" ENDL, header.type);
        printf("File size:           %lu bytes" ENDL, header.size);
        printf("Data offset:         %lu bytes" ENDL, header.offset);
        printf("DIB header size:     %lu bytes" ENDL, header.dib_header_size);
        printf("Width:               %ld px" ENDL, header.width_px);
        printf("Height:              %ld px (negative = top-down)" ENDL, header.height_px);
        printf("Planes:              %u" ENDL, header.num_planes);
        printf("Bits per pixel:      %u" ENDL, header.bits_per_pixel);
        printf("Compression:         0x%08lx" ENDL, header.compression);
        printf("Image size:          %lu bytes" ENDL, header.image_size_bytes);
        printf("X resolution:        %ld ppm" ENDL, header.x_resolution_ppm);
        printf("Y resolution:        %ld ppm" ENDL, header.y_resolution_ppm);
        printf("Colors in palette:   %lu" ENDL, header.num_colors);
        printf("Important colors:    %lu" ENDL, header.important_colors);
        
        f_close(&fil);
        return 0;
      }
      
      // Test pattern drawing
      if (strcmp(argv[1], "test") == 0) {
        printf("test screen" ENDL);

        if (BMP_draw_pixel == 0) {
          printf("BMP_draw_pixel is not set!!!" ENDL);
          return -EIO;
        }

        if (BMP_scr_width == 0 || BMP_scr_heigth == 0) {
          printf("Screen size is not set!!!" ENDL);
          return -EIO;
        }

        if (sscanf(argv[2], "%d", &test_parameter) != 1) {
          printf("set default test parameter %d" ENDL, test_parameter);
        }

        for (uint16_t x = 0; x < BMP_scr_heigth; x++) {
          for (uint16_t y = 0; y < BMP_scr_width; y++) { 
            BMP_draw_pixel(y, x, (uint16_t)(x*y*test_parameter));
          }
        }

        return 0;
      }
      
      // Load and display BMP image
      if (strcmp(argv[1], "load") == 0) {
        printf("Loading BMP: %s" ENDL, argv[2]);

        if (BMP_draw_pixel == 0) {
          printf("BMP_draw_pixel is not set!!!" ENDL);
          return -EIO;
        }

        if (BMP_scr_width == 0 || BMP_scr_heigth == 0) {
          printf("Screen size is not set!!!" ENDL);
          return -EIO;
        }

        FIL fil;
        FRESULT res = f_open(&fil, argv[2], FA_READ);
        if (res != FR_OK) {
          printf("Open error: %d" ENDL, res);
          return -EIO;
        }

        BMPHeader_t header;
        UINT bytes_read;
        res = f_read(&fil, &header, sizeof(BMPHeader_t), &bytes_read);
        if (res != FR_OK || bytes_read != sizeof(BMPHeader_t)) {
          f_close(&fil);
          printf("Read header error: %d" ENDL, res);
          return -EIO;
        }

        // Validate header
        if (header.type != 0x4D42) {
          f_close(&fil);
          printf("Not a BMP file" ENDL);
          return -EIO;
        }

        if (header.bits_per_pixel != 16) {
          f_close(&fil);
          printf("Only 16 bpp supported" ENDL);
          return -EIO;
        }

        if (header.compression != 0 && header.compression != 3) {
          f_close(&fil);
          printf("Compression not supported" ENDL);
          return -EIO;
        }

        int32_t abs_height = header.height_px;
        int top_down = 0;
        if (abs_height < 0) {
          abs_height = -abs_height;
          top_down = 1;
        }

        // Calculate row size with padding
        uint32_t row_size = (header.width_px * 2 + 3) & ~3;
        uint32_t data_offset = header.offset;

        // Seek to pixel data
        res = f_lseek(&fil, data_offset);
        if (res != FR_OK) {
          f_close(&fil);
          printf("Seek error: %d" ENDL, res);
          return -EIO;
        }

        // Determine drawable area
        uint32_t draw_width = (header.width_px < BMP_scr_width) ? 
                              header.width_px : BMP_scr_width;

        // Line buffer
        uint16_t buffer[MAX_BMP_DRAW_WIDTH];

        // Process each row
        for (int32_t row = 0; row < abs_height; row++) {
          // Read pixel data
          res = f_read(&fil, buffer, draw_width * 2, &bytes_read);
          if (res != FR_OK || bytes_read != draw_width * 2) {
            printf("Read error at row %ld: %d" ENDL, row, res);
            break;
          }

          // Skip padding bytes
          uint32_t skip_bytes = row_size - (draw_width * 2);
          if (skip_bytes > 0) {
            f_lseek(&fil, f_tell(&fil) + skip_bytes);
          }

          // Calculate Y coordinate
          int32_t y;
          if (top_down) {
            y = row;
          } else {
            y = abs_height - 1 - row;
          }

          // Skip off-screen rows
          if (y < 0 || (uint32_t)y >= BMP_scr_heigth) {
            continue;
          }

          // Draw pixels
          for (uint32_t x = 0; x < draw_width; x++) {
            if (x < BMP_scr_width) {
              BMP_draw_pixel(x, y, buffer[x]);
            }
          }
        }

        f_close(&fil);
        printf("BMP loaded successfully!" ENDL);
        return 0;
      }
      break;

    default:
      printf("Invalid command or arguments!" ENDL);
      return -EINVAL;
  }
  
  printf("Invalid command or arguments!" ENDL);
  return -EINVAL;
}
