#include "display.h"

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// ---------------------------------------------------------------------------
//  Broches (identiques à ton test qui marche)
// ---------------------------------------------------------------------------
#define TFT_SCK   22
#define TFT_MOSI  21
#define TFT_CS    27
#define TFT_DC    26
#define TFT_MISO  -1
#define TFT_RST   -1

// ---------------------------------------------------------------------------
//  Écran en paysage : 320 large x 240 haut
// ---------------------------------------------------------------------------
static const int16_t W = 320;
static const int16_t H = 240;

// Puissance affichée (le module SA/DRA818 fait ~1W en HIGH, pas 5W).
// Change ces libellés si tu veux autre chose.
static const char* POWER_HI = "1W";
static const char* POWER_LO = "0.5W";

// ---------------------------------------------------------------------------
//  Thème "minimal flat" : fond sombre, cartes sombres, accent par mode
// ---------------------------------------------------------------------------
static uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) { return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3); }

static const uint16_t C_BG     = rgb(10, 12, 16);    // fond
static const uint16_t C_CARD   = rgb(24, 28, 36);    // cartes
static const uint16_t C_TEXT   = rgb(236, 240, 245); // texte principal
static const uint16_t C_MUTED  = rgb(120, 130, 145); // labels
static const uint16_t C_RX     = rgb(46, 204, 113);  // accent réception (vert)
static const uint16_t C_TX     = rgb(231, 76, 60);   // accent émission (rouge)
static const uint16_t C_IDLE   = rgb(90, 100, 115);  // accent arrêt (gris)
static const uint16_t C_SEG_OFF= rgb(38, 44, 54);    // segment S-mètre éteint

RadioUiState uiState;  // définition (déclarée extern dans display.h)

Adafruit_ILI9341 tft(&SPI, TFT_CS, TFT_DC, TFT_RST);

// Accent courant selon le mode
static uint16_t accentColor() {
  if (uiState.stopped) return C_IDLE;
  return uiState.tx ? C_TX : C_RX;
}

