using System.Windows;
using WpfDSCADA;
using WpfDSCADA.SVG;
using WpfDSCADA.Properties;
using System.Windows.Threading;
using System;
using System.Windows.Input;
using System.Collections.ObjectModel;
using System.IO;
using WpfDSCADA.Model;

namespace WpfDSCADA
{
   /// <summary>
   /// Interaction logic for GraphicWindow.xaml
   /// </summary>
   public partial class GraphicWindow : Window
   {
      private const double zoomConst = 0.5;
      public SVGInterface svgInterface = new SVGInterface();
      public ObservableCollection<ProcVar> ProcVarList;
      public bool graphWindowInitFlag = false;
      public SvgShapeInfo svgShapeInfoWindow = new SvgShapeInfo();

      public GraphicWindow()
      {
         InitializeComponent();
      }

      private void canvas_MouseEnter(object sender, System.Windows.Input.MouseEventArgs e)
      {
         canvas.Focus();
      }

      private void canvas_MouseWheel(object sender, System.Windows.Input.MouseWheelEventArgs e)
      {
         if (Keyboard.IsKeyDown(Key.LeftCtrl) || Keyboard.IsKeyDown(Key.RightCtrl))
         {
            if (e.Delta > 0)
            {
               zoomSlider.Value += zoomConst;
            }
            else
            {
               zoomSlider.Value -= zoomConst;
            }
         }
      }

      private void zoomOut_Click(object sender, RoutedEventArgs e)
      {
         if (zoomSlider.Value - zoomConst < zoomSlider.Minimum)
         {
            zoomSlider.Value = zoomSlider.Minimum;
         }
         else
         {
            zoomSlider.Value -= zoomConst;
         }
      }

      private void zoomIn_Click(object sender, RoutedEventArgs e)
      {
         if (zoomSlider.Value + zoomConst > zoomSlider.Maximum)
         {
            zoomSlider.Value = zoomSlider.Maximum;
         }
         else
         {
            zoomSlider.Value += zoomConst;
         }
      }

      private void hideGraphicsButton_Click(object sender, RoutedEventArgs e)
      {
         this.Hide();
      }

      private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
      {
         this.Visibility = Visibility.Hidden;
         e.Cancel = true;
      }

      public void svgShapeClicked(object sender, EventArgs e, int mID)
      {
         // obrisi info prozor ako je prikazan
         if (svgShapeInfoWindow.IsVisible)
         {
            svgShapeInfoWindow.Close();
         }

         SVGShape shapeClicked = FindSVGShape(mID);

         switch (shapeClicked.TypeOfShape)
         {

            case SVGSHAPETYPE.SVG_BUTTON:
               // klik na dugme za otvaranje novog dijagrama
               String svgFilePath = Environment.CurrentDirectory + String.Format(@"\SVG files\{0}", shapeClicked.cp.svgLink);
               if ( File.Exists(svgFilePath) )
               {
                  svgInterface.SVGRoot(svgFilePath, this.canvas);
                  OnCallInitSVGPage(EventArgs.Empty);
               }
               else
               {
                  MessageBox.Show(String.Format("Greska! {0} ne postoji.", shapeClicked.cp.svgLink), "Greska!");
               }
               break;

            case SVGSHAPETYPE.SVG_SYMBOL:
               // klik na simbol
               string[] splitPVID = shapeClicked.cp.pvid.Split(':');

               ProcVar dev = FindProcVar(splitPVID);

               if (dev != null)
               {
                  bool openCmd = false;
                  switch (dev.DevType.ToString())
                  {
                     case "Digital":
                     case "DigObj":
                        if (!dev.EuOrComm.Contains("NEMA_IZLAZA"))
                        {
                           openCmd = true;
                        }
                        break;
                     case "Counter":
                     case "AnaOut":
                     case "AnaObj":
                        openCmd = true;
                        break;
                     default:
                        break;
                  }
                  if (openCmd)
                  {
                     OpenSendCommandDialog(dev);
                  }
               }
               break;

            default:
               break;
         }
      }

      public void svgShapeRightClicked(object sender, EventArgs e, int mID)
      {
         // skloni info dijalog
         if (svgShapeInfoWindow.IsVisible)
         {
            svgShapeInfoWindow.Close();
         }
         // i ponovo ga prikazi ako ima sta
         SVGShape shapeClicked = FindSVGShape(mID);
         if (shapeClicked.cp.pvid != "")
         {
            string[] splitPVID = shapeClicked.cp.pvid.Split(':');
            string Stanje;
            ProcVar dev = FindProcVar(splitPVID);
            if (dev != null)
            {
               Point mousePoint = Mouse.GetPosition(this);
               string Ime = dev.Rtu + " : " + dev.Name + " ,  " + dev.DevType;
               if ((dev.DevType == "Digital") || (dev.DevType == "DigObj"))
               {
                  Stanje = dev.StateOrVal + " / " + dev.EuOrComm + " ,  " + dev.Status;
               }
               else
               {
                  Stanje = dev.StateOrVal + "   " + dev.EuOrComm + " , " + dev.Status;
               }
               svgShapeInfoWindow = new SvgShapeInfo(Ime, dev.Description, Stanje, dev.TimeStamp);
               svgShapeInfoWindow.Left = mousePoint.X + this.Left;
               svgShapeInfoWindow.Top = mousePoint.Y + this.Top;
               svgShapeInfoWindow.Show();
            }
         }
      }

      public ProcVar FindProcVar(string[] pvid)
      {
         ProcVar procVar = null;
         foreach (ProcVar pv in ProcVarList)
         {
            if (pv.Rtu.Equals(pvid[0]) && pv.Name.Equals(pvid[1]))
            {
               procVar = pv;
               break;
            }
         }
         return procVar;
      }

      public SVGShape FindSVGShape(int mID)
      {
         if (svgInterface.svgPage.shape_list.Count > 0)
         {
            foreach (SVGShape s in svgInterface.svgPage.shape_list)
            {
               if (s.mID == mID)
               {
                  return s;
               }
            }
         }
         return null;
      }

      private void OpenSendCommandDialog(ProcVar procVar)
      {
         string stanje, komanda;
         if (procVar.DevType.Equals("Digital") || procVar.DevType.Equals("DigObj"))
         {
            stanje = procVar.StateOrVal;
            komanda = procVar.EuOrComm;
         }
         else
         {
            stanje = procVar.Status;
            komanda = procVar.StateOrVal;
         }
         SendCommandDialog dlg = new SendCommandDialog(procVar.DevType, procVar.Rtu, procVar.Name, stanje, komanda, procVar.TimeStamp);
         dlg.Owner = this;
         dlg.ShowDialog();
         if (dlg.DialogResult == true)
         {
            OnGraphWindowCommandSend(EventArgs.Empty, dlg.command);
         }
      }

      public delegate void graphWindowCommandEventHandler(object sender, EventArgs e, String gCommand);

      public event graphWindowCommandEventHandler graphWindowCommandEvHandler;
      
      protected virtual void OnGraphWindowCommandSend(EventArgs e, String gCommand)
      {
         if (graphWindowCommandEvHandler != null)
            graphWindowCommandEvHandler(this, e, gCommand);
      }
      
      public delegate void callInitSVGPageEventHandler(object sender, EventArgs e);
      
      public event callInitSVGPageEventHandler callInitSVGPageEvHandler;
      
      protected virtual void OnCallInitSVGPage(EventArgs e)
      {
         if (callInitSVGPageEvHandler != null)
            callInitSVGPageEvHandler(this, e);
      }
   }
}
