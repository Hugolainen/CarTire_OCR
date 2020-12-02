#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <windows.h>
#include <cmath>
#include <vector>
#include <ctime>

#include "ImageClasse.h"

#define MAGIC_NUMBER_BMP ('B'+('M'<<8)) // signature bitmap windows
#define PI 3.14159265

// constructeurs et destructeur
CImageClasse::CImageClasse() {

	this->m_iHauteur   = 0;
	this->m_iLargeur   = 0;
	this->m_sNom       = "vide";
	this->m_lNbRegions = 0;
	this->m_pucPixel   = NULL;
}

CImageClasse::CImageClasse(int hauteur, int largeur) {

	this->m_iHauteur   = hauteur;
	this->m_iLargeur   = largeur;
	this->m_sNom       = "inconnu";
	this->m_lNbRegions = 0;
	this->m_pucPixel   = new unsigned long[hauteur*largeur];
	for (int i=0;i<this->lireNbPixels();i++)
		this->m_pucPixel[i] = 0; 
}

CImageClasse::CImageClasse(const CImageNdg& im, std::string choix) {

	this->m_iHauteur   = im.lireHauteur();
	this->m_iLargeur   = im.lireLargeur();
	this->m_sNom       = im.lireNom()+"E";
	this->m_lNbRegions = 0;
	this->m_pucPixel   = new unsigned long[m_iHauteur*m_iLargeur];


	CImageClasse withBords(im.lireHauteur()+2,im.lireLargeur()+2); 
	int i,ii,j,jj;

    for (i=0,ii=1; i < im.lireHauteur(); i++, ii++) // pure copie 
      for (j=0,jj=1; j < im.lireLargeur(); j++, jj++)
		  withBords(ii,jj) = (unsigned long)im(i,j);

	int* tableEtiquette = NULL;

	int k=1;
	int eps;
	int* predNonNul = new int [6];
	int nbPredNonNuls;

	int nbMaxEtiquettes = im.lireHauteur()*im.lireLargeur();

	tableEtiquette = new int [nbMaxEtiquettes];
	for (int kk=0;kk<nbMaxEtiquettes;kk++) {
		tableEtiquette[kk]= kk;
	}

	if (choix.compare("V4") == 0) {
		for (i=1;i<withBords.lireHauteur()-1;i++)
			for (j=1;j<withBords.lireLargeur()-1;j++) {
				if (withBords(i,j) != 0) {
				// V4 
				int pred1 = withBords(i,j-1);
				int pred3 = withBords(i-1,j);
			
				nbPredNonNuls = 0;
				if (pred1) predNonNul[nbPredNonNuls++] = pred1; 
				if (pred3) predNonNul[nbPredNonNuls++] = pred3; 
		
				if (nbPredNonNuls == 0) {
					withBords(i,j) = k;
					k++;
				}
				else {
					eps = 0;
					int minPred = tableEtiquette[predNonNul[0]];
					int maxPred = tableEtiquette[predNonNul[0]];
					while (eps<nbPredNonNuls) {
						if (tableEtiquette[predNonNul[eps]] < minPred) minPred = tableEtiquette[predNonNul[eps]];
						if (tableEtiquette[predNonNul[eps]] > maxPred) maxPred = tableEtiquette[predNonNul[eps]];
						eps ++;
					}
				if (minPred == maxPred) 
					withBords(i,j) = minPred;
				else {
					withBords(i,j) = minPred;
					for (eps=0;eps<nbPredNonNuls;eps++) {
						int beta = predNonNul[eps];
						while (tableEtiquette[beta] != minPred) {
							int m = tableEtiquette[beta];
							tableEtiquette[beta] = minPred;
							beta = m;
						}
					}
				}
				}
				}
			}
	}
	else { // V8
		for (i=1;i<withBords.lireHauteur()-1;i++)
			for (j=1;j<withBords.lireLargeur()-1;j++) {
				if (withBords(i,j) != 0) {
					// V8 
					int pred1 = withBords(i,j-1);
					int pred2 = withBords(i-1,j-1);
					int pred3 = withBords(i-1,j);
					int pred4 = withBords(i-1,j+1);
		
					nbPredNonNuls = 0;
					if (pred1) predNonNul[nbPredNonNuls++] = pred1; 
					if (pred2) predNonNul[nbPredNonNuls++] = pred2;
					if (pred3) predNonNul[nbPredNonNuls++] = pred3; 
					if (pred4) predNonNul[nbPredNonNuls++] = pred4;

					if (nbPredNonNuls == 0) {
						withBords(i,j) = k;
						k++;
					}
					else {
						eps = 0;
						int minPred = tableEtiquette[predNonNul[0]];
						int maxPred = tableEtiquette[predNonNul[0]];
						while (eps<nbPredNonNuls) {
							if (tableEtiquette[predNonNul[eps]] < minPred) minPred = tableEtiquette[predNonNul[eps]];
							if (tableEtiquette[predNonNul[eps]] > maxPred) maxPred = tableEtiquette[predNonNul[eps]];
							eps ++;
						}
					if (minPred == maxPred) 
						withBords(i,j) = minPred;
					else {
						withBords(i,j) = minPred;
						for (eps=0;eps<nbPredNonNuls;eps++) {
							int beta = predNonNul[eps];
							while (tableEtiquette[beta] != minPred) {
								int m = tableEtiquette[beta];
								tableEtiquette[beta] = minPred;
								beta = m;
							}
						}
					}
				}
			}
		}	
	} 
  
	// actualisation de la table d'équivalence
	for (int kk=1;kk<k;kk++) {
		int m = kk;
		while (tableEtiquette[m] != m) 
		m = tableEtiquette[m];
		tableEtiquette[kk] = m;
	}

	// on vire les trous histoire que nbObjets = max de l'image label
	int* etiquettes = new int [k+1];
	for (int kk=0;kk<k;kk++)
		etiquettes[kk] = 0;

	// histo pour repérer les trous
	for (int kk=1;kk<k;kk++)
		etiquettes[tableEtiquette[kk]]++;

	// on remet à jour les index (etiquetage définitif hors trou)
	etiquettes[0] = 0;
	int compt = 1;
	for (int kk=1;kk<k;kk++) {
		if (etiquettes[kk]) 
		etiquettes[kk] = compt++;
	}

	for (i=0;i<withBords.lireHauteur();i++)
		for (j=0;j<withBords.lireLargeur();j++) {
			withBords(i,j) = etiquettes[tableEtiquette[withBords(i,j)]];
		}

	delete [] etiquettes;
	delete [] tableEtiquette;

	// création image labélisée
	// suppression des 2 lignes et 2 colonnes artificiellement créées

	for (i=0,ii=1; i < this->lireHauteur(); i++, ii++)  
		for (j=0,jj=1; j < this->lireLargeur(); j++, jj++)
			this->operator()(i,j) = withBords(ii,jj);

	m_lNbRegions = compt-1;
	//std::cout << compt-1 << " objets dans l'image..." << std::endl;
}

