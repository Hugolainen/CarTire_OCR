#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <vector>

#include "ImageNdg.h"

// data bitmap windows
// pourrait se faire par l'inclusion de <windows.h> 

#define MAGIC_NUMBER_BMP ('B'+('M'<<8)) 

typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned long LONG;

typedef struct tagBITMAPFILEHEADER {
	WORD  bfType;
	DWORD bfSize;
	WORD  bfReserved1;
	WORD  bfReserved2;
	DWORD bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
	DWORD biSize;
	LONG  biWidth;
	LONG  biHeight;
	WORD  biPlanes;
	WORD  biBitCount;
	DWORD biCompression;
	DWORD biSizeImage;
	LONG  biXPelsPerMeter;
	LONG  biYPelsPerMeter;
	DWORD biClrUsed;
	DWORD biClrImportant;
} BITMAPINFOHEADER;


// constructeurs et destructeur
CImageNdg::CImageNdg() {

	this->m_iHauteur  = 0;
	this->m_iLargeur  = 0;
	this->m_bBinaire  = false;
	this->m_sNom      = "vide";
	
	this->m_pucPixel  = NULL;
	this->m_pucPalette = NULL; 
}

CImageNdg::CImageNdg(int hauteur, int largeur, int valeur) {

	this->m_iHauteur = hauteur;
	this->m_iLargeur = largeur;
	this->m_bBinaire	= false; // Image Ndg par défaut, binaire après seuillage
	this->m_sNom      = "inconnu";

	this->m_pucPixel = new unsigned char[hauteur*largeur];
	this->m_pucPalette = new unsigned char[256*4];	
	choixPalette("grise"); // palette grise par défaut, choix utilisateur 
	if (valeur != -1) 
		for (int i=0;i<this->lireNbPixels();i++)
			this->m_pucPixel[i] = valeur;
}

CImageNdg::CImageNdg(const std::string& name) {

	BITMAPFILEHEADER header;
	BITMAPINFOHEADER infoHeader;
	
	std::ifstream f(name.c_str(),std::ios::in | std::ios::binary); 
		if (f.is_open()) {
			f.read((char*)&header.bfType,2);
			f.read((char*)&header.bfSize, 4);
			f.read((char*)&header.bfReserved1, 2);
			f.read((char*)&header.bfReserved2, 2);
			f.read((char*)&header.bfOffBits, 4);
			if (header.bfType != MAGIC_NUMBER_BMP) 
				throw std::string("ouverture format BMP impossible ..."); 
			else {
				f.read((char*)&infoHeader.biSize, 4);
				f.read((char*)&infoHeader.biWidth, 4);
				f.read((char*)&infoHeader.biHeight, 4);
				f.read((char*)&infoHeader.biPlanes, 2);
				f.read((char*)&infoHeader.biBitCount, 2);
				f.read((char*)&infoHeader.biCompression, 4);
				f.read((char*)&infoHeader.biSizeImage, 4);
				f.read((char*)&infoHeader.biXPelsPerMeter, 4);
				f.read((char*)&infoHeader.biYPelsPerMeter, 4);
				f.read((char*)&infoHeader.biClrUsed, 4);
				f.read((char*)&infoHeader.biClrImportant, 4);
				if (infoHeader.biCompression > 0) 
					throw std::string("Format compresse non supporte...");
				else {
					if (infoHeader.biBitCount == 8) {
						this->m_iHauteur = infoHeader.biHeight;
						this->m_iLargeur = infoHeader.biWidth;
						this->m_bBinaire = false;
						this->m_sNom.assign(name.begin(),name.end()-4);
						this->m_pucPalette = new unsigned char[256*4];	
						this->m_pucPixel = new unsigned char[infoHeader.biHeight * infoHeader.biWidth];

						// gérer multiple de 32 bits via zéros éventuels ignorés
						int complement = (((this->m_iLargeur-1)/4) + 1)*4 - this->m_iLargeur;
						for (int indice=0;indice<4*256;indice++) 
							f.read((char*)&this->m_pucPalette[indice],sizeof(char));

						for (int i= this->m_iHauteur-1; i >= 0; i--) {
							for (int j=0; j<this->m_iLargeur; j++) 
								f.read((char*)&this->m_pucPixel[i*this->m_iLargeur+j],sizeof(char));

							char inutile;
							for (int k=0; k< complement; k++)
								f.read((char*)&inutile,sizeof(char));
						}
					}	
					else {
						// cas d'une image couleur
						this->m_iHauteur = infoHeader.biHeight;
						this->m_iLargeur = infoHeader.biWidth;
						this->m_bBinaire = false;
						this->m_sNom.assign(name.begin(),name.end()-4);
						this->m_pucPalette = new unsigned char[256*4];	
						this->choixPalette("grise"); // palette grise par défaut
						this->m_pucPixel = new unsigned char[infoHeader.biHeight * infoHeader.biWidth];

						// extraction plan luminance
						int complement = (((this->m_iLargeur*3-1)/4) + 1)*4 - this->m_iLargeur*3;
						for (int i= this->m_iHauteur-1; i >= 0; i--) {
							for (int j=0;j<this->m_iLargeur*3;j+=3) {
								unsigned char rouge,vert,bleu;
								f.read((char*)&rouge,sizeof(char));
								f.read((char*)&vert,sizeof(char)); 
								f.read((char*)&bleu,sizeof(char));
								this->m_pucPixel[i*this->m_iLargeur+j/3]=(unsigned char)(((int)rouge+(int)vert+(int)bleu)/3);
							}	

							char inutile;
							for (int k=0; k< complement; k++)
								f.read((char*)&inutile,sizeof(char));
						}
					}
				}
			}
			f.close();
		}
		else
			throw std::string("ERREUR : Image absente (ou pas ici en tout cas) !");
}

