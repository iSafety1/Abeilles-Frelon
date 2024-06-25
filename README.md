# Abeilles-Frelon

*Table des matières :
    -Introduction
    -Gestion du projet
          o Répartition des tâches
          o Utilisation du projet
    -Fonctions/méthodes
    -Compilation
    -Conclusion

-Introduction :
  Ce document a pour but de décrire le déroulement de notre projet de programmation en C. Ce rapport fait la synthèse de l'ensemble des parties du projet. Pour ce faire, Nous allons d'abord exposer sa gestion. Puis, nous allons présenter un guide de son utilisation.     
  Le but du jeu est de simuler les interactions entre des abeilles et des frelons, il se joue à deux (camp abeilles et camp frelons). Les joueurs doivent donc utiliser leur sens de gestion dans le but de détruire le camp ennemi.

-Gestion du projet
    -Utilisation
        Une fois le programme compilé et exécuter nous arrivons sur le plateau :
         L'équipe qui commence en premier est choisie au hasard dès le début du jeu, et chaque équipe commence avec 10 ressources dans son camp.
         La première étape concerne la production. Pour produire quelque chose, choisissez une case sur la grille. Ensuite, la liste des unités disponibles s'affiche, et vous devez sélectionner l'unité que vous souhaitez utiliser pour la production.
         Une fois la production terminée, vous pouvez appuyer sur "fin de commande de production" pour passer à l'étape suivante, qui est le déplacement.
         Lors du déplacement, le processus est similaire. Cliquez d'abord sur une case, puis sur une unité qui apparaît. Enfin, cliquez sur une case adjacente pour planifier son déplacement.
        Lorsque vous avez terminé vos actions, cliquez sur "fin de tour" pour passer la main à l'autre joueur.
         En cas de déplacement de votre unité sur une case occupée par une unité adverse, un combat débute. Le résultat du combat est déterminé aléatoirement à la fin du tour, en fonction de la puissance de votre unité. Si votre unité l'emporte contre un habitat                   ennemi, celui-ci se transforme en un habitat de votre camp. Toutes les unités liées à cet habitat adverse sont détruites.
         La victoire survient lorsqu'il ne reste qu'un seul camp sur toute la grille.
         Vous pouvez enregistrer cette partie en appuyant sur "sauver". Cela va sauvegarder le tour actuel, les ressources des deux camps, ainsi que toutes les unités présentes sur la grille.
         Une fois de retour dans le jeu, vous pouvez charger votre sauvegarde précédente en cliquant sur "charger" et reprendre votre partie.


-Fonctions / méthodes
      • La fonction "init_grille" a pour but d'initialiser la grille de jeu. Elle initialise le tour, les ressources de départ, ainsi que les abeilles et les frelons à NULL. Pour chaque case du plateau, on effectue une initialisation individuelle.
      • La fonction "creer_unite" prend en entrée l'adresse de la grille qui pointe vers le camp (frelon ou abeille), le camp, le type d'unité et la position où cette unité doit être placée. Pour l'insertion de l'unité dans la liste chaînée correspondante au camp, elle          parcourt la liste chaînée avec une boucle while qui s'arrête lorsque la liste est NULL, puis elle insère l'unité à la fin de cette liste.
      • La fonction "supprimer_unite" prend en paramètre l'adresse de l'unité à supprimer. Elle parcourt la liste chaînée jusqu'à trouver l'unité à supprimer, met à jour le chaînage, puis libère la variable avec la fonction "free".
      • La fonction "remplir_grille" est utilisée pour actualiser l'affichage. Elle parcourt les listes chaînées d'abeilles et de frelons pour les placer sur la grille. Pour chaque insecte, elle met à jour le pointeur "vsuiv" en fonction des nouveaux déplacements.
      • La fonction "coo_to_case" prend en paramètre les coordonnées de la souris. Elle renvoie -1 si le clic n'est pas sur la grille de jeu, sinon elle ajuste les coordonnées x et y pour obtenir les coordonnées de la grille de jeu et renvoie 0.
      • La fonction "deplacement" effectue plusieurs vérifications avec des conditions pour déterminer si l'unité est autorisée à se déplacer (par exemple, si c'est un habitat ou une ouvrière qui récolte). Si l'unité satisfait ces conditions, elle est autorisée à se             déplacer. Ensuite, la fonction vérifie si la nouvelle position de l'unité est une case adjacente.
      • La fonction "coo_to_button" prend en paramètre les coordonnées de la souris et l'adresse d'un entier. Si la souris est sur un bouton, alors la fonction modifie l'entier pour obtenir le numéro du bouton correspondant.
      • La fonction "combat" est cruciale pour le jeu, car elle permet la résolution des combats, aboutissant ainsi à la fin de la partie. Au début, elle entre dans une boucle qui se termine lorsque seule un camp reste sur la case. À l'intérieur de la boucle, la                   fonction recherche sur la case une unité de chaque type dans l'ordre de combat. Ces unités sont désignées comme les combattants de leur camp respectif. Ensuite, un tirage au sort est effectué (avec MLV_get_random_integer) pour chaque combattant, en multipliant           ce nombre par la force du combattant. Le combattant avec le tirage le plus bas est déclaré perdant, et l'autre est déclaré gagnant.
      • Si le perdant du combat est un habitat, des actions spéciales sont effectuées : les ressources des frelons sont augmentées ou non, l'habitat est remplacé par son homologue ennemi, et toutes les unités liées à l'habitat perdant sont supprimées.
      • La fonction "sauvegarder" permet de créer un fichier au format txt pour stocker les valeurs des deux camps. En tête du fichier, on place le camp qui est actuellement en tour, les ressources des camps, et le numéro du tour actuel (nous avons choisi d’ajouter le             numéro du tour en cours). Ensuite, la sauvegarde des données des abeilles est effectuée avec une boucle permettant le parcours de la liste chaînée d'abeilles pour récupérer toutes les informations, de même pour les frelons. On utilise la fonction fprintf pour            écrire les lignes de valeurs dans le fichier.
      • La fonction "charger" permet de lire le contenu du fichier généré par la fonction "sauvegarder". Elle récupère ces données dans des variables et les initialise avec la fonction "cree_unite" (pour les frelons et les abeilles).


-Compilation du jeu
      Le jeu n'as pas besoin d'options spécifiques pour être compiler et le fichier cible est "main.c". Par exemple avec clang : "clang –std=c17 –Wall –Wfatal-errors main.c –o main -lMLV”


-Difficulté rencontrée et bugs non résolus
      Actuellement, nous rencontrons un seul problème : lors de la sauvegarde, nous ne prenons pas en compte les productions en cours.