// ---------------------------------------------------------------------------
//  Helpers de dessin
// ---------------------------------------------------------------------------
static void drawCenteredText(const char* s, int16_t cx, int16_t y, uint8_t size, uint16_t color) {
  tft.setTextSize(size);
  tft.setTextColor(color);
  int16_t x1, y1; uint16_t w, h;
  tft.getTextBounds(s, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor(cx - w / 2, y);
  tft.print(s);
}

// Cadre statique : dessiné une seule fois
static void drawStaticLayout() {
  tft.fillScreen(C_BG);

  // Header
  tft.fillRoundRect(8, 6, W - 16, 34, 6, C_CARD);
  tft.setTextSize(2);
  tft.setTextColor(C_TEXT);
  tft.setCursor(18, 14);
  tft.print("KV4P-HT");

  // Carte fréquence
  tft.fillRoundRect(8, 48, W - 16, 74, 8, C_CARD);

  // Carte S-mètre
  tft.fillRoundRect(8, 130, W - 16, 42, 8, C_CARD);
  tft.setTextSize(1);
  tft.setTextColor(C_MUTED);
  tft.setCursor(18, 138);
  tft.print("S-METER");

  // 3 cellules VOL / SQL / PWR
  const char* labels[3] = { "VOL", "SQL", "PWR" };
  for (int i = 0; i < 3; i++) {
    int16_t x = 8 + i * ((W - 16) / 3) + (i ? 4 : 0);
    int16_t w = (W - 16) / 3 - 4;
    tft.fillRoundRect(x, 180, w, 34, 6, C_CARD);
    tft.setTextSize(1);
    tft.setTextColor(C_MUTED);
    tft.setCursor(x + 8, 186);
    tft.print(labels[i]);
  }

  // Footer boutons
  tft.setTextSize(1);
  tft.setTextColor(C_MUTED);
  tft.setCursor(30, 224);   tft.print("[ MENU ]");
  tft.setCursor(W - 90, 224); tft.print("[ PTT ]");
}

// ---------------------------------------------------------------------------
//  Zones dynamiques (redessinées seulement quand la valeur change)
// ---------------------------------------------------------------------------
static void drawModeBadge() {
  const char* txt = uiState.stopped ? "OFF" : (uiState.tx ? "TX" : "RX");
  int16_t bw = 74, bx = W - 8 - bw - 6, by = 12, bh = 22;
  tft.fillRoundRect(bx, by, bw, bh, 5, accentColor());
  tft.setTextSize(2);
  tft.setTextColor(C_BG);
  int16_t x1, y1; uint16_t w, h;
  tft.getTextBounds(txt, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor(bx + (bw - w) / 2, by + (bh - h) / 2);
  tft.print(txt);
}

static void drawFreq() {
  char buf[12];
  if (uiState.freqMhz > 1.0f) snprintf(buf, sizeof(buf), "%.3f", uiState.freqMhz);
  else                        snprintf(buf, sizeof(buf), "---.---");

  tft.fillRect(12, 56, W - 24, 42, C_CARD);          // efface l'ancienne valeur
  drawCenteredText(buf, W / 2 - 22, 60, 4, C_TEXT);   // gros chiffres
  tft.setTextSize(2);
  tft.setTextColor(accentColor());
  tft.setCursor(W - 62, 74);
  tft.print("MHz");

  // Ligne mode + puissance sous la fréquence
  tft.fillRect(12, 100, W - 24, 18, C_CARD);
  tft.setTextSize(2);
  tft.setTextColor(C_MUTED);
  tft.setCursor(18, 101);
  tft.print(uiState.stopped ? "IDLE" : (uiState.tx ? "TX" : "RX"));
  tft.setCursor(W - 90, 101);
  tft.print(uiState.highPower ? POWER_HI : POWER_LO);
}

static void drawSMeter() {
  const int segs = 10;
  int16_t x0 = 18, y0 = 150, gap = 3;
  int16_t segW = (W - 16 - 20 - (segs - 1) * gap) / segs;
  int16_t segH = 14;

  int active = 0;
  if (!uiState.squelched) active = map(uiState.rssi, 0, 255, 1, segs);
  active = constrain(active, 0, segs);

  uint16_t on = accentColor();
  for (int i = 0; i < segs; i++) {
    int16_t x = x0 + i * (segW + gap);
    tft.fillRoundRect(x, y0, segW, segH, 2, (i < active) ? on : C_SEG_OFF);
  }
}

static void drawCellValue(int idx, const String& val) {
  int16_t x = 8 + idx * ((W - 16) / 3) + (idx ? 4 : 0);
  int16_t w = (W - 16) / 3 - 4;
  tft.fillRect(x + 6, 196, w - 12, 14, C_CARD);   // efface ancienne valeur
  tft.setTextSize(2);
  tft.setTextColor(C_TEXT);
  tft.setCursor(x + 8, 195);
  tft.print(val);
}

// ---------------------------------------------------------------------------
//  API
// ---------------------------------------------------------------------------
void displaySetup() {
  pinMode(TFT_CS, OUTPUT); digitalWrite(TFT_CS, HIGH);
  pinMode(TFT_DC, OUTPUT); digitalWrite(TFT_DC, HIGH);

  SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);
  tft.begin();
  tft.setRotation(1);   // paysage 320x240

  drawStaticLayout();
  // Force un premier rendu complet des valeurs
  drawModeBadge();
  drawFreq();
  drawSMeter();
  drawCellValue(0, String(uiState.volume));
  drawCellValue(1, String(uiState.squelch));
  drawCellValue(2, uiState.highPower ? POWER_HI : POWER_LO);
}

void displayLoop() {
  // On mémorise le dernier état rendu pour ne redessiner que le nécessaire.
  static RadioUiState prev;
  static bool first = true;

  bool modeChanged = first || prev.tx != uiState.tx || prev.stopped != uiState.stopped;
  bool freqChanged = first || prev.freqMhz != uiState.freqMhz || prev.highPower != uiState.highPower || modeChanged;
  bool rssiChanged = first || prev.rssi != uiState.rssi || prev.squelched != uiState.squelched || modeChanged;

  if (modeChanged) drawModeBadge();
  if (freqChanged) drawFreq();
  if (rssiChanged) drawSMeter();

  if (first || prev.volume   != uiState.volume)   drawCellValue(0, String(uiState.volume));
  if (first || prev.squelch  != uiState.squelch)  drawCellValue(1, String(uiState.squelch));
  if (first || prev.highPower != uiState.highPower) drawCellValue(2, uiState.highPower ? POWER_HI : POWER_LO);

  prev  = uiState;
  first = false;
}