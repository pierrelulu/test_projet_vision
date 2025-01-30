#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "../include/ImageDouble.hpp"
#include "../include/ImageNdg.hpp"

// constructeurs et destructeur
CImageNdg::CImageNdg()
{

    this->m_iHauteur   = 0;
    this->m_iLargeur   = 0;
    this->m_bBinaire   = false;
    this->m_sNom       = "vide";

    this->m_pucPixel   = NULL;
    this->m_pucPalette = NULL;
}

CImageNdg::CImageNdg(int hauteur, int largeur, int valeur)
{

    this->m_iHauteur   = hauteur;
    this->m_iLargeur   = largeur;
    this->m_bBinaire   = false; // Image Ndg par d�faut, binaire apr�s seuillage
    this->m_sNom       = "inconnu";

    this->m_pucPixel   = new unsigned char[hauteur * largeur];
    this->m_pucPalette = new unsigned char[256 * 4];
    choixPalette("grise"); // palette grise par d�faut, choix utilisateur
    if (valeur != -1)
        for (int i = 0; i < this->lireNbPixels(); i++)
            this->m_pucPixel[i] = valeur;
}

CImageNdg::CImageNdg(const std::string name)
{

    BITMAPFILEHEADER header;
    BITMAPINFOHEADER infoHeader;

    std::ifstream f(name.c_str(), std::ios::in | std::ios::binary);
    if (f.is_open())
    {
        f.read((char *)&header.bfType, 2);
        f.read((char *)&header.bfSize, 4);
        f.read((char *)&header.bfReserved1, 2);
        f.read((char *)&header.bfReserved2, 2);
        f.read((char *)&header.bfOffBits, 4);
        if (header.bfType != MAGIC_NUMBER_BMP)
            throw std::string("ouverture format BMP impossible ...");
        else
        {
            f.read((char *)&infoHeader.biSize, 4);
            f.read((char *)&infoHeader.biWidth, 4);
            f.read((char *)&infoHeader.biHeight, 4);
            f.read((char *)&infoHeader.biPlanes, 2);
            f.read((char *)&infoHeader.biBitCount, 2);
            f.read((char *)&infoHeader.biCompression, 4);
            f.read((char *)&infoHeader.biSizeImage, 4);
            f.read((char *)&infoHeader.biXPelsPerMeter, 4);
            f.read((char *)&infoHeader.biYPelsPerMeter, 4);
            f.read((char *)&infoHeader.biClrUsed, 4);
            f.read((char *)&infoHeader.biClrImportant, 4);
            if (infoHeader.biCompression > 0)
                throw std::string("Format compresse non supporte...");
            else
            {
                if (infoHeader.biBitCount == 8)
                {
                    this->m_iHauteur = infoHeader.biHeight;
                    this->m_iLargeur = infoHeader.biWidth;
                    this->m_bBinaire = false;
                    this->m_sNom.assign(name.begin(), name.end() - 4);
                    this->m_pucPalette = new unsigned char[256 * 4];
                    this->m_pucPixel   = new unsigned char[infoHeader.biHeight * infoHeader.biWidth];

                    // g�rer multiple de 32 bits via z�ros �ventuels ignor�s
                    int complement     = (((this->m_iLargeur - 1) / 4) + 1) * 4 - this->m_iLargeur;
                    for (int indice = 0; indice < 4 * 256; indice++)
                        f.read((char *)&this->m_pucPalette[indice], sizeof(char));

                    for (int i = this->m_iHauteur - 1; i >= 0; i--)
                    {
                        for (int j = 0; j < this->m_iLargeur; j++)
                            f.read((char *)&this->m_pucPixel[i * this->m_iLargeur + j], sizeof(char));

                        char inutile;
                        for (int k = 0; k < complement; k++)
                            f.read((char *)&inutile, sizeof(char));
                    }
                }
                else
                {
                    // cas d'une image couleur
                    this->m_iHauteur = infoHeader.biHeight;
                    this->m_iLargeur = infoHeader.biWidth;
                    this->m_bBinaire = false;
                    this->m_sNom.assign(name.begin(), name.end() - 4);
                    this->m_pucPalette = new unsigned char[256 * 4];
                    this->choixPalette("grise"); // palette grise par d�faut
                    this->m_pucPixel = new unsigned char[infoHeader.biHeight * infoHeader.biWidth];

                    // extraction plan luminance
                    int complement   = (((this->m_iLargeur * 3 - 1) / 4) + 1) * 4 - this->m_iLargeur * 3;
                    for (int i = this->m_iHauteur - 1; i >= 0; i--)
                    {
                        for (int j = 0; j < this->m_iLargeur * 3; j += 3)
                        {
                            unsigned char rouge, vert, bleu;
                            f.read((char *)&rouge, sizeof(char));
                            f.read((char *)&vert, sizeof(char));
                            f.read((char *)&bleu, sizeof(char));
                            this->m_pucPixel[i * this->m_iLargeur + j / 3] =
                                (unsigned char)(((int)rouge + (int)vert + (int)bleu) / 3);
                        }

                        char inutile;
                        for (int k = 0; k < complement; k++)
                            f.read((char *)&inutile, sizeof(char));
                    }
                }
            }
        }
        f.close();
    }
    else
        throw std::string("ERREUR : Image absente (ou pas ici en tout cas) !");
}

CImageNdg::CImageNdg(const CImageNdg & im)
{

    this->m_iHauteur   = im.lireHauteur();
    this->m_iLargeur   = im.lireLargeur();
    this->m_bBinaire   = im.lireBinaire();
    this->m_sNom       = im.lireNom();
    this->m_pucPixel   = NULL;
    this->m_pucPalette = NULL;

    if (im.m_pucPalette != NULL)
    {
        this->m_pucPalette = new unsigned char[256 * 4];
        memcpy(this->m_pucPalette, im.m_pucPalette, 4 * 256);
    }
    if (im.m_pucPixel != NULL)
    {
        this->m_pucPixel = new unsigned char[im.lireHauteur() * im.lireLargeur()];
        memcpy(this->m_pucPixel, im.m_pucPixel, im.lireNbPixels());
    }
}

CImageNdg::~CImageNdg()
{
    if (this->m_pucPixel)
    {
        delete[] this->m_pucPixel;
        this->m_pucPixel = NULL;
    }

    if (this->m_pucPalette)
    {
        delete[] this->m_pucPalette;
        this->m_pucPalette = NULL;
    }
}

void CImageNdg::sauvegarde(const std::string file)
{
    BITMAPFILEHEADER header;
    BITMAPINFOHEADER infoHeader;

    if (!this->m_pucPixel)
        throw std::string("Pas de donnee a sauvegarder !");

    // Dossier ou chemin cible (à adapter)
    std::string nomFichier = "..\\output\\";
    if (!file.empty())
        nomFichier += file + ".bmp";
    else
        nomFichier += this->lireNom() + ".bmp";

    std::ofstream f(nomFichier.c_str(), std::ios::binary);
    if (!f.is_open())
        throw std::string("Impossible de creer le fichier de sauvegarde !");

    // Calcul du "padding" pour que chaque ligne soit multiple de 4 octets
    int complement = (((this->m_iLargeur - 1) / 4) + 1) * 4 - this->m_iLargeur;

    // Remplir l'en‐tête BMP
    header.bfType = MAGIC_NUMBER_BMP;
    f.write((char*)&header.bfType, 2);

    header.bfOffBits = 14 /*Taille bfHeader*/ + 40 /*Taille biHeader*/ + 4 * 256 /*Palette*/;
    header.bfSize = header.bfOffBits + (complement + lireLargeur()) * lireHauteur() * sizeof(char);

    header.bfReserved1 = 0;
    header.bfReserved2 = 0;

    f.write((char*)&header.bfSize, 4);
    f.write((char*)&header.bfReserved1, 2);
    f.write((char*)&header.bfReserved2, 2);
    f.write((char*)&header.bfOffBits, 4);

    infoHeader.biSize = 40;
    infoHeader.biWidth = this->m_iLargeur;
    infoHeader.biHeight = this->m_iHauteur;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 8;     // On écrit en 8 bits
    infoHeader.biCompression = 0;
    infoHeader.biSizeImage = this->lireNbPixels();
    infoHeader.biXPelsPerMeter = 0;
    infoHeader.biYPelsPerMeter = 0;
    infoHeader.biClrUsed = 256;
    infoHeader.biClrImportant = 0;

    f.write((char*)&infoHeader.biSize, 4);
    f.write((char*)&infoHeader.biWidth, 4);
    f.write((char*)&infoHeader.biHeight, 4);
    f.write((char*)&infoHeader.biPlanes, 2);
    f.write((char*)&infoHeader.biBitCount, 2);
    f.write((char*)&infoHeader.biCompression, 4);
    f.write((char*)&infoHeader.biSizeImage, 4);
    f.write((char*)&infoHeader.biXPelsPerMeter, 4);
    f.write((char*)&infoHeader.biYPelsPerMeter, 4);
    f.write((char*)&infoHeader.biClrUsed, 4);
    f.write((char*)&infoHeader.biClrImportant, 4);

    // Palette (256 * 4 octets)
    for (int indice = 0; indice < 4 * 256; indice++)
        f.write((char*)&this->m_pucPalette[indice], sizeof(char));

    // Écriture des pixels (de bas en haut)
    for (int i = this->m_iHauteur - 1; i >= 0; i--)
    {
        for (int j = 0; j < m_iLargeur; j++)
        {
            // Récupérer la valeur 0 ou 1 (ou 0..255 si c’est déjà une image NDG)
            unsigned char val = this->m_pucPixel[i * m_iLargeur + j];

            // Si vous voulez explicitement forcer toute image binaire 0/1
            // à devenir 0/255, vous pouvez faire :
            // if (val <= 1) val = (val == 1) ? 255 : 0;

            // ---- Option 1) Si vous TENEZ pour binaire => 1 => 255, 0 => 0 :
            // if (this->m_bBinaire && val == 1) val = 255;

            // ---- Option 2) Si vous supposez simplement "val <= 1" => val*255 :
            // if (val <= 1) val = (unsigned char)(val * 255);

            // Par exemple :
            if (val <= 1) val *= 255;

            f.write((char*)&val, sizeof(char));
        }

        // Gérer le padding
        char inutile = 0;
        for (int k = 0; k < complement; k++)
            f.write((char*)&inutile, sizeof(char));
    }
    f.close();
}

