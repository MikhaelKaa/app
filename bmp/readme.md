# BMP Image Viewer for STM32 Microcontrollers

## Overview
This module provides BMP image loading and display functionality for STM32 microcontrollers with ILI9341 displays and FATFS filesystem support. The implementation features minimal memory usage (640-byte buffer) and supports standard 16-bit BMP images.

## Features
- Load and display BMP images from FATFS-formatted storage
- Support for 16-bit RGB565 BMP format
- Automatic handling of top-down and bottom-up image orientation
- Image cropping for oversized images
- Minimal memory footprint (640-byte line buffer)
- Command-line interface for testing and diagnostics

## Requirements
### Image Format
- **Bits per pixel:** 16-bit (RGB565)
- **Compression:** Uncompressed (0) or BI_BITFIELDS (3)
- **Orientation:** Top-down or bottom-up
- **Size:** Any (will be cropped to screen dimensions)

### Tested on hardware
- STM32 microcontroller
- ILI9341 display (320x240)
- FATFS-compatible storage (SD card)

## Usage
### Initialization
In your main application code, initialize the display parameters:

```c
// Set pixel drawing callback
BMP_draw_pixel = your_draw_pixel_function;

// Set display dimensions
BMP_scr_width = 320;    // Display width in pixels
BMP_scr_heigth = 240;   // Display height in pixels
```

Commands

Use the following commands via your serial interface:

#### Display BMP header information:

```bash
bmp head 0:/image.bmp
```

#### Display test pattern:

```bash
bmp test 123
```

#### Load and display BMP image:

```bash
bmp load 0:/image.bmp
```

Behavior
* Oversized images: Cropped to fit screen dimensions

* Undersized images: Displayed in top-left corner

* Bottom-up images: Automatically flipped vertically

* Memory usage: Uses 640-byte buffer (320 pixels × 2 bytes)

Implementation Notes

* The BMP_draw_pixel function pointer must be implemented to handle pixel drawing

* Display dimensions must be set before image loading

* Images are loaded line-by-line for minimal memory usage

* File operations use FATFS library functions