CImageClasse::CImageClasse(const CImageClasse& in, std::string miseAJour, std::string voisinage) {

	if (miseAJour.compare("sans") == 0) { // simple copie
		this->m_iHauteur   = in.lireHauteur();
		this->m_iLargeur   = in.lireLargeur();
		this->m_sNom = in.lireNom();
		this->m_lNbRegions = in.lireNbRegions();
		this->m_pucPixel = new unsigned long[in.lireHauteur()*in.lireLargeur()];
		if (in.m_pucPixel != NULL)
			memcpy(this->m_pucPixel,in.m_pucPixel,in.lireNbPixels()*sizeof(unsigned long));
	}
	else { 
		// détection cas objets/fond ou ND, présence ou non d'une classe à 0
		bool objetsFond = false;
		int pix=0;
		while ( (pix < in.lireNbPixels()) && (!objetsFond) ) {
			if (in(pix) == 0) objetsFond = true;
			pix++;
		}

		if (objetsFond) { // cas présence fond à 0
			CImageNdg im(in.lireHauteur(),in.lireLargeur());
			for (int i=0;i<in.lireNbPixels();i++)
				im(i) = (in(i) > 0) ? 1 : 0;
			CImageClasse out(im,voisinage);

			this->m_iHauteur = in.lireHauteur();
			this->m_iLargeur = in.lireLargeur();
			this->m_sNom       = in.lireNom()+"RE";
			this->m_lNbRegions = out.lireNbRegions();
			this->m_pucPixel   = new unsigned long[in.lireHauteur()*in.lireLargeur()];
			if (out.m_pucPixel != NULL)
				memcpy(this->m_pucPixel,out.m_pucPixel,out.lireNbPixels()*sizeof(unsigned long));
		}
		else { // cas nuées dynamiques donc ré-étiquetage classe après classe
			this->m_iHauteur = in.lireHauteur();
			this->m_iLargeur = in.lireLargeur();
			this->m_sNom = in.lireNom() + "RE";
			this->m_lNbRegions = 0;

			this->m_pucPixel = new unsigned long[in.lireHauteur()*in.lireLargeur()];
			for (int pix = 0; pix < this->lireNbPixels(); pix++)
				this->m_pucPixel[pix] = 0;

			for (int classe = 1; classe <= in.lireNbRegions(); classe++) {
				CImageNdg im(in.lireHauteur(), in.lireLargeur());

				for (int i = 0; i < in.lireNbPixels(); i++)
					im(i) = (in(i) == classe) ? 1 : 0;
				CImageClasse inter(im, voisinage);
				for (int pix = 0; pix < inter.lireNbPixels(); pix++)
					if (inter(pix))
						this->operator()(pix) = inter(pix) + this->lireNbRegions();

				this->ecrireNbRegions(this->lireNbRegions() + inter.lireNbRegions());
			}
			std::cout << this->lireNbRegions() << " nouvelles régions connexes dans l'image..." << std::endl;
		}
	}
}

CImageClasse::CImageClasse(const CImageNdg& im, int nbClusters, std::string choix) {

	this->m_iHauteur   = im.lireHauteur();
	this->m_iLargeur   = im.lireLargeur();
	this->m_sNom       = im.lireNom()+"ND";
	this->m_lNbRegions = 0;
	this->m_pucPixel   = new unsigned long[m_iHauteur*m_iLargeur];


	// contiendra la somme des elements de la classe
	// pour pouvoir calculer le centre de gravité !
	std::vector<unsigned long> sommeClasses;

	// contiendra le nombre des elements de la classe
	// pour pouvoir calculer le centre de gravité !
	std::vector<unsigned long> nbEltsClasses;

	// contiendra le centre de chaque classe
	std::vector<unsigned char> indexClasses;

	// contiendra la classe de chaque niveau de gris
	// imageFinale = lut[imageEntree]
	std::vector<int> lut;

	// allocation de k+1 classes
	// volonté d'affecter l'indice i à la classe i
	nbEltsClasses.resize(nbClusters+1); 
	for (int i=1;i<=nbClusters;i++)
		nbEltsClasses[i]=0;
	sommeClasses.resize(nbClusters+1);
	for (int i=1;i<=nbClusters;i++)
		sommeClasses[i]=0L;

	// moyenne image et histo pour initialisation
	std::vector<unsigned long> h;
	h.resize(256,0);
	for (int i=0;i<this->lireNbPixels();i++) 
		h[im(i)] += 1L;
	float mean=0;
	for (int i=0;i<256;i++) 
		mean += h[i]*i;
	mean /= this->lireNbPixels();

	// nb de niveaux de gris dans image initiale
	// si k supérieur, alors on diminue le nb de classes en sortie : nbClasses
	int nbLevels=0;
	for (int i=0;i<256;i++)
		if (h[i]) nbLevels++;
	int nbClasses;
	nbClasses=min(nbClusters,nbLevels);
	m_lNbRegions = nbClasses;

	if (strcmp(choix.c_str(),"aleatoire") == 0) { // tirage aléatoire des germes initiaux 

		//srand((unsigned)time(NULL));
		indexClasses.resize(nbClasses+1);

		int num=1;
		while (num <= nbClasses) {
			int encours = rand()%256;
			bool dejaPris=false;
			for (int j=1;j<num;j++)
				if (indexClasses[j] == encours) dejaPris=true;
			if (dejaPris==false) {
				indexClasses[num] = encours;
				num++;
			}
		}
	}
	else if (strcmp(choix.c_str(),"regulier") == 0) { // intervalles réguliers entre germes 
		indexClasses.resize(nbClasses+1);
		// recherche du min et du max
		int min = 0, max = 255;
		while (h[min] == 0)
			min++;
		while (h[max] == 0)
			max--;

		int num=1;
		int encours = min;
		int pas = (max-min)/nbClasses;

		while (num <= nbClasses) {  
			indexClasses[num] = encours;
			encours += pas;
			num++;
		}
	}

	//std::cout << "classification en " << nbClasses << " classes..." << std::endl;

	lut.resize(256);
	int nbTours = 0 ;
	bool change = true ;

	while ( (!nbTours) ||  ((change) && ( nbTours < MAX_ITER))) {
		// Remise à zéro à chaque tour 
		for (int i=1;i<=nbClasses;i++) {
			nbEltsClasses[i]=0;
			sommeClasses[i]=0L;
		}
		change = false ;

		// Pour chaque point trouver son plus proche
		for (int i =0; i < 256 ; i++)
		{    
		  unsigned char encours = i;
		  int plusproche = 1 ;
		  for (int j = 2; j <= nbClasses ; j++)
			{              
			  if (EUCLIDEAN(encours, indexClasses[j]) < EUCLIDEAN(encours, indexClasses[plusproche]))
				plusproche = j ;
			}        
		  lut[i] = plusproche ;
		  nbEltsClasses[plusproche] += h[encours] ;
		  sommeClasses[plusproche] += h[encours]*encours ;
		}

	  // On recalcule le centre de chaque classe
	  // a partir des nouveaux germes.
	  for (int j = 1 ; j <= nbClasses ; j++)
		{
		  if (nbEltsClasses[j])
			  sommeClasses[j] /= nbEltsClasses[j] ;
		  else
			  sommeClasses[j] = (unsigned long)mean;
		  if (EUCLIDEAN(indexClasses[j],sommeClasses[j]) > 0.001)  
			  change = true ;
		  indexClasses[j] = (unsigned char)sommeClasses[j] ;
		}
	  nbTours ++ ;
	//  std::cout << "Iteration : " << nbTours <<    std::endl ;     
	}

	// on remplace chaque pixel par sa classe
	for (int i=0; i < this->lireNbPixels(); i++) 
		this->operator()(i) = lut[im(i)]; 
}

