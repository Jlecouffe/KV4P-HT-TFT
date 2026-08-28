🖥️ Support écran TFT SPI tactile 2.8" — ESP32

Ajout du support pour un écran TFT SPI tactile 2,8 pouces sur ESP32.

L’affichage fonctionne actuellement en SPI.
⚠️ La partie tactile n’est pas encore prise en charge dans cette version. Elle sera intégrée ultérieurement.

🔌 Câblage
Écran TFT	ESP32
VCC	3.3V
GND	GND
CS	GPIO 26
RESET	3.3V
DC	GPIO 27
SDI / MOSI	GPIO 21
SCK	GPIO 22
LED / BL	3.3V
SDO / MISO	GPIO 35
Schéma rapide
        TFT 2.8" SPI
        ┌─────────────┐
VCC  ───┤ 3.3V        │
GND  ───┤ GND         │
CS   ───┤ GPIO 26     │
RESET───┤ 3.3V        │
DC   ───┤ GPIO 27     │
SDI  ───┤ GPIO 21     │
SCK  ───┤ GPIO 22     │
LED  ───┤ 3.3V        │
SDO  ───┤ GPIO 35     │
        └─────────────┘
📌 État actuel
✅ Affichage TFT SPI
✅ Communication SPI
✅ Initialisation de l'écran
✅ Affichage des informations sur le TFT
⏳ Tactile non pris en charge pour le moment
⏳ Gestion des interactions tactiles à venir
⚠️ Remarque sur le tactile

Le contrôleur tactile est présent sur le module, mais aucune gestion du tactile n'est actuellement implémentée dans le projet.

L'écran peut donc être utilisé normalement pour l'affichage, mais toucher l'écran n'aura pour l'instant aucun effet.

🛠️ Connexions SPI utilisées
MOSI / SDI → GPIO 21
MISO / SDO → GPIO 35
SCK        → GPIO 22
CS         → GPIO 26
DC         → GPIO 27

Le RESET est maintenu à 3.3 V et le rétroéclairage LED est également alimenté en 3.3 V.

🚧 À venir

Les prochaines étapes seront notamment :

Ajouter le contrôleur tactile.
Définir les GPIO utilisés par le tactile.
Ajouter la lecture des coordonnées X/Y.
Ajouter la gestion des événements tactiles.
Utiliser le tactile pour contrôler l'interface.

Projet en cours de développement — contributions et retours bienvenus.