CImageNdg::CImageNdg(const CImageNdg& im) {

	this->m_iHauteur = im.lireHauteur();
	this->m_iLargeur = im.lireLargeur();
	this->m_bBinaire = im.lireBinaire(); 
	this->m_sNom     = im.lireNom();
	this->m_pucPixel = NULL; 
	this->m_pucPalette = NULL;

	if (im.m_pucPalette != NULL) {
		this->m_pucPalette = new unsigned char[256*4];
		memcpy(this->m_pucPalette,im.m_pucPalette,4*256);
	}
	if (im.m_pucPixel != NULL) {
		this->m_pucPixel = new unsigned char[im.lireHauteur() * im.lireLargeur()];
		memcpy(this->m_pucPixel,im.m_pucPixel,im.lireNbPixels());
	}
}

CImageNdg::~CImageNdg() {
	if (this->m_pucPixel) {
		delete[] this->m_pucPixel;
		this->m_pucPixel = NULL;
	}

	if (this->m_pucPalette) {
		delete[] this->m_pucPalette;
		this->m_pucPalette = NULL;
	}
}

void CImageNdg::sauvegarde(std::string name) {
	BITMAPFILEHEADER header;
	BITMAPINFOHEADER infoHeader;

	if (this->m_pucPixel) {
		std::string nomFichier;

		if(name !="")
			nomFichier = "../../../Resultats/" + name +".bmp"; // force sauvegarde dans répertoire Res (doit exister)
		else
			nomFichier = "../../../Resultats/" + this->lireNom() + ".bmp"; // force sauvegarde dans répertoire Res (doit exister)
		
		std::ofstream f(nomFichier.c_str(),std::ios::binary);
		if (f.is_open()) {

			int complement = (((this->m_iLargeur-1)/4) + 1)*4 - this->m_iLargeur;

			header.bfType = MAGIC_NUMBER_BMP;
			f.write((char*)&header.bfType, 2);
			header.bfOffBits = 14 * sizeof(char) + 40 * sizeof(char) + 4 * 256 * sizeof(char); // palette
			header.bfSize = header.bfOffBits + (complement + lireLargeur())*lireHauteur()*sizeof(char);
			f.write((char*)&header.bfSize, 4);
			header.bfReserved1 = 0;
			f.write((char*)&header.bfReserved1, 2);
			header.bfReserved2 = 0;
			f.write((char*)&header.bfReserved2, 2);
			f.write((char*)&header.bfOffBits, 4);

			infoHeader.biSize = 40*sizeof(char);
			f.write((char*)&infoHeader.biSize, 4);
			infoHeader.biWidth = this->m_iLargeur;
			f.write((char*)&infoHeader.biWidth, 4);
			infoHeader.biHeight = this->m_iHauteur;
			f.write((char*)&infoHeader.biHeight, 4);
			infoHeader.biPlanes = 1;
			f.write((char*)&infoHeader.biPlanes, 2);
			infoHeader.biBitCount = 8;
			f.write((char*)&infoHeader.biBitCount, 2);
			infoHeader.biCompression = 0; // pas de compression
			f.write((char*)&infoHeader.biCompression, 4);
			infoHeader.biSizeImage = this->lireNbPixels();
			f.write((char*)&infoHeader.biSizeImage, 4);
			infoHeader.biXPelsPerMeter = 0;
			f.write((char*)&infoHeader.biXPelsPerMeter, 4);
			infoHeader.biYPelsPerMeter = 0;
			f.write((char*)&infoHeader.biYPelsPerMeter, 4);
			infoHeader.biClrUsed = 256;
			f.write((char*)&infoHeader.biClrUsed, 4);
			infoHeader.biClrImportant = 0;
			f.write((char*)&infoHeader.biClrImportant, 4);

			// on remplit la palette
			for (int indice=0;indice<4*256;indice ++) 
				f.write((char*)&this->m_pucPalette[indice],sizeof(char)); 

			for (int i= this->m_iHauteur-1; i >= 0; i--) {
				for (int j=0;j<m_iLargeur;j++) 
					f.write((char*)&this->m_pucPixel[i*m_iLargeur+j],sizeof(char));
					
				// gérer multiple de 32 bits
				char inutile;
				for (int k=0; k< complement; k++)
					f.write((char*)&inutile,sizeof(char)); 
				}
		f.close();
		}
		else
			throw std::string("Impossible de creer le fichier de sauvegarde !");
	}
	else
		throw std::string("Pas de donnee a sauvegarder !");
}

