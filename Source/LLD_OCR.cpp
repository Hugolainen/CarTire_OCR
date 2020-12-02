
#include "LLD_OCR.h"

/*
* @file : LLD_OCR.cpp
* @brief : Ce fichier contient les méthodes et traitement de l'image,\n
* ainsi que la reconnaissance de caractère par OCR.
*/

///////////////////////////////////////
// Methodes publiques
///////////////////////////////////////

// Constructeur par defaut
LLD_OCR::LLD_OCR() {

}

// Destructeur par defaut
LLD_OCR::~LLD_OCR() {

}

// Analyse de l'image pour retourner le DOT
std::string LLD_OCR::LLD_AnalyzePicture(CImageNdg image, int *presets) {

	std::string DOT = "Error LLD_OCR.cpp";

	CImageNdg image_proceeded;
	CImageClasse image_etiqued;


	// Mode démo désactivé, images à traiter
	if (presets[PRESET_ACTIVATE_DEMO] != MODE_DEMO)
	{
		image_proceeded = LLD_ProcessPicture(image, presets);
		CImageClasse imgClasse(image_proceeded, "V4");
		image_etiqued = imgClasse
			.filtrage("taille_min", presets[PRESET_SIZE_MIN], true) // Fil_size_min
			.filtrage("taille_max", presets[PRESET_SIZE_MAX], true); // Fil_size_max

		image_etiqued.sauvegarde("img_OCR");
		std::vector < SIGNATURE_Forme >sigForme = image_etiqued.sigComposantesConnexes(true);
		DOT = LLD_OCR_reading(sigForme);
	}
	else
	{
		image_proceeded = image.transformation("complement");
		CImageNdg img_seuil = image_proceeded.seuillage();
		CImageClasse image_etiqued(img_seuil, "V8");

		image_etiqued.sauvegarde("img_OCR");
		std::vector < SIGNATURE_Forme >sigForme = image_etiqued.sigComposantesConnexes(true);
		DOT = LLD_OCR_reading(sigForme);

	}


	return DOT;
}

///////////////////////////////////////
// Methodes privées
///////////////////////////////////////

CImageNdg LLD_OCR::LLD_ProcessPicture(CImageNdg image, int *presets) {

	CImageNdg img_seuil = image
		.filtrage("moyen", presets[PRESET_FILTER_PAR_1], presets[PRESET_FILTER_PAR_2]) // Fil_param_1, Fil_param_2
		.seuillage("auto", 0, 0);

	CImageNdg img_morpho = img_seuil
		.morphologie("dilatation", "V8");

	return img_morpho;

	/*
	// Selection ROI
	CImageNdg img_ROI = image
	.ROI(presets[PRESET_ROI_I], presets[PRESET_ROI_J], presets[PRESET_ROI_H], presets[PRESET_ROI_W]);
	CImageDouble image_double(img_ROI, "normalise");

	// Seuillage et traitement
	CImageNdg img_seuil = image_double
	.vecteurGradient("angle")
	.toNdg("expansion")
	.filtrage("gaussien", presets[PRESET_FILTER_PAR_1], presets[PRESET_FILTER_PAR_2])
	.transformation("egalisation", presets[PRESET_EGAL_PAR_1], presets[PRESET_EGAL_PAR_2])
	.seuillage("auto", 0, 0)
	.transformation("complement", 0, 255);

	// Segmentation
	CImageClasse imgClasse(img_seuil, "V4");
	imgClasse = imgClasse
	.filtrage("taille_min", presets[PRESET_SIZE_MIN], true)
	.filtrage("taille_max", presets[PRESET_SIZE_MAX], true);

	// Morphologie
	CImageNdg img_fil_size = imgClasse.toNdg("defaut").seuillage("manuel", 250, 255)
	.morphologie("dilatation", "V8")
	.morphologie("dilatation", "V8")
	.morphologie("dilatation", "V8")
	.morphologie("erosion", "V8")
	.morphologie("erosion", "V8")
	.morphologie("erosion", "V8");

	// Nouvele segmentation
	CImageClasse imgFinale(img_fil_size, "V8");
	imgFinale = imgFinale
	.filtrage("taille_min", 100, true)
	.filtrage("taille_max", 5000, true);
	*/
}



