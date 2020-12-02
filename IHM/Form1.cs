using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Windows.Forms;
using System.IO;
using System.Globalization;
using System.Diagnostics;

namespace IHM
{
    public partial class Form1 : Form
    {
        private static int NUMBER_OF_PRESETS = 15; // Attention : Ce nombre est définie dans le fichier "LLD_OCR.h", et ne doit PAS être modifié

        private int[] _presets;
        private Wrapper _lldConnexion;
        private List<string> _list_path;
        private int _list_path_pos;
        private int _list_path_size;
        private int tirePassed, tireNotPassed, tireControled;
        private bool presetLoaded;

        public Form1()
        {
            InitializeComponent();

            _presets = new int[NUMBER_OF_PRESETS]; // Tableau contenant les presets
            _lldConnexion = new Wrapper(); // Importation des fonctions du low-level driver c++
            _list_path = new List<string>(); // Liste contenant le chemin d'acces des images
            _list_path_pos = 0; // Position actuelle de l'image dans la liste
            _list_path_size = 0; // Nombre d'images dans le dossier
            tirePassed = 0;
            tireNotPassed = 0;
            tireControled = 0;
            presetLoaded=false;

            _lldConnexion.LLD_Init(); // Initialise LLD;
        }

        private void button_open_Click(object sender, EventArgs e)
        {
            if (folderBrowserDialog1.ShowDialog() == DialogResult.OK)
            {
                try
                {
                    // Création d'un tableau de string contenant tous les fichiers
                    string[] list1 = Directory.GetFiles(folderBrowserDialog1.SelectedPath);
                    for (int i = 0; i < list1.Length; i++)
                    {
                        // Génération à partir de ce tableau d'une liste de string contenant le nom des fichiers
                        if (list1[i].Contains(".bmp"))
                        {
                            _list_path.Add(list1[i]);
                        }
                    }

                    _list_path_pos = 0;
                    _list_path_size = _list_path.Count();

                    pictureBox1.ImageLocation = _list_path[_list_path_pos];
                    label_chemin.Text = _list_path[_list_path_pos];
                }
                catch (Exception ex)
                {
                    MessageBox.Show("ERREUR : Issue relative to folder : \n" + ex.Message);
                }
            }
        }

        private void button_quitter_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void button_next_Click(object sender, EventArgs e)
        {
            try
            {

                // Empeche de charger un _list_path_pos null (génere une erreur)
                if (_list_path_pos >= _list_path_size - 1)
                {
                    _list_path_pos = 0;
                    pictureBox1.ImageLocation = _list_path[_list_path_pos];
                }
                else
                {
                    _list_path_pos++;
                    pictureBox1.ImageLocation = _list_path[_list_path_pos];
                }

                label_chemin.Text = _list_path[_list_path_pos];

                //Reinitialisation des resultats à chaque changement d'image
                label_CodeUsine.Text = "";
                label_CodeDim.Text = "";
                label_CodeOpt.Text = "";
                label_DateDeFab.Text = "";

                label_pass.BackColor = Color.LightGray;
                label_failed.BackColor = Color.LightGray;
            }
            catch (Exception ex)
            {
                MessageBox.Show("ERREUR : No image in folder : \n" + ex.Message);
            }
        }

        private void button_process_Click(object sender, EventArgs e)
        {
            //pictureBox2.ImageLocation = list_path[indice];
            if(presetLoaded)
            {
                try
                {
                    string OCR_result = "Sheet error";
                    OCR_result = _lldConnexion.LLD_GetDOT((Bitmap)pictureBox1.Image, _presets);

                    string folder = Path.GetDirectoryName(Process.GetCurrentProcess().MainModule.FileName) + @"..\..\..\..\Resultats";
                    string filter = "img_OCR.bmp";
                    string[] files = Directory.GetFiles(folder, filter);

                    pictureBox2.ImageLocation = files[0]; // En considérant un seul fichier comprenant ce nom, sa position dans le tableau vaut 0.

                    int currentWeek = CultureInfo.CurrentCulture.Calendar.GetWeekOfYear(DateTime.Now, CalendarWeekRule.FirstDay, DayOfWeek.Monday);
                    int currentYear = DateTime.Now.Year;
                    bool result = false;

                    /////////////////////////////////////////////////////////// OCR
                    string toTest = OCR_result;

                    /////////////////////////////////////////////////////////// Recomposition du DOT
                    string[] DOT = new string[4];
                    DOT = getDOT(toTest);
                    label_CodeUsine.Text = DOT[0];
                    label_CodeDim.Text = DOT[1];
                    label_CodeOpt.Text = DOT[2];
                    label_DateDeFab.Text = DOT[3];

                    /////////////////////////////////////////////////////////// Control du DOT
                    result = testDOT(DOT[3], currentWeek, currentYear);


                    if (result)
                    {
                        label_pass.BackColor = Color.Chartreuse;
                        label_failed.BackColor = Color.LightGray;
                        tirePassed++;
                    }
                    else
                    {
                        label_pass.BackColor = Color.LightGray;
                        label_failed.BackColor = Color.Red;
                        tireNotPassed++;
                    }
                    tireControled++;

                    label_statsPassed.Text = ((tirePassed * 100) / tireControled).ToString() + " %";
                    label_statsNotPassed.Text = ((tireNotPassed * 100) / tireControled).ToString() + " %";
                    label_tireControlled.Text = tireControled.ToString();

                    if (((tirePassed * 100) / tireControled) >= 50)
                    {
                        panel_result.BackColor = Color.PaleGreen;
                    }
                    else
                    {
                        panel_result.BackColor = Color.LightSalmon;
                    }

                }
                catch (Exception ex)
                {
                    MessageBox.Show("Error: " + ex.Message);
                }
            }
            else
                MessageBox.Show("Presets are missing, please load them using the button 'PRESETS'");

        }