CImageNdg& CImageNdg::operator=(const CImageNdg& im) {

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
	this->m_pucPalette = new unsigned char[256*4];

	if (im.m_pucPalette != NULL)
		memcpy(this->m_pucPalette,im.m_pucPalette,4*256);
	if (im.m_pucPixel != NULL)
		memcpy(this->m_pucPixel,im.m_pucPixel,im.lireNbPixels());

return *this;
}

// fonctionnalités histogramme 
std::vector<unsigned long> CImageNdg::histogramme(bool enregistrementCSV) {

	std::vector<unsigned long> h;

	h.resize(256,0);
	for (int i=0;i<this->lireNbPixels();i++) 
		h[this->operator()(i)] += 1L; 

	if (enregistrementCSV) {
	 std::string fichier = "../Res/" + this->lireNom() + ".csv";
		std::ofstream f (fichier.c_str());

		if (!f.is_open())
			std::cout << "Impossible d'ouvrir le fichier en ecriture !" << std::endl;
		else {
			for (int i=0;i<(int)h.size();i++)
				f << h[i] << std::endl;
		}
		f.close();
	}

	return h;
}

// signatures globales
MOMENTS CImageNdg::signatures(const std::vector<unsigned long>& h) {

	MOMENTS globales;
	
	// min
	int i=0;
	while ((i < (int)h.size()) && (h[i] == 0))
		i++;
	globales.minNdg = i;
		
	// max
	i=h.size()-1;
	while ((i > 0) && (h[i] == 0))
		i--;
	globales.maxNdg = i;

	// mediane
	int moitPop = this->lireNbPixels()/2;

	i=globales.minNdg;
	int somme = h[i];
	while (somme < moitPop) {
		i += 1;
		if (i < (int)h.size())
			somme += h[i];
	}
	globales.medianeNdg = i;

	// moyenne et écart-type
	float moy=0,sigma=0;
	for (i=globales.minNdg;i<=globales.maxNdg;i++) {
		moy += ((float)h[i])*i;
		sigma += ((float)h[i])*i*i;
	}
	moy /= (float)this->lireNbPixels();
	sigma = sqrt(sigma/(float)this->lireNbPixels() - (moy*moy));
	globales.moyenneNdg = moy;
	globales.ecartTypeNdg = sigma;

	return globales;
}

