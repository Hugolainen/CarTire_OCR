#pragma once

#ifndef _LLD_OCR_H_
#define _LLD_OCR_H_

/*
* @file : LLD_OCR.h
* @brief : Ce fichier définie les méthodes de traitement et d'OCR.
*/

#include "ImageNdg.h"
#include "ImageCouleur.h"
#include "ImageDouble.h"
#include "ImageClasse.h"

/*
* @brief : Active le mode demo
*/

#define MODE_DEMO							1

/*
* @brief : Nombre de presets (15 par défaut)
*/
#define NUMBER_OF_PRESETS					15

/*
* @brief : Activation du mode demo
*/
#define PRESET_ACTIVATE_DEMO				0

/*
* @brief : Taille du 1er element du DOT
*/
#define PRESET_DOT_SIZE_ELEMENT_1			1

/*
* @brief : Taille du 2nd element du DOT
*/
#define PRESET_DOT_SIZE_ELEMENT_2			2

/*
* @brief : Taille du 3eme element du DOT
*/
#define PRESET_DOT_SIZE_ELEMENT_3			3

/*
* @brief : Taille du 4eme element du DOT
*/
#define PRESET_DOT_SIZE_ELEMENT_4			4

/*
* @brief : Position de la ROI en I (position dans tableau de preset)
*/
#define PRESET_ROI_I						5

/*
* @brief : Position de la ROI en J (position dans tableau de preset)
*/
#define PRESET_ROI_J						6

/*
* @brief : Hauteur de la ROI (position dans tableau de preset)
*/
#define PRESET_ROI_W						7

/*
* @brief : Largeur de la ROI (position dans tableau de preset)
*/
#define PRESET_ROI_H						8

/*
* @brief : Filtrage paramètre 1 (position dans tableau de preset)
*/
#define PRESET_FILTER_PAR_1					9

/*
* @brief : Filtrage paramètre 2 (position dans tableau de preset)
*/
#define PRESET_FILTER_PAR_2					10

/*
* @brief : Egalisation paramètre 1 (position dans tableau de preset)
*/
#define PRESET_EGAL_PAR_1					11

/*
* @brief : Egalisation paramètre 2 (position dans tableau de preset)
*/
#define PRESET_EGAL_PAR_2					12

/*
* @brief : Filtrage taille min (position dans tableau de preset)
*/
#define PRESET_SIZE_MIN 					13

/*
* @brief : Filtrage taille max (position dans tableau de preset)
*/
#define PRESET_SIZE_MAX 					14

/*
* @brief : Structure contenant le caractere de l'OCR, ainsi que sa probabilité
*/
struct OCR_alphabet {
	/*
	* @brief : Caractère actuel de l'alphabet
	*/
	char letter;

	/*
	* @brief : Probabilité d'appartenance
	*/
	int probability;
};


class LLD_OCR {

	/*************************************/
public:
	/**************************************/

	/*
	* @brief Constructeur par défaut. Cette function charge la classe LLD_OCR
	*/
	_declspec(dllexport) LLD_OCR();

	/*
	* @brief Destructeur par défaut. Cette function détruit la classe LLD_OCR
	*/
	_declspec(dllexport) ~LLD_OCR();

	/*
	* @brief Analyse l'image actuelle pour retourner le DOT
	* @params (CImageNdg) image : image à traiter
	* @params (int*) presets : tableau de presets pour le pneu actuel (ex : Michelin, Dunlop,...)
	* @return (string) Chaine contenant le DOT
	*/
	_declspec(dllexport) std::string LLD_AnalyzePicture(CImageNdg image, int *presets);



	/*************************************/
private:
	/**************************************/

	/*
	* @brief Traitement bas-niveau de l'image. Cette fonction contient l'ensemble des méthodes utilisées pour traiter l'image,\n
	* et faire apparaitre les caractères.
	* @params (CImageNdg) image : image à traiter
	* @params (int*) presets : tableau de presets pour le pneu actuel (ex : Michelin, Dunlop,...)
	* @return (CImageNdg) Image Ndg traitée
	*/

	_declspec(dllexport) CImageNdg LLD_ProcessPicture(CImageNdg image, int *presets);


	/*
	* @brief : Interpretation de l'ensemble des signatures pour trouver les caractères présents.
	* @params : (SIGNATURE_Forme) sigOCR : Signatures liées à le forme et robustes à l'échelle de l'image à traiter
	* @return : (string) Chaine de caractère correspondant au DOT
	*/
	_declspec(dllexport) std::string LLD_OCR_reading(std::vector <SIGNATURE_Forme> sigOCR);

};


#endif