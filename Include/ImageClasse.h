// ajouter _declspec(dllexport) devant tout public pour permettre à la dll d'exporter ces méthodes 
// pour qu'elles puissent être utilisées par d'autres applications ou programmes

#pragma once

#ifndef _IMAGE_CLASSE_
#define _IMAGE_CLASSE_

#include <string>
#include <vector>

#include "ImageNdg.h"
#include "ImageCouleur.h"

#define EUCLIDEAN(a,b) ((a-b) * (a-b)) 
#define MAX_ITER 100

typedef struct  {
	float  moyenne;
	int    surface;
} SIGNATURE_Ndg;

typedef struct  {
	float  moyenne[3];
	int    surface;
} SIGNATURE_Couleur;

typedef struct {
	float		centreGravite_i;
	float		centreGravite_j;
	std::string codeFreeman; // au sens V8
	float		histoFreeman[8]; // histogramme normalisé du code de Freeman
	int			premierPt_i; // premier point rencontré sens de parcours avant
	int			premierPt_j;
	int			rectEnglob_Hi; // coins supérieur gauche et inférieur droit
	int			rectEnglob_Hj;
	int			rectEnglob_Bi;
	int			rectEnglob_Bj;

	/////////////////// Used signatures
	// Utilities (non-robust signatures)
	float		perimetre; // au sens V8
	int			surface;
	float		aireConvex; // Width * Height / Surface
	float       perimetreConvex; // 2*Width + 2*Height / Perimetre
	float		width;
	float		height;
	float		longueurFibre; // (perimetre-sqrt(perimetre²-16*area))/4
	float		axeMajeur_longueur; // distance max entre points (x1,y1) et (x2,y2) -> sqrt[(x2-x1)²+(y2-y1)²]
	float		axeMineur_longueur; // distance entre points extreme de la perpendiculaire à axe majeur (x1,y1) et (x2,y2) -> sqrt[(x2-x1)²+(y2-y1)²]
	float		dist_euclid; //d=sqrt(width² + height²)
	float		layer; // Couche sur laquel se trouve le charactere

					   // OCR signatures (robust signatures)
	float		elongation; // width/lenght
	float		axeMajeur_angle; // tan-1[(y2-y1)/(x2-x1)]
	float		compacite; // permietre²/(4*Pi*aire)
	float		ellipticite; // short axis lenght / long axe lenght
	float		circulartite; //4*Pi*aire*/perimetre convex²
	float		convexite; //perimetre convex/perimetre
	float		ratioAspect; //height/width
	float		boucle; //longueur / longueur de fibre
	float		solidite; //aire/aire convex
	float		nombreEuler; //nombre de trous dans l'objet
} SIGNATURE_Forme;

// définition classe Image Classe --> images étiquetées pour analyse objets, nuées dynamiques pour analyse régions

class CImageClasse {

	///////////////////////////////////////
	private : 
	///////////////////////////////////////

		int              m_iHauteur;
		int              m_iLargeur;
		std::string      m_sNom;
		long			 m_lNbRegions;

		unsigned long*   m_pucPixel;

	///////////////////////////////////////
	public : 
	///////////////////////////////////////
		
		// constructeurs
		_declspec(dllexport) CImageClasse(); 
		_declspec(dllexport) CImageClasse(int hauteur, int largeur); 
		_declspec(dllexport) CImageClasse(const CImageNdg& im, std::string choix = "V8"); // objets
		_declspec(dllexport) CImageClasse(const CImageClasse& in, std::string misAJour = "sans", std::string voisinage = "V8"); // re-étiquetage éventuel
		_declspec(dllexport) CImageClasse(const CImageNdg& im, int nbClusters = 2, std::string choix = "aleatoire"); // clustering 
		_declspec(dllexport) CImageClasse(const CImageCouleur& im, int nbClusters = 2, std::string choix = "aleatoire", std::string espace = "hsv", int plan = 0);

		_declspec(dllexport) ~CImageClasse(); // destructeur

		// sauvegarde au format bmp
		// attention : cast des informations car pertes potentielles
		_declspec(dllexport) void sauvegarde(const std::string& file); // sauvegarde data au format BMP avec cast des long en char

		// pouvoir accéder à un pixel par image(i)
		_declspec(dllexport) unsigned long& operator() (int i) const { 
		return m_pucPixel[i];
		}

		// pouvoir accéder à un pixel par image(i,j)
		_declspec(dllexport) unsigned long& operator() (int i, int j) const { 
		return m_pucPixel[i*m_iLargeur+j];
		}

		// opérateur copie image par imOut = imIn
		_declspec(dllexport) CImageClasse& operator=(const CImageClasse& im);

		// get et set 

		_declspec(dllexport) int lireHauteur() const { 
		return m_iHauteur;
		}

		_declspec(dllexport) int lireLargeur() const {
		return m_iLargeur;
		}

		_declspec(dllexport) int lireNbRegions() const {
		return m_lNbRegions;
		}

		_declspec(dllexport) std::string lireNom() const {
		return m_sNom;
		}

		_declspec(dllexport) int lireNbPixels() const {
		return m_iHauteur*m_iLargeur;
		}

		_declspec(dllexport) void ecrireHauteur(int hauteur) {
		m_iHauteur = hauteur;
		}

		_declspec(dllexport) void ecrireLargeur(int largeur) {
		m_iLargeur = largeur;
		}

		_declspec(dllexport) void ecrireNom(std::string nom) {
		m_sNom = nom;
		}

		_declspec(dllexport) void ecrireNbRegions(int nb) {
		m_lNbRegions = nb;
		}

		// signatures pour Image_Ndg et Image_Couleur
		_declspec(dllexport) std::vector<SIGNATURE_Ndg> signatures(const CImageNdg& img, bool enregistrementCSV = false);
		_declspec(dllexport) std::vector<SIGNATURE_Couleur> signatures(const CImageCouleur& img, bool enregistrementCSV = false);

		// affichage chaque région -> sa valeur moyenne
		_declspec(dllexport) CImageCouleur affichageMoyenne(const std::vector<SIGNATURE_Couleur>& tab, bool fondAPart = true);
		_declspec(dllexport) CImageNdg affichageMoyenne(const std::vector<SIGNATURE_Ndg>& tab, bool fondAPart = true);

		// sélection région par région
		_declspec(dllexport) CImageClasse selection(const std::string& methode = "selection", int classe=0);
		_declspec(dllexport) CImageNdg selectionNdg(const std::string& methode, int classe);

		// filtrage selon critères taille, bords, etc
		_declspec(dllexport) CImageClasse filtrage(const std::string& methode = "taille", int taille=50, bool miseAJour = false);

		// Conversion en ndg
		_declspec(dllexport) CImageNdg toNdg(const std::string& methode);

		// signatures forme pour Image_Ndg et Image_Couleur
		_declspec(dllexport) std::vector<SIGNATURE_Forme> sigComposantesConnexes(bool enregistrementCSV = false);

		// morphologie
		_declspec(dllexport) CImageClasse CImageClasse::morphologie(const std::string& methode = "erosion", const std::string& eltStructurant = "V8");
		_declspec(dllexport) CImageClasse CImageClasse::pavage();

		// masquage
		_declspec(dllexport) CImageCouleur masquage(const CImageNdg& img, int valR = 255, int valG = 0, int valB = 0);

		_declspec(dllexport) CImageCouleur covar(const CImageCouleur& imgInit);


};

#endif _IMAGE_CLASSE_