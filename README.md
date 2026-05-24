# 🤖 Navigation Autonome - Robot Cherokey 4WD

Ce dépôt rassemble les différents algorithmes de navigation et de traitement d'image développés pour le robot **Cherokey 4WD** (DFRobot) basé sur une architecture Arduino (ATmega328P / ATmega2560).

L'objectif de ce projet est de doter le robot de capacités d'adaptation face à son environnement à travers trois modes de fonctionnement distincts.

---

## 🛠️ Matériel Utilisé
* **Châssis :** Cherokey 4WD Mobile Platform (pont en H intégré)
* **Microcontrôleur :** Arduino Uno / Mega
* **Capteur de distance :** URM37 Ultrasons (Mode PWM)
* **Caméra IA :** HuskyLens (DFRobot) connecté en I2C
* **Actuateur :** Micro Servo-moteur 9g (pour le balayage du capteur)

---

## 🗂️ Architecture des Codes

Le projet est divisé en 3 scripts indépendants, optimisés pour éviter le gel du processeur grâce à l'utilisation de tâches cadencées (`Metro.h`) plutôt que des fonctions bloquantes (`delay`).

### 1. 🌀 Résolution de Labyrinthe (`/01-labyrinthe`)
* **Principe :** Navigation autonome par évitement d'obstacles pour sortir d'un labyrinthe.
* **Logique :** * Un servo fait osciller le capteur ultrason entre 60° et 120° pour scanner l'horizon en continu.
  * L'exécution utilise une **machine à états finis** stricte (`DRIVE_FORWARD` ➔ `EVADE_BACK` ➔ `EVADE_TURN`).
  * En cas d'obstacle ($\le 25$ cm), le robot analyse la position du servo pour en déduire la direction du mur, s'arrête, effectue un recul franc, puis pivote du côté libre.
    
## Pour en savoir plus sur le cablage du servo moteur et du capteur ultrason utilisé:https://wiki.dfrobot.com/rob0117/docs/21533 .



### 🎯 2. Suivi de Ligne Automatique (`/02-suivi_de_ligne`)
* **Principe :** Suivi de trajectoire au sol (ruban noir sur fond blanc par exemple!).
* **Logique :**
  * Utilisation des capteurs infrarouges / HuskyLens dédiés au suivi de ligne.
  * Gestion fine de la vitesse différentielle des moteurs droit et gauche du Cherokey 4WD pour négocier les virages serrés sans déraper.
    
 ## Pour bien configuré la fonction "line tracking " du Huskylens:https://wiki.dfrobot.com/sen0305/docs/22638

### 🛣️ 3. Suivi de Cible par Couleur (`/03-suivi_de_couleur`)
* **Principe :** Suivi visuel en temps réel d'un objet ou d'une couleur spécifique.
* **Logique :**
  * Utilisation de l'algorithme de reconnaissance de couleur (`COLOR_RECOGNITION`) de la caméra HuskyLens.
  * Récupération instantanée du centre X du bloc détecté. Si l'objet sort du champ de vision, le robot déclenche un frein électronique d'urgence (`LOST`).
  * Si l'objet bouge, le robot ajuste sa trajectoire via une zone morte (`DEAD_ZONE`) centrale pour le suivre de manière fluide.

## Apprendre un peu plus sur la fonction "COLOR_RECOGNITION" :https://wiki.dfrobot.com/sen0305/docs/22646

---

## 💻 Installation et Déploiement

1. Clonez ce dépôt sur votre machine :
   ```bash
   git clone [https://github.com/VOTRE_PSEUDO/robot-cherokey-navigation.git](https://github.com/VOTRE_PSEUDO/robot-cherokey-navigation.git)







