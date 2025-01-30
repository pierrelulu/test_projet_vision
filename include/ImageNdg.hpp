#ifndef _IMAGE_NDG_
#define _IMAGE_NDG_

#define _EXPORT_ __declspec(dllexport)

#include <algorithm>
#include <string>
#include <vector>
#include "windows.h"
#include "../include/elemStruct.h"

#define PI 3.14159265358979323846
#define MAGIC_NUMBER_BMP ('B' + ('M' << 8))


typedef struct
{
    int   minNdg;
    int   maxNdg;
    int   medianeNdg;
    float moyenneNdg;
    float ecartTypeNdg;
} MOMENTS;

struct POINT_POLAIRE
{
    int rho;
    int theta;

    std::string toString() const
    {
        return "rho: " + std::to_string(rho) + "\ttheta: " + std::to_string(theta) + "\n";
    }
};

class CImageNdg
{
private:
    int             m_iHauteur;
    int             m_iLargeur;
    bool            m_bBinaire;
    std::string     m_sNom;
    unsigned char * m_pucPixel;

    unsigned char * m_pucPalette;

public:
    // constructeurs
    _EXPORT_ CImageNdg();                                       // par defaut
    _EXPORT_ CImageNdg(int hauteur, int largeur, int val = -1); // si -1 alors non pixels non initialises
    _EXPORT_ CImageNdg(const std::string nom);                  // fichier en entree
    _EXPORT_ CImageNdg(const CImageNdg & im);                   // image en entree

    // destructeur
    _EXPORT_ ~CImageNdg();

    // sauvegarde au format bmp
    _EXPORT_ void sauvegarde(const std::string file = ""); // sauvegarde data au format BMP

    // pouvoir acceder e un pixel par image(i)
    _EXPORT_ unsigned char & operator()(int i) const
    {
        int ti = max(0, min(i, m_iHauteur * m_iLargeur - 1));
        return m_pucPixel[ti];
    }

    // pouvoir acceder e un pixel par image(i,j)
    _EXPORT_ unsigned char & operator()(int i, int j) const
    {
        int ti = max(0, min(i, m_iHauteur - 1));
        int tj = max(0, min(j, m_iLargeur - 1));
        return this->operator()(ti * m_iLargeur + tj);
    }

    // pouvoir acceder e un pixel par image(i,j)
    _EXPORT_ CImageNdg operator-(CImageNdg &other)
    {
        CImageNdg temp(*this);

        return temp.operation(other, "-");
    }

    // operateur copie image par imOut = imIn
    _EXPORT_ CImageNdg & operator=(const CImageNdg & im);

    // get et set

    _EXPORT_ int lireHauteur() const
    {
        return m_iHauteur;
    }

    _EXPORT_ int lireLargeur() const
    {
        return m_iLargeur;
    }

    _EXPORT_ bool lireBinaire() const
    {
        return m_bBinaire;
    }

    _EXPORT_ std::string lireNom() const
    {
        return m_sNom;
    }

    _EXPORT_ void ecrireHauteur(int hauteur)
    {
        m_iHauteur = hauteur;
    }

    _EXPORT_ void ecrireLargeur(int largeur)
    {
        m_iLargeur = largeur;
    }

    _EXPORT_ void ecrireBinaire(bool type)
    {
        m_bBinaire = type;
    }

    _EXPORT_ void ecrireNom(std::string nom)
    {
        m_sNom = nom;
    }

    _EXPORT_ int lireNbPixels() const
    {
        return m_iHauteur * m_iLargeur;
    }