CImageNdg & CImageNdg::operator=(const CImageNdg & im)
{

    if (&im == this)
        return *this;

    this->m_iHauteur = im.lireHauteur();
    this->m_iLargeur = im.lireLargeur();
    this->m_bBinaire = im.lireBinaire();
    this->m_sNom     = im.lireNom();

    if (this->m_pucPixel)
        delete[] this->m_pucPixel;
    this->m_pucPixel = new unsigned char[this->m_iHauteur * this->m_iLargeur];

    if (this->m_pucPalette)
        delete[] this->m_pucPalette;
    this->m_pucPalette = new unsigned char[256 * 4];

    if (im.m_pucPalette != NULL)
        memcpy(this->m_pucPalette, im.m_pucPalette, 4 * 256);
    if (im.m_pucPixel != NULL)
        memcpy(this->m_pucPixel, im.m_pucPixel, im.lireNbPixels());

    return *this;
}

// fonctionnalit�s histogramme
std::vector<unsigned long> CImageNdg::histogramme(bool enregistrementCSV)
{

    std::vector<unsigned long> h;

    h.resize(256, 0);
    for (int i = 0; i < this->lireNbPixels(); i++)
        h[this->operator()(i)] += 1L;

    if (enregistrementCSV)
    {
        std::string   fichier = "res/" + this->lireNom() + ".csv";
        std::ofstream f(fichier.c_str());

        if (!f.is_open())
            throw std::string("Impossible d'ouvrir le fichier en ecriture !");
        else
        {
            for (int i = 0; i < (int)h.size(); i++)
                f << h[i] << std::endl;
        }
        f.close();
    }

    return h;
}

// signatures globales
MOMENTS CImageNdg::signatures(const std::vector<unsigned long> h)
{
    MOMENTS globales;

    // min
    int i = 0;
    while ((i < (int)h.size()) && (h[i] == 0))
        i++;
    globales.minNdg = i;

    // max
    i               = h.size() - 1;
    while ((i > 0) && (h[i] == 0))
        i--;
    globales.maxNdg = i;

    // mediane
    int moitPop     = this->lireNbPixels() / 2;

    i               = globales.minNdg;
    int somme       = h[i];
    while (somme < moitPop)
    {
        i += 1;
        if (i < (int)h.size())
            somme += h[i];
    }
    globales.medianeNdg = i;

    // moyenne et �cart-type
    float moy = 0, sigma = 0;
    for (i = globales.minNdg; i <= globales.maxNdg; i++)
    {
        moy += ((float)h[i]) * i;
        sigma += ((float)h[i]) * i * i;
    }
    moy /= (float)this->lireNbPixels();
    sigma                 = sqrt(sigma / (float)this->lireNbPixels() - (moy * moy));
    globales.moyenneNdg   = moy;
    globales.ecartTypeNdg = sigma;

    return globales;
}

MOMENTS CImageNdg::signatures()
{
    MOMENTS globales = {0, 0, 0, 0, 0};

    std::vector<unsigned long> hist;
    hist     = this->histogramme();

    globales = this->signatures(hist);

    return globales;
}

// op�rations ensemblistes images binaires
CImageNdg & CImageNdg::operation(const CImageNdg & im, const std::string methode)
{

    this->m_iHauteur = im.lireHauteur();
    this->m_iLargeur = im.lireLargeur();
    this->m_bBinaire = im.lireBinaire();
    this->choixPalette("binaire"); // images binaires -> palettes binaires
    this->m_sNom = im.lireNom() + "Op";

    if (methode.compare("et") == 0)
    {
        if ((&im == this) || !(this->lireBinaire() && im.lireBinaire()))
        {
            throw std::string("Operation logique uniquement possible entre 2 images binaires");
            return *this;
        }

        for (int i = 0; i < this->lireNbPixels(); i++)
            this->operator()(i) = this->operator()(i) && im(i);
    }
    else if (methode.compare("ou") == 0)
    {
        if ((&im == this) || !(this->lireBinaire() && im.lireBinaire()))
        {
            throw std::string("Operation logique uniquement possible entre 2 images binaires");
            return *this;
        }

        for (int i = 0; i < this->lireNbPixels(); i++)
            this->operator()(i) = this->operator()(i) || im(i);
    }
    else if (methode.compare("-") == 0)
    {
        for (int i = 0; i < this->lireNbPixels(); i++)
            this->operator()(i) = this->operator()(i) - im(i); // vigilance sur les images op�r�r�es !
    }

    return *this;
}

// seuillage
CImageNdg CImageNdg::seuillage(const std::string methode, int seuilBas, int seuilHaut)
{

    if (!this->m_bBinaire)
    {
        CImageNdg out(this->lireHauteur(), this->lireLargeur());
        out.choixPalette("binaire"); // palette binaire par d�faut
        out.m_bBinaire = true;

        // cr�ation lut pour optimisation calcul
        std::vector<int> lut;
        lut.resize(256);

        // recherche valeur seuil
        // cas "manuel"
        if (methode.compare("manuel") == 0)
        {
            out.m_sNom = this->lireNom() + "SeMa";
        }
        else if (methode.compare("otsu") == 0)
        {
            out.m_sNom                      = this->lireNom() + "SeAu";
            // recherche seuil via Otsu
            std::vector<unsigned long> hist = this->histogramme();
            std::vector<unsigned long> histC; // histogramme cumul�
            histC.resize(256, 0);
            histC[0] = hist[0];
            for (int i = 1; i < (int)hist.size(); i++)
                histC[i] = histC[i - 1] + hist[i];

            MOMENTS globales = this->signatures(hist);
            int     min = globales.minNdg, max = globales.maxNdg;

            // f(s)
            std::vector<double> tab;
            tab.resize(256, 0);

            double M1, M2, w1;

            // initialisation
            M1        = min;
            seuilBas  = min;
            seuilHaut = 255;

            w1        = (double)histC[min] / (double)(this->lireNbPixels());
            M2        = 0;
            for (int i = min + 1; i <= max; i++)
                M2 += (double)hist[i] * i;
            M2 /= (double)(histC[max] - hist[min]);
            tab[min] = w1 * (1 - w1) * (M1 - M2) * (M1 - M2);

            for (int i = min + 1; i < max; i++)
            {
                M1     = ((double)histC[i - 1] * M1 + (double)hist[i] * i) / histC[i];
                M2     = ((double)(histC[255] - histC[i - 1]) * M2 - hist[i] * i) / (double)(histC[255] - histC[i]);
                w1     = (double)histC[i] / (double)(this->lireNbPixels());
                tab[i] = w1 * (1 - w1) * (M1 - M2) * (M1 - M2);
                if (tab[i] > tab[seuilBas])
                    seuilBas = i;
            }
        }
        else
        {
            // gestion des seuils valant "moyenne" et "mediane"
            MOMENTS globales = this->signatures();
            if (methode.compare("moyenne") == 0)
            {
                out.m_sNom = this->lireNom() + "SeMo";
                seuilBas   = (int)globales.moyenneNdg;
            }
            else if (methode.compare("mediane") == 0)
            {
                out.m_sNom = this->lireNom() + "SeMe";
                seuilBas   = (int)globales.medianeNdg;
            }
        }

        // g�n�ration lut
        for (int i = 0; i < seuilBas; i++)
            lut[i] = 0;
        for (int i = seuilBas; i <= seuilHaut; i++)
            lut[i] = 1;
        for (int i = seuilHaut + 1; i <= 255; i++)
            lut[i] = 0;

        // cr�ation image seuill�e
        //std::cout << "Seuillage des pixels entre " << seuilBas << " et " << seuilHaut << std::endl;
        for (int i = 0; i < out.lireNbPixels(); i++)
            out(i) = lut[this->operator()(i)];

        return out;
    }
    else
    {
        throw std::string("Seuillage image binaire impossible");
        return (*this);
    }
}

