# 🖥️ Écran TFT SPI 2.8" tactile — ESP32

Support d'un **écran TFT SPI tactile 2.8 pouces** avec un ESP32.

> ⚠️ **Important : la partie tactile n'est pas encore prise en charge.**
> Pour le moment, seul l'affichage sur l'écran TFT est utilisé.

## 📺 Fonctionnalités

* ✅ Écran TFT 2.8"
* ✅ Communication SPI
* ✅ Affichage graphique
* ✅ Compatible ESP32
* ⏳ Support du tactile
* ⏳ Lecture des coordonnées tactiles
* ⏳ Gestion des événements tactiles

---

## 🔌 Câblage

### Connexion écran → ESP32

| Pin écran |       ESP32 | Fonction       |
| --------- | ----------: | -------------- |
| **VCC**   |    **3.3V** | Alimentation   |
| **GND**   |     **GND** | Masse          |
| **CS**    | **GPIO 26** | Chip Select    |
| **RESET** |    **3.3V** | Reset          |
| **DC**    | **GPIO 27** | Data / Command |
| **SDI**   | **GPIO 21** | SPI MOSI       |
| **SCK**   | **GPIO 22** | SPI Clock      |
| **LED**   |    **3.3V** | Rétroéclairage |
| **SDO**   | **GPIO 35** | SPI MISO       |

### Schéma

```text
              ÉCRAN TFT 2.8"
             ┌───────────────┐
             │               │
  VCC ───────┤ VCC       3.3V├──── ESP32 3.3V
  GND ───────┤ GND       GND ├──── ESP32 GND
  CS  ───────┤ CS      GPIO26├──── ESP32 GPIO 26
  RESET ─────┤ RESET     3.3V├──── ESP32 3.3V
  DC  ───────┤ DC      GPIO27├──── ESP32 GPIO 27
  SDI ───────┤ SDI     GPIO21├──── ESP32 GPIO 21
  SCK ───────┤ SCK     GPIO22├──── ESP32 GPIO 22
  LED ───────┤ LED       3.3V├──── ESP32 3.3V
  SDO ───────┤ SDO     GPIO35├──── ESP32 GPIO 35
             │               │
             └───────────────┘
```

---

## ⚙️ Configuration SPI

Les GPIO utilisés pour l'interface SPI sont :

```text
MOSI / SDI → GPIO 21
MISO / SDO → GPIO 35
SCK        → GPIO 22
CS         → GPIO 26
DC         → GPIO 27
```

Le rétroéclairage de l'écran est directement alimenté en **3.3 V** :

```text
LED → 3.3V
```

Le pin RESET est également maintenu à **3.3 V** :

```text
RESET → 3.3V
```

---

## 👆 Tactile

Le module dispose d'une fonction tactile, mais celle-ci **n'est actuellement pas implémentée dans le logiciel**.

### État actuel

```text
Affichage TFT       ✅ Fonctionnel
SPI                 ✅ Fonctionnel
Rétroéclairage      ✅ Fonctionnel
Tactile             ❌ Non pris en charge
Coordonnées X/Y     ❌ Non disponibles
Interface tactile   ❌ Non disponible
```

Le support du contrôleur tactile sera ajouté dans une prochaine évolution du projet.

---

## 🚧 Roadmap

* [x] Initialisation du TFT
* [x] Communication SPI
* [x] Affichage sur l'écran
* [ ] Identification du contrôleur tactile
* [ ] Connexion du tactile
* [ ] Lecture X/Y
* [ ] Calibration
* [ ] Gestion des pressions
* [ ] Intégration du tactile dans l'interface

---

## ⚠️ Attention

Vérifiez que votre écran accepte une alimentation **3.3 V** avant de réaliser le câblage.

Les GPIO utilisés ici correspondent à la configuration actuelle du projet et peuvent être différents selon votre carte ESP32 ou votre version du matériel.

---

## 📁 Projet

Ce projet est destiné à intégrer un écran TFT SPI 2.8" à une interface basée sur ESP32.

Le développement est toujours en cours et de nouvelles fonctionnalités seront ajoutées progressivement.

**Version actuelle : affichage TFT uniquement — tactile à venir.**
