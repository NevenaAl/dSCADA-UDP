using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using System.Threading;

namespace WpfDSCADA
{
   /// <summary>
   /// Interaction logic for SvgShapeInfo.xaml
   /// </summary>
   public partial class SvgShapeInfo : Window
   {
      public SvgShapeInfo(string Ime, string Opis, string Stanje, string Vreme)
        {
            InitializeComponent();

            this.labVar.Content = Ime;
            this.labDesc.Content = Opis;
            this.labCurrent.Content = Stanje;
            this.labTime.Content = Vreme;
        }

      public SvgShapeInfo()
      {
         InitializeComponent();

         this.labVar.Content = "";
         this.labDesc.Content = "";
         this.labCurrent.Content = "";
         this.labTime.Content = "";
      }
   }
}
