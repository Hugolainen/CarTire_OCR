#ifndef _WRAPPER_H_
#define _WRAPPER_H_

#pragma once

/*
* @file : Wrapper.h
* @brief : Ce fichier définie les méthodes utilisées pour le Wrapper C++ -> C#
*/

#include "ImageClasse.h"
#include "ImageNdg.h"
#include "ImageCouleur.h"
#include "ImageDouble.h"
#include "LLD_OCR.h"

#include <windows.h>
#include <cstring>

class Wrapper
{

	///////////////////////////////////////
	private:
	///////////////////////////////////////

	/*
	* @brief : Classe OCR du Low Level Driver (Traitement de l'image)
	*/
	LLD_OCR						*_LLD_OCR;		

	/*
	* @brief : Image présente dans la fenetre de l'IHM
	*/
	CImageCouleur				*_IHMpicture;	

	/*
	* @brief : Nombre de champs de l'IHM (gestion de l'image)
	*/
	int							 _IHMnbData;	

	/*
	* @brief : Champs de l'IHM (gestion de l'image)
	*/
	std::vector<double>			 _IHMdata;											

	/*
	* @brief : Enleve d'image de l'IHM présente dans le wrapper C#/C++
	*/
	void Wrapper_CleanImgIHM();		

	/*
	* @brief : Met à jour l'image de l'IHM présente dans le wrapper C#/C++
	*/
	void Wrapper_ChangeImgIHM(int nbChamps, byte* data, int stride, int nbLig, int nbCol);		


	///////////////////////////////////////
	public:
	///////////////////////////////////////

	/*
	* @brief : Constructeur par défaut du Wrapper
	*/
	_declspec(dllexport) Wrapper();	

	/*
	* @brief : Destructeur par défaut
	*/
	_declspec(dllexport) ~Wrapper();				

	/*
	* @brief : Cette fonction accède au LLD afin de réaliser l'OCR sur l'image d'éntrée
	* @param : (int) nbChamps : Nombre de champs dans l'image
	* @param : (byte) data : Donnée pour chaque pixel
	* @param : (int) stride : Largeur d'une ligne, gestion multiple 32 bits
	* @param : (int) nbLig : Nombre de lignes de l'image
	* @param : (int) nbCol : Nombre de colonnes de l'image
	* @param : (int*) presets : Tableau de presets contenant les paramètres du traitement
	*/
	_declspec(dllexport) std::string Wrapper_getDOT(int nbChamps, byte* data, int stride, int nbLig, int nbCol, int *presets);	// Analyse de l'image et retourne le DOT
};


extern "C" _declspec(dllexport) void LLD_getDOT(Wrapper *wrapper, char* dot, int nbChamps, byte* data, int stride, int nbLig, int nbCol, int* presets)
{
	std::string dot_temp = wrapper->Wrapper_getDOT(nbChamps, data, stride, nbLig, nbCol, presets);
	strcpy(dot, dot_temp.c_str());
}

extern "C" _declspec(dllexport) Wrapper *LLD_init()
{
	Wrapper *pLLD_wrapper = new Wrapper();
	return pLLD_wrapper;
}

#endif