// transformation
CImageNdg CImageNdg::transformation(const std::string methode)
{
    CImageNdg out(this->lireHauteur(), this->lireLargeur());

    std::vector<int> map = this->lirePalette();
    out.choixPalette(map);             // conservation de la palette
    out.m_bBinaire = this->m_bBinaire; // conservation du type

    if (methode.compare("complement") == 0)
    {
        out.m_sNom = this->lireNom() + "TComp";
        // ndg -> 255-ndg
        // cr�ation lut pour optimisation calcul
        std::vector<int> lut;
        lut.resize(256);

        for (int i = 0; i < 256; i++)
        {
            if (out.m_bBinaire)
                lut[i] = 1 - i;
            else
                lut[i] = 255 - i;
        }
        for (int i = 0; i < out.lireNbPixels(); i++)
            out(i) = lut[this->operator()(i)];
    }
    else if (methode.compare("expansion") == 0)
    {
        out.m_sNom = this->lireNom() + "TExp";
        int min = 255, max = 0;
        for (int i = 0; i < this->lireNbPixels(); i++)
        {
            if (this->operator()(i) > max)
                max = this->operator()(i);
            if (this->operator()(i) < min)
                min = this->operator()(i);
        }

        double a = 255 / (double)(max - min);
        double b = -a * min;

        std::vector<int> lut;
        lut.resize(256);

        for (int i = 0; i < 256; i++)
            lut[i] = (int)(a * i + b);
        for (int i = 0; i < out.lireNbPixels(); i++)
            out(i) = lut[this->operator()(i)];
    }
    else if (methode.compare("egalisation") == 0)
    {
        out.m_sNom                    = this->lireNom() + "TEga";
        std::vector<unsigned long> h  = this->histogramme();
        std::vector<unsigned long> hC = h;
        for (int i = 1; i < (int)h.size(); i++)
            hC[i] = hC[i - 1] + h[i];

        // recherche min et max image
        int min = 0, max = 255;
        int i = 0;
        while (h[i] == 0)
            i++;
        min = i;
        i   = 255;
        while (h[i] == 0)
            i--;
        max = i;

        std::vector<int> lut;
        lut.resize(256);

        for (int i = min; i <= max; i++)
            lut[i] = (int)(((double)hC[i] / (double)this->lireNbPixels()) * (double)255);

        for (int i = 0; i < out.lireNbPixels(); i++)
            out(i) = lut[this->operator()(i)];
    }

    return out;
}

// morphologie
CImageNdg CImageNdg::morphologie(const std::string methode, const elemStruct& eltStructurant)
{

    CImageNdg out(this->lireHauteur(), this->lireLargeur());

    out.choixPalette(this->lirePalette()); // conservation de la palette

    out.m_bBinaire = this->m_bBinaire; // conservation du type

    if (methode.compare("erosion") == 0)
    {
        int dx = eltStructurant.lireHauteur() / 2;
        int dy = eltStructurant.lireLargeur() / 2;

        unsigned char minVal = 255;

        for (int i = 0; i < this->lireHauteur(); i++) {
            for (int j = 0; j < this->lireLargeur(); j++) {
                for (int k = -dx; k <= dx; k++) {
                    for (int l = -dy; l <= dy; l++) {
                        if (eltStructurant(k + dx, l + dy) == 0)
                            continue;

                        minVal = min(minVal, this->operator()(i + k, j + l)); // Gestion des bords par l'accesseur (<0 -> 0, >max -> max)
                    }
                }

                out(i, j) = minVal;
                minVal = 255;
            }
        }
    }
    else if (methode.compare("dilatation") == 0)
    {
        int dx = eltStructurant.lireHauteur() / 2;
        int dy = eltStructurant.lireLargeur() / 2;

        unsigned char maxVal = 0;

        for (int i = 0; i < this->lireHauteur(); i++){
            for (int j = 0; j < this->lireLargeur(); j++){
                for (int k = -dx; k <= dx; k++) {
                    for (int l = -dy; l <= dy; l++){
                        if (eltStructurant(k + dx, l + dy) == 0)
                            continue;

						maxVal = max(maxVal, this->operator()(i + k, j + l)); // Gestion des bords par l'accesseur (<0 -> 0, >max -> max)
					}
				}
                
                out(i, j) = maxVal;
                maxVal = 0;
            }
        }
    }
    else if (methode.compare("ouverture") == 0) {
          
        out = this->morphologie("erosion", eltStructurant).morphologie("dilatation", eltStructurant);
        //out.ecrireNom(this->lireNom() + "Mouv");
    }

    else if (methode.compare("fermeture") == 0) {
           
        out = this->morphologie("dilatation", eltStructurant).morphologie("erosion", eltStructurant);
        //out.ecrireNom(this->lireNom() + "Mouv");
    }

    else if (methode.compare("BTH") == 0) {
          
        CImageNdg ouverture = this->morphologie("ouverture", eltStructurant);
        out = *this - ouverture;
        //out.ecrireNom(this->lireNom() + "MBth");
    }

    else if (methode.compare("WTH") == 0) {
           
        CImageNdg fermeture = this->morphologie("fermeture", eltStructurant);
        out = fermeture - *this;
        //out.ecrireNom(this->lireNom() + "Mwth");
    }

    else if (methode.compare("moyenne")) {
        int dx = eltStructurant.lireHauteur() / 2;
        int dy = eltStructurant.lireLargeur() / 2;
        int surface = 0;

        //Calcul de la surface non-nulle de l'element structurant
        for (int i = 0; i < eltStructurant.lireHauteur(); i++)
            for (int j = 0; j < eltStructurant.lireLargeur(); j++)
				surface += (int)eltStructurant(i, j);


        int moyVal = 0;

        for (int i = 0; i < this->lireHauteur(); i++) {
            for (int j = 0; j < this->lireLargeur(); j++) {
                for (int k = -dx; k <= dx; k++) {
                    for (int l = -dy; l <= dy; l++) {
                        moyVal += (int)eltStructurant(k + dx, l + dy) * this->operator()(i + k, j + l); // Gestion des bords par l'accesseur (<0 -> 0, >max -> max)
                    }
                }

                out(i, j) = moyVal/surface;
                moyVal = 0;
            }
        }
        //out.ecrireNom(this->lireNom() + "Moy");
	}

    else if (methode.compare("median")) {
        int dx = eltStructurant.lireHauteur() / 2;
        int dy = eltStructurant.lireLargeur() / 2;
        std::vector<char> voisinage;

        for (int i = 0; i < this->lireHauteur(); i++) {
            for (int j = 0; j < this->lireLargeur(); j++) {
                for (int k = -dx; k <= dx; k++) {
                    for (int l = -dy; l <= dy; l++) {
                        if (eltStructurant(k + dx, l + dy) == 0)
                            continue;

                        voisinage.push_back(this->operator()(i + k, j + l)); // Gestion des bords par l'accesseur (<0 -> 0, >max -> max)
                    }
                }
                std::sort(voisinage.begin(), voisinage.end());
                out(i, j) = voisinage.at(voisinage.size()/2);
                voisinage.clear();
            }
        }
        
        //out.ecrireNom(this->lireNom() + "Med");
    }

    return out;
}