CImageClasse::CImageClasse(const CImageCouleur& im, int nbClusters, std::string choix, std::string espace, int plan) {

	this->m_iHauteur   = im.lireHauteur();
	this->m_iLargeur   = im.lireLargeur();
	this->m_sNom       = im.lireNom()+"ND_H";
	this->m_lNbRegions = 0;
	this->m_pucPixel   = new unsigned long[m_iHauteur*m_iLargeur];

	// contiendra la somme des elements de la classe
	// pour pouvoir calculer le centre de gravité !
	std::vector<unsigned long> sommeClasses;

	// contiendra le nombre des elements de la classe
	// pour pouvoir calculer le centre de gravité !
	std::vector<unsigned long> nbEltsClasses;

	// contiendra le centre de chaque classe
	std::vector<unsigned char> indexClasses;

	// contiendra la classe de chaque niveau de gris
	// imageFinale = lut[imageEntree]
	std::vector<int> lut;

	// allocation de k+1 classes
	// volonté d'affecter l'indice i à la classe i
	nbEltsClasses.resize(nbClusters+1); 
	for (int i=1;i<=nbClusters;i++)
		nbEltsClasses[i]=0;
	sommeClasses.resize(nbClusters+1);
	for (int i=1;i<=nbClusters;i++)
		sommeClasses[i]=0L;

	// extraction plan H pour optimisation basée histogramme 

	CImageCouleur hsv; 
	if (strcmp(espace.c_str(), "hsv") == 0)
		hsv = im.conversion("HSV");
	else
		hsv = im;

	// moyenne sur plan  et histo pour initialisation 

	std::vector<unsigned long> h;
	h.resize(256,0);
	for (int i=0;i<this->lireNbPixels();i++) 
		h[hsv(i)[plan]] += 1L;
	float mean=0;
	for (int i=0;i<256;i++) 
		mean += h[i]*i;
	mean /= this->lireNbPixels();

	// nb de niveaux de gris dans image initiale
	// si k supérieur, alors on diminue le nb de classes en sortie : nbClasses
	int nbLevels=0;
	for (int i=0;i<256;i++)
		if (h[i]) nbLevels++;
	int nbClasses;
	nbClasses=min(nbClusters,nbLevels);
	m_lNbRegions = nbClasses;

	if (strcmp(choix.c_str(),"aleatoire") == 0) { // tirage aléatoire des germes initiaux 

		indexClasses.resize(nbClasses+1);

		int num=1;
		while (num <= nbClasses) {
			int encours = rand()%256;
			bool dejaPris=false;
			for (int j=1;j<num;j++)
				if (indexClasses[j] == encours) dejaPris=true;
			if (dejaPris==false) {
				indexClasses[num] = encours;
				num++;
			}
		}
	}
	else if (strcmp(choix.c_str(),"regulier") == 0) { // intervalles réguliers entre germes 
		indexClasses.resize(nbClasses+1);
		// recherche du min et du max
		int min = 0, max = 255;
		while (h[min] == 0)
			min++;
		while (h[max] == 0)
			max--;

		int num=1;
		int encours = min;
		int pas = (max-min)/nbClasses;

		while (num <= nbClasses) {  
			indexClasses[num] = encours;
			encours += pas;
			num++;
		}
	}

	std::cout << "classification en " << nbClasses << " classes..." << std::endl;

	lut.resize(256);
	int nbTours = 0 ;
	bool change = true ;

	while ( (!nbTours) ||  ((change) && ( nbTours < MAX_ITER))) {
		// Remise à zéro à chaque tour 
		for (int i=1;i<=nbClasses;i++) {
			nbEltsClasses[i]=0;
			sommeClasses[i]=0L;
		}
		change = false ;

		// Pour chaque point trouver son plus proche
		for (int i =0; i < 256 ; i++)
		{    
		  unsigned char encours = i;
		  int plusproche = 1 ;
		  for (int j = 2; j <= nbClasses ; j++)
			{              
			  if (EUCLIDEAN(encours, indexClasses[j]) < EUCLIDEAN(encours, indexClasses[plusproche]))
				plusproche = j ;
			}        
		  lut[i] = plusproche ;
		  nbEltsClasses[plusproche] += h[encours] ;
		  sommeClasses[plusproche] += h[encours]*encours ;
		}

	  // On recalcule le centre de chaque classe
	  // a partir des nouveaux germes.
	  for (int j = 1 ; j <= nbClasses ; j++)
		{
		  if (nbEltsClasses[j])
			  sommeClasses[j] /= nbEltsClasses[j] ;
		  else
			  sommeClasses[j] = (unsigned long)mean;
		  if (EUCLIDEAN(indexClasses[j],sommeClasses[j]) > 0.001)  
			  change = true ;
		  indexClasses[j] = (unsigned char)sommeClasses[j] ;
		}
	  nbTours ++ ;
	  std::cout << "Iteration : " << nbTours <<    std::endl ;     
	}

	// on remplace chaque "niveau de Hue" par sa classe
	for (int i=0; i < this->lireNbPixels(); i++) 
		this->operator()(i) = lut[hsv(i)[plan]]; 
}

