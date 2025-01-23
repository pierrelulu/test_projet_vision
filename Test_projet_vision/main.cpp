#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cfloat>   // pour FLT_MAX si besoin
#include <algorithm>
#include "../include/ImageNdg.hpp"
#include "../include/ImageCouleur.hpp"

//
// ----------------------------------------------------------
// Exemple d'utilisation GLOBALE (pseudo-main) 
// ----------------------------------------------------------
//
//  Bien sûr, vous adapterez la lecture/écriture via vos propres
//  appels (par ex. CImageNdg img("fichier.bmp")) et (img.sauvegarde("xxx.bmp")),
//  mais l'enchaînement des traitements sera le même.
//

int main()
{
    // Charger l'image
    CImageCouleur imageDepthColor("line_calib_depth.bmp");

    // Étape 1 : Extraire le plan bleu (dans ce cas, l'image est déjà en niveaux de gris)
    CImageNdg imageDepth = imageDepthColor.plan(2);
    imageDepth.sauvegarde("line_calib_depth_origine");

    // 2) Squelettisation et labélisation
    CImageNdg labelsDepth, squeletteDepth;
    int numDepth;
    process_image(imageDepth, labelsDepth, numDepth, squeletteDepth);

    // 3) Correction des lignes
    std::vector< std::vector< std::pair<int,int> > > correctionsDepth;
    CImageNdg correctedLabelsDepth;
    correctLines(labelsDepth, numDepth, correctionsDepth, correctedLabelsDepth);

    // 4) Construction du champ de correction, interpolation, flou gaussien
    std::vector<std::vector<float>> corrFieldDepth;
    buildCorrectionField(labelsDepth, numDepth, correctionsDepth, corrFieldDepth);
    verticalInterpolation(corrFieldDepth);

    // Flou gaussien
    std::vector<std::vector<float>> corrFieldDepthBlur;
    gaussianBlur2D(corrFieldDepth, 20.0f, corrFieldDepthBlur);

    // 5) Appliquer ce champ à une nouvelle image "line_calib_volume.jpg"
    CImageNdg imageVolume("line_calib_volume.bmp");  // à adapter
    CImageNdg labelsVolume, squeletteVolume;
    int numVolume;
    process_image(imageVolume, labelsVolume, numVolume, squeletteVolume);
    squeletteVolume.sauvegarde("squeletteVolume");

    CImageNdg correctedVolume;
    applyCorrectionField(labelsVolume, corrFieldDepthBlur, correctedVolume);
    correctedVolume.sauvegarde("correctedVolume");
    // 6) (Eventuellement) relabelliser/corriger cette nouvelle image, etc.
    //    pour l'estimation d'échelle selon votre script final

    // 7) Extraction des points 3D
    std::vector<float> x, y, z;
    build3DPointsFromCorrection(corrFieldDepthBlur, x, y, z);
    centerAndScalePoints(x, y, z, 1.0f);

    // 8) Écriture .ply (à adapter à votre code)
    //    ...
    std::cout << "Traitement terminé. Toutes les étapes ont été sauvegardées." << std::endl;
    return 0;
}