    _EXPORT_ CImageNdg & choixPalette(std::string name = "grise")
    {
        if (name.compare("rouge") == 0)
        {
            for (int indice = 0; indice < 256; indice++)
            {
                this->m_pucPalette[indice * 4]     = 0;
                this->m_pucPalette[indice * 4 + 1] = 0;
                this->m_pucPalette[indice * 4 + 2] = (unsigned char)(indice);
                this->m_pucPalette[indice * 4 + 3] = 0;
            }
        }
        else if (name.compare("verte") == 0)
        {
            for (int indice = 0; indice < 256; indice++)
            {
                this->m_pucPalette[indice * 4]     = 0;
                this->m_pucPalette[indice * 4 + 1] = (unsigned char)(indice);
                this->m_pucPalette[indice * 4 + 2] = 0;
                this->m_pucPalette[indice * 4 + 3] = 0;
            }
        }
        else if (name.compare("bleue") == 0)
        {
            for (int indice = 0; indice < 256; indice++)
            {
                this->m_pucPalette[indice * 4]     = (unsigned char)(indice);
                this->m_pucPalette[indice * 4 + 1] = 0;
                this->m_pucPalette[indice * 4 + 2] = 0;
                this->m_pucPalette[indice * 4 + 3] = 0;
            }
        }
        else if (name.compare("binaire") == 0)
        {
            // fond toujours e 0
            this->m_pucPalette[0]  = 0;
            this->m_pucPalette[1]  = 0;
            this->m_pucPalette[2]  = 0;
            this->m_pucPalette[3]  = 0;

            // 8 couleurs de base
            // ensuite periodicite, de sorte que l'objet 1 a le meme label que l'objet 9 par exemple
            this->m_pucPalette[4]  = 255;
            this->m_pucPalette[5]  = 0;
            this->m_pucPalette[6]  = 0;
            this->m_pucPalette[7]  = 0;
            this->m_pucPalette[8]  = 0;
            this->m_pucPalette[9]  = 255;
            this->m_pucPalette[10] = 0;
            this->m_pucPalette[11] = 0;
            this->m_pucPalette[12] = 0;
            this->m_pucPalette[13] = 0;
            this->m_pucPalette[14] = 255;
            this->m_pucPalette[15] = 0;
            this->m_pucPalette[16] = 255;
            this->m_pucPalette[17] = 255;
            this->m_pucPalette[18] = 0;
            this->m_pucPalette[19] = 0;
            this->m_pucPalette[20] = 0;
            this->m_pucPalette[21] = 255;
            this->m_pucPalette[22] = 255;
            this->m_pucPalette[23] = 0;
            this->m_pucPalette[24] = 255;
            this->m_pucPalette[25] = 0;
            this->m_pucPalette[26] = 255;
            this->m_pucPalette[27] = 0;
            this->m_pucPalette[28] = 0;
            this->m_pucPalette[29] = 128;
            this->m_pucPalette[30] = 255;
            this->m_pucPalette[31] = 0;
            this->m_pucPalette[32] = 128;
            this->m_pucPalette[33] = 255;
            this->m_pucPalette[34] = 128;
            this->m_pucPalette[35] = 0;

            for (int indice = 9; indice < 256; indice++)
            {
                this->m_pucPalette[indice * 4]     = this->m_pucPalette[indice * 4 - 32];
                this->m_pucPalette[indice * 4 + 1] = this->m_pucPalette[indice * 4 + 1 - 32];
                ;
                this->m_pucPalette[indice * 4 + 2] = this->m_pucPalette[indice * 4 + 2 - 32];
                ;
                this->m_pucPalette[indice * 4 + 3] = 0;
            }
        }
        else // palette gris
            for (int indice = 0; indice < 256; indice++)
            {
                this->m_pucPalette[indice * 4]     = (unsigned char)(indice);
                this->m_pucPalette[indice * 4 + 1] = (unsigned char)(indice);
                this->m_pucPalette[indice * 4 + 2] = (unsigned char)(indice);
                this->m_pucPalette[indice * 4 + 3] = 0;
            }
        return *this;
    }

    _EXPORT_ CImageNdg & choixPalette(std::vector<int> map)
    {
        // couleurs donnees par tableau de 256 couleurs RGB)
        for (int indice = 0; indice < 256; indice++)
        {
            this->m_pucPalette[indice * 4]     = (unsigned char)map[indice * 3 + 2];
            this->m_pucPalette[indice * 4 + 1] = (unsigned char)map[indice * 3 + 1];
            this->m_pucPalette[indice * 4 + 2] = (unsigned char)map[indice * 3];
            this->m_pucPalette[indice * 4 + 3] = 0;
        }
        return *this;
    }

    _EXPORT_ std::vector<int> lirePalette() const
    {
        // palette image sortie au format tableau de 256 couleurs RGB
        std::vector<int> palette;
        palette.resize(256 * 3);
        for (int indice = 0; indice < 256; indice++)
        {
            palette[indice * 3 + 2] = this->m_pucPalette[indice * 4];
            palette[indice * 3 + 1] = this->m_pucPalette[indice * 4 + 1];
            palette[indice * 3]     = this->m_pucPalette[indice * 4 + 2];
        }
        return palette;
    }

    // histogramme
    _EXPORT_ std::vector<unsigned long> histogramme(bool enregistrementCSV = false);