CImageClasse::~CImageClasse() {

	if (m_pucPixel) {
		delete[] m_pucPixel;
		m_pucPixel = NULL;
	}
}

void CImageClasse::sauvegarde(const std::string& file) {
	std::string nomFichier;
	if (file.empty())
		nomFichier = this->lireNom(); 
	else
		nomFichier = file;

	CImageNdg temp(this->lireHauteur(),this->lireLargeur());
	temp.choixPalette("binaire");

	for (int i=0;i<temp.lireNbPixels();i++)
		if (this->operator()(i) >= 256) // cast pour rentrer sur un unsigned char
			temp(i) = (unsigned char)(this->operator()(i)%255+1);
		else 
			temp(i) = (unsigned char)(this->operator()(i));
	temp.sauvegarde(file);
}

CImageClasse& CImageClasse::operator=(const CImageClasse& im) {

	if (&im == this)
		return *this;

	this->m_iHauteur   = im.lireHauteur();
	this->m_iLargeur   = im.lireLargeur();
	this->m_sNom       = im.lireNom();
	this->m_lNbRegions = im.lireNbRegions();
	this->m_pucPixel   = new unsigned long[m_iHauteur*m_iLargeur];

	if (this->m_pucPixel) 
		delete[] this->m_pucPixel;
	this->m_pucPixel = new unsigned long[im.lireHauteur() * im.lireLargeur()];

	if (im.m_pucPixel != NULL)
		memcpy(this->m_pucPixel,im.m_pucPixel,im.lireNbPixels()*sizeof(unsigned long));

	return *this;
}

// signatures pour Image_Ndg et Image_Couleur
std::vector<SIGNATURE_Ndg> CImageClasse::signatures(const CImageNdg& img, bool enregistrementCSV) {

	std::vector<SIGNATURE_Ndg> tab;

	if (this->lireNbRegions() > 0) {
		tab.resize(this->lireNbRegions()+1); // gestion du fond éventuel, cas des objets, vide si nuées dynamiques
		
		for (int k=0;k<(int)tab.size();k++) {
			tab[k].moyenne = 0;
			tab[k].surface = 0;
		}

		for (int i=0;i<this->lireHauteur();i++)
			for (int j=0;j<this->lireLargeur();j++) {
				tab[this->operator()(i,j)].moyenne += (float)img(i,j);
				tab[this->operator()(i,j)].surface += 1;
				}

		for (int k=0;k<(int)tab.size();k++) 
			if (tab[k].surface > 0) {
				tab[k].moyenne /= tab[k].surface;
			}

		if (enregistrementCSV) {
			std::string fichier = "../Res/" + this->lireNom() + "_SNdg.csv";
			std::ofstream f(fichier.c_str());

			if (!f.is_open())
				std::cout << "Impossible d'ouvrir le fichier en ecriture !" << std::endl;
			else {
				f << "Objet; Surface; Moyenne_Ndg " << std::endl;
				for (int k = 0; k<(int)tab.size(); k++)
					f << k << ";" << tab[k].surface << " ; " << tab[k].moyenne << std::endl;
			}
			f.close();
		}

		/*

		if (enregistrementCSV) {
			std::string fichier = "../Res/" + this->lireNom() + "_SForme.csv";
			std::ofstream f(fichier.c_str());

			if (!f.is_open())
				std::cout << "Impossible d'ouvrir le fichier en ecriture !" << std::endl;
			else {
				f << "Objet; CG_i; CG_j; Surface; codeFreeman ; RE_Hi ; RE_Hj ; RE_Bi ; RE_Bj ; perimetre;freemanHist[0];freemanHist[1];freemanHist[2];freemanHist[3];freemanHist[4];freemanHist[5];freemanHist[6];freemanHist[7]" << std::endl;
				for (int k = 0; k<(int)tab.size(); k++)
					f << k << ";" << tab[k].centreGravite_i << ";" << tab[k].centreGravite_j << ";" << tab[k].surface << ";" << tab[k].codeFreeman << " ; " << tab[k].rectEnglob_Hi << " ; " << tab[k].rectEnglob_Hj << " ; " << tab[k].rectEnglob_Bi << " ; " << tab[k].rectEnglob_Bj << " ; " << tab[k].perimetre << " ; " << tab[k].freemanHist[0] << " ; " << tab[k].freemanHist[1] << " ; " << tab[k].freemanHist[2] << " ; " << tab[k].freemanHist[3] << " ; " << tab[k].freemanHist[4] << " ; " << tab[k].freemanHist[5] << " ; " << tab[k].freemanHist[6] << " ; " << tab[k].freemanHist[7] << std::endl;
			}
		f.close();
		}
			*/
			
		
	}

	return tab;
}

