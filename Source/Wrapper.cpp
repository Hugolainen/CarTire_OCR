#include "Wrapper.h"

/*
* Classe dédiée au passage C++ -> C#
*/

// Constructeur par defaut
Wrapper::Wrapper()
{
	this->_LLD_OCR = new LLD_OCR();
	this->_IHMpicture = NULL;
	this->_IHMnbData = 0;
	this->_IHMdata.clear();
}

// Destructeur par defaut
Wrapper::~Wrapper()
{
	this->Wrapper_CleanImgIHM();
	this->_LLD_OCR = NULL;
	this->_IHMpicture = NULL;
}

// Enleve d'image de l'IHM présente dans le wrapper C#/C++
void Wrapper::Wrapper_CleanImgIHM() {
	if (this->_IHMpicture != NULL) {
		(*this->_IHMpicture).~CImageCouleur();
		this->_IHMpicture = NULL;
		this->_IHMdata.clear();
	}
}

// Met à jour l'image de l'IHM présente dans le wrapper C#/C++
void Wrapper::Wrapper_ChangeImgIHM(int nbChamps, byte* data, int stride, int nbLig, int nbCol) {
	this->Wrapper_CleanImgIHM();
	this->_IHMnbData = nbChamps;
	this->_IHMdata.resize(nbChamps);
	this->_IHMpicture = new CImageCouleur(nbLig, nbCol);
	byte* pixPtr = (byte*)data;

	for (int y = 0; y < nbLig; y++)			// Remplissage des pixels
	{
		for (int x = 0; x < nbCol; x++)
		{
			this->_IHMpicture->operator()(y, x)[0] = pixPtr[3 * x + 2];
			this->_IHMpicture->operator()(y, x)[1] = pixPtr[3 * x + 1];
			this->_IHMpicture->operator()(y, x)[2] = pixPtr[3 * x];
		}
		pixPtr += stride;				  // Largeur une seule ligne, gestion multiple 32 bits
	}
}

// Analyse de l'image et retourne le DOT
std::string Wrapper::Wrapper_getDOT(int nbChamps, byte* data, int stride, int nbLig, int nbCol, int *presets) {
	std::string dot = "Error Wrapper.cpp";
	
	this->Wrapper_ChangeImgIHM(nbChamps, data, stride, nbLig, nbCol);	// Update de l'image présente dans l'IHM
	
	CImageNdg imgForLLD(nbLig, nbCol);
	imgForLLD = this->_IHMpicture->plan(3, 0.33, 0.33, 0.33);			// Transformation image couleure de l'IHM en Ndg (plan luminance 33% chacun)
	dot = this->_LLD_OCR->LLD_AnalyzePicture(imgForLLD, presets);			// Analyse de la feuille
	
	return dot;
}