MOMENTS CImageNdg::signatures() {
	MOMENTS globales = { 0,0,0,0,0 };
	std::vector<unsigned long> hist;
	hist = this->histogramme();
	globales = this->signatures(hist);
	return globales;
}

// opérations ensemblistes images binaires
CImageNdg& CImageNdg::operation(const CImageNdg& im, const std::string& methode) {

	if ((&im == this) || !(this->lireBinaire() && im.lireBinaire())) {
		std::cout << "operation logique uniquement possible entre 2 images binaires" << std::endl;
		return *this;
	}

	this->m_iHauteur = im.lireHauteur();
	this->m_iLargeur = im.lireLargeur();
	this->m_bBinaire = im.lireBinaire(); 
	this->choixPalette("binaire"); // images binaires -> palettes binaires
	this->m_sNom     = im.lireNom()+"Op";

	if (methode.compare("et") == 0) {
		for (int i=0;i<this->lireNbPixels();i++)
			this->operator()(i) = this->operator()(i) && im(i);
	}
	else
		if (methode.compare("ou") == 0) {
			for (int i=0;i<this->lireNbPixels();i++)
				this->operator()(i) = this->operator()(i) || im(i);
		}

return *this;
}

CImageNdg CImageNdg::Difference(const CImageNdg& im2) { // Difference current img - im2
	CImageNdg out(this->lireHauteur(),im2.lireLargeur(), 0);
	int pixel;

	for (int i = 0; i < this->lireNbPixels(); i++) {
		pixel = this->operator()(i) - im2.operator()(i);
		if (pixel >= 0 && pixel <=255) {
			out.operator()(i) = (unsigned char)pixel;
		}
		else if (pixel >255){
			out.operator()(i) = 255;
		}
		else {
			out.operator()(i) = 0;
		}
	}
	return out;
}