CImageNdg CImageNdg::filtrage(const std::string & methode, int Ni, int Nj)
{

    CImageNdg out(this->lireHauteur(), this->lireLargeur());
    out.m_sNom = this->lireNom() + "F";
    out.choixPalette(this->lirePalette()); // conservation de la palette
    out.m_bBinaire = this->m_bBinaire;     // conservation du type

    if (methode.compare("moyennage") == 0)
    {
        int nbBordsi = Ni / 2;
        int nbBordsj = Nj / 2;

        for (int i = 0; i < this->lireHauteur(); i++)
            for (int j = 0; j < this->lireLargeur(); j++)
            {
                // gestion des bords
                int dk      = max(0, i - nbBordsi);
                int fk      = min(i + nbBordsi, this->lireHauteur() - 1);
                int dl      = max(0, j - nbBordsj);
                int fl      = min(j + nbBordsj, this->lireLargeur() - 1);

                float somme = 0;
                float moy   = 0;
                for (int k = dk; k <= fk; k++)
                    for (int l = dl; l <= fl; l++)
                    {
                        moy += (float)this->operator()(k, l);
                        somme += 1;
                    }
                out(i, j) = (int)(moy / somme);
            }
    }
    else if (methode.compare("median") == 0)
    {
        int nbBordsi = Ni / 2;
        int nbBordsj = Nj / 2;

        std::vector<int> voisinage;

        for (int i = 0; i < this->lireHauteur(); i++)
            for (int j = 0; j < this->lireLargeur(); j++)
            {
                // gestion des bords
                int dk = max(0, i - nbBordsi);
                int fk = min(i + nbBordsi, this->lireHauteur() - 1);
                int dl = max(0, j - nbBordsj);
                int fl = min(j + nbBordsj, this->lireLargeur() - 1);

                voisinage.resize((fk - dk + 1) * (fl - dl + 1));
                int indMed = (fk - dk + 1) * (fl - dl + 1) / 2;

                // empilement
                int indice = 0;
                for (int k = dk; k <= fk; k++)
                    for (int l = dl; l <= fl; l++)
                    {
                        voisinage.at(indice) = (int)this->operator()(k, l);
                        indice++;
                    }

                // tri croissant
                std::sort(voisinage.begin(), voisinage.end());

                out(i, j) = voisinage.at(indMed);

                voisinage.clear();
            }
    }

    return out;
}

_EXPORT_ CImageNdg CImageNdg::hough(int threshold, bool colorForEachLine)
{
    /*-------------------- Images initialization ----------------------*/
    // Tresholded image
    CImageNdg    imgSeuil = seuillage().transformation();
    CImageDouble imgDoubleSeuil(imgSeuil);
    // Contours image
    CImageNdg    imgContours = imgDoubleSeuil.vecteurGradient("norme").toNdg();
    // Hough image
    CImageNdg    res(lireHauteur(), lireLargeur(), 0);

    /*-------------------------- Parameters ----------------------------*/
    int       width    = lireLargeur();
    int       height   = lireHauteur();
    const int maxTheta = 180; // Nombre d'angles possibles (degrés)
    const int maxRho =
        static_cast<int>(sqrt(width * width + height * height)); // Distance maximale possible depuis l'origine

    /*-------------------------- Hough transform -----------------------*/
    std::vector<std::vector<int>> accumulator(maxTheta, std::vector<int>(maxRho, 0));
    // Go through the contours image
    for (int x = 0; x < height; ++x)
    {
        for (int y = 0; y < width; ++y)
        {
            // If current pixel is part of contours
            if (imgContours(x, y) > 0)
            {
                // Go through all possible angles
                for (int theta = 0; theta < maxTheta; ++theta)
                {
                    // Get the angle in radians
                    double radians = theta * PI / 180.0;
                    // Compute rho from cartesian coordinates (rho = x * cos(theta) + y * sin(theta)
                    int    rho     = static_cast<int>(y * cos(radians) + x * sin(radians));

                    // Make sure rho is positive
                    if (rho < 0)
                    {
                        rho = (rho + maxRho) % maxRho; // Shift the negative value to the positive range
                    }
                    if (theta >= 0 && theta < maxTheta && rho >= 0 && rho < maxRho)
                    {
                        accumulator[theta][rho]++;
                    }
                }
            }
        }
    }

    /*-------------------------- Hough lines ---------------------------*/
    // Go through the accumulator
    std::vector<POINT_POLAIRE> houghLines;
    for (int theta = 0; theta < maxTheta; ++theta)
    {
        for (int rho = 0; rho < maxRho; ++rho)
        {
            // If the current value is above the threshold
            if (accumulator[theta][rho] > threshold)
            {
                bool isLocalMaxima = true;

                // Check the neighbors
                for (int dTheta = -1; dTheta <= 1; ++dTheta)
                {
                    for (int dRho = -1; dRho <= 1; ++dRho)
                    {
                        int neighborTheta = theta + dTheta;
                        int neighborRho   = rho + dRho;

                        // Skip if the neighbor is out of bounds
                        if (neighborTheta < 0 || neighborTheta >= maxTheta || neighborRho < 0 || neighborRho >= maxRho)
                        {
                            continue;
                        }

                        // Check if the neighbor has a higher value
                        if (accumulator[neighborTheta][neighborRho] > accumulator[theta][rho])
                        {
                            isLocalMaxima = false;
                            break;
                        }
                    }
                    if (!isLocalMaxima)
                    {
                        break;
                    }
                }

                // Add the point to the list of lines if it's a local maxima
                if (isLocalMaxima)
                {
                    houghLines.push_back({theta, rho});
                }
            }
        }
    }

    /*-------------------------- Draw lines ----------------------------*/
    if (colorForEachLine)
    {
        int grayIncrement    = 255 / houghLines.size();
        int currentGrayLevel = 1;

        for (const auto & line : houghLines)
        {
            int theta      = line.rho;
            int rho        = line.theta;

            // Convertir les coordonnées polaires en coordonnées cartésiennes
            double radians = theta * PI / 180.0;
            for (int y = 0; y < width; ++y)
            {
                int x = static_cast<int>((rho - y * cos(radians)) / sin(radians));
                if (x >= 0 && x < height)
                {
                    // Dessiner un pixel sur l'image résultante
                    res(x, y) = currentGrayLevel;
                }
            }
            for (int x = 0; x < height; ++x)
            {
                int y = static_cast<int>((rho - x * sin(radians)) / cos(radians));
                if (y >= 0 && y < width)
                {
                    // Dessiner un pixel sur l'image résultante
                    res(x, y) = 255;
                }
            }
            currentGrayLevel += grayIncrement;
            grayIncrement++;
            if (currentGrayLevel >= 255)
            {
                currentGrayLevel = 0;
            }
        }
    }
    else
    {
        for (const auto & line : houghLines)
        {
            int theta      = line.rho;
            int rho        = line.theta;

            // Convertir les coordonnées polaires en coordonnées cartésiennes
            double radians = theta * PI / 180.0;
            for (int y = 0; y < width; ++y)
            {
                int x = static_cast<int>((rho - y * cos(radians)) / sin(radians));
                if (x >= 0 && x < height)
                {
                    // Dessiner un pixel sur l'image résultante
                    res(x, y) = 255;
                }
            }
            for (int x = 0; x < height; ++x)
            {
                int y = static_cast<int>((rho - x * sin(radians)) / cos(radians));
                if (y >= 0 && y < width)
                {
                    // Dessiner un pixel sur l'image résultante
                    res(x, y) = 255;
                }
            }
        }
    }

    return res;
}

_EXPORT_ CImageNdg CImageNdg::horizontalConcatenate(const CImageNdg & im)
{
    CImageNdg res(lireHauteur(), lireLargeur() + im.lireLargeur());

    for (int i = 0; i < lireHauteur(); i++)
    {
        for (int j = 0; j < lireLargeur(); j++)
        {
            res(i, j) = operator()(i, j);
        }
    }

    for (int i = 0; i < im.lireHauteur(); i++)
    {
        for (int j = 0; j < im.lireLargeur(); j++)
        {
            res(i, j + lireLargeur()) = im(i, j);
        }
    }

    return res;
}

