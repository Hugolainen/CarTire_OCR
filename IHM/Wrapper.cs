using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.Drawing.Imaging;
using System.Drawing;
using System.Windows.Forms;

/*
 * Classe dédiée au passage C# -> C++ 
 */

namespace IHM
{
    class Wrapper
    {
        // Création d'une classe C# avec pointeur sur l'objet C++
        // Création des static extern exportées de chaque méthode utile de la classe C++

        public IntPtr ClPtr;

        public Wrapper()
        {
            ClPtr = IntPtr.Zero;
        }

        ~Wrapper()
        {
            if (ClPtr != IntPtr.Zero)
                ClPtr = IntPtr.Zero;
        }

        // Va-et-vient avec constructeur C#/C++ obligatoire dans toute nouvelle classe propre à l'application

        [DllImport("LibraryImage.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void LLD_getDOT(IntPtr ClPtr, StringBuilder dot, int nbChamps, IntPtr data, int stride, int nbLig, int nbCol, int[] presets);

        public String LLD_GetDOT(Bitmap bmp, int[] presets)
        {
            StringBuilder str = new StringBuilder();
            unsafe
            {
                BitmapData bmpData = bmp.LockBits(new Rectangle(0, 0, bmp.Width, bmp.Height), ImageLockMode.ReadWrite, PixelFormat.Format24bppRgb);
                try
                {
                    LLD_getDOT(ClPtr, str, 1, bmpData.Scan0, bmpData.Stride, bmp.Height, bmp.Width, presets);    // Envoie de l'image à la connexion C# du LLD
                }
                catch (Exception ex)
                {
                    MessageBox.Show("Error: " + ex.Message);
                }
                bmp.UnlockBits(bmpData);
            }

            return str.ToString();
        }

        [DllImport("LibraryImage.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr LLD_init();

        public void LLD_Init()
        {
            ClPtr = LLD_init();
        }
    }
}