// seuillage
CImageNdg CImageNdg::seuillage(const std::string& methode, int seuilBas, int seuilHaut) {
	
	if (!this->m_bBinaire) {
		CImageNdg out(this->lireHauteur(),this->lireLargeur());
		out.choixPalette("binaire"); // palette binaire par défaut
		out.m_bBinaire = true;

		// création lut pour optimisation calcul
		std::vector<int> lut;
		lut.resize(256);

		// recherche valeur seuil
		// cas "manuel" 
		if (methode.compare("manuel") == 0)
		{
			out.m_sNom = this->lireNom() + "SeMa";
		}
		else 
			if (methode.compare("auto") == 0)
			{
				out.m_sNom = this->lireNom() + "SeAu";
				// recherche seuil via Otsu
				std::vector<unsigned long> hist = this->histogramme();
				std::vector<unsigned long> histC; // histogramme cumulé
				histC.resize(256, 0);
				histC[0] = hist[0];
				for (int i = 1; i < (int)hist.size(); i++)
					histC[i] = histC[i - 1] + hist[i];

				MOMENTS globales = this->signatures(hist);
				int min = globales.minNdg,
					max = globales.maxNdg;

				// f(s)
				std::vector<double> tab;
				tab.resize(256, 0);

				double M1, M2, w1;

				// initialisation
				M1 = min;
				seuilBas = min;
				seuilHaut = 255;

				w1 = (double)histC[min] / (double)(this->lireNbPixels());
				M2 = 0;
				for (int i = min + 1; i <= max; i++)
					M2 += (double)hist[i] * i;
				M2 /= (double)(histC[max] - hist[min]);
				tab[min] = w1*(1 - w1)*(M1 - M2)*(M1 - M2);

				for (int i = min + 1; i < max; i++) {
					M1 = ((double)histC[i - 1] * M1 + (double)hist[i] * i) / histC[i];
					M2 = ((double)(histC[255] - histC[i - 1])*M2 - hist[i] * i) / (double)(histC[255] - histC[i]);
					w1 = (double)histC[i] / (double)(this->lireNbPixels());
					tab[i] = w1*(1 - w1)*(M1 - M2)*(M1 - M2);
					if (tab[i] > tab[seuilBas])
						seuilBas = i;
				}
			}
			else {
			// gestion des seuils valant "moyenne" et "mediane"
				std::vector<unsigned long> hist = this->histogramme();
				MOMENTS globales = this->signatures(hist);

				if (methode.compare("moyenne") == 0)
				{
					out.m_sNom = this->lireNom() + "SeMo";
					seuilBas = (int)globales.moyenneNdg;
					seuilHaut = 255;
				}

				if (methode.compare("mediane") == 0)
				{
					out.m_sNom = this->lireNom() + "SeMe";
					seuilBas = globales.medianeNdg;
					seuilHaut = 255;
				}
			}
 

		// génération lut
		for (int i = 0; i < seuilBas; i++)
			lut[i] =  0; 
		for (int i = seuilBas; i <= seuilHaut; i++)
			lut[i] = 1;
		for (int i = seuilHaut+1; i <= 255; i++)
			lut[i] = 0;

		// création image seuillée
	//	std::cout << "Seuillage des pixels entre " << seuilBas << " et " << seuilHaut << std::endl;
		for (int i=0; i < out.lireNbPixels(); i++) 
			out(i) = lut[this->operator ()(i)]; 

		return out;
		}
	else {
	//	std::cout << "Seuillage image binaire impossible" << std::endl;
		return (*this);
	}
}

// transformation
CImageNdg CImageNdg::transformation(const std::string& methode,int vMinOut, int vMaxOut) {

	CImageNdg out(this->lireHauteur(),this->lireLargeur());
	
	out.choixPalette(this->lirePalette()); // conservation de la palette
	out.m_bBinaire = this->m_bBinaire; // conservation du type

	if (methode.compare("complement") == 0) {
		out.m_sNom = this->lireNom() + "TComp";
		/*if (!this->m_bBinaire) {*/
			// ndg -> 255-ndg
			// création lut pour optimisation calcul
			std::vector<int> lut;
			lut.resize(256);

			for (int i = 0; i < 256; i++)
				lut[i] = (int)(255 - i);
			for (int i = 0; i < out.lireNbPixels(); i++)
				out(i) = lut[this->operator()(i)];
		//}
		//else {
		//	// 0 -> 1 et 1 -> 0
		//	for (int i = 0; i < out.lireNbPixels(); i++)
		//		out(i) = !this->operator()(i);
		//}
	}
	else 
		if (methode.compare("expansion") == 0) {
			out.m_sNom = this->lireNom() + "TExp";
			int min = 255, max = 0;
			for (int i = 0; i < this->lireNbPixels(); i++) {
				if (this->operator()(i) > max)
					max = this->operator()(i);
				if (this->operator()(i) < min)
					min = this->operator()(i);
			}

			double a = 255 / (double)(max - min);
			double b = -a*min;

			std::vector<int> lut;
			lut.resize(256);

			for (int i = 0; i < 256; i++)
				lut[i] = (int)(a*i + b);
			for (int i = 0; i < out.lireNbPixels(); i++)
				out(i) = lut[this->operator()(i)];
		}
		else 
			if (methode.compare("egalisation") == 0) {
				out.m_sNom = this->lireNom() + "TEga";
				std::vector<unsigned long> hist = this->histogramme();
				std::vector<unsigned long> histC; // histogramme cumulé
				std::vector<int> lut;
				lut.resize(256);
				histC.resize(256, 0);

				histC[0] = hist[0];
				lut[0] = (((double)255 / ((double)this->lireNbPixels()) * histC[0]));

				for (int i = 1; i < (int)hist.size(); i++) {
					histC[i] = histC[i - 1] + hist[i];
					lut[i] =  (((double)255 / ((double)this->lireNbPixels()) * histC[i]));
				}
				
				for (int i = 0; i < out.lireNbPixels(); i++)
					out(i) = lut[this->operator()(i)];

			}
	return out;
}

