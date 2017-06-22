# LivraisonDrone

Simulation de flotte de drone de livraison réalisée dans le cadre de l'UV de LO41 à l'UTBM.
Ce programme tient compte des différents prérequis des clients et des contraintes sur 
les drones afin de définir une situation réaliste de déploiment de la flotte de drone. 

# Compilation 
Un MAKEFILE a été créé pour permettre la compilation du programme en utilisant la commande make.
Un make clean permet de nettoyer les fichiers de compilations.

# Exécution
Pour exécuter le programme, il faut utiliser la commande ./Drone en se trouvant dans le répertoire du projet avec un terminal bash. Il est possible de spécifier le nombre de drone de chaque types en ajoutant des paramètres après ./Drone.

#
Ex : ./Drone 5 3 2 20 permet de lancer le programme avec 5 petits drones, 3 drones de moyenne taille, 2 gros drones et 20 clients.
Le 4ème paramètre (nombre de clients) n'est pas obligatoire et sera de 15 par défaut s'il n'est pas indiqué.


#
Réalisé par Florian RIFFLART et Pierrick GRAF.
