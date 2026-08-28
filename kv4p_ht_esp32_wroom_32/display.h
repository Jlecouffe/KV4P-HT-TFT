#pragma once

#include <Arduino.h>

// État d'affichage rempli par le .ino avant chaque displayLoop().
// display.cpp ne lit QUE des types primitifs -> aucune dépendance
// aux headers internes (globals.h / protocol.h), donc pas de double
// définition au link.
struct RadioUiState {
  float    freqMhz   = 0.0f;   // 0 => affiche "---.---"
  bool     tx        = false;  // true = émission
  bool     stopped   = true;   // true = radio à l'arrêt
  bool     highPower  = true;  // flag HOST_STATE_HIGH_POWER
  uint8_t  rssi      = 0;      // 0-255, pour le S-mètre
  uint8_t  volume    = 0;      // 0-8
  uint8_t  squelch   = 0;      // 0-9
  bool     squelched = true;   // true = pas de signal (muet)
};

extern RadioUiState uiState;

void displaySetup();
void displayLoop();