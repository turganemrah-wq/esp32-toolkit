// ============================================================
// tft_setup.h
// 3.5" ILI9486 480x320 SPI ekran icin TFT_eSPI ayar dosyasi.
//
// ONEMLI: Bu dosyayi Arduino Cloud'da sketch'ine IKINCI BIR
// SEKME/DOSYA olarak ekle, adini TAM OLARAK "tft_setup.h" yap.
// Kutuphane bu dosyayi kendisi otomatik bulup kullanir,
// baska bir ayar yapmana gerek yok.
// ============================================================

#define ILI9486_DRIVER

#define TFT_WIDTH  320
#define TFT_HEIGHT 480

#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST   4

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

#define SPI_FREQUENCY  20000000