std::string LLD_OCR::LLD_OCR_reading(std::vector<SIGNATURE_Forme> sigOCR)
{
	std::string OCR_result = "";
	double elongation, compacite, ellipticite, cirularite, convexite, ratioAspect, boucle, solidite;
	OCR_alphabet OCR_proba[36];
	int j = 0, probaMax = 0;
	int newLetterIndex = 0;

	for (int i = 0; i < 36; i++)
	{
		if (i < 26)
		{
			OCR_proba[i].letter = 'A' + i;
		}
		else
		{
			OCR_proba[i].letter = '0' + j;
			j++;
		}
	}

	int tampon = 0;
	tampon = sigOCR.size() - 1;
	int nbLayer = sigOCR[tampon].layer;
	int layerTab[40];
	int layerLenght = 0;
	int index = 0, prevIndex = 0;
	int layerIndex[5] = { 0, 1, 0, 0, 0 };
	float averageSpace[5] = { 0, 0, 0, 0, 0 };


	for (int i = 2; i < sigOCR.size(); i++)
	{
		if (sigOCR[i].layer > sigOCR[i - 1].layer)
		{
			tampon = sigOCR[i].layer;
			layerIndex[tampon] = i;
		}
	}


	for (int layer = 1; layer <= nbLayer; layer++)
	{
		if (layer == nbLayer)
		{
			layerLenght = sigOCR.size() - layerIndex[layer];
		}
		else
		{
			layerLenght = layerIndex[layer + 1] - layerIndex[layer];
		}


		for (int i = 0; i < layerLenght; i++)
		{
			tampon = sigOCR[layerIndex[layer] + i].rectEnglob_Bj;
			layerTab[i] = tampon;

			if (i < layerLenght - 1)
			{
				tampon = sigOCR[layerIndex[layer] + i + 1].rectEnglob_Hj - sigOCR[layerIndex[layer] + i].rectEnglob_Bj;
				averageSpace[layer] += tampon;
			}
		}
		// Calcul de l'espace moyen entre les caractères d'une ligne
		averageSpace[layer] = averageSpace[layer] / layerLenght;


		int TODO = 1;
		int nbSpace = 0;
		while (TODO)
		{
			TODO = 0;
			for (int i = 0; i < layerLenght - 1; i++)
			{
				if (layerTab[i] > layerTab[i + 1])
				{
					tampon = layerTab[i + 1];
					layerTab[i + 1] = layerTab[i];
					layerTab[i] = tampon;
					TODO++;
				}
			}
		}



		for (int i = 0; i < layerLenght; i++)
		{
			prevIndex = 0;

			for (int j = 0; j < layerLenght; j++)
			{


				if (layerTab[i] == sigOCR[layerIndex[layer] + j].rectEnglob_Bj)
				{
					if (i > 0)
					{
						prevIndex = index;
					}

					index = layerIndex[layer] + j;

					// Gestion des espaces (entre deux "mots")
					nbSpace = 0;
					if (prevIndex != 0)
					{
						tampon = sigOCR[index].rectEnglob_Hj - sigOCR[prevIndex].rectEnglob_Bj;
						nbSpace = tampon / (averageSpace[layer] * 5);  // Used the value 5 to cancel this option
						if (nbSpace > 0)
						{
							for (int i = 0; i<nbSpace; i++)
								OCR_result += " ";
						}
					}



					// Use of local variables for easier to read code
					elongation = sigOCR[index].elongation, compacite = sigOCR[index].compacite, ellipticite = sigOCR[index].ellipticite, cirularite = sigOCR[index].circulartite,
						convexite = sigOCR[index].convexite, ratioAspect = sigOCR[index].ratioAspect, boucle = sigOCR[index].boucle, solidite = sigOCR[index].solidite;

					// Reset of the alphabet probabilities for the next charactere
					for (int i = 0; i < 36; i++)
					{
						OCR_proba[i].probability = 0;
					}

					// Decision tree

					////////////////////////////////////////////////////////////////////// Elongation
					if (elongation < 0.49) {
						OCR_proba[8].probability++;
						OCR_proba[27].probability++;
					}
					else if (0.49 <= elongation && elongation < 0.53)
					{
						OCR_proba[9].probability++;
						OCR_proba[11].probability++;
						OCR_proba[5].probability++;
						OCR_proba[35].probability++;
					}
					else if (0.53 <= elongation && elongation < 0.54)
					{
						OCR_proba[33].probability++;
						OCR_proba[32].probability++;
					}
					else if (0.54 <= elongation && elongation < 0.56)
					{
						OCR_proba[16].probability++;
						OCR_proba[1].probability++;
						OCR_proba[24].probability++;
						OCR_proba[4].probability++;
						OCR_proba[15].probability++;
						OCR_proba[10].probability++;
					}
					else if (0.56 <= elongation && elongation < 0.57)
					{
						OCR_proba[20].probability++;
						OCR_proba[21].probability++;
						OCR_proba[28].probability++;
						OCR_proba[31].probability++;
						OCR_proba[23].probability++;
					}
					else if (0.57 <= elongation && elongation < 0.58)
					{
						OCR_proba[2].probability++;
						OCR_proba[6].probability++;
						OCR_proba[14].probability++;
					}
					else if (0.58 <= elongation && elongation < 0.59)
					{
						OCR_proba[19].probability++;
						OCR_proba[30].probability++;
						OCR_proba[34].probability++;
						OCR_proba[3].probability++;
						OCR_proba[25].probability++;
						OCR_proba[7].probability++;
						OCR_proba[13].probability++;
					}
					else if (0.59 <= elongation && elongation < 0.6)
					{
						OCR_proba[29].probability++;
						OCR_proba[18].probability++;
					}
					else if (0.60 <= elongation && elongation < 0.65)
					{
						OCR_proba[0].probability++;
					}
					else if (0.65 <= elongation && elongation < 0.75)
					{
						OCR_proba[26].probability++;
					}
					else if (0.75 <= elongation)
					{
						OCR_proba[22].probability++;
					}

					////////////////////////////////////////////////////////////////////// Compacité
					if (compacite < 3) {
						OCR_proba[22].probability++;
						OCR_proba[1].probability++;
						OCR_proba[34].probability++;
						OCR_proba[3].probability++;
						OCR_proba[16].probability++;
						OCR_proba[14].probability++;
						OCR_proba[26].probability++;
						OCR_proba[15].probability++;
					}
					else if (3 <= compacite && compacite < 5)
					{
						OCR_proba[35].probability++;
						OCR_proba[32].probability++;
						OCR_proba[0].probability++;
						OCR_proba[11].probability++;
					}
					else if (5 <= compacite && compacite < 5.9)
					{
						OCR_proba[19].probability++;
						OCR_proba[27].probability++;
						OCR_proba[8].probability++;
						OCR_proba[9].probability++;
						OCR_proba[24].probability++;
						OCR_proba[5].probability++;
					}
					else if (5.9 <= compacite && compacite < 6.8)
					{
						OCR_proba[33].probability++;
						OCR_proba[30].probability++;
						OCR_proba[25].probability++;
					}
					else if (6.8 <= compacite && compacite < 7.3)
					{
						OCR_proba[2].probability++;
						OCR_proba[4].probability++;
						OCR_proba[21].probability++;
						OCR_proba[28].probability++;
					}
					else if (7.3 <= compacite && compacite < 7.5)
					{
						OCR_proba[7].probability++;
						OCR_proba[29].probability++;
						OCR_proba[10].probability++;
					}
					else if (7.5 <= compacite && compacite < 8)
					{
						OCR_proba[23].probability++;
						OCR_proba[20].probability++;
					}
					else if (8 <= compacite && compacite < 8.5)
					{
						OCR_proba[6].probability++;
						OCR_proba[31].probability++;
					}
					else if (8.5 <= compacite)
					{
						OCR_proba[18].probability++;
						OCR_proba[13].probability++;
					}

					////////////////////////////////////////////////////////////////////// Ellipticite
					if (ellipticite < 0.26) {
						OCR_proba[8].probability++;
						OCR_proba[27].probability++;
					}
					else if (0.26 <= ellipticite && ellipticite < 0.28)
					{
						OCR_proba[9].probability++;
						OCR_proba[35].probability++;
						OCR_proba[11].probability++;
						OCR_proba[5].probability++;
					}
					else if (0.28 <= ellipticite && ellipticite < 0.31)
					{
						OCR_proba[32].probability++;
						OCR_proba[33].probability++;
						OCR_proba[16].probability++;
						OCR_proba[1].probability++;
						OCR_proba[15].probability++;
						OCR_proba[24].probability++;
						OCR_proba[4].probability++;
						OCR_proba[10].probability++;
					}
					else if (0.31 <= ellipticite && ellipticite < 0.33)
					{
						OCR_proba[20].probability++;
						OCR_proba[21].probability++;
						OCR_proba[28].probability++;
						OCR_proba[23].probability++;
						OCR_proba[31].probability++;
						OCR_proba[14].probability++;
						OCR_proba[2].probability++;
						OCR_proba[6].probability++;
					}
					else if (0.33 <= ellipticite && ellipticite < 0.34)
					{
						OCR_proba[34].probability++;
						OCR_proba[3].probability++;
						OCR_proba[19].probability++;
						OCR_proba[30].probability++;
						OCR_proba[25].probability++;
						OCR_proba[7].probability++;
						OCR_proba[13].probability++;
					}
					else if (0.34 <= ellipticite && ellipticite < 0.36)
					{
						OCR_proba[29].probability++;
						OCR_proba[18].probability++;
					}
					else if (0.36 <= ellipticite && ellipticite < 0.4)
					{
						OCR_proba[0].probability++;
					}
					else if (0.4 <= ellipticite && ellipticite < 0.6)
					{
						OCR_proba[26].probability++;
					}
					else if (0.6 <= ellipticite)
					{
						OCR_proba[22].probability++;
					}


					////////////////////////////////////////////////////////////////////// Cirularite
					if (cirularite < 42) {
						OCR_proba[18].probability++;
					}
					else if (42 <= cirularite && cirularite < 44.5)
					{
						OCR_proba[9].probability++;
						OCR_proba[33].probability++;
						OCR_proba[23].probability++;
						OCR_proba[21].probability++;
						OCR_proba[24].probability++;
						OCR_proba[2].probability++;
					}
					else if (44.5 <= cirularite && cirularite < 46)
					{
						OCR_proba[20].probability++;
						OCR_proba[19].probability++;
						OCR_proba[31].probability++;
						OCR_proba[28].probability++;
						OCR_proba[30].probability++;
						OCR_proba[27].probability++;
						OCR_proba[11].probability++;
					}
					else if (46 <= cirularite && cirularite < 47)
					{
						OCR_proba[6].probability++;
						OCR_proba[29].probability++;
						OCR_proba[10].probability++;
						OCR_proba[22].probability++;
					}
					else if (47 <= cirularite && cirularite < 49)
					{
						OCR_proba[13].probability++;
						OCR_proba[25].probability++;
						OCR_proba[5].probability++;
						OCR_proba[8].probability++;
					}
					else if (49 <= cirularite && cirularite < 55)
					{
						OCR_proba[7].probability++;
						OCR_proba[4].probability++;
					}
					else if (55 <= cirularite && cirularite < 60)
					{
						OCR_proba[0].probability++;
					}
					else if (60 <= cirularite && cirularite < 70)
					{
						OCR_proba[32].probability++;
						OCR_proba[35].probability++;
					}
					else if (70 <= cirularite && cirularite < 75)
					{
						OCR_proba[15].probability++;
					}
					else if (75 <= cirularite && cirularite < 83)
					{
						OCR_proba[14].probability++;
					}
					else if (83 <= cirularite && cirularite < 91)
					{
						OCR_proba[3].probability++;
						OCR_proba[16].probability++;
						OCR_proba[34].probability++;
					}
					else if (91 <= cirularite)
					{
						OCR_proba[1].probability++;
						OCR_proba[26].probability++;
					}

					////////////////////////////////////////////////////////////////////// Convexite
					if (convexite < 0.2) {
						OCR_proba[13].probability++;
					}
					else if (0.2 <= convexite && convexite < 0.23)
					{
						OCR_proba[31].probability++;
						OCR_proba[12].probability++;
						OCR_proba[6].probability++;
					}
					else if (0.23 <= convexite && convexite < 0.25)
					{
						OCR_proba[4].probability++;
						OCR_proba[23].probability++;
						OCR_proba[10].probability++;
						OCR_proba[7].probability++;
						OCR_proba[18].probability++;
						OCR_proba[20].probability++;
					}
					else if (0.25 <= convexite && convexite < 0.27)
					{
						OCR_proba[29].probability++;
						OCR_proba[28].probability++;
						OCR_proba[8].probability++;
						OCR_proba[21].probability++;
						OCR_proba[22].probability++;
						OCR_proba[25].probability++;
					}
					else if (0.27 <= convexite && convexite < 0.29)
					{
						OCR_proba[35].probability++;
						OCR_proba[2].probability++;
						OCR_proba[5].probability++;
						OCR_proba[32].probability++;
					}
					else if (0.29 <= convexite && convexite < 0.31)
					{
						OCR_proba[27].probability++;
						OCR_proba[17].probability++;
						OCR_proba[30].probability++;
						OCR_proba[33].probability++;
					}
					else if (0.31 <= convexite && convexite < 0.35)
					{
						OCR_proba[9].probability++;
						OCR_proba[24].probability++;
						OCR_proba[0].probability++;
					}
					else if (0.35 <= convexite && convexite < 0.385)
					{
						OCR_proba[11].probability++;
						OCR_proba[19].probability++;
						OCR_proba[1].probability++;
						OCR_proba[16].probability++;
					}
					else if (0.385 <= convexite)
					{
						OCR_proba[15].probability++;
						OCR_proba[3].probability++;
						OCR_proba[34].probability++;
						OCR_proba[14].probability++;
						OCR_proba[26].probability++;
					}

					////////////////////////////////////////////////////////////////////// Ratio d'aspect
					if (ratioAspect < 1.5) {
						OCR_proba[22].probability++;
						OCR_proba[26].probability++;
					}
					else if (1.5 <= ratioAspect && ratioAspect < 1.67)
					{
						OCR_proba[12].probability++;
						OCR_proba[0].probability++;
					}
					else if (1.67 <= ratioAspect && ratioAspect < 1.73)
					{
						OCR_proba[3].probability++;
						OCR_proba[7].probability++;
						OCR_proba[14].probability++;
						OCR_proba[17].probability++;
						OCR_proba[18].probability++;
						OCR_proba[29].probability++;
						OCR_proba[30].probability++;
						OCR_proba[34].probability++;
					}
					else if (1.73 <= ratioAspect && ratioAspect < 1.77)
					{
						OCR_proba[2].probability++;
						OCR_proba[6].probability++;
						OCR_proba[19].probability++;
						OCR_proba[25].probability++;
					}
					else if (1.77 <= ratioAspect && ratioAspect < 1.81)
					{
						OCR_proba[4].probability++;
						OCR_proba[10].probability++;
						OCR_proba[13].probability++;
						OCR_proba[15].probability++;
						OCR_proba[28].probability++;
						OCR_proba[31].probability++;
						OCR_proba[20].probability++;
					}
					else if (1.81 <= ratioAspect && ratioAspect < 1.85)
					{
						OCR_proba[1].probability++;
						OCR_proba[21].probability++;
						OCR_proba[24].probability++;
					}
					else if (1.85 <= ratioAspect && ratioAspect < 1.885)
					{
						OCR_proba[15].probability++;
						OCR_proba[11].probability++;
						OCR_proba[32].probability++;
						OCR_proba[33].probability++;
						OCR_proba[16].probability++;
					}
					else if (1.885 <= ratioAspect && ratioAspect < 1.95)
					{
						OCR_proba[9].probability++;
						OCR_proba[23].probability++;
					}
					else if (1.95 <= ratioAspect && ratioAspect < 2.1)
					{
						OCR_proba[27].probability++;
						OCR_proba[36].probability++;
					}
					else if (2.1 <= ratioAspect)
					{
						OCR_proba[8].probability++;
					}

					////////////////////////////////////////////////////////////////////// Boucle
					if (boucle < 27.5) {
						OCR_proba[26].probability++;
						OCR_proba[34].probability++;
						OCR_proba[1].probability++;
					}
					else if (27.5 <= boucle && boucle < 33)
					{
						OCR_proba[3].probability++;
						OCR_proba[16].probability++;
						OCR_proba[14].probability++;
					}
					else if (33 <= boucle && boucle < 40)
					{
						OCR_proba[15].probability++;
					}
					else if (40 <= boucle && boucle < 50)
					{
						OCR_proba[32].probability++;
						OCR_proba[0].probability++;
						OCR_proba[35].probability++;
					}
					else if (50 <= boucle && boucle < 56.5)
					{
						OCR_proba[8].probability++;
						OCR_proba[4].probability++;
						OCR_proba[7].probability++;
						OCR_proba[5].probability++;
						OCR_proba[25].probability++;
					}
					else if (56.5 <= boucle && boucle < 59.5)
					{
						OCR_proba[13].probability++;
						OCR_proba[11].probability++;
						OCR_proba[10].probability++;
						OCR_proba[27].probability++;
						OCR_proba[29].probability++;
					}
					else if (59.5 <= boucle && boucle < 61)
					{
						OCR_proba[22].probability++;
						OCR_proba[19].probability++;
						OCR_proba[30].probability++;
						OCR_proba[28].probability++;
					}
					else if (61 <= boucle && boucle < 62)
					{
						OCR_proba[31].probability++;
						OCR_proba[24].probability++;
						OCR_proba[6].probability++;
					}
					else if (62 <= boucle && boucle < 63)
					{
						OCR_proba[21].probability++;
						OCR_proba[33].probability++;
						OCR_proba[23].probability++;
					}
					else if (63 <= boucle && boucle < 65)
					{
						OCR_proba[2].probability++;
						OCR_proba[20].probability++;
					}
					else if (65 <= boucle)
					{
						OCR_proba[9].probability++;
						OCR_proba[18].probability++;
					}

					////////////////////////////////////////////////////////////////////// Solidite
					if (solidite < 225) {
						OCR_proba[19].probability++;
						OCR_proba[9].probability++;
					}
					else if (225 <= solidite && solidite < 300)
					{
						OCR_proba[11].probability++;
						OCR_proba[24].probability++;
						OCR_proba[33].probability++;
						OCR_proba[27].probability++;
					}
					else if (300 <= solidite && solidite < 400)
					{
						OCR_proba[30].probability++;
						OCR_proba[2].probability++;
						OCR_proba[21].probability++;
					}
					else if (400 <= solidite && solidite < 440)
					{
						OCR_proba[5].probability++;
						OCR_proba[8].probability++;
						OCR_proba[23].probability++;
						OCR_proba[18].probability++;
					}
					else if (440 <= solidite && solidite < 460)
					{
						OCR_proba[28].probability++;
						OCR_proba[0].probability++;
					}
					else if (460 <= solidite && solidite < 500)
					{
						OCR_proba[25].probability++;
						OCR_proba[20].probability++;
						OCR_proba[29].probability++;
					}
					else if (500 <= solidite && solidite < 550)
					{
						OCR_proba[15].probability++;
						OCR_proba[14].probability++;
						OCR_proba[10].probability++;
					}
					else if (550 <= solidite && solidite < 650)
					{
						OCR_proba[31].probability++;
						OCR_proba[6].probability++;
						OCR_proba[7].probability++;
						OCR_proba[4].probability++;
					}
					else if (650 <= solidite && solidite < 700)
					{
						OCR_proba[32].probability++;
						OCR_proba[35].probability++;

					}
					else if (700 <= solidite && solidite < 800)
					{
						OCR_proba[3].probability++;
						OCR_proba[22].probability++;
						OCR_proba[16].probability++;
						OCR_proba[34].probability++;
					}
					else if (800 <= solidite)
					{
						OCR_proba[13].probability++;
						OCR_proba[26].probability++;
						OCR_proba[1].probability++;
					}

					// Finding the letter with the highest probability rate
					probaMax = 0;
					for (int i = 0; i < 36; i++)
					{
						if (OCR_proba[i].probability > probaMax)
						{
							probaMax = OCR_proba[i].probability;
							newLetterIndex = i;
						}
					}

					// Adding the above letter to the result string
					OCR_result += OCR_proba[newLetterIndex].letter;

					prevIndex = index;

					break;
				}
			}


		}

		OCR_result += "\n";
	}


	return OCR_result;
}