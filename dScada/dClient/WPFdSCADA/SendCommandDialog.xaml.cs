using System;
using System.Diagnostics;
using System.Windows;
using System.Collections.Generic;


namespace WpfDSCADA
{
    /// <summary>
    /// Interaction logic for SendCommandDialog.xaml
    /// </summary>
    public partial class SendCommandDialog : Window
    {

        public string command { get; set; }
        public string devType { get; set; }

        public SendCommandDialog(string devType, string rtu, string name, string sts, string cmd, string tstamp)
        {
            InitializeComponent();

            this.devType = devType;

            RtuTextBlock.Text = rtu;
            NameTextBlock.Text = name;
            TextBlock41.Text = sts;
            TextBlock51.Text = cmd;
            TextBlock61.Text = tstamp;

            //u zavisnosti od tipa, podesavanje forme
            switch (devType)
            {
                case "Digital":
                case "DigObj":
                    Title = name + "  - zadavanje komande";
                    TextBlock3.Text = "Trenutno stanje:";
                    TextBlock4.Text = "[stanje]";
                    TextBlock5.Text = "[komanda]";
                    TextBlock7.Text = "Komanda:";
                    cbComm.ItemsSource = MainWindow.getAllCommands();
                    cbComm.SelectedValue = cmd;
                    cbComm.Focus();
                    tbComm.Visibility = Visibility.Hidden;
                    break;

                case "Counter":
                    Title = name + "- reset brojaca";
                    TextBlock3.Text = "Trenutna vrednost:";
                    TextBlock4.Text = "[status]";
                    TextBlock5.Text = "[vrednost]";
                    TextBlock7.Visibility = Visibility.Hidden;
                    //tbComm.Visibility = Visibility.Hidden;
                    cbComm.Visibility = Visibility.Hidden;
                    btnOK.Content = "Reset";
                    break;

                case "AnaOut":
                case "AnaObj":
                    Title = name + "  - zadavanje komande";
                    TextBlock3.Text = "Trenutna vrednost:";
                    TextBlock4.Text = "[status]";
                    TextBlock5.Text = "[vrednost]";
                    TextBlock7.Text = "Nova vrednost:";
                    cbComm.Visibility = Visibility.Hidden;
                    tbComm.Text = cmd;
                    tbComm.Focus();
                    tbComm.SelectAll();
                    break;

                default:
                    break;
            }
        }

        private void okButton_Click(object sender, RoutedEventArgs e)
        {
            bool inputOK = false;
            string valString = "";

            if (devType == "Counter")
            {
                inputOK = true;   // nema validacije... nema unosa podataka
                valString = "0";
            }
            else
            {
                // proveri sta je uneto
                if(cbComm.Visibility == Visibility.Hidden)
                    valString = this.tbComm.Text.Trim();
                else
                    valString = this.cbComm.Text.Trim();
                if (valString != "")
                {
                    if (devType == "AnaOut" || devType == "AnaObj")
                    {
                        try
                        {
                            double val = Convert.ToDouble(valString);
                            inputOK = true;
                            valString = val.ToString();
                        }
                        catch (Exception)
                        {
                            //Debug.WriteLine(ex);
                            inputOK = false;
                        }
                    }
                    else if (devType == "Digital" || devType == "DigObj")
                    {
                        inputOK = true;
                    }
                }
            }

            if (inputOK)
            {
                command = "cmd" + " ; " + RtuTextBlock.Text + " ; " + NameTextBlock.Text + " ; " + valString;
                this.DialogResult = true;
            }
            else
            {
                MessageBox.Show("Uneta vrednost nije ispravna!\nNeuspelo slanje komande!", "Greska!!!", MessageBoxButton.OK, MessageBoxImage.Error);
                this.DialogResult = false;
            }
        }

        private void cancelButton_Click(object sender, RoutedEventArgs e)
        {
            this.DialogResult = false;
        }

        //private List<string> getAllCommands1()
        //{
        //    List<string> lista = new List<string>();
        //    foreach (var dc in MainWindow.DcCatalog)
        //    {
        //        lista.Add(dc.command);
        //    }
        //    return lista;
        //}

    }
}
