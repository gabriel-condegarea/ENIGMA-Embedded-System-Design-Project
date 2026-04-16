# ENIGMA Project
Projet Multi HEIG-VD
2026

The aim of the project is to design and build a fully functional, accurate prototype of the Enigma machine using modern technology. The Enigma machine was used in various forms from 1930 until the end of the Second World War to encrypt and decrypt messages for commercial and military purposes. Since then, the machine has become highly sought-after
by history and cryptography museums.

## Présentation du Projet ENIGMA-Embedded-System-Design-Project

**Contexte et Objectif**  
Ce projet vise à concevoir et construire un prototype fonctionnel et précis de la machine Enigma, célèbre pour son rôle dans le chiffrement de messages pendant la Seconde Guerre mondiale. L'approche moderne intègre des technologies embarquées pour recréer fidèlement le mécanisme mécanique original, tout en permettant le chiffrement et déchiffrement de messages. Il s'agit d'un projet académique (Multi HEIG-VD, 2026) combinant algorithmique et ingénierie matérielle.

**Technologies Utilisées**  
- **Logiciel** : Langage C (standard C11) pour l'implémentation de l'algorithme de chiffrement Enigma. Compilation via GCC avec un Makefile pour automatiser le build.  
- **Matériel** : Plateformes Arduino pour les composants embarqués, incluant :  
  - Moteurs pas à pas (pour simuler la rotation des rotors physiques).  
  - LED WS2812B (via bibliothèque Adafruit NeoPixel) pour l'affichage des lettres chiffrées.  
  - Claviers matriciels (scanning de touches).  
  - Capteurs magnétiques et analogiques pour la détection de positions (ex. : pôles N/S pour identifier les rotors).  

**Structure du Projet**  
Le projet est organisé autour d'un dossier racine avec un README.md descriptif, et un sous-dossier `Tests/` contenant :  
- `algorithme/` : Cœur logiciel en C.  
  - `enigma.h` : Définitions des structures (rotors, machine Enigma) et prototypes de fonctions (rotation, chiffrement).  
  - `enigma.c` : Implémentation des fonctions de chiffrement (rotation des rotors, passage avant/arrière, cycle des rotors).  
  - `main.c` : Point d'entrée avec commentaires détaillés sur les configurations historiques des rotors et réflecteurs Enigma.  
  - `Makefile` : Automatisation de la compilation (cible `app` pour l'exécutable).  
- Tests matériels individuels (fichiers `.ino` pour Arduino) :  
  - `Keyboard_LED.ino` : Test du clavier matriciel + affichage LED.  
  - `LED_simple.ino` : Test d'effets visuels sur LED NeoPixel.  
  - `magnetic_sensor.ino` : Test de capteurs magnétiques pour détection de pôles.  
  - `Rotor_ID.ino` : Test d'identification des rotors via capteurs.  
  - `Stepper.ino` : Contrôle de moteurs pas à pas pour rotation mécanique.  

**Architecture Générale**  
- **Couche Logicielle** : Implémentation pure de l'algorithme Enigma en C. La machine est modélisée avec des rotors (structures avec offset, position, câblage) et un réflecteur. Le chiffrement suit le processus historique : passage à travers les rotors (avant), réflexion, puis retour (arrière), avec rotation automatique des rotors.  
- **Couche Matérielle** : Intégration embarquée via Arduino. Les composants physiques (rotors mécaniques, clavier, LED) sont contrôlés pour synchroniser avec la logique logicielle. Les tests isolent chaque composant pour validation.  
- **Interface** : Entrée via clavier matriciel ; sortie via LED pour visualisation des lettres chiffrées.  

**Flux Principal de l'Application**  
1. **Entrée** : L'utilisateur appuie sur une touche du clavier matriciel (scanné par Arduino).  
2. **Chiffrement Logiciel** : La lettre est passée à l'algorithme Enigma (rotation des rotors virtuels, substitution via câblages).  
3. **Action Matérielle** : Les moteurs pas à pas tournent les rotors physiques pour refléter l'état logique. Les capteurs vérifient les positions.  
4. **Sortie** : La lettre chiffrée est affichée sur les LED WS2812B.  
Le processus est réversible (chiffrement = déchiffrement grâce au réflecteur).  

Ce projet illustre une fusion entre cryptographie historique et ingénierie moderne, idéal pour l'apprentissage de systèmes embarqués et d'algorithmes de sécurité. Pour un nouveau développeur, commencez par compiler et exécuter `Tests/algorithme/app` pour tester l'algorithme, puis flashez les sketches Arduino pour les composants matériels.