_EXPORT_ CImageNdg CImageNdg::verticalConcatenate(const CImageNdg & im)
{
    CImageNdg res(lireHauteur() + im.lireHauteur(), lireLargeur());

    for (int i = 0; i < lireHauteur(); i++)
    {
        for (int j = 0; j < lireLargeur(); j++)
        {
            res(i, j) = operator()(i, j);
        }
    }

    for (int i = 0; i < im.lireHauteur(); i++)
    {
        for (int j = 0; j < im.lireLargeur(); j++)
        {
            res(i + lireHauteur(), j) = im(i, j);
        }
    }

    return res;
}

_EXPORT_ double CImageNdg::correlation_croisee_normalisee(const CImageNdg & imgRef) {
    double sum_A = 0, sum_B = 0;
    double mean_A, mean_B;
    double numerator = 0, denominator_A = 0, denominator_B = 0;
    double correlation;

    for (int i = 0; i < this->lireHauteur(); i++) {
        for (int j = 0; j < this->lireLargeur(); j++) {
            sum_A += this->operator()(i, j);
            sum_B += imgRef(i, j);
        }
    }

    mean_A = sum_A / (this->lireHauteur() * this->lireLargeur());
    mean_B = sum_B / (imgRef.lireHauteur() * imgRef.lireLargeur());

    for (int i = 0; i < this->lireHauteur(); i++) {
        for (int j = 0; j < this->lireLargeur(); j++) {
            double diff_A = abs(this->operator()(i, j) - mean_A);
            double diff_B = abs(imgRef(i, j) - mean_B);

            numerator += diff_A * diff_B;
            denominator_A += diff_A * diff_A;
            denominator_B += diff_B * diff_B;
        }
    }

    correlation = numerator / (sqrt(denominator_A) * sqrt(denominator_B));
    return correlation;
}

_EXPORT_ void zhangSuenSkeletonization(CImageNdg& binImg)
{
    // binImg(i,j) : 0 ou 1 (on suppose déjà seuillé)
    // On va itérer jusqu'à stabiliser le squelette
    bool somethingChanged = true;
    int height = binImg.lireHauteur();
    int width = binImg.lireLargeur();

    while (somethingChanged)
    {
        somethingChanged = false;

        // Tableau temporaire pour marquer les pixels à retirer en phase 1
        std::vector<bool> toRemovePhase1(height * width, false);

        // ---- Phase 1
        for (int i = 1; i < height - 1; i++)
        {
            for (int j = 1; j < width - 1; j++)
            {
                if (binImg(i, j) == 1)
                {
                    // Récupération des 8 voisins dans l'ordre N, NE, E, SE, S, SW, W, NW
                    int N = binImg(i - 1, j);
                    int S = binImg(i + 1, j);
                    int E = binImg(i, j + 1);
                    int W = binImg(i, j - 1);
                    int NE = binImg(i - 1, j + 1);
                    int NW = binImg(i - 1, j - 1);
                    int SE = binImg(i + 1, j + 1);
                    int SW = binImg(i + 1, j - 1);

                    // On construit un petit vecteur pour compter les transitions 0->1
                    // en parcourant les voisins dans l'ordre circulaire
                    int neighbors[9] = { N, NE, E, SE, S, SW, W, NW, N };
                    int transitions = 0;
                    for (int k = 0; k < 8; k++)
                    {
                        if (neighbors[k] == 0 && neighbors[k + 1] == 1)
                            transitions++;
                    }

                    // Somme des voisins (combien de voisins = 1)
                    int neighborsSum = N + NE + E + SE + S + SW + W + NW;

                    // Conditions de la phase 1
                    if ((transitions == 1) &&
                        (neighborsSum >= 2 && neighborsSum <= 6) &&
                        (N * E * S == 0) &&
                        (E * S * W == 0))
                    {
                        toRemovePhase1[i * width + j] = true;
                    }
                }
            }
        }

        // On supprime les pixels marqués en phase 1
        for (int idx = 0; idx < height * width; idx++)
        {
            if (toRemovePhase1[idx])
            {
                binImg(idx) = 0;
            }
        }

        // Tableau temporaire pour marquer les pixels à retirer en phase 2
        std::vector<bool> toRemovePhase2(height * width, false);

        // ---- Phase 2
        for (int i = 1; i < height - 1; i++)
        {
            for (int j = 1; j < width - 1; j++)
            {
                if (binImg(i, j) == 1)
                {
                    int N = binImg(i - 1, j);
                    int S = binImg(i + 1, j);
                    int E = binImg(i, j + 1);
                    int W = binImg(i, j - 1);
                    int NE = binImg(i - 1, j + 1);
                    int NW = binImg(i - 1, j - 1);
                    int SE = binImg(i + 1, j + 1);
                    int SW = binImg(i + 1, j - 1);

                    int neighbors[9] = { N, NE, E, SE, S, SW, W, NW, N };
                    int transitions = 0;
                    for (int k = 0; k < 8; k++)
                    {
                        if (neighbors[k] == 0 && neighbors[k + 1] == 1)
                            transitions++;
                    }

                    int neighborsSum = N + NE + E + SE + S + SW + W + NW;

                    if ((transitions == 1) &&
                        (neighborsSum >= 2 && neighborsSum <= 6) &&
                        (N * E * W == 0) &&
                        (N * S * W == 0))
                    {
                        toRemovePhase2[i * width + j] = true;
                    }
                }
            }
        }

        // On supprime les pixels marqués en phase 2
        for (int idx = 0; idx < height * width; idx++)
        {
            if (toRemovePhase2[idx])
            {
                binImg(idx) = 0;
            }
        }

        // Vérifier s'il y a eu des changements pendant ces deux phases
        for (bool b : toRemovePhase1) if (b) { somethingChanged = true; break; }
        if (!somethingChanged)
        {
            for (bool b : toRemovePhase2) if (b) { somethingChanged = true; break; }
        }
    }
}