std::vector<SIGNATURE_Couleur> CImageClasse::signatures(const CImageCouleur& img, bool enregistrementCSV) {

	std::vector<SIGNATURE_Couleur> tab;

	if (this->lireNbRegions() > 0) {
		tab.resize(this->lireNbRegions() + 1); // gestion de l'"objet" fond

		for (int k = 0; k<(int)tab.size(); k++) {
			tab[k].moyenne[0] = 0;
			tab[k].moyenne[1] = 0;
			tab[k].moyenne[2] = 0;
			tab[k].surface = 0;
		}

		for (int i = 0; i<this->lireHauteur(); i++)
			for (int j = 0; j<this->lireLargeur(); j++) {
				tab[this->operator()(i, j)].moyenne[0] += (float)img(i, j)[0];
				tab[this->operator()(i, j)].moyenne[1] += (float)img(i, j)[1];
				tab[this->operator()(i, j)].moyenne[2] += (float)img(i, j)[2];
				tab[this->operator()(i, j)].surface += 1;
			}

		for (int k = 0; k<(int)tab.size(); k++)
			if (tab[k].surface > 0) {
				tab[k].moyenne[0] /= tab[k].surface;
				tab[k].moyenne[1] /= tab[k].surface;
				tab[k].moyenne[2] /= tab[k].surface;
			}

		if (enregistrementCSV) {
			std::string fichier = "../Res/" + this->lireNom() + "_SCouleur.csv";
			std::ofstream f(fichier.c_str());

			if (!f.is_open())
				std::cout << "Impossible d'ouvrir le fichier en ecriture !" << std::endl;
			else {
				f << "Objet; Surface; Moyenne_0; Moyenne_1; Moyenne_2 " << std::endl;
				for (int k = 0; k<(int)tab.size(); k++)
					f << k << ";" << tab[k].surface << " ; " << tab[k].moyenne[0] << ";" << tab[k].moyenne[1] << " ; " << tab[k].moyenne[2] << std::endl;
			}
			f.close();
		}
	}

	return tab;
}

// propage couleur moyenne à chaque région, fond restant à 0 ou pas 
CImageCouleur CImageClasse::affichageMoyenne(const std::vector<SIGNATURE_Couleur>& tab, bool fondAPart) {
	CImageCouleur out(this->lireHauteur(),this->lireLargeur(),0,0,0);
	out.ecrireNom(this->lireNom() + "_Moyenne");

	if (!fondAPart) {
		for (int i = 0; i < this->lireNbPixels(); i++) {
			out(i)[0] = (int)tab[this->operator()(i)].moyenne[0];
			out(i)[1] = (int)tab[this->operator()(i)].moyenne[1];
			out(i)[2] = (int)tab[this->operator()(i)].moyenne[2];
		}
	}
	else {
		for (int i = 0; i < this->lireNbPixels(); i++)
			if (this->operator()(i))
			{
				out(i)[0] = (int)tab[this->operator()(i)].moyenne[0];
				out(i)[1] = (int)tab[this->operator()(i)].moyenne[1];
				out(i)[2] = (int)tab[this->operator()(i)].moyenne[2];
			}
	}

	return out;
}

CImageNdg CImageClasse::affichageMoyenne(const std::vector<SIGNATURE_Ndg>& tab, bool fondAPart) {
	CImageNdg out(this->lireHauteur(), this->lireLargeur(), 0);
	out.choixPalette("grise");
	out.ecrireNom(this->lireNom() + "_Moyenne");

	if (!fondAPart) {
		for (int i = 0; i < this->lireNbPixels(); i++) 
			out(i) = (int)tab[this->operator()(i)].moyenne;
			
	}
	else {
		for (int i = 0; i < this->lireNbPixels(); i++)
			if (this->operator()(i))
				out(i) = (int)tab[this->operator()(i)].moyenne;
	}

	return out;
}

// région par région autres -> 0
CImageClasse CImageClasse::selection(const std::string& methode, int classe) {


	CImageClasse out(this->lireHauteur(),this->lireLargeur());
	out.ecrireNom(this->lireNom());
	out.ecrireNbRegions(this->lireNbRegions());

	if (methode.compare("selection") == 0) {
		std::stringstream convert;
		convert << classe;
		out.ecrireNom(this->lireNom() + "_Sel_" + convert.str());

		for (int i=0;i<this->lireNbPixels();i++)
			out(i) = (this->operator()(i) == classe) ? classe : 0;
	}

	return out;
}

CImageNdg CImageClasse::selectionNdg(const std::string& methode, int classe) {

	CImageNdg out(this->lireHauteur(), this->lireLargeur());
	out.ecrireNom(this->lireNom());
	out.choixPalette("binaire");

	if (methode.compare("selection") == 0) {
		std::stringstream convert;
		convert << classe;
		out.ecrireNom(this->lireNom() + "_Sel_" + convert.str());

		for (int i = 0; i < this->lireNbPixels(); i++) {
			if (this->operator()(i) == classe) {
				out(i) = 1;
			}
			else {
				out(i) = 0;
			}
		}
	}
	return out;
}

// filtrage selon critères comme taille
CImageClasse CImageClasse::filtrage(const std::string& methode, int taille, bool miseAJour) {

	CImageClasse out(this->lireHauteur(), this->lireLargeur());
	out.ecrireNbRegions(this->lireNbRegions());

	if (methode.compare("taille_min") == 0) {
		std::stringstream convert;
		convert << taille;
		out.ecrireNom(this->lireNom() + "_Surf_" + convert.str());

		std::vector<unsigned long> surface;
		surface.resize(this->lireNbRegions() + 1, 0);

		for (int i = 0; i < this->lireNbPixels(); i++)
			surface[this->operator()(i)] += 1;

		std::vector<unsigned long> lut;
		lut.resize(this->lireNbRegions() + 1);

		for (int k = 0; k < (int)lut.size(); k++)
			lut[k] = (surface[k] >(unsigned long)taille) ? k : 0;

		if (!miseAJour) {
			for (int i = 0; i < this->lireNbPixels(); i++)
				out(i) = lut[this->operator()(i)];
		}
		else {
			std::vector<unsigned long> renumerotation;
			renumerotation.resize(out.lireNbRegions() + 1, 0);

			int valEnCours = 1;
			for (int k = 0; k < (int)lut.size(); k++)
				if (lut[k])
					renumerotation[k] = valEnCours++;
			out.ecrireNbRegions(valEnCours - 1);

			for (int i = 0; i < out.lireNbPixels(); i++)
				out(i) = renumerotation[this->operator()(i)];
		}
	}

	if (methode.compare("taille_max") == 0) {
		std::stringstream convert;
		convert << taille;
		out.ecrireNom(this->lireNom() + "_Surf_" + convert.str());

		std::vector<unsigned long> surface;
		surface.resize(this->lireNbRegions() + 1, 0);

		for (int i = 0; i < this->lireNbPixels(); i++)
			surface[this->operator()(i)] += 1;

		std::vector<unsigned long> lut;
		lut.resize(this->lireNbRegions() + 1);

		for (int k = 0; k < (int)lut.size(); k++)
			lut[k] = (surface[k] <(unsigned long)taille) ? k : 0;

		if (!miseAJour) {
			for (int i = 0; i < this->lireNbPixels(); i++)
				out(i) = lut[this->operator()(i)];
		}
		else {
			std::vector<unsigned long> renumerotation;
			renumerotation.resize(out.lireNbRegions() + 1, 0);

			int valEnCours = 1;
			for (int k = 0; k < (int)lut.size(); k++)
				if (lut[k])
					renumerotation[k] = valEnCours++;
			out.ecrireNbRegions(valEnCours - 1);

			for (int i = 0; i < out.lireNbPixels(); i++)
				out(i) = renumerotation[this->operator()(i)];
		}
	}
	return out;
}