        private void button_reset_Click(object sender, EventArgs e)
        {
            if(pictureBox1.Image != null)
            {
                pictureBox1.Image.Dispose();
                pictureBox1.Image = null;
            }
            
            if (pictureBox2.Image != null)
            {
                pictureBox2.Image.Dispose();
                pictureBox2.Image = null;
            }

            label_chemin.Text = "C://";
            label_CodeUsine.Text = "";
            label_CodeDim.Text = "";
            label_CodeOpt.Text = "";
            label_DateDeFab.Text = "";
            label_statsPassed.Text = "";
            label_statsNotPassed.Text = "";
            label_tireControlled.Text = "";

            label_pass.BackColor = Color.LightGray;
            label_failed.BackColor = Color.LightGray;
            panel_result.BackColor = Color.LightGray;

            presetLoaded = false;
            tirePassed = 0;
            tireNotPassed = 0;
            tireControled = 0;
            _list_path = new List<string>();
            _list_path_pos = 0; 
            _list_path_size = 0;
        }

        private void button_preset_Click(object sender, EventArgs e)
        {
            string fileName;

            // On met des filtres pour les types de fichiers: "Nom|*.extension|autreNom|*.autreExtension" (autant de filtres qu'on veut)
            openFileDialog1.Filter = "Fichiers texte|*.txt|Tous les fichiers|*.*";

            // On affiche le dernier dossier ouvert
            openFileDialog1.RestoreDirectory = true;

            if (openFileDialog1.ShowDialog() == DialogResult.OK)
            {
                try
                {
                    // On récupère le nom du fichier
                    fileName = openFileDialog1.FileName;

                    // On lit le fichier
                    string[] lignes = File.ReadAllLines(fileName);

                    // Correspond à une ligne du tableau séparée en deux
                    string[] tab = new string[2];

                    // Selection des valeurs de chaque ligne
                    for (int i = 0; i < lignes.Length; i++)
                    {
                        tab = lignes[i].Split('='); // Parcours chaque ligne du tableau et split les valeur du reste
                        _presets[i] = int.Parse(tab[1]); // Positions : 0 = nom de la ligne, 1 = valeur
                    }
                    MessageBox.Show("Presets saved");
                    presetLoaded = true;
                }
                // En cas d'erreur
                catch (Exception)
                {
                    MessageBox.Show("Unexpected error in preset loading");
                }
            }
        }

      
        public string[] getDOT(string imageOutput)
        {
            string[] DOT = new string[4];
            DOT[0] = "";
            DOT[1] = "";
            DOT[2] = "";
            DOT[3] = "";
            string test = "";
            int stringStart = 0;

            for (int i = 0; i < 3; i++)
            {
                test += imageOutput[i];
            }


            if (test == "DOT")
            {
                stringStart = 3;
            }

            try
            {
                DOT[0] = imageOutput.Substring(stringStart, _presets[1]);
                stringStart += 2;
                DOT[1] = imageOutput.Substring(stringStart, _presets[2]);
                stringStart += 2;
                DOT[2] = imageOutput.Substring(stringStart, _presets[3]);
                stringStart += 4;
                DOT[3] = imageOutput.Substring(stringStart, _presets[4]);
            }
            catch (Exception ex)
            {
                MessageBox.Show("One or several characters are missing or cannot be proceeded");
            }
            /*
            for (int i = stringStart; i < imageOutput.Length; i++)
            {
                if (DOT[0].Length < 2)
                    DOT[0] += imageOutput[i];
                else if (DOT[1].Length < 2)
                    DOT[1] += imageOutput[i];
                else if(DOT[2].Length < 4)
                    DOT[2] += imageOutput[i];
                else if(DOT[3].Length < 4)
                    DOT[3] += imageOutput[i];
            }
            */
            return DOT;
        }

        public bool testDOT(string tireDate, int actualWeek, int actualYear)
        {
            bool result = false;
            string tireWeek = tireDate.Substring(0,2);
            string tireYear = tireDate.Substring(2,2);
            int week = int.Parse(tireWeek);
            int year = int.Parse(tireYear) + 2000;
            int weekCount = 0;

            int diffYear = actualYear - year;
            int diffWeek = actualWeek - week;
            weekCount = (diffYear * 52) + diffWeek;

            if (weekCount > 260)
            {
                result = false;
            }
            else
            {
                result = true;
            }

            return result;
        }

    }
}