_EXPORT_ void connectedComponents(
    const CImageNdg& binImg,
    CImageNdg& labels,
    int& numLabels)
{
    int height = binImg.lireHauteur();
    int width = binImg.lireLargeur();

    // labels doit être de même taille, initialisé à 0
    labels = CImageNdg(height, width, 0);
    numLabels = 0;

    // Parcourt l'image binaire
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            if (binImg(i, j) == 1 && labels(i, j) == 0)
            {
                // On a trouvé un nouvel objet, on incrémente le label
                numLabels++;
                int currentLabel = numLabels;

                // On fait un flood fill (DFS ou BFS) pour marquer tout l'objet
                // Ici on fait un BFS simple avec une file
                std::vector<std::pair<int, int>> queue;
                queue.push_back({ i, j });
                labels(i, j) = (unsigned char)currentLabel;

                while (!queue.empty())
                {
                    auto p = queue.back();
                    int r = p.first;
                    int c = p.second;

                    queue.pop_back();

                    // On parcourt les 8 voisins
                    for (int dr = -1; dr <= 1; dr++)
                    {
                        for (int dc = -1; dc <= 1; dc++)
                        {
                            if (dr == 0 && dc == 0)
                                continue;
                            int rr = r + dr;
                            int cc = c + dc;
                            if (rr >= 0 && rr < height && cc >= 0 && cc < width)
                            {
                                if (binImg(rr, cc) == 1 && labels(rr, cc) == 0)
                                {
                                    labels(rr, cc) = (unsigned char)currentLabel;
                                    queue.push_back({ rr, cc });
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

_EXPORT_ void process_image(
    const CImageNdg& inputColorOrGray,
    CImageNdg& labels,
    int& numLabels,
    CImageNdg& imageSquelette)
{
    // 1) Extraire le plan bleu (dans le code MATLAB : plan_bleu = image(:,:,3))
    //    Ici, si vous n'avez qu'un canal dans CImageNdg, adaptez en fonction
    //    de votre manière de stocker l'image couleur.
    //    Supposons ici que inputColorOrGray est déjà le canal bleu,
    //    ou bien que vous avez une fonction extractBlueChannel(...).
    CImageNdg planBleu = inputColorOrGray; // simplifié

    // 2) Appliquer un seuillage binaire. Dans le script MATLAB, c’est un
    //    "imbinarize(..., 0.8)", donc on compare planBleu(i,j)/255.0 à 0.8
    int h = planBleu.lireHauteur();
    int w = planBleu.lireLargeur();
    CImageNdg binImage(h, w, 0);

    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            float val = (float)planBleu(i, j) / 255.0f;
            if (val > 0.7f)
                binImage(i, j) = 1;  // pixel blanc
            else
                binImage(i, j) = 0;  // pixel noir
        }
    }
    binImage.sauvegarde("binImage");
    // 3) (Morphologie fermeture) - Dans le code MATLAB, c'est commenté,
    //    donc on reproduit la logique "inutile" : image_fermee = binImage;
    CImageNdg imageFermee = binImage;

    // 4) Squelettisation
    zhangSuenSkeletonization(imageFermee);
    imageSquelette = imageFermee; // On renvoie l'image squelette

    // 5) Labélisation
    connectedComponents(imageSquelette, labels, numLabels);
}

_EXPORT_ void correctLines(
    const CImageNdg& labels,
    int numLabels,
    std::vector<std::vector<std::pair<int, int>>>& corrections,
    CImageNdg& correctedLabels
)
{
    int h = labels.lireHauteur();
    int w = labels.lireLargeur();
    correctedLabels = CImageNdg(h, w, 0);

    // On redimensionne le vecteur "corrections" pour contenir
    // le résultat de chaque label (1..numLabels). On peut utiliser
    // l'indice [label - 1] pour stocker.
    corrections.clear();
    corrections.resize(numLabels);

    // Pour chaque étiquette
    for (int label = 1; label <= numLabels; label++)
    {
        // Récupérer tous les pixels (rows, cols)
        std::vector<int> rows;
        std::vector<int> cols;
        rows.reserve(h * w);
        cols.reserve(h * w);

        for (int i = 0; i < h; i++)
        {
            for (int j = 0; j < w; j++)
            {
                if (labels(i, j) == label)
                {
                    rows.push_back(i);
                    cols.push_back(j);
                }
            }
        }

        if (!rows.empty())
        {
            // On trie par ordre croissant de colonne
            // => On réorganise (rows,cols) par l'ordre de cols
            // On peut créer un vecteur d'indices, puis trier ce vecteur
            std::vector<int> indices(rows.size());
            for (size_t k = 0; k < indices.size(); k++) indices[k] = (int)k;

            // Tri par colonne
            std::sort(indices.begin(), indices.end(), [&](int a, int b) {
                return cols[a] < cols[b];
                });

            // Après tri, on obtient la col triée, row triée
            // On vérifie qu'on a plus d'un pixel
            if (indices.size() > 1)
            {
                // On utilise le 2ème pixel comme référence
                int reference_row = rows[indices[1]];

                // On corrige toutes les lignes en imposant row = reference_row
                // offset = (old_row - corrected_row)
                // => "rows - corrected_rows" en MATLAB
                // ICI, corrected_row = reference_row
                // offset = old_row - reference_row
                for (auto idx : indices)
                {
                    int r = rows[idx];
                    int c = cols[idx];
                    int offset = r - reference_row;

                    // On stocke (col, offset)
                    corrections[label - 1].push_back({ c, offset });

                    // On place le pixel dans correctedLabels
                    int new_r = reference_row;
                    if (new_r >= 0 && new_r < h)
                    {
                        correctedLabels(new_r, c) = (unsigned char)label;
                    }
                }
            }
        }
    }
}

_EXPORT_ void buildCorrectionField(
    const CImageNdg& labels,
    int numLabels,
    const std::vector<std::vector<std::pair<int, int>>>& corrections,
    std::vector<std::vector<float>>& correctionField
)
{
    int h = labels.lireHauteur();
    int w = labels.lireLargeur();
    correctionField.assign(h, std::vector<float>(w, 0.0f));

    // Pour chaque label
    for (int label = 1; label <= numLabels; label++)
    {
        // Récupère tous les pixels de ce label
        // Et on a déjà corrections[label-1] = vecteur (col, offset)
        // Il peut y avoir des doublons pour un même col => on en prend un
        // Cf. code MATLAB: unique(...) => on ne met qu'une correction par colonne
        // On va donc construire un dictionnaire col -> offset unique
        std::vector<std::pair<int, int>> col_offsets = corrections[label - 1];
        // On trie par col
        std::sort(col_offsets.begin(), col_offsets.end(),
            [](auto& a, auto& b) { return a.first < b.first; });

        // On ne garde qu'une valeur unique de offset par col
        std::vector<std::pair<int, int>> uniqueColOffset;
        uniqueColOffset.reserve(col_offsets.size());
        int lastCol = -1;
        for (auto& p : col_offsets)
        {
            int c = p.first;
            int off = p.second;
            if (c != lastCol)
            {
                uniqueColOffset.push_back({ c, off });
                lastCol = c;
            }
        }

        // Maintenant, pour chaque pixel du label, on place la correction
        // correspondante dans correctionField. On regarde la colonne, on
        // trouve le offset dans uniqueColOffset
        // (en Matlab, on faisait un "for col in unique_cols", etc.)
        // Ici, on refait un pass sur l'image pour tous les pixels = label
        // -> c'est moins optimisé mais plus simple à lire
        for (int i = 0; i < h; i++)
        {
            for (int j = 0; j < w; j++)
            {
                if (labels(i, j) == label)
                {
                    // On cherche si j existe dans uniqueColOffset
                    // plus simple : on fait une recherche linéaire
                    // (OK si peu de pixels). Ou alors on construit
                    // un map<int,int>.
                    for (auto& colOff : uniqueColOffset)
                    {
                        if (colOff.first == j)
                        {
                            correctionField[i][j] = (float)colOff.second;
                            break;
                        }
                    }
                }
            }
        }
    }
}

_EXPORT_ void verticalInterpolation(std::vector<std::vector<float>>& corrField)
{
    int h = (int)corrField.size();
    if (h == 0) return;
    int w = (int)corrField[0].size();
    if (w == 0) return;

    for (int col = 0; col < w; col++)
    {
        // On extrait la colonne
        // On repère les indices non nuls
        std::vector<int> nonZeroIdx;
        nonZeroIdx.reserve(h);

        for (int row = 0; row < h; row++)
        {
            if (std::abs(corrField[row][col]) > 1e-9) // != 0
            {
                nonZeroIdx.push_back(row);
            }
        }

        // S'il y a au moins 2 pixels non nuls
        if (nonZeroIdx.size() > 1)
        {
            for (size_t k = 0; k < nonZeroIdx.size() - 1; k++)
            {
                int start_idx = nonZeroIdx[k];
                int end_idx = nonZeroIdx[k + 1];
                float start_val = corrField[start_idx][col];
                float end_val = corrField[end_idx][col];

                // interpolation linéaire
                int lengthSeg = end_idx - start_idx;
                for (int n = 0; n <= lengthSeg; n++)
                {
                    float t = (lengthSeg == 0) ? 0.0f : (float)n / (float)lengthSeg;
                    corrField[start_idx + n][col] = start_val + t * (end_val - start_val);
                }
            }
        }
    }
}

_EXPORT_ void gaussianBlur2D(
    const std::vector<std::vector<float>>& input,
    float sigma,
    std::vector<std::vector<float>>& output
)
{
    int h = (int)input.size();
    if (h == 0) { output.clear(); return; }
    int w = (int)input[0].size();
    if (w == 0) { output.clear(); return; }

    output.assign(h, std::vector<float>(w, 0.0f));

    // On crée un noyau 1D gaussien en séparant le flou en 2 passes
    // (horizontal puis vertical) pour simplifier.
    // Taille: on prend radius = 3*sigma arrondi
    int radius = (int)std::ceil(3.0f * sigma);
    // Construction du noyau 1D
    std::vector<float> kernel(2 * radius + 1, 0.0f);
    float sumK = 0.0f;
    float invTwoSigma2 = 1.0f / (2.0f * sigma * sigma);

    for (int i = -radius; i <= radius; i++)
    {
        float val = std::exp(-(i * i) * invTwoSigma2);
        kernel[i + radius] = val;
        sumK += val;
    }
    // normalisation
    for (float& v : kernel) v /= sumK;

    // Première passe: flou horizontal => tmp
    std::vector<std::vector<float>> tmp(h, std::vector<float>(w, 0.0f));

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            float accum = 0.0f;
            for (int k = -radius; k <= radius; k++)
            {
                int xx = x + k;
                if (xx < 0) xx = 0;
                if (xx >= w) xx = w - 1;
                accum += input[y][xx] * kernel[k + radius];
            }
            tmp[y][x] = accum;
        }
    }

    // Deuxième passe: flou vertical => output
    for (int x = 0; x < w; x++)
    {
        for (int y = 0; y < h; y++)
        {
            float accum = 0.0f;
            for (int k = -radius; k <= radius; k++)
            {
                int yy = y + k;
                if (yy < 0) yy = 0;
                if (yy >= h) yy = h - 1;
                accum += tmp[yy][x] * kernel[k + radius];
            }
            output[y][x] = accum;
        }
    }
}

_EXPORT_ void applyCorrectionField(
    const CImageNdg& newLabels,
    const std::vector<std::vector<float>>& correctionField,
    CImageNdg& correctedNewLabels
)
{
    int h = newLabels.lireHauteur();
    int w = newLabels.lireLargeur();
    correctedNewLabels = CImageNdg(h, w, 0);

    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            unsigned char lbl = newLabels(i, j);
            if (lbl != 0)
            {
                // calcul du nouvel indice vertical
                float offset = correctionField[i][j];
                float corrected_row_f = (float)i - offset;
                int corrected_row = (int)std::round(corrected_row_f);

                // Vérifier les limites
                if (corrected_row >= 0 && corrected_row < h)
                {
                    correctedNewLabels(corrected_row, j) = lbl;
                }
            }
        }
    }
}

 /**
 * @brief Construit un nuage de points (X,Y,Z) à partir d'une image corrigée (correctedVolume)
 *        et d'un champ de correction (corrField). Seuls les pixels dont le label != 0 sont utilisés.
 *
 * @param correctedVolume  L'image corrigée (labellisée), dimensions h x w, type CImageNdg
 * @param corrField        Le champ de correction (même taille h x w), type float
 * @param X                Vecteur où seront ajoutées les coordonnées X
 * @param Y                Vecteur où seront ajoutées les coordonnées Y
 * @param Z                Vecteur où seront ajoutées les coordonnées Z
 * @param applyTransform   Si true, applique la logique x=-col, y=row - z, z=-z
 */
_EXPORT_ void build3DPointsFromCorrection(
        const CImageNdg& correctedVolume,
        const std::vector<std::vector<float>>& corrField,
        std::vector<float>& X,
        std::vector<float>& Y,
        std::vector<float>& Z,
        bool applyTransform = true
    )
{
    // Récupération des dimensions
    int h = correctedVolume.lireHauteur();
    int w = correctedVolume.lireLargeur();

    // Sécurité : vérifier qu'on a la même taille pour corrField
    if ((int)corrField.size() != h || (h > 0 && (int)corrField[0].size() != w))
    {
        // Erreur de dimensions
        throw std::string("build3DPointsFromCorrection: Dimensions non cohérentes entre correctedVolume et corrField!");
    }

    // Réserver la place (optionnel, juste pour limiter les realloc)
    X.reserve(h * w);
    Y.reserve(h * w);
    Z.reserve(h * w);

    // Parcourt tous les pixels
    for (int row = 0; row < h; row++)
    {
        for (int col = 0; col < w; col++)
        {
            unsigned char labelVal = correctedVolume(row, col);
            if (labelVal != 0)
            {
                // Ce pixel appartient à une ligne (label != 0)
                float zVal = corrField[row][col];

                // Transformations éventuelles
                // (Le code MATLAB faisait x=-col, y=row - z, z=-z)
                float xCoord = (float)col;
                float yCoord = (float)row;
                float zCoord = zVal;

                if (applyTransform)
                {
                    xCoord = -(float)col;
                    yCoord = (float)row -zVal;
                    zCoord = -zVal;
                }

                // On ajoute dans les vecteurs
                X.push_back(xCoord);
                Y.push_back(yCoord);
                Z.push_back(zCoord);
            }
        }
    }
}

_EXPORT_ void centerAndScalePoints(
    std::vector<float>& xcoords,
    std::vector<float>& ycoords,
    std::vector<float>& zcoords,
    float scale_factor
)
{
    // centre = moyenne
    float cx = 0.f, cy = 0.f, cz = 0.f;
    size_t n = xcoords.size();
    if (n == 0) return;

    for (size_t i = 0; i < n; i++)
    {
        cx += xcoords[i];
        cy += ycoords[i];
        cz += zcoords[i];
    }
    cx /= (float)n;
    cy /= (float)n;
    cz /= (float)n;

    // On recentre et on scale
    for (size_t i = 0; i < n; i++)
    {
        xcoords[i] = (xcoords[i] - cx) / scale_factor;
        ycoords[i] = (ycoords[i] - cy) / scale_factor;
        zcoords[i] = (zcoords[i] - cz) / scale_factor;
    }
}

/**
 * @brief Calcule la scale_y (mm/pixel) en se basant sur l'image corrected_labels
 *        où chaque label est "aplatit" horizontalement.
 *
 * @param correctedLabels  Matrice 2D (h x w) de labels (int).
 * @param num              Nombre total de labels (1..num).
 * @return float           scale_y en mm/pixel (ou 0 si non calculable).
 */
_EXPORT_ float computeScaleY(
    const std::vector<std::vector<int>>& correctedLabels,
    int num
)
{
    // line_positions contiendra la coordonnée i (ligne) correspondant
    // au pixel le plus à gauche (col minimum) pour chaque label.
    std::vector<int> linePositions;
    linePositions.resize(num, 0); // par défaut 0

    int h = (int)correctedLabels.size();
    if (h == 0) return 0.0f;
    int w = (int)correctedLabels[0].size();

    // 1) Pour chaque label, on récupère le min_col
    //    et la ligne correspondante
    for (int label = 1; label <= num; label++)
    {
        // Recherche des pixels du label
        // On stocke leur (row, col) pour trouver min_col
        int minCol = w + 1;
        int rowForMinCol = -1;

        for (int i = 0; i < h; i++)
        {
            for (int j = 0; j < w; j++)
            {
                if (correctedLabels[i][j] == label)
                {
                    if (j < minCol)
                    {
                        minCol = j;
                        rowForMinCol = i;
                    }
                }
            }
        }

        // Si on a trouvé un pixel, rowForMinCol >= 0
        if (rowForMinCol >= 0)
        {
            // label-1 pour indexer linePositions
            linePositions[label - 1] = rowForMinCol;
        }
    }

    // 2) Supprimer les éléments = 0 (qui n'ont pas été remplis)
    //    car label sans pixel ou rowForMinCol == 0
    std::vector<int> filtered;
    filtered.reserve(num);
    for (auto val : linePositions)
    {
        if (val != 0) filtered.push_back(val);
    }
    if (filtered.size() < 2)
    {
        // Pas assez de lignes pour calculer un écart
        return 0.0f;
    }

    // 3) Trier
    std::sort(filtered.begin(), filtered.end());

    // 4) Prendre la médiane des diff successifs
    //    diffs[i] = filtered[i+1] - filtered[i]
    std::vector<int> diffs;
    diffs.reserve(filtered.size() - 1);
    for (size_t i = 0; i < filtered.size() - 1; i++)
    {
        int d = filtered[i + 1] - filtered[i];
        diffs.push_back(d);
    }
    // Tri des diffs pour en extraire la médiane
    std::sort(diffs.begin(), diffs.end());
    int mid = (int)diffs.size() / 2;

    float d_pix = 0.0f;
    if (diffs.size() % 2 == 0)
    {
        // nombre pair => moyenne de 2 valeurs
        d_pix = 0.5f * (diffs[mid - 1] + diffs[mid]);
    }
    else
    {
        // nombre impair
        d_pix = (float)diffs[mid];
    }

    // 5) Hypothèse: l'écart entre lignes vaut 5 mm
    float real_spacing_mm = 5.0f;
    float scale_y = 0.0f;
    if (std::fabs(d_pix) > 1e-9)
        scale_y = real_spacing_mm / d_pix; // mm/pixel

    return scale_y;
}


/**
 * @brief Calcule la scale_z (mm/pixel) en se basant sur new_labels et depth_map
 *        en cherchant le plus grand écart (max - min) parmi toutes les lignes.
 *
 * @param newLabels    Matrice 2D (h x w) de labels (int).
 * @param depthMap     Matrice 2D (h x w) de float (valeurs de correction).
 * @param realMaxDiffMm  Valeur réelle en mm correspondant au plus grand écart
 *                       (ex. 50 mm).
 * @return float       scale_z en mm/pixel
 */
_EXPORT_ float computeScaleZ(
    const std::vector<std::vector<int>>& newLabels,
    const std::vector<std::vector<float>>& depthMap,
    float realMaxDiffMm = 50.0f
)
{
    int h = (int)newLabels.size();
    if (h == 0) return 0.0f;
    int w = (int)newLabels[0].size();

    // on récupère le nombre max de label
    int numLabels = 0;
    // Rechercher le maximum dans newLabels
    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            if (newLabels[i][j] > numLabels)
                numLabels = newLabels[i][j];
        }
    }

    float maxDiffPixels = 0.0f;

    // 1) Pour chaque label, on calcule local_diff = max(vals) - min(vals)
    for (int label = 1; label <= numLabels; label++)
    {
        float localMin = 1e9f;
        float localMax = -1e9f;
        bool foundAny = false;

        for (int i = 0; i < h; i++)
        {
            for (int j = 0; j < w; j++)
            {
                if (newLabels[i][j] == label)
                {
                    float val = depthMap[i][j];
                    if (val < localMin) localMin = val;
                    if (val > localMax) localMax = val;
                    foundAny = true;
                }
            }
        }

        if (foundAny)
        {
            float localDiff = localMax - localMin;
            if (localDiff > maxDiffPixels)
                maxDiffPixels = localDiff;
        }
    }

    // 2) Si maxDiffPixels=0 => impossible de calculer
    if (std::fabs(maxDiffPixels) < 1e-9)
        return 0.0f;

    // 3) scale_z = realMaxDiffMm / maxDiffPixels
    float scale_z = realMaxDiffMm / maxDiffPixels;
    return scale_z;
}