// Conversion en image ndg
CImageNdg CImageClasse::toNdg(const std::string& methode) {

	CImageNdg out(this->lireHauteur(), this->lireLargeur());
	out.ecrireBinaire(false);
	out.choixPalette("grise");
	out.ecrireNom(this->lireNom() + "2NDG");

	if (methode.compare("defaut") == 0) {
		for (int i = 0; i < this->lireNbPixels(); i++)
			if (this->operator()(i) < 0)
				out(i) = 0;
			else
				if (this->operator()(i) > 256)
					out(i) = 255;
				else
					out(i) = (unsigned char)this->operator()(i);
	}

	return(out);
}

// signatures forme pour Image_Ndg et Image_Couleur
std::vector<SIGNATURE_Forme> CImageClasse::sigComposantesConnexes(bool enregistrementCSV) {

	std::vector<SIGNATURE_Forme> tab;

	if (this->lireNbRegions() > 0) {
		tab.resize(this->lireNbRegions() + 1); // gestion de l'"objet" fond

		for (int k = 0; k<(int)tab.size(); k++) {
			tab[k].centreGravite_i = 0;
			tab[k].centreGravite_j = 0;
			tab[k].premierPt_i = -1;
			tab[k].premierPt_j = -1;
			tab[k].codeFreeman = "";
			for (int i = 0; i < 8; i++)
				tab[k].histoFreeman[i] = 0;
			tab[k].rectEnglob_Bi = 0;
			tab[k].rectEnglob_Bj = 0;
			tab[k].rectEnglob_Hi = this->lireHauteur() - 1;
			tab[k].rectEnglob_Hj = this->lireLargeur() - 1;

			////////////////////////////////////// Used signatures
			// Utilities (non-robust signatures)
			tab[k].surface = 0;
			tab[k].perimetre = 0;
			tab[k].aireConvex = 0;
			tab[k].perimetreConvex = 0;
			tab[k].width = 0;
			tab[k].height = 0;
			tab[k].longueurFibre = 0;
			tab[k].axeMajeur_longueur = 0;
			tab[k].axeMineur_longueur = 0;
			tab[k].layer = 1;

			// OCR signatures (robust signatures)	
			tab[k].elongation = 1;
			tab[k].axeMajeur_angle = 1;
			tab[k].compacite = 1;
			tab[k].ellipticite = 1;
			tab[k].circulartite = 1;
			tab[k].convexite = 1;
			tab[k].ratioAspect = 1;
			tab[k].boucle = 1;
			tab[k].solidite = 1;
			tab[k].nombreEuler = 1;
		}

		for (int i = 0; i<this->lireHauteur(); i++)
			for (int j = 0; j<this->lireLargeur(); j++) {
				int top = 0, bot = this->lireLargeur(), left = this->lireHauteur(), right = 0;
				tab[this->operator()(i, j)].centreGravite_i += i;
				tab[this->operator()(i, j)].centreGravite_j += j;
				tab[this->operator()(i, j)].surface += 1;
				if (tab[this->operator()(i, j)].premierPt_i == -1)
					tab[this->operator()(i, j)].premierPt_i = i;
				if (tab[this->operator()(i, j)].premierPt_j == -1)
					tab[this->operator()(i, j)].premierPt_j = j;
				tab[this->operator()(i, j)].rectEnglob_Bi = max(tab[this->operator()(i, j)].rectEnglob_Bi, i);
				tab[this->operator()(i, j)].rectEnglob_Bj = max(tab[this->operator()(i, j)].rectEnglob_Bj, j);
				tab[this->operator()(i, j)].rectEnglob_Hi = min(tab[this->operator()(i, j)].rectEnglob_Hi, i);
				tab[this->operator()(i, j)].rectEnglob_Hj = min(tab[this->operator()(i, j)].rectEnglob_Hj, j);
			}


		for (int k = 0; k<(int)tab.size(); k++)
			if (tab[k].surface > 0) {
				tab[k].centreGravite_i /= tab[k].surface;
				tab[k].centreGravite_j /= tab[k].surface;
			}


		CImageClasse agrandie(this->lireHauteur() + 2, this->lireLargeur() + 2); // bords à 0 pour bonne gestion des contours des objets
		agrandie.ecrireNbRegions(this->lireNbRegions());
		CImageClasse copie(this->lireHauteur(), this->lireLargeur());
		copie.ecrireNbRegions(this->lireNbRegions());

		// presence objets/bord ou ND
		std::vector<unsigned long> surface;
		surface.resize(this->lireNbRegions() + 1, 0);

		for (int i = 0; i<this->lireNbPixels(); i++)
			surface[this->operator()(i)] += 1;

		// gestion du coeur
		for (int i = 0; i<this->lireHauteur(); i++)
			for (int j = 0; j<this->lireLargeur(); j++) {
				agrandie(i + 1, j + 1) = this->operator()(i, j);
				copie(i, j) = this->operator()(i, j);
			}

		if (surface[0] > 0) { // cas objets/fond  
			for (int i = 1; i<agrandie.lireHauteur() - 1; i++)
				for (int j = 1; j<agrandie.lireLargeur() - 1; j++)
					if (this->operator()(i - 1, j - 1) != 0) {
						int minH = min(agrandie(i, j - 1), agrandie(i, j + 1));
						int minV = min(agrandie(i - 1, j), agrandie(i + 1, j));
						int minV4 = min(minH, minV);
						copie(i - 1, j - 1) = minV4;
					}

			for (int pix = 0; pix<lireNbPixels(); pix++)
				copie(pix) = this->operator()(pix) - copie(pix);
		}
		else { // cas ND
			for (int i = 1; i<agrandie.lireHauteur() - 1; i++)
				for (int j = 1; j<agrandie.lireLargeur() - 1; j++) {
					int minH = min(agrandie(i, j - 1), agrandie(i, j + 1));
					int maxH = max(agrandie(i, j - 1), agrandie(i, j + 1));
					int minV = min(agrandie(i - 1, j), agrandie(i + 1, j));
					int maxV = max(agrandie(i - 1, j), agrandie(i + 1, j));
					int minV4 = min(minH, minV);
					int maxV4 = max(maxH, maxV);
					if (!((agrandie(i, j) == minV4) && (agrandie(i, j) == maxV4))) // pixel différent de ses voisins
						copie(i - 1, j - 1) = 0;
				}

			for (int pix = 0; pix<lireNbPixels(); pix++)
				copie(pix) = this->operator()(pix) - copie(pix);
		}
		// 	copie -> image des bords intérieurs des objets

		// parcours dans sens jusqu'à rejoindre point initial ou "stabilisation" au même point
		for (int num = 1; num <= this->lireNbRegions(); num++) {
			int pIniti = tab[num].premierPt_i;
			int pInitj = tab[num].premierPt_j;

			int pi = pIniti;
			int pj = pInitj;

			bool retour = false;

			while (!retour) {
				int pi_encours = pi;
				int pj_encours = pj;

				if ((pj != copie.lireLargeur() - 1) && (copie(pi, pj + 1) == num)) {
					tab[num].codeFreeman += "0";
					copie(pi, pj + 1) = 0;
					pj = pj + 1;
					tab[num].histoFreeman[0] += 1.;
				}
				else
					if ((pi != copie.lireHauteur() - 1) && (pj != copie.lireLargeur() - 1) && (copie(pi + 1, pj + 1) == num)) {
						tab[num].codeFreeman += "7";
						copie(pi + 1, pj + 1) = 0;
						pi = pi + 1;
						pj = pj + 1;
						tab[num].histoFreeman[7] += 1.;
					}
					else
						if ((pi != copie.lireHauteur() - 1) && (copie(pi + 1, pj) == num)) {
							tab[num].codeFreeman += "6";
							copie(pi + 1, pj) = 0;
							pi = pi + 1;
							tab[num].histoFreeman[6] += 1.;
						}
						else
							if ((pi != copie.lireHauteur() - 1) && (pj != 0) && (copie(pi + 1, pj - 1) == num)) {
								tab[num].codeFreeman += "5";
								copie(pi + 1, pj - 1) = 0;
								pi = pi + 1;
								pj = pj - 1;
								tab[num].histoFreeman[5] += 1.;
							}
							else
								if ((pj != 0) && (copie(pi, pj - 1) == num)) {
									tab[num].codeFreeman += "4";
									copie(pi, pj - 1) = 0;
									pj = pj - 1;
									tab[num].histoFreeman[4] += 1.;
								}
								else
									if ((pi != 0) && (pj != 0) && (copie(pi - 1, pj - 1) == num)) {
										tab[num].codeFreeman += "3";
										copie(pi - 1, pj - 1) = 0;
										pi = pi - 1;
										pj = pj - 1;
										tab[num].histoFreeman[3] += 1.;
									}
									else
										if ((pi != 0) && (copie(pi - 1, pj) == num)) {
											tab[num].codeFreeman += "2";
											copie(pi - 1, pj) = 0;
											pi = pi - 1;
											tab[num].histoFreeman[2] += 1.;
										}
										else
											if ((pi != 0) && (pj != copie.lireLargeur() - 1) && (copie(pi - 1, pj + 1) == num)) {
												tab[num].codeFreeman += "1";
												copie(pi - 1, pj + 1) = 0;
												pi = pi - 1;
												pj = pj + 1;
												tab[num].histoFreeman[1] += 1.;
											}

				if ((((pi == pIniti) && (pj == pInitj))) || ((pi == pi_encours) && (pj == pj_encours)))
					retour = true;
			}
		}

		for (int num = 1; num <= this->lireNbRegions(); num++)
			for (int bin = 0; bin < 8; bin++)
				tab[num].histoFreeman[bin] /= (float)tab[num].codeFreeman.length();

		// calcul du périmètre
		for (int num = 1; num <= this->lireNbRegions(); num++) {
			for (int code = 0; code<(int)tab[num].codeFreeman.size(); code++) {
				if ((tab[num].codeFreeman[code] == '0') || (tab[num].codeFreeman[code] == '2') || (tab[num].codeFreeman[code] == '4') || (tab[num].codeFreeman[code] == '6'))
					tab[num].perimetre += 1;
				else
					tab[num].perimetre += (float)sqrt(2);
			}

			// Droites Majeur et Mineur
			double somme_x, somme_y, somme_x2, somme_y2, somme_xy, n;
			int nb = 0;
			somme_x = 0.;
			somme_y = 0.;
			somme_x2 = 0.;
			somme_y2 = 0.;
			somme_xy = 0.;
			n = 0;
			for (int i = tab[num].rectEnglob_Hj; i<tab[num].rectEnglob_Bj; i++)
				for (int j = tab[num].rectEnglob_Hi; j < tab[num].rectEnglob_Bi; j++) {
					somme_x += i;
					somme_y += j;
					somme_x2 += i * i;
					somme_y2 += j * j;
					somme_xy += i * j;
					n += 1;
				}
			somme_x /= n;
			somme_y /= n;
			somme_x2 /= n;
			somme_y2 /= n;
			somme_xy /= n;

			//ACP
			double COVxx = somme_x2 - somme_x*somme_x;
			double COVyy = somme_y2 - somme_y*somme_y;
			double COVxy = somme_xy - somme_x*somme_y; // Cov(X,Y) = E(XY) - E(X)E(Y)

													   // covariance matrix is 
													   // | a b | = | COVxx COVxy |
													   // | c d |   | COVxy COVyy |
			double a = COVxx, b = COVxy, c = COVxy, d = COVyy;

			double T = a + d;
			double Det = a*d - b*c;

			// eingenvalues L1,L2 = roots of characteristic polynomial
			double sqrtt = std::sqrt(T*T - 4 * Det);
			double L1 = 0.5 * (T + sqrtt);
			double L2 = 0.5 * (T - sqrtt);

			// eigenvectors => A.v = L.v, then: 
			// (a-L).vx +     b.vy = 0
			//     c.vx + (d-L).vy = 0
			double V1x, V1y, V1n, V2x, V2y, V2n, EPSILON = 1E-8;
			if (std::abs(b) < EPSILON && std::abs(c) < EPSILON) {
				V1x = 1.0; V1y = 0.0;
				V2x = 0.0; V2y = 1.0;
			}
			else {
				V1x = b; V1y = L1 - a;
				V2x = b; V2y = L2 - a;
			}

			// normalize eigenvectors
			V1n = std::sqrt(V1x*V1x + V1y*V1y);
			if (V1n > EPSILON) { V1x /= V1n; V1y /= V1n; }

			V2n = std::sqrt(V2x*V2x + V2y*V2y);
			if (V2n > EPSILON) { V2x /= V2n; V2y /= V2n; }

			//L'axe principal est la droite définie par le point (Ex,Ey) et le vecteur (V1x,V1y).
			//L'axe secondaire est la droite définie par le point (Ex,Ey) et le vecteur (V2x,V2y). 

			// width
			tab[num].width = tab[num].rectEnglob_Bj - tab[num].rectEnglob_Hj;
			// height
			tab[num].height = tab[num].rectEnglob_Bi - tab[num].rectEnglob_Hi;
			// Aire convex
			tab[num].aireConvex = tab[num].width * tab[num].height / tab[num].surface;
			// Perimetre convex
			tab[num].perimetreConvex = 2 * tab[num].width + 2 * tab[num].height / tab[num].perimetre;
			// Longueur fibre
			tab[num].longueurFibre = (tab[num].perimetre - sqrt((tab[num].perimetre *tab[num].perimetre) - (16 * tab[num].surface))) / 4;
			// Longueur Axe majeur
			tab[num].axeMajeur_longueur = L1;
			// LongueurAxe mineur
			tab[num].axeMineur_longueur = L2;

			// Elongation
			if (tab[num].width > tab[num].height)
				tab[num].elongation = tab[num].height / tab[num].width;
			else
				tab[num].elongation = tab[num].width / tab[num].height;

			// Compacité
			tab[num].compacite = (tab[num].perimetre * tab[num].perimetre) / (4 * PI*tab[num].surface);
			// Ellipticité
			tab[num].ellipticite = tab[num].axeMineur_longueur / tab[num].axeMajeur_longueur;
			// Circularité
			tab[num].circulartite = 4 * PI * tab[num].surface / tab[num].perimetre;
			// Convexité
			tab[num].convexite = tab[num].perimetreConvex / tab[num].perimetre;
			// Ratio d'aspect
			tab[num].ratioAspect = tab[num].height / tab[num].width;
			// Boucle
			tab[num].boucle = tab[num].axeMajeur_longueur / tab[num].longueurFibre;
			// Solidité
			tab[num].solidite = tab[num].surface / tab[num].aireConvex;

		}

		int cornerHeight = 0, objectHeight = 0, prevBotCorner = tab[1].rectEnglob_Bi;
		int nbLayer = 1;
		tab[1].layer = 1;
		int layerIndex[5] = { 0, 1, 0, 0, 0 };
		for (int num = 2; num <= this->lireNbRegions(); num++)
		{
			if (prevBotCorner < tab[num].rectEnglob_Hi)
			{
				nbLayer++;
				layerIndex[nbLayer] = num;
			}
			tab[num].layer = nbLayer;
			prevBotCorner = tab[num].rectEnglob_Bi;
		}

		// Enregistrement 1 : all Utilities signatures (+ Base ones)
		if (enregistrementCSV) {
			std::string fichier = "../Res/" + this->lireNom() + "_SForme.csv";
			std::ofstream f(fichier.c_str());

			if (!f.is_open())
				std::cout << "Impossible d'ouvrir le fichier en ecriture !" << std::endl;
			else {
				f << "Objet; CG_i; CG_j; Surface; codeFreeman ; hFreeman[0] ; hFreeman[1] ; hFreeman[2] ; hFreeman[3] ; hFreeman[4] ; hFreeman[5] ; hFreeman[6] ; hFreeman[7] ; RE_Hi ; RE_Hj ; RE_Bi ; RE_Bj ; perimetre ; width ; Height ; aire Convex ; perimetre Convex ; longueur fibre ; longueur axe majeur ; longueur axe mineur; Layer" << std::endl;
				for (int k = 0; k < (int)tab.size(); k++)
					f << k << ";" << tab[k].centreGravite_i << ";" << tab[k].centreGravite_j << ";" << tab[k].surface << ";" << tab[k].codeFreeman << " ; " << tab[k].histoFreeman[0] << " ; " << tab[k].histoFreeman[1] << " ; " << tab[k].histoFreeman[2] << " ; " << tab[k].histoFreeman[3] << " ; " << tab[k].histoFreeman[4] << " ; " << tab[k].histoFreeman[5] << " ; " << tab[k].histoFreeman[6] << " ; " << tab[k].histoFreeman[7] << " ; " << tab[k].rectEnglob_Hi << "; " << tab[k].rectEnglob_Hj << "; " << tab[k].rectEnglob_Bi << "; " << tab[k].rectEnglob_Bj << "; " << tab[k].perimetre << "; " << tab[k].width << "; " << tab[k].height << "; " << tab[k].aireConvex << "; " << tab[k].perimetreConvex << ";" << tab[k].longueurFibre << "; " << tab[k].axeMajeur_longueur << "; " << tab[k].axeMineur_longueur << ";" << tab[k].layer << std::endl;
			}
			f.close();
		}

		// Enregistrement 2 : OCR signatures
		if (enregistrementCSV) {
			std::string fichier = "../Res/" + this->lireNom() + "_SOCR.csv";
			std::ofstream f(fichier.c_str());

			if (!f.is_open())
				std::cout << "Impossible d'ouvrir le fichier en ecriture !" << std::endl;
			else {
				f << "Objet; elongation; compacité; ellipticité; circularité; Convexité; Ratio d'aspect; boucle; solidité " << std::endl;
				for (int k = 0; k < (int)tab.size(); k++)
					f << k << ";" << tab[k].elongation << ";" << tab[k].compacite << ";" << tab[k].ellipticite << " ; " << tab[k].circulartite << " ; " << tab[k].convexite << " ; " << tab[k].ratioAspect << " ; " << tab[k].boucle << " ; " << tab[k].solidite << std::endl;
			}
			f.close();
		}
	}
	return tab;

}