/*
int main() {
    try {
        // Charger l'image
        CImageCouleur image("line_test.bmp");

        // Étape 1 : Extraire le plan bleu (dans ce cas, l'image est déjà en niveaux de gris)
        CImageNdg planBleu = image.plan(2);

        // Sauvegarder l'image plan bleu
        planBleu.sauvegarde("01_plan_bleu");

        // Étape 2 : Seuillage Otsu
        CImageNdg imageSeuillage = planBleu.seuillage("manuel", 230, 255);

        // Sauvegarder l'image seuillée
        imageSeuillage.sauvegarde("02_image_seuillee");

        // Étape 3 : Dilatation
        elemStruct eltStructDilatation = elemStruct::V8(); // Élément structurant en voisinage-8
        CImageNdg imageDilatee = imageSeuillage.morphologie("dilatation", eltStructDilatation);

        // Sauvegarder l'image dilatée
        imageDilatee.sauvegarde("03_image_dilatee");

        // Étape 4 : Érosion
        CImageNdg imageFermee = imageDilatee.morphologie("erosion", eltStructDilatation);

        // Sauvegarder l'image après fermeture
        imageFermee.sauvegarde("04_image_fermee");

        // Étape 5 : Squelettisation (Zhang-Suen)
        CImageNdg imageSquelette = imageFermee;
        bool change = true;

        while (change) {
            change = false;
            CImageNdg toRemove(imageFermee.lireHauteur(), imageFermee.lireLargeur(), 0);

            // Phase 1
            for (int i = 1; i < imageFermee.lireHauteur() - 1; ++i) {
                for (int j = 1; j < imageFermee.lireLargeur() - 1; ++j) {
                    if (imageSquelette(i, j) == 1) { //255
                        // Obtenir les 8 voisins
                        int N = imageSquelette(i - 1, j);
                        int S = imageSquelette(i + 1, j);
                        int E = imageSquelette(i, j + 1);
                        int W = imageSquelette(i, j - 1);
                        int NE = imageSquelette(i - 1, j + 1);
                        int NW = imageSquelette(i - 1, j - 1);
                        int SE = imageSquelette(i + 1, j + 1);
                        int SW = imageSquelette(i + 1, j - 1);

                        // Compter les transitions 0->1 dans le voisinage (sens horaire)
                        int neighbors[] = { N, NE, E, SE, S, SW, W, NW, N };
                        int transitions = 0;
                        for (int k = 0; k < 8; ++k) {
                            if (neighbors[k] == 0 && neighbors[k + 1] == 1) { //255
                                transitions++;
                            }
                        }

                        // Compter le nombre de voisins blancs
                        int neighborsSum = N + S + E + W + NE + NW + SE + SW;

                        // Conditions de suppression (Phase 1)
                        if (transitions == 1 && neighborsSum >= 2 * 1 && neighborsSum <= 6 * 1 && //1, 255, 255
                            (N * E * S == 0) && (E * S * W == 0)) {
                            toRemove(i, j) = 1; //255
                            change = true;
                        }
                    }
                }
            }

            // Supprimer les pixels marqués dans Phase 1
            for (int i = 0; i < imageFermee.lireHauteur(); ++i) {
                for (int j = 0; j < imageFermee.lireLargeur(); ++j) {
                    if (toRemove(i, j) == 1) { //255
                        imageSquelette(i, j) = 0;
                    }
                }
            }

            // Réinitialiser les pixels à supprimer pour Phase 2
            toRemove = CImageNdg(imageFermee.lireHauteur(), imageFermee.lireLargeur(), 0);

            // Phase 2
            for (int i = 1; i < imageFermee.lireHauteur() - 1; ++i) {
                for (int j = 1; j < imageFermee.lireLargeur() - 1; ++j) {
                    if (imageSquelette(i, j) == 1) { //255
                        // Obtenir les 8 voisins
                        int N = imageSquelette(i - 1, j);
                        int S = imageSquelette(i + 1, j);
                        int E = imageSquelette(i, j + 1);
                        int W = imageSquelette(i, j - 1);
                        int NE = imageSquelette(i - 1, j + 1);
                        int NW = imageSquelette(i - 1, j - 1);
                        int SE = imageSquelette(i + 1, j + 1);
                        int SW = imageSquelette(i + 1, j - 1);

                        // Compter les transitions 0->1 dans le voisinage (sens horaire)
                        int neighbors[] = { N, NE, E, SE, S, SW, W, NW, N };
                        int transitions = 0;
                        for (int k = 0; k < 8; ++k) {
                            if (neighbors[k] == 0 && neighbors[k + 1] == 1) { //255
                                transitions++;
                            }
                        }

                        // Compter le nombre de voisins blancs
                        int neighborsSum = N + S + E + W + NE + NW + SE + SW;

                        // Conditions de suppression (Phase 2)
                        if (transitions == 1 && neighborsSum >= 2 * 1 && neighborsSum <= 6 * 1 && //1, 255, 255
                            (N * E * W == 0) && (N * S * W == 0)) {
                            toRemove(i, j) = 1; //255
                            change = true;
                        }
                    }
                }
            }

            // Supprimer les pixels marqués dans Phase 2
            for (int i = 0; i < imageFermee.lireHauteur(); ++i) {
                for (int j = 0; j < imageFermee.lireLargeur(); ++j) {
                    if (toRemove(i, j) == 1) { //255
                        imageSquelette(i, j) = 0;
                    }
                }
            }
        }

        // Sauvegarder l'image squelettisée
        imageSquelette.sauvegarde("05_image_squelette");

        std::cout << "Traitement terminé. Toutes les étapes ont été sauvegardées." << std::endl;

    }
    catch (const std::string& err) {
        std::cerr << "Erreur : " << err << std::endl;
        return -1;
    }

    return 0;
}
*/