// morphologie
CImageNdg CImageNdg::morphologie(const std::string& methode, const std::string& eltStructurant) {
		
	CImageNdg out(this->lireHauteur(),this->lireLargeur());
	out.choixPalette(this->lirePalette()); // conservation de la palette
	out.m_bBinaire = this->m_bBinaire; // conservation du type
		
	if (methode.compare("erosion") == 0) {	
		out.m_sNom = this->lireNom() + "MEr";
		CImageNdg agrandie(this->lireHauteur()+2,this->lireLargeur()+2);

		// gestion des bords
		if (this->lireBinaire()) {
			int pix;

			for (pix=0;pix<agrandie.lireLargeur();pix++) {
				agrandie(0,pix) = 1;
				agrandie(this->lireHauteur()-1,pix) = 1;
			}
			for (pix=1;pix<agrandie.lireHauteur()-1;pix++) {
				agrandie(pix,0) = 1;
				agrandie(pix,this->lireLargeur()-1) = 1;
			}
		}
		else {
			int pix;

			for (pix=0;pix<agrandie.lireLargeur();pix++) {
				agrandie(0,pix) = 255;
				agrandie(this->lireHauteur()-1,pix) = 255;
			}
			for (pix=1;pix<agrandie.lireHauteur()-1;pix++) {
				agrandie(pix,0) = 255;
				agrandie(pix,this->lireLargeur()-1) = 255;
			}
		}

		// gestion du coeur
		for (int i=0;i<this->lireHauteur();i++)
			for (int j=0;j<this->lireLargeur();j++) {
				agrandie(i+1,j+1)=this->operator()(i,j);
			}

		if (eltStructurant.compare("V4") == 0) {
			for (int i=1;i<agrandie.lireHauteur()-1;i++)
				for (int j=1;j<agrandie.lireLargeur()-1;j++) {
					int minH = std::min(agrandie(i,j-1),agrandie(i,j+1));
					int minV = std::min(agrandie(i-1,j),agrandie(i+1,j));
					int minV4 = std::min(minH,minV);
					out(i-1,j-1)= std::min(minV4,(int)agrandie(i,j));
				}
		}
		else {
			if (eltStructurant.compare("V8") == 0) {
			for (int i=1;i<agrandie.lireHauteur()-1;i++)
				for (int j=1;j<agrandie.lireLargeur()-1;j++) {
					int minH = std::min(agrandie(i,j-1),agrandie(i,j+1));
					int minV = std::min(agrandie(i-1,j),agrandie(i+1,j));
					int minV4 = std::min(minH,minV);
					int minD1 = std::min(agrandie(i-1,j-1),agrandie(i+1,j+1));
					int minD2 = std::min(agrandie(i-1,j+1),agrandie(i+1,j-1));
					int minD = std::min(minD1,minD2);
					int minV8 = std::min(minV4,minD);
					out(i-1,j-1) = std::min(minV8,(int)agrandie(i,j));
				}
			}
		}
	}
	else {
		if (methode.compare("dilatation") == 0) {
			out.m_sNom = this->lireNom() + "MDi";
			CImageNdg agrandie(this->lireHauteur()+2,this->lireLargeur()+2);

			// gestion des bords
			int pix;

			for (pix=0;pix<agrandie.lireLargeur();pix++) {
				agrandie(0,pix) = 0;
				agrandie(agrandie.lireHauteur()-1,pix) = 0;
			}
			for (pix=1;pix<agrandie.lireHauteur()-1;pix++) {
				agrandie(pix,0) = 0;
				agrandie(pix,agrandie.lireLargeur()-1) = 0;
			}

			// gestion du coeur
			for (int i=0;i<this->lireHauteur();i++)
				for (int j=0;j<this->lireLargeur();j++) {
					agrandie(i+1,j+1)=this->operator()(i,j);
				}

			if (eltStructurant.compare("V4") == 0) {
				for (int i=1;i<agrandie.lireHauteur()-1;i++)
					for (int j=1;j<agrandie.lireLargeur()-1;j++) {
						int maxH = std::max(agrandie(i,j-1),agrandie(i,j+1));
						int maxV = std::max(agrandie(i-1,j),agrandie(i+1,j));
						int maxV4 = std::max(maxH,maxV);
						out(i-1,j-1)= std::max(maxV4,(int)agrandie(i,j));
					}
			}
			else {
				if (eltStructurant.compare("V8") == 0) {
				for (int i=1;i<agrandie.lireHauteur()-1;i++)
					for (int j=1;j<agrandie.lireLargeur()-1;j++) {
						int maxH = std::max(agrandie(i,j-1),agrandie(i,j+1));
						int maxV = std::max(agrandie(i-1,j),agrandie(i+1,j));
						int maxV4 = std::max(maxH,maxV);
						int maxD1 = std::max(agrandie(i-1,j-1),agrandie(i+1,j+1));
						int maxD2 = std::max(agrandie(i-1,j+1),agrandie(i+1,j-1));
						int maxD = std::max(maxD1,maxD2);
						int maxV8 = std::max(maxV4,maxD);
						out(i-1,j-1)= std::max(maxV8,(int)agrandie(i,j));
					}
				}
			}
		}
	}

	return out;
}

