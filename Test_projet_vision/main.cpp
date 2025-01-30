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
    try
    {
        // =============== PARTIE 1 : IMAGE PERSPECTIVE ===============
        // 1) Charger l'image (couleur) "perspective"
        CImageCouleur imageDepthColor("images/1.bmp");

        // 2) Extraire le plan bleu -> NDG
        CImageNdg imageDepth = imageDepthColor.plan(2);
        imageDepth.sauvegarde("01_line_calib_depth_origine");

        // 3) Squelettisation et labélisation
        CImageNdg labelsDepth, squeletteDepth;
        int numDepth = 0;
        process_image(imageDepth, labelsDepth, numDepth, squeletteDepth);

        // 4) Correction des lignes => correctedLabelsDepth
        std::vector<std::vector<std::pair<int, int>>> correctionsDepth;
        CImageNdg correctedLabelsDepth;
        correctLines(labelsDepth, numDepth, correctionsDepth, correctedLabelsDepth);

        // 5) Champ de correction => corrFieldDepth
        std::vector<std::vector<float>> corrFieldDepth;
        buildCorrectionField(labelsDepth, numDepth, correctionsDepth, corrFieldDepth);

        // Interpolation verticale
        verticalInterpolation(corrFieldDepth);

        // Flou gaussien => corrFieldDepthBlur
        std::vector<std::vector<float>> corrFieldDepthBlur;
        gaussianBlur2D(corrFieldDepth, 20.0f, corrFieldDepthBlur);

        // 6) Calcul de scale_y (mm/pixel) depuis "correctedLabelsDepth"
        //    On convertit correctedLabelsDepth en matrice d'entiers :
        int hDepth = correctedLabelsDepth.lireHauteur();
        int wDepth = correctedLabelsDepth.lireLargeur();
        std::vector<std::vector<int>> correctedLabelsInt(hDepth, std::vector<int>(wDepth, 0));

        for (int i = 0; i < hDepth; i++)
        {
            for (int j = 0; j < wDepth; j++)
            {
                correctedLabelsInt[i][j] = (int)correctedLabelsDepth(i, j);
            }
        }

        float scaleY = computeScaleY(correctedLabelsInt, numDepth);
        //TEMPOROAIRE TODO
        //scaleY /= 10;

        // scaleX = scaleY
        float scaleX = scaleY;
        std::cout << "[Info] scaleY (mm/pixel) = " << scaleY << std::endl;

        // =============== PARTIE 2 : IMAGE VOLUME ===============
        // 7) Charger la deuxième image (volume)
        CImageNdg imageVolume("images_obj/1.bmp");

        // Squelettisation et labélisation
        CImageNdg labelsVolume, squeletteVolume;
        int numVolume = 0;
        process_image(imageVolume, labelsVolume, numVolume, squeletteVolume);
        squeletteVolume.sauvegarde("02_squeletteVolume");

        // 8) Appliquer le champ corrFieldDepthBlur sur la 2e image => correctedVolume
        CImageNdg correctedVolume;
        applyCorrectionField(labelsVolume, corrFieldDepthBlur, correctedVolume);
        correctedVolume.sauvegarde("03_correctedVolume");

        // === NOUVEAU : Re-labellisation / Correction #2 comme en MATLAB
        // 9) correctLines(...) sur correctedVolume => correctedLabels2
        std::vector<std::vector<std::pair<int, int>>> corrections2;
        CImageNdg correctedLabels2;
        correctLines(correctedVolume, numVolume, corrections2, correctedLabels2);
        correctedLabels2.sauvegarde("04_corrected_labels2");

        // 10) buildCorrectionField(...) => depthMap2
        std::vector<std::vector<float>> depthMap2;
        buildCorrectionField(correctedVolume, numVolume, corrections2, depthMap2);
        // On pourrait l'enregistrer via mat2gray si on veut (ex: recompose en NDG)

        // 11) Calcul de scale_z (mm/pixel) en se basant sur correctedVolume et depthMap2
        //     On convertit correctedVolume en std::vector<std::vector<int>>
        int hVol = correctedVolume.lireHauteur();
        int wVol = correctedVolume.lireLargeur();
        std::vector<std::vector<int>> newLabelsInt(hVol, std::vector<int>(wVol, 0));
        for (int i = 0; i < hVol; i++)
        {
            for (int j = 0; j < wVol; j++)
            {
                newLabelsInt[i][j] = (int)correctedVolume(i, j);
            }
        }

        float scaleZ = computeScaleZ(newLabelsInt, depthMap2, 50.0f);
        std::cout << "[Info] scaleZ (mm/pixel) = " << scaleZ << std::endl;

        // =============== PARTIE 3 : GÉNÉRATION DU NUAGE 3D FINAL ===============
        // 12) Construire le nuage (X2, Y2, Z2) depuis correctedLabels2 + depthMap2
        //     On applique la même transformation x=-col, y=row - z, z=-z  (param "true")
        std::vector<float> X2, Y2, Z2;
        build3DPointsFromCorrection(correctedVolume, depthMap2, X2, Y2, Z2, true);

        // 13) Appliquer l'échelle et recadrer
        //     On a scaleX=scaleY (déjà calculé) et scaleZ
        scaleAndShiftCloud(X2, Y2, Z2, scaleX, scaleY, scaleZ);

        // 14) Sauvegarder le nuage en .ply
        exportPointsToPLY(X2, Y2, Z2, "../output/my_cloud_final.ply");

        // 15) (Optionnel) Recomposer une image NDG depuis (X2, Y2, Z2)
        //     juste pour visualiser
        CImageNdg recompose2 = recomposeImageFromXYZ(X2, Y2, Z2, 128);
        recompose2.sauvegarde("05_recompose_from_xyz2");

        std::cout << "Traitement terminé. Toutes les étapes ont été sauvegardées." << std::endl;
    }
    catch (const std::string& err)
    {
        std::cerr << "Erreur : " << err << std::endl;
        return -1;
    }

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