    // signatures
    // methode avec histogramme prealablement calcule
    _EXPORT_ MOMENTS signatures(const std::vector<unsigned long> h);
    // methode sans histo prealablement calcule
    _EXPORT_ MOMENTS signatures();

    // operations ensemblistes pour images binaires
    // im1 = im1 operation im2;
    // options : "et", "ou", "-"
    _EXPORT_ CImageNdg & operation(const CImageNdg & im, const std::string methode = "et");

    // Score
    // _EXPORT_ CImageNdg& score(const CImageNdg& im, const std::string methode = "iou");

    // seuillage
    // options : "manuel", "otsu"
    _EXPORT_ CImageNdg seuillage(
        const std::string methode = "otsu", int seuilBas = 128,
        int seuilHaut = 255); // seuillage automatique Otsu par defaut, sinon "manuel" et choix des seuils Bas et Haut

    // transformation
    // options : "complement", "expansion", "egalisation"
    _EXPORT_ CImageNdg transformation(const std::string methode = "complement");

    // morphologie
    // m�thode options : "erosion", "dilatation"
    // �l�ment structurant options : "V4" ou "V8"
    _EXPORT_ CImageNdg morphologie(const std::string methode = "dilatation", const elemStruct& eltStructurant = elemStruct::V8());

    // Filtrage
    // options : "moyennage", "median"
    _EXPORT_ CImageNdg filtrage(const std::string & methode = "moyennage", int Ni = 3,
                                int Nj = 3); // choix "moyennage" / "median"

    // Transformee de Hough
    _EXPORT_ CImageNdg hough(int threshold = 140, bool colorForEachLine = false);

    _EXPORT_ CImageNdg horizontalConcatenate(const CImageNdg & im);
    _EXPORT_ CImageNdg verticalConcatenate(const CImageNdg & im);

    _EXPORT_ double correlation_croisee_normalisee(const CImageNdg& imgRef);

    

};

#endif // _IMAGE_NDG_

_EXPORT_ void zhangSuenSkeletonization(CImageNdg& binImg);

_EXPORT_ void connectedComponents(
    const CImageNdg& binImg,
    CImageNdg& labels,
    int& numLabels);

_EXPORT_ void process_image(
    const CImageNdg& inputColorOrGray,
    CImageNdg& labels,
    int& numLabels,
    CImageNdg& imageSquelette);

_EXPORT_ void correctLines(
    const CImageNdg& labels,
    int numLabels,
    std::vector<std::vector<std::pair<int, int>>>& corrections,
    CImageNdg& correctedLabels
);

_EXPORT_ void buildCorrectionField(
    const CImageNdg& labels,
    int numLabels,
    const std::vector<std::vector<std::pair<int, int>>>& corrections,
    std::vector<std::vector<float>>& correctionField
);

_EXPORT_ void verticalInterpolation(std::vector<std::vector<float>>& corrField);

_EXPORT_ void gaussianBlur2D(
    const std::vector<std::vector<float>>& input,
    float sigma,
    std::vector<std::vector<float>>& output
);

_EXPORT_ void applyCorrectionField(
    const CImageNdg& newLabels,
    const std::vector<std::vector<float>>& correctionField,
    CImageNdg& correctedNewLabels
);

_EXPORT_ void build3DPointsFromCorrection(
    const CImageNdg& correctedVolume,
    const std::vector<std::vector<float>>& corrField,
    std::vector<float>& X,
    std::vector<float>& Y,
    std::vector<float>& Z,
    bool applyTransform
);

_EXPORT_ void centerAndScalePoints(
    std::vector<float>& xcoords,
    std::vector<float>& ycoords,
    std::vector<float>& zcoords,
    float scale_factor
);

_EXPORT_ float computeScaleY(
    const std::vector<std::vector<int>>& correctedLabels,
    int num
);

_EXPORT_ float computeScaleZ(
    const std::vector<std::vector<int>>& newLabels,
    const std::vector<std::vector<float>>& depthMap,
    float realMaxDiffMm
);

_EXPORT_ void scaleAndShiftCloud(
    std::vector<float>& X,
    std::vector<float>& Y,
    std::vector<float>& Z,
    float scaleX,
    float scaleY,
    float scaleZ
);

_EXPORT_ CImageNdg recomposeImageFromXYZ(
    const std::vector<float>& X,
    const std::vector<float>& Y,
    const std::vector<float>& Z,
    unsigned char valFond
);

_EXPORT_ void exportPointsToPLY(
    const std::vector<float>& X,
    const std::vector<float>& Y,
    const std::vector<float>& Z,
    const std::string& filename
);