// filtrage : moyen ou gaussien (
CImageNdg CImageNdg::filtrage(const std::string& methode, int N, double sigma) {
		
	CImageNdg out(this->lireHauteur(),this->lireLargeur());

	out.choixPalette(this->lirePalette()); // conservation de la palette
	out.m_bBinaire = this->m_bBinaire; // conservation du type
		
	if (methode.compare("moyen") == 0) {
		out.m_sNom = this->lireNom() + "FiMo";
		int nbBords = N/2;

		for (int i=0;i<this->lireHauteur();i++)
			for (int j=0;j<this->lireLargeur();j++) {
				// gestion des bords
				int dk= std::max(0,i-nbBords);
				int fk= std::min(i+nbBords,this->lireHauteur()-1);
				int dl= std::max(0,j-nbBords);
				int fl= std::min(j+nbBords,this->lireLargeur()-1);

				double somme=0;
				double moy=0;
				for (int k=dk;k<=fk;k++)
					for (int l=dl;l<=fl;l++) {
						moy += (double)this->operator()(k,l);
						somme += 1;
					}
				out(i,j)=(int)(moy/somme);
			}
	}
	else
		if (methode.compare("gaussien") == 0)
		{
			out.m_sNom = this->lireNom() + "FiGa";
			// définition du noyau
			double **noyau = NULL;
			noyau = (double**)malloc(N*sizeof(double*));
			noyau[0] = (double*)malloc(N*N*sizeof(double));
			for (int i = 1; i < N; i++)
				noyau[i] = &noyau[0][i*N];

			double somme = 0; // normalisation
			for (int i = 0; i < N; i++)
			{
				for (int j = 0; j < N; j++)
				{
					noyau[i][j] = exp(-((i - N / 2)*(i - N / 2) + (j - N / 2)*(j - N / 2)) / (2 * sigma*sigma));
					somme += noyau[i][j];
				}
			}

			// filtrage
			int nbBords = N / 2;

			CImageNdg agrandie(this->lireHauteur() + nbBords * 2, this->lireLargeur() + nbBords * 2);

			// gestion du coeur
			for (int i = 0; i < this->lireHauteur(); i++)
				for (int j = 0; j < this->lireLargeur(); j++) {
					agrandie(i + nbBords, j + nbBords) = this->operator()(i, j);
				}

			// gestion des bords
			for (int pix = 0; pix < agrandie.lireLargeur(); pix++) {
				for (int t = nbBords - 1; t >= 0; t--)
					agrandie(t, pix) = agrandie(nbBords, pix);
				for (int t = agrandie.lireHauteur() - 1; t >= agrandie.lireHauteur() - 1 - nbBords; t--)
					agrandie(t, pix) = agrandie(agrandie.lireHauteur() - 1 - nbBords, pix);
			}
			for (int pix = 0; pix < agrandie.lireHauteur(); pix++) {
				for (int t = nbBords - 1; t >= 0; t--)
					agrandie(pix, t) = agrandie(pix, nbBords);
				for (int t = agrandie.lireLargeur() - 1; t >= agrandie.lireLargeur() - 1 - nbBords; t--)
					agrandie(pix, t) = agrandie(pix, agrandie.lireLargeur() - 1 - nbBords);
			}

			for (int i = nbBords; i < agrandie.lireHauteur() - nbBords; i++)
				for (int j = nbBords; j < agrandie.lireLargeur() - nbBords; j++) {
					double somme = 0;
					double moy = 0;

					for (int k = -nbBords; k <= nbBords; k++)
						for (int l = -nbBords; l <= nbBords; l++) {
							moy += (double)agrandie(i - k, j - l)*noyau[k + nbBords][l + nbBords];
							somme += noyau[k + nbBords][l + nbBords];
						}
					out(i - nbBords, j - nbBords) = (int)(moy / somme);
				}

			free(noyau[0]);
			free(noyau);
			noyau = NULL;
		}

	return out;
}

// extraction maxima locaux -> sortie image binaire
CImageNdg CImageNdg::maxiLocaux(int N, int M) {
	CImageNdg out(this->lireHauteur(), this->lireLargeur(),0);

	out.choixPalette("binaire"); // palette binaire par défaut
	out.m_bBinaire = true;
	out.m_sNom = this->lireNom() + "ML";

	int ns2 = N / 2;
	int ms2 = M / 2;

	for (int i = 0; i<this->lireHauteur(); i++)
		for (int j = 0; j<this->lireLargeur(); j++) {
			int dk = std::max(0, i - ns2);
			int fk = std::min(i + ns2, this->lireHauteur() - 1);
			int dl = std::max(0, j - ms2);
			int fl = std::min(j + ms2, this->lireLargeur() - 1);

			int maxVal = this->operator()(i, j);
			bool flag = true;
			int k = dk;
			while ((k <= fk) && (flag == true)) {
				int l = dl;
				while ((l <= fl) && (flag == true)) {
					if (this->operator()(k, l) > maxVal)
						flag = false;
					l++;
				}
				k++;
			}
			if (flag == true)
				out(i, j) = 1;
		}

	return out;
}

// extraction ROI
CImageNdg CImageNdg::ROI(int ci, int cj, int N, int M) {
	int ns2 = N / 2;
	int ms2 = M / 2;

	int dk = std::max(0, ci - ns2);
	int fk = std::min(ci + ns2, this->lireHauteur() - 1);
	int dl = std::max(0, cj - ms2);
	int fl = std::min(cj + ms2, this->lireLargeur() - 1);

	CImageNdg out(fk-dk+1,fl-dl+1);

	out.choixPalette(this->lirePalette()); // conservation de la palette
	out.m_bBinaire = this->m_bBinaire; // conservation du type
	out.m_sNom = this->lireNom() + "ROI";

	int i = 0;
	int j = 0;
	for (int k = dk; k <= fk; k++) {
		j = 0;
		for (int l = dl; l <= fl; l++) {
			out(i, j) = this->operator()(k, l);
			j += 1;
		}
		i += 1;
	}

	return out;
}