/**
 * @brief Applique l'échelle (scale_x, scale_y, scale_z) à un nuage de points (X, Y, Z)
 *        puis recadre en soustrayant leurs minima pour démarrer à (0,0,0).
 *
 * @param X         vecteur des X
 * @param Y         vecteur des Y
 * @param Z         vecteur des Z
 * @param scaleX    échelle à appliquer sur X
 * @param scaleY    échelle à appliquer sur Y
 * @param scaleZ    échelle à appliquer sur Z
 */
_EXPORT_ void scaleAndShiftCloud(
    std::vector<float>& X,
    std::vector<float>& Y,
    std::vector<float>& Z,
    float scaleX,
    float scaleY,
    float scaleZ
)
{
    // 1) On applique l'échelle
    for (size_t i = 0; i < X.size(); i++)
    {
        X[i] *= scaleX;
        Y[i] *= scaleY;
        Z[i] *= scaleZ;
    }

    // 2) On calcule minX, minY, minZ pour recadrer à 0
    float minX = 1e9f, minY = 1e9f, minZ = 1e9f;
    for (size_t i = 0; i < X.size(); i++)
    {
        if (X[i] < minX) minX = X[i];
        if (Y[i] < minY) minY = Y[i];
        if (Z[i] < minZ) minZ = Z[i];
    }

    // 3) Soustraction
    for (size_t i = 0; i < X.size(); i++)
    {
        X[i] -= minX;
        Y[i] -= minY;
        Z[i] -= minZ;
    }
}

/**
 * @brief Recompose une image NDG à partir d'un nuage de points (X, Y, Z).
 *        Chaque point devient un pixel sur la coordonnée (row=Y, col=X),
 *        la valeur du pixel est obtenue en remappant Z dans [0..255].
 *        Le fond (pixels sans point) est initialisé à 128.
 *
 * @param X              Coordonnées X (col) de chaque point
 * @param Y              Coordonnées Y (row) de chaque point
 * @param Z              Coordonnées Z de chaque point
 * @param valFond        Valeur NDG du fond (ex: 128 = 255/2)
 * @return CImageNdg     L'image NDG reconstituée
 */
_EXPORT_ CImageNdg recomposeImageFromXYZ(
    const std::vector<float>& X,
    const std::vector<float>& Y,
    const std::vector<float>& Z,
    unsigned char valFond = 128
)
{
    // 1) Vérifier cohérence
    if (X.size() != Y.size() || Y.size() != Z.size())
    {
        throw std::string("recomposeImageFromXYZ: Taille incohérente entre X, Y et Z !");
    }

    size_t n = X.size();
    if (n == 0)
    {
        // Pas de points => on retourne une image vide
        return CImageNdg(0, 0, 0);
    }

    // 2) Trouver minX, maxX, minY, maxY
    float minX = X[0], maxX = X[0];
    float minY = Y[0], maxY_ = Y[0];
    for (size_t i = 1; i < n; i++)
    {
        if (X[i] < minX) minX = X[i];
        if (X[i] > maxX) maxX = X[i];
        if (Y[i] < minY) minY = Y[i];
        if (Y[i] > maxY_) maxY_ = Y[i];
    }

    // 3) Calcul de la taille de l'image
    //    On fait un arrondi (on suppose que X, Y peuvent être non entiers).
    //    => on va décaler minX, minY à 0, ou on va allouer +1 pour couvrir tout.
    int iMinX = (int)std::floor(minX);
    int iMaxX = (int)std::ceil(maxX);
    int iMinY = (int)std::floor(minY);
    int iMaxY = (int)std::ceil(maxY_);

    int width = iMaxX - iMinX + 1;
    int height = iMaxY - iMinY + 1;

    if (width <= 0 || height <= 0)
    {
        throw std::string("recomposeImageFromXYZ: Dimensions négatives ou nulles !");
    }

    // 4) Trouver minZ, maxZ pour un remappage dans [0..255]
    float minZ = Z[0], maxZ = Z[0];
    for (size_t i = 1; i < n; i++)
    {
        if (Z[i] < minZ) minZ = Z[i];
        if (Z[i] > maxZ) maxZ = Z[i];
    }
    float rangeZ = maxZ - minZ;
    if (std::fabs(rangeZ) < 1e-9)
    {
        // Cas spécial: Z est constant => tout le monde aura la même valeur => 255 ou 0, par exemple.
        rangeZ = 1.0f;
    }

    // 5) Créer l'image NDG, initialisée à valFond (128 par défaut).
    CImageNdg out(height, width, -1);
    // -1 => non-initialisé dans le constructeur, donc on va le faire nous-mêmes:
    for (int i = 0; i < out.lireNbPixels(); i++)
    {
        out(i) = valFond;
    }

    // 6) Parcourir les points et remplir out(row, col) = remap(Z)
    for (size_t idx = 0; idx < n; idx++)
    {
        // Calcul col, row (coord pixels)
        float fx = X[idx];
        float fy = Y[idx];
        int col = (int)std::round(fx) - iMinX;
        int row = (int)std::round(fy) - iMinY;
        // Vérifier que col, row tombent dans [0..width-1], [0..height-1]
        if (col < 0 || col >= width || row < 0 || row >= height)
        {
            // Le point sort de l'image -> on ignore ou on gère autrement
            continue;
        }

        // Remappage de Z[idx] en [0..255]
        float valZ = (Z[idx] - minZ) / rangeZ; // => [0..1]
        float gray = valZ * 255.0f;
        if (gray < 0.f) gray = 0.f;
        if (gray > 255.f) gray = 255.f;

        out(row, col) = (unsigned char)std::round(gray);
    }

    return out;
}

/**
 * @brief Exporte un nuage de points (X, Y, Z) en format .ply ASCII.
 *
 * @param X          Vecteur contenant la coordonnée X de chaque point.
 * @param Y          Vecteur contenant la coordonnée Y de chaque point.
 * @param Z          Vecteur contenant la coordonnée Z de chaque point.
 * @param filename   Nom (ou chemin) du fichier .ply à créer.
 */
_EXPORT_ void exportPointsToPLY(
    const std::vector<float>& X,
    const std::vector<float>& Y,
    const std::vector<float>& Z,
    const std::string& filename
)
{
    // Vérifie qu'on a le même nombre de points pour X, Y, et Z
    if (X.size() != Y.size() || Y.size() != Z.size())
    {
        std::cerr << "Erreur: X, Y et Z n'ont pas la même taille !" << std::endl;
        return;
    }

    // Ouvre le fichier en écriture texte
    std::ofstream ofs(filename.c_str());
    if (!ofs.is_open())
    {
        std::cerr << "Impossible de créer le fichier : " << filename << std::endl;
        return;
    }

    size_t nbPoints = X.size();

    // Ecriture de l'en‐tête PLY en format ASCII
    ofs << "ply\n";
    ofs << "format ascii 1.0\n";
    ofs << "element vertex " << nbPoints << "\n";
    ofs << "property float x\n";
    ofs << "property float y\n";
    ofs << "property float z\n";
    ofs << "end_header\n";

    // Ecriture des points
    for (size_t i = 0; i < nbPoints; i++)
    {
        ofs << X[i] << " "
            << Y[i] << " "
            << Z[i] << "\n";
    }

    ofs.close();
    std::cout << "Fichier PLY exporté : " << filename
        << " (" << nbPoints << " points)" << std::endl;
}
