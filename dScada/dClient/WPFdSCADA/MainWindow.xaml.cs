using System;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO;
using System.IO.Pipes;
using System.Threading;
using System.Windows;
using System.Windows.Media;
using System.Net.Sockets;
using System.Windows.Threading;
using WpfDSCADA.Model;
using WpfDSCADA.SVG;
using System.Collections.Generic;
using WpfDSCADA.Catalogs;
using System.Windows.Media.Imaging;
using System.Diagnostics;
using System.Media;
using System.Text;
using System.Net;
using System.Linq.Expressions;

namespace WpfDSCADA
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window, INotifyPropertyChanged
    {

        #region INotifyPropertyChanged

        /// <summary>
        /// Occurs when a property value changes
        /// </summary>
        public event PropertyChangedEventHandler PropertyChanged;

        /// <summary>
        /// Raise the  <see cref="PropertyChanged"/> event.
        /// </summary>

        /// <param name="propertyName"></param>
        protected void NotifyPropertyChange(string propertyName)
        {
            if (PropertyChanged != null)

                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
        }
        #endregion

        private ObservableCollection<ProcVar> _procVarList = new ObservableCollection<ProcVar>();

        public ObservableCollection<ProcVar> ProcVarList
        {
            get { return _procVarList; }
            set
            {
                _procVarList = value;
                NotifyPropertyChange("ProcVarList");
            }
        }

        private ObservableCollection<AUBEvent> _aubEventList = new ObservableCollection<AUBEvent>();

        public ObservableCollection<AUBEvent> AUBEventList
        {
            get { return _aubEventList; }
            set
            {
                _aubEventList = value;
                NotifyPropertyChange("AUBEventList");
            }
        }

        string StaName;         // Station Name
        bool initDone;
        ProcVar selectedDevice;
        static AutoResetEvent sendCommandEvent = new AutoResetEvent(false);
        public List<EuCatElement> EuCatalog = new List<EuCatElement>();
        static public List<DcCatElement> DcCatalog = new List<DcCatElement>();
        //public List<DcCatElement> DcCatalog = new List<DcCatElement>();
        public List<DsCatElement> DsCatalog = new List<DsCatElement>();

        string SvgFile;
        public SVGInterface svgInterface = new SVGInterface();
        public GraphicWindow graphWindow = new GraphicWindow();
        public Uri iconUri;

        static Socket ClientSocket = null;
        static bool ClientConnected;            // povezan sa AUBom
        public bool LoadDone;                   // napunjen od AUBa
        public int LastMessageReceived = 0;     // seq number of last received message

        #region constants

        private const int EVENT_LIST_MAX_COUNT = 50;
        private const int MSG_MAX_LEN = 15000;
        private const int DEFAULT_AUB_PORT = 8080;

        #endregion

        public MainWindow()
        {
            InitializeComponent();

            ReadConfig();
            Closing += new CancelEventHandler(MainWindow_Closing);

            Thread ThreadReadClientInst = new Thread(ThreadReadClient);
            ThreadReadClientInst.Start();

            Thread ThreadTimerInst = new Thread(ThreadTimer);
            ThreadTimerInst.Start();

            // comandni tab
            commandLabel.Visibility = Visibility.Hidden;
            commandTextBox.Visibility = Visibility.Hidden;
            commandComboBox.Visibility = Visibility.Hidden;
            sendButton.Visibility = Visibility.Hidden;
            // manual tab
            manualLabel.Visibility = Visibility.Hidden;
            manualSendButton.Visibility = Visibility.Hidden;
            manualInputTextBox.Visibility = Visibility.Hidden;
            manualInputComboBox.Visibility = Visibility.Hidden;
            manualCheckBox.Visibility = Visibility.Hidden;
            // tag tab
            actLabel.Visibility = Visibility.Hidden;
            actCheckBox.Visibility = Visibility.Hidden;
            eventLabel.Visibility = Visibility.Hidden;
            eventCheckBox.Visibility = Visibility.Hidden;
            cmdLabel.Visibility = Visibility.Hidden;
            cmdCheckBox.Visibility = Visibility.Hidden;
            mcmdLabel.Visibility = Visibility.Hidden;
            mcmdCheckBox.Visibility = Visibility.Hidden;
            tagSendButton.Visibility = Visibility.Hidden;

            graphWindow.graphWindowCommandEvHandler += new GraphicWindow.graphWindowCommandEventHandler(graphWindow_SendComand);
            graphWindow.callInitSVGPageEvHandler += new GraphicWindow.callInitSVGPageEventHandler(graphWindow_CallInitSVGPage);

            this.DataContext = this;
        }
        void MainWindow_Closing(object sender, CancelEventArgs e)
        {
            SendToHost("disconnected");
            ClientSocket.Close();
        }
        private void ReadConfig()
        {
            // Open the configuration file 
            try
            {
                using (StreamReader sr = new StreamReader("dConfig.txt"))
                {
                    string s = "";
                    while ((s = sr.ReadLine()) != null)
                    {
                        if (s.StartsWith("Station") || s.StartsWith("SvgFile"))
                        {
                            s = s.Replace('\t', ' ');
                            s = s.Trim();
                            char[] charSeparators = new char[] { ' ' };
                            string[] split = s.Split(charSeparators, StringSplitOptions.RemoveEmptyEntries);
                            if (split[0].Equals("Station"))
                            {
                                StaName = split[1];
                                this.Title += StaName;
                            }
                            else if (split[0].Equals("SvgFile"))
                            {
                                if (split[1].EndsWith(".svg"))
                                    SvgFile = "SVG files\\" + split[1];
                            }
                        }
                    }
                }
            }
            catch (Exception e)
            {
                MessageBox.Show("ReadCfgFile:\n\n" + e.Message, "dClient");
                Close();
            }
        }

        #region communication threads
        public void ThreadReadClient(object obj)
        {
            byte[] RxBuff = new byte[MSG_MAX_LEN];

            while (true)
            {
                if (ClientSocket == null)
                {
                    // napravi socket za vezu sa AUB-om
                    ClientSocket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp)
                    {
                        //NoDelay = true,
                        //SendBufferSize = 1
                    };
                    IPAddress mcIP;  // Multicast group to join
                    int mcPort;   // Port to receive on
                    mcPort = 9995;
                    mcIP = IPAddress.Parse("238.255.255.255");
                    ClientSocket.SetSocketOption(SocketOptionLevel.Socket,
                                     SocketOptionName.ReuseAddress,
                                     1);
                    // Create an IPEndPoint and bind to it
                    IPEndPoint ipep = new IPEndPoint(IPAddress.Any, mcPort);
                    ClientSocket.Bind(ipep);

                    // Add membership in the multicast group
                    ClientSocket.SetSocketOption(SocketOptionLevel.IP,
                                         SocketOptionName.AddMembership,
                                         new MulticastOption(mcIP, IPAddress.Any)); 

                  
                }

                if (!ClientSocket.Connected)
                {
                    // pokusaj konekciju na AUB, na prvu ili drugu host masinu
                    try
                    {
                        ClientSocket.Connect("localhost", DEFAULT_AUB_PORT);
                        // uspesna konekcija na Host
                        ClientConnected = true;
                        // izbrisi sve iz listi 
                        Dispatcher.Invoke(DispatcherPriority.Normal, new DelegateClearAllCollections(clearAllCollections));
                        // oznaci konekciju promenom ikonice
                        Dispatcher.Invoke(new DelegateUpdateIcon(updateIcon));

                        SendToHost("connected");
                        
                    }
                    catch (ArgumentNullException ane)
                    {
                        MessageBox.Show("ArgumentNullException : {0}", ane.ToString());
                    }
                    catch (SocketException se)
                    {
                        if (se.ErrorCode != 10061)                  // Connection Refused Error
                        {
                            MessageBox.Show("ConnectException: " + se.Message);
                        }
                        Thread.Sleep(1000);
                    }
                    catch (Exception e)
                    {
                        //Thread.Sleep(1000);
                        MessageBox.Show("Unexpected exception : {0}", e.ToString());
                    }
                }

                if (ClientConnected)
                {
                    
                    try
                    {
                        int rsts = ClientSocket.Receive(RxBuff, 0, MSG_MAX_LEN, SocketFlags.None);
                        
                        if (rsts <= 0)
                        {
                            ClientConnected = false;
                            Dispatcher.Invoke(new DelegateUpdateIcon(updateIcon));      // namesti crvenu ikonicu
                            ClientSocket.Shutdown(SocketShutdown.Both);                 // Release the socket.  
                            ClientSocket.Close();
                            ClientSocket = null;
                        }
                        else
                        {
                            string msg = Encoding.ASCII.GetString(RxBuff, 4, rsts-4);
                            Dispatcher.Invoke(DispatcherPriority.Normal, new DelegateProcessMsg(processMsg), msg);

                            int msgSeqNumber = 0; 
                            byte[] tempBuff = new byte[4];
                            tempBuff[0] = RxBuff[0];
                            tempBuff[1] = RxBuff[1];
                            tempBuff[2] = RxBuff[2];
                            tempBuff[3] = RxBuff[3];
                            msgSeqNumber = BitConverter.ToInt32(tempBuff,0);
                            if (LastMessageReceived != 0)
                            {
                                int NextMessageToReceive = LastMessageReceived + 1;
                                if (NextMessageToReceive != msgSeqNumber)
                                {
                                    SendToHost("NACK:" + NextMessageToReceive.ToString());
                                }
                            }
                            LastMessageReceived = msgSeqNumber;
                        }
                    } catch (SocketException se)
                    {
                        switch (se.ErrorCode)
                        {
                            case 10054:
                            case 10060:
                            //break;
                            default:
                                // konekcija je prekinuta
                                ClientConnected = false;
                                Dispatcher.Invoke(new DelegateUpdateIcon(updateIcon));      // namesti crvenu ikonicu
                                ClientSocket.Shutdown(SocketShutdown.Both);                 // Release the socket.  
                                ClientSocket.Close();
                                ClientSocket = null;
                                break;
                        }
                    }
                }
                //Application.Current.Dispatcher.Invoke(new Action(() => printEvent(ClientSocket.Connected.ToString())));
                //Thread.Sleep(500);
            }
        }

        static public void SendToHost(string str)
        {
            if (MainWindow.ClientConnected)
            {
                byte[] msg = Encoding.ASCII.GetBytes(str + "\n");
                //byte[] msg = Encoding.Unicode.GetBytes(str + "\n");
                ClientSocket.Send(msg);
            }
        }


        #endregion

        // po rekonekciji veze sa AUB-om brise konfiguraciju pre novog punjenja
        public void clearAllCollections()
        {
            // liste promenljivih, kataloga i eventa
            ProcVarList.Clear();
            AUBEventList.Clear();
            DcCatalog.Clear();
            DsCatalog.Clear();
            EuCatalog.Clear();
            if (svgInterface.svgPage != null)
            {
                // grafika
                svgInterface.svgPage.shape_list.Clear();
                svgInterface.svgParser.page_list.Clear();
                graphWindow.Close();
                graphWindow.graphWindowInitFlag = false;
                //clear Event Handlers
                graphWindow.svgInterface.SvgShapeClicked -= new SVGInterface.SvgShapeClickedEventHandler(graphWindow.svgShapeClicked);
                graphWindow.svgInterface.SvgShapeRightClicked -= new SVGInterface.SvgShapeRightClickedEventHandler(graphWindow.svgShapeRightClicked);
            }
            initDone = false;
        }

        void setCmdType(ProcVar dev)
        {
            dev.CmdType = cmd_type.CMD_NONE;
            switch (dev.DevType)
            {
                case "Digital":
                case "DigObj":
                    if (!dev.EuOrComm.Equals("NEMA_IZLAZA"))
                    {
                        dev.CmdType = cmd_type.CMD_DIG;
                    }
                    break;
                case "Counter":
                    dev.CmdType = cmd_type.CMD_CNT;
                    break;
                case "AnaOut":
                case "AnaObj":
                    dev.CmdType = cmd_type.CMD_ANA;
                    break;
                case "AnaIn":
                default:
                    break;
            }
        }

        // obrada poruke sa pipe-a
        public void processMsg(string msg)
        {
            try
            {
                string[] split = msg.Split(';');

                switch (split[0])
                {
                    case "init_start":
                        if (!initDone)
                        {
                            ProcVarList.Clear();
                            Grid0.Visibility = Visibility.Hidden;
                            initDone = false;
                        }
                       
                        break;
                    case "init":
                        if (!initDone)
                        {
                            ProcVar devTemp = new ProcVar(split);
                            setCmdType(devTemp);
                            ProcVarList.Add(devTemp);
                        }
                        break;
                    case "init_done\n":
                        if (!initDone)
                        {
                            Grid0.Visibility = Visibility.Visible;
                            // napravi potrebne combo kutije
                            commandComboBox.ItemsSource = getAllCommands();
                            manualInputComboBox.ItemsSource = getAllStates();
                            initDone = true;
                        }
                        break;
                    case "tags":
                        UpdateDevTags(split);                              // osvezavanje tagova PVars
                        break;
                    case "refresh":
                        if (initDone)
                        {
                            UpdateDevList(split);                              // osvezavanje liste PVars
                            if (graphWindow.graphWindowInitFlag)
                            {
                                UpdateSVGPage(split);                           // osvezavanje grafike
                            }
                        }
                        break;
                    case "event":
                        PutEvent(split[1]);
                        break;
                    case "rtu":
                        updateRtuStatus(split);
                        break;
                    case "cat":
                        updateCatalogues(split);
                        break;
                    default:
                        break;
                }
            }
            catch (Exception e)
            {
                MessageBox.Show("processMsg:\n\n" + e.Message, "dClient");
            }

        }


        public void PutEvent(string event_msg)
        {
            AUBEvent aubEventTemp = new AUBEvent(event_msg);
            AUBEventList.Insert(0, aubEventTemp);
            if (AUBEventList.Count == EVENT_LIST_MAX_COUNT + 1)
            {
                AUBEventList.RemoveAt(EVENT_LIST_MAX_COUNT);
            }
        }

        static public List<string> getAllCommands()
        {
            List<string> lista = new List<string>();
            foreach (var dc in DcCatalog)
            {
                lista.Add(dc.command);
            }
            return lista;
        }

        private List<string> getAllStates()
        {
            List<string> lista = new List<string>();
            foreach (var ds in DsCatalog)
            {
                lista.Add(ds.state);
            }
            return lista;
        }


        #region graphic support

        public void InitSVGPage()
        {
            if ((svgInterface.svgPage == null) || (svgInterface.svgPage.shape_list.Count == 0))
                return;

            foreach (SVGShape s in svgInterface.svgPage.shape_list)                    // za svaki shape
            {
                SetShapeColor(s);                                                       // namesti boju prikaza
                if (s.TypeOfShape == SVGSHAPETYPE.SVG_LABEL)                         // ako treba i tekstualnu vrednost
                {
                    foreach (ProcVar d in ProcVarList)
                    {
                        if (s.cp.pvid.Equals(String.Format("{0}:{1}", d.Rtu, d.Name)))    //nadji isti pvid u devices
                        {
                            switch (d.DevType)
                            {
                                case "Digital":
                                case "DigObj":
                                    if (d.StateOrVal.Contains("NEMA_ULAZA"))
                                        s.Text.text = String.Format("{0}", d.EuOrComm);
                                    else
                                        s.Text.text = String.Format("{0}", d.StateOrVal);
                                    break;
                                case "AnaIn":
                                case "AnaOut":
                                case "AnaObj":
                                case "Counter":
                                    s.Text.text = String.Format("{0} {1}", d.StateOrVal, d.EuOrComm);
                                    break;
                                default:
                                    s.Text.text = "???";
                                    break;
                            }
                        }
                    }
                }
            }
            return;
        }

        private void UpdateSVGPage(string[] split)
        {
            // mada je ovo suvisna provera
            if ((svgInterface.svgPage == null) || (svgInterface.svgPage.shape_list.Count == 0))
                return;

            foreach (SVGShape s in svgInterface.svgPage.shape_list)
            {
                // set color
                SetShapeColor(s);
                // and text if required
                if (s.cp.pvid.Equals(String.Format("{0}:{1}", split[1], split[2])))
                {
                    if (s.TypeOfShape == SVGSHAPETYPE.SVG_LABEL)
                    {
                        string[] pvid = new string[2] { split[1], split[2] };
                        ProcVar dev = graphWindow.FindProcVar(pvid);
                        switch (dev.DevType)
                        {
                            case "Digital":
                            case "DigObj":
                                if (split[4].Contains("NEMA_ULAZA"))
                                {
                                    s.Text.text = String.Format("{0}", split[5]);
                                }
                                else
                                {
                                    s.Text.text = String.Format("{0}", split[4]);
                                }
                                break;
                            case "AnaIn":
                            case "AnaOut":
                            case "AnaObj":
                            case "Counter":
                                s.Text.text = String.Format("{0} {1}", split[4], dev.EuOrComm);
                                break;
                            default:
                                s.Text.text = "???";
                                break;
                        }
                    }
                }
            }
        }

        // trazi PVar u listi promenljivih
        private ProcVar FindProcVar(string pvid)
        {
            string[] splitPVID = pvid.Split(':');
            foreach (ProcVar d in ProcVarList)
            {
                if (splitPVID[0].Equals(d.Rtu) && splitPVID[1].Equals(d.Name))
                    return d;
            }
            return null;
        }

        private void SetShapeColor(SVGShape s)
        {
            if (s.cp.pvid == "")                      // nema razloga da se kreci
                return;

            ProcVar d;
            switch (s.TypeOfShape)
            {
                case SVGSHAPETYPE.SVG_SYMBOL:
                    d = FindProcVar(s.cp.pvid);
                    if (d != null)
                    {
                        SetSymbolColor(s, d);
                    }
                    break;
                case SVGSHAPETYPE.SVG_LABEL:
                    d = FindProcVar(s.cp.pvid);
                    if (d != null)
                    {
                        SetLabelColor(s, d);
                    }
                    break;
            }
        }

        private void SetSymbolColor(SVGShape s, ProcVar d)
        {
            s.colorCode = Brushes.White.ToString();
            switch (d.DevType)
            {
                //  mapiramo boje po stanju kao za data grid:            LawnGreen: Normal
                case "Counter":                                 //       Magenta: !Valid            
                case "AnaIn":                                   //       Tomato: LoLow, HiHigh, Alarm, Stall
                case "AnaOut":                                  //       Red: High, Low
                case "AnaObj":                                  //       Violet: ORL, LoClmp, HiClmp
                    switch (d.Status)
                    {
                        case "Normal":
                            s.colorCode = Brushes.LawnGreen.ToString();
                            break;
                        case "!Valid":
                            s.colorCode = Brushes.Magenta.ToString();
                            break;
                        case "ORL":
                        case "LoClmp":
                        case "HiClmp":
                            s.colorCode = Brushes.Violet.ToString();
                            break;
                        case "HiHigh":
                        case "LoLow":
                        case "Alarm":
                        case "Stall":
                            s.colorCode = Brushes.Tomato.ToString();
                            break;
                        default:
                            break;
                    }
                    break;
                case "Digital":
                case "DigObj":
                    // prvo proveri ima li ekstra pravila za osvezavanja koje je TRUE!
                    if (s.cp.refRules.Count > 0)
                    {
                        foreach (SVGREFRULE rrule in s.cp.refRules)
                        {
                            if (rrule.state.Equals(d.StateOrVal) || rrule.state.Equals(d.EuOrComm))
                            {
                                if (rrule.color != null)
                                {
                                    s.colorCode = rrule.color.ToString();
                                    return;
                                }
                                if (rrule.blinkColor1 != null)
                                {
                                    return;
                                }
                            }
                        }
                    }
                    // default farbanje
                    if (d.StateOrVal.Contains("NEMA_ULAZA"))
                    {                                                   // ako nema ulaza bojimo po komandi, a ona je u euCom
                        foreach (DcCatElement dcCatEl in DcCatalog)
                        {
                            if (dcCatEl.command.Equals(d.EuOrComm))
                            {
                                s.colorCode = dcCatEl.color.ToString();
                                break;
                            }
                        }
                    }
                    else
                    {                                                   // ima ulazr, farbamo po stanju
                        foreach (DsCatElement dsCatEl in DsCatalog)
                        {
                            if (dsCatEl.state.Equals(d.StateOrVal))
                            {
                                s.colorCode = dsCatEl.color.ToString();
                                break;
                            }
                        }
                    }
                    break;

                default:
                    break;
            }
        }

        private void SetLabelColor(SVGShape s, ProcVar d)
        {
            switch (d.DevType.ToString())
            {
                case "Digital":
                case "DigObj":
                    if (d.StateOrVal.Contains("NEMA_ULAZA"))
                    {
                        s.colorCode = GetDcColor(d.EuOrComm);
                    }
                    else
                    {
                        s.colorCode = GetDsColor(d.StateOrVal);
                    }
                    break;
                case "AnaIn":
                case "AnaOut":
                case "AnaObj":
                case "Counter":
                    s.colorCode = GetEuColor(d.EuOrComm);
                    break;
                default:
                    s.colorCode = Brushes.White.ToString();
                    break;
            }
            if (d.Status.Contains("Manual") || d.Status.Contains("Fail"))
            {
                s.InvertColor = false;
                s.Italic = true;
            }
            else if (d.Status.Contains("Normal"))
            {
                s.InvertColor = false;
                s.Italic = false;
            }
            else
            {
                s.InvertColor = true;
                s.Italic = false;
            }
        }

        public String GetEuColor(string eu)
        {
            String ret = Brushes.White.ToString();

            foreach (EuCatElement euCatEl in EuCatalog)
            {
                if (euCatEl.eu.Equals(eu))
                {
                    ret = euCatEl.color.ToString();
                    break;
                }
            }
            return ret;
        }

        public String GetDcColor(string dc)
        {
            String ret = Brushes.White.ToString();

            foreach (DcCatElement dcCatEl in DcCatalog)
            {
                if (dcCatEl.command.Equals(dc))
                {
                    ret = dcCatEl.color.ToString();
                    break;
                }
            }
            return ret;
        }

        public String GetDsColor(string ds)
        {
            String ret = Brushes.White.ToString();

            foreach (DsCatElement dsCatEl in DsCatalog)
            {
                if (dsCatEl.state.Equals(ds))
                {
                    ret = dsCatEl.color.ToString();
                    break;
                }
            }
            return ret;
        }

        static public Brush StringToBrush(string s)
        {
            // pustimo sve C# boje da se koriste
            Brush color;
            try
            {
                color = (Brush)new BrushConverter().ConvertFromString(s);
            }
            catch (Exception e)
            {
                MessageBox.Show("StringToBrush:\n\n" + e.Message, "dClient");
                color = Brushes.White;
            }
            return color;
        }
        #endregion

        #region update lists and labels

        void ShowTags(ProcVar dev)
        {
            // pripremi indikaciju da je promenljiva oznacena
            if (!dev.Active || dev.EventInh || dev.CmdInh || dev.ManCmdInh)
                dev.Tags = "#";
            else
                dev.Tags = "";
        }

        public void UpdateDevTags(string[] split)
        {
            foreach (ProcVar devTemp in this.ProcVarList)
            {
                if (devTemp.Name.Equals(split[2]) && devTemp.Rtu.Equals(split[1]))
                {
                    for (int i = 0; i < 5; i++)
                    {
                        bool flag = false;
                        if (split[i + 3] == "on")
                            flag = true;
                        switch (i)
                        {
                            case 0:
                                devTemp.Active = flag;
                                break;
                            case 1:
                                devTemp.ManValue = flag;
                                break;
                            case 2:
                                devTemp.EventInh = flag;
                                break;
                            case 3:
                                devTemp.CmdInh = flag;
                                break;
                            case 4:
                                devTemp.ManCmdInh = flag;
                                break;
                        }
                    }
                    ShowTags(devTemp);
                }
            }
        }

        public void UpdateDevList(string[] split)
        {
            foreach (ProcVar devTemp in this.ProcVarList)
            {
                if (devTemp.Name.Equals(split[2]) && devTemp.Rtu.Equals(split[1]))
                {
                    if (devTemp.DevType.Equals("Digital") || devTemp.DevType.Equals("DigObj"))
                    {
                        devTemp.Status = split[3];
                        devTemp.StateOrVal = split[4];
                        devTemp.EuOrComm = split[5];
                        devTemp.TimeStamp = split[6];
                    }
                    else
                    {
                        // jer se EU polje ne menja, pa se ni ne osvezava
                        devTemp.Status = split[3];
                        devTemp.StateOrVal = split[4];
                        devTemp.TimeStamp = split[5];
                    }
                }
            }
        }

        private void updateCatalogues(string[] split)
        {
            switch (split[1])
            {
                case "eu":
                    EuCatalog.Add(new EuCatElement(split[2], StringToBrush(split[3])));
                    break;
                case "ds":
                    DsCatalog.Add(new DsCatElement(split[2], StringToBrush(split[3])));
                    break;
                case "dc":
                    DcCatalog.Add(new DcCatElement(split[2], StringToBrush(split[3])));
                    break;
                default:
                    break;
            }
        }

        private void updateRtuStatus(string[] split)
        {
            foreach (ProcVar devTemp in this.ProcVarList)
            {
                if (devTemp.Rtu.Equals(split[2]))
                {
                    devTemp.RtuStatus = split[1];
                }
            }
        }

        public void updateIcon()
        {
            if (ClientConnected)
            {
                iconUri = new Uri("pack://application:,,,/WpfDSCADA;component/Icons/greenIcon.ico", UriKind.RelativeOrAbsolute);
                this.Icon = BitmapFrame.Create(iconUri);
            }
            else
            {
                iconUri = new Uri("pack://application:,,,/WpfDSCADA;component/Icons/redIcon.ico", UriKind.RelativeOrAbsolute);
                this.Icon = BitmapFrame.Create(iconUri);
            }
        }

        #endregion

        #region Send command functions

        private void buttonSend_Click(object sender, RoutedEventArgs e)
        {
            ProcVar dev = (ProcVar)dataGrid.SelectedItem;

            bool inputOK = false;
            string valString = "";

            if (this.commandTextBox.Text.Trim() != "" || this.commandComboBox.Text.Trim() != "")
            {
                switch (dev.DevType)
                {
                    case "Counter":
                        inputOK = true;
                        valString = "0";
                        break;

                    case "AnaObj":
                    case "AnaOut":
                        try
                        {
                            double val = Convert.ToDouble(this.commandTextBox.Text.Trim());
                            inputOK = true;
                            valString = val.ToString();
                        }
                        catch (Exception)
                        {
                            inputOK = false;
                        }
                        break;
                    case "DigObj":
                    case "Digital":
                        valString = this.commandComboBox.Text.Trim();
                        inputOK = true;
                        break;
                    default:
                        break;
                }
                if (inputOK)
                {
                    SendToHost(string.Format("cmd ; {0} ; {1} ; {2}", dev.Rtu, dev.Name, valString));
                    return;
                }
            }
            MessageBox.Show("Uneta vrednost nije ispravna!\nNeuspelo slanje komande!", "Greška!!!", MessageBoxButton.OK, MessageBoxImage.Error);
        }

        private void manualSendButton_Click(object sender, RoutedEventArgs e)
        {
            if (this.manualInputTextBox.Text.Trim() != "" || this.manualInputComboBox.Text.Trim() != "")
            {
                ProcVar dev = (ProcVar)dataGrid.SelectedItem;
                bool inputOK = false;
                string valString = "";

                switch (dev.DevType)
                {
                    case "Counter":
                    case "AnaObj":
                    case "AnaOut":
                    case "AnaIn":
                    default:
                        try
                        {
                            double val = Convert.ToDouble(this.manualInputTextBox.Text.Trim());
                            inputOK = true;
                            valString = val.ToString();
                        }
                        catch (Exception)
                        {
                            inputOK = false;
                        }
                        break;
                    case "DigObj":
                    case "Digital":
                        valString = this.manualInputComboBox.Text.Trim();
                        inputOK = true;
                        break;
                }
                if (inputOK)
                {
                    string manToSend = string.Format("man ; {0} ; {1} ; {2} ; ", dev.Rtu, dev.Name, valString);
                    if ((bool)manualCheckBox.IsChecked)
                    {
                        manToSend += "on";
                    }
                    else
                    {
                        manToSend += "off";
                    }
                    // zahtevaj slanje
                    SendToHost(manToSend);
                    return;
                }
            }
            MessageBox.Show("Neuspelo slanje ručno unete vrednosti!", "Greška!!!", MessageBoxButton.OK, MessageBoxImage.Error);
        }

        private void tagSendButton_Click(object sender, RoutedEventArgs e)
        {
            ProcVar dev = (ProcVar)dataGrid.SelectedItem;

            string tagToSend = "tag" + " ; " + dev.Rtu + " ; " + dev.Name;
            for (int i = 0; i < 4; i++)
            {
                bool TagValue = false;
                switch (i)
                {
                    case 0: TagValue = dev.Active = (bool)actCheckBox.IsChecked; break;
                    case 1: TagValue = dev.EventInh = (bool)eventCheckBox.IsChecked; break;
                    case 2: TagValue = dev.CmdInh = (bool)cmdCheckBox.IsChecked; break;
                    case 3: TagValue = dev.ManCmdInh = (bool)mcmdCheckBox.IsChecked; break;
                }
                if (TagValue)
                    tagToSend += " ; on";
                else
                    tagToSend += " ; off";
            }
            // zahtevaj slanje
            SendToHost(tagToSend);

            // prikazi oznake
            ShowTags(dev);
        }

        private void dataGrid_SelectionChanged(object sender, System.Windows.Controls.SelectionChangedEventArgs e)
        {
            // odredi selekciju
            selectedDevice = (ProcVar)dataGrid.SelectedItem;

            if (selectedDevice == null)
            {
                // command tab
                commandLabel.Visibility = commandTextBox.Visibility = commandComboBox.Visibility = Visibility.Hidden;
                sendButton.Visibility = Visibility.Hidden;
                // manual tab
                manualLabel.Visibility = manualInputTextBox.Visibility = manualInputComboBox.Visibility = manualCheckBox.Visibility = Visibility.Hidden;
                manualSendButton.Visibility = Visibility.Hidden;
                // tag tab
                actLabel.Visibility = actCheckBox.Visibility = Visibility.Hidden;
                eventLabel.Visibility = eventCheckBox.Visibility = Visibility.Hidden;
                cmdLabel.Visibility = cmdCheckBox.Visibility = Visibility.Hidden;
                mcmdLabel.Visibility = mcmdCheckBox.Visibility = Visibility.Hidden;
                tagSendButton.Visibility = Visibility.Hidden;
                // gotovo
                return;
            }
            // nesto je selektovano, nastavi 

            // tag tab, ovo ima svaka SCADA promenljiva
            actCheckBox.IsChecked = selectedDevice.Active;
            actLabel.Visibility = actCheckBox.Visibility = Visibility.Visible;
            eventCheckBox.IsChecked = selectedDevice.EventInh;
            eventLabel.Visibility = eventCheckBox.Visibility = Visibility.Visible;
            tagSendButton.Visibility = Visibility.Visible;

            // manual opciju ima svaka promenljiva koja nije "NEMA_ULAZA", iniciraj prema izboru 
            if (!selectedDevice.StateOrVal.Equals("NEMA_ULAZA"))
            {
                // napuni pocetno stanje
                switch (selectedDevice.DevType)
                {
                    case "DigObj":
                    case "Digital":
                        manualInputComboBox.SelectedValue = selectedDevice.StateOrVal;
                        manualInputComboBox.Visibility = Visibility.Visible;
                        manualInputTextBox.Visibility = Visibility.Hidden;
                        break;
                    case "AnaIn":
                    case "Counter":
                    case "AnaOut":
                    case "AnaObj":
                    default:
                        manualInputTextBox.Text = selectedDevice.StateOrVal;
                        manualInputTextBox.Visibility = Visibility.Visible;
                        manualInputComboBox.Visibility = Visibility.Hidden;
                        break;
                }
                // otvori checkbox
                if (selectedDevice.ManValue)
                    manualCheckBox.IsChecked = true;
                else
                    manualCheckBox.IsChecked = false;
                // prikazi na dijalogu         
                manualLabel.Visibility = manualCheckBox.Visibility = Visibility.Visible;
                manualSendButton.Visibility = Visibility.Visible;
            }
            else
            {
                // skloni dugmice i text box
                manualLabel.Visibility = manualInputTextBox.Visibility = manualInputComboBox.Visibility = manualCheckBox.Visibility = Visibility.Hidden;
                manualSendButton.Visibility = Visibility.Hidden;
            }

            // za komande moramo proveriti i podesiti
            if (selectedDevice.CmdType == cmd_type.CMD_NONE)
            {
                commandLabel.Visibility = commandTextBox.Visibility = commandComboBox.Visibility = Visibility.Hidden;
                sendButton.Visibility = Visibility.Hidden;
                // Cmd tag-ovi se takodje sklanjaju
                cmdLabel.Visibility = cmdCheckBox.Visibility = Visibility.Hidden;
                mcmdLabel.Visibility = mcmdCheckBox.Visibility = Visibility.Hidden;
                return;
            }

            // izabrana promenljiva ima komande, namesti na Analog (TextBox) ili Digital (ComboBox)
            if (selectedDevice.CmdType == cmd_type.CMD_DIG)
            {
                commandTextBox.Visibility = Visibility.Hidden;
                commandComboBox.SelectedValue = selectedDevice.EuOrComm;
                commandComboBox.Visibility = Visibility.Visible;
                commandComboBox.IsEnabled = true;
                sendButton.Content = "Pošalji";
            }
            else if (selectedDevice.CmdType == cmd_type.CMD_ANA)
            {
                commandComboBox.Visibility = Visibility.Hidden;
                // preuzmi tekucu vrednost
                commandTextBox.Text = selectedDevice.StateOrVal;
                commandTextBox.Visibility = Visibility.Visible;
                commandTextBox.IsEnabled = true;
                sendButton.Content = "Pošalji";
            }
            else if (selectedDevice.CmdType == cmd_type.CMD_CNT)
            {
                commandComboBox.Visibility = Visibility.Hidden;
                commandTextBox.Text = "0.00";
                commandTextBox.Visibility = Visibility.Visible;
                commandTextBox.IsEnabled = false;
                sendButton.Content = "Obriši";
            }
            // prikazi sve sto treba, prvo za komandu 
            commandLabel.Visibility = Visibility.Visible;
            sendButton.Visibility = Visibility.Visible;
            sendButton.IsEnabled = true;
            // potom namesti i prikazi tagove
            cmdCheckBox.IsChecked = selectedDevice.CmdInh;
            cmdLabel.Visibility = cmdCheckBox.Visibility = Visibility.Visible;
            mcmdCheckBox.IsChecked = selectedDevice.ManCmdInh;
            mcmdLabel.Visibility = mcmdCheckBox.Visibility = Visibility.Visible;

        }
        #endregion

        private void showGraphicButton_Click(object sender, RoutedEventArgs e)
        {
            if (graphWindow.graphWindowInitFlag == false)
            {
                graphWindow.svgInterface = svgInterface;
                graphWindow.svgInterface.SvgShapeClicked += new SVGInterface.SvgShapeClickedEventHandler(graphWindow.svgShapeClicked);
                graphWindow.svgInterface.SvgShapeRightClicked += new SVGInterface.SvgShapeRightClickedEventHandler(graphWindow.svgShapeRightClicked);
                graphWindow.ProcVarList = ProcVarList;
                //svgInterface.OpenSVGFromFile("", graphWindow.canvas, true);
                svgInterface.SVGRoot(SvgFile, graphWindow.canvas);
                if (svgInterface.svgPage != null)
                {
                    InitSVGPage();
                    graphWindow.Show();
                    graphWindow.graphWindowInitFlag = true;
                }
            }
            else
            {
                graphWindow.Visibility = Visibility.Visible;
                if (graphWindow.WindowState == WindowState.Minimized)
                {
                    graphWindow.WindowState = WindowState.Normal;
                }
            }
        }

        AUBEvent currEvent = null;
        private void AUBEventSelectionChanged(object sender, System.Windows.Controls.SelectionChangedEventArgs e)
        {
            // odredi selekciju
            currEvent = (AUBEvent)EventsListBox.SelectedItem;
        }

        private void clearEventsButton_Click(object sender, RoutedEventArgs e)
        {
            if (currEvent == null)
            {
                AUBEventList.Clear();
            }
            else
            {
                AUBEventList.Remove(currEvent);
            }
        }
        private void closeClient_Click(object sender, RoutedEventArgs e)
        {

        }

        private void unselectDataGridButton_Click(object sender, RoutedEventArgs e)
        {
            dataGrid.SelectedIndex = -1;
            EventsListBox.SelectedIndex = -1;
        }

        #region Timer

        public Stopwatch sw = new Stopwatch();
        public void ThreadTimer(object obj)
        {
            //sw.Start();
            while (true)
            {
                Thread.Sleep(1000);
                Dispatcher.Invoke(DispatcherPriority.Normal, new DelegateUpdateGraphics(DrawPageWrapper));
            }
        }

        #endregion

        #region Delegates

        private delegate void DelegateProcessMsg(string msg);
        private delegate void DelegateUpdateIcon();
        private delegate void DelegateUpdateGraphics();
        private delegate void DelegateClearAllCollections();

        #endregion

        public void DrawPageWrapper()
        {
            //long t1, dt;
            if (svgInterface.svgPage != null)
            {
                //t1 = sw.ElapsedMilliseconds;

                UpdateSVG_TimeAtrrubuttes();
                svgInterface.DrawPage();

                //dt = sw.ElapsedMilliseconds - t1;
                //PutEvent("dT=" + dt.ToString());

            }
        }

        private void UpdateSVG_TimeAtrrubuttes()
        {
            foreach (SVGShape s in svgInterface.svgPage.shape_list)
            {
                if (s.TypeOfShape == SVGSHAPETYPE.SVG_SYMBOL)
                {
                    ProcVar d = FindProcVar(s.cp.pvid);
                    if (d != null)
                    {
                        if (s.cp.refRules.Count > 0)
                        {
                            foreach (SVGREFRULE rrule in s.cp.refRules)
                            {
                                if (rrule.state.Equals(d.StateOrVal) || rrule.state.Equals(d.EuOrComm))
                                {
                                    if (rrule.audioPath != null)
                                    {
                                        // Bata
                                        if (!rrule.stopwatch.IsRunning)
                                        {
                                            SoundPlayer player = new SoundPlayer("Audio files\\" + rrule.audioPath);
                                            player.Play();
                                            rrule.stopwatch.Start();
                                        }
                                        else if (rrule.stopwatch.ElapsedMilliseconds >= rrule.refreshTime * 1000 && graphWindow.IsVisible)
                                        {
                                            rrule.stopwatch.Reset();
                                        }
                                        // Milos
                                        //if (!rrule.stopwatch.IsRunning)
                                        //{
                                        //    rrule.stopwatch.Start();
                                        //}
                                        //if (rrule.stopwatch.ElapsedMilliseconds >= rrule.refreshTime * 1000 && graphWindow.IsVisible)
                                        //{
                                        //    SoundPlayer player = new SoundPlayer("Audio files\\" + rrule.audioPath);
                                        //    player.Play();
                                        //    rrule.stopwatch.Restart();
                                        //}
                                    }
                                    if (rrule.blinkColor1 != null)
                                    {
                                        if (s.colorCode.Equals(rrule.blinkColor1.ToString()))
                                        {
                                            s.colorCode = rrule.blinkColor2.ToString();
                                        }
                                        else
                                        {
                                            s.colorCode = rrule.blinkColor1.ToString();
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }


        private void Window_Closed(object sender, EventArgs e)
        {
            System.Environment.Exit(0);
        }

        void graphWindow_SendComand(object sender, EventArgs e, string gCommand)
        {
            SendToHost(gCommand);
        }

        void graphWindow_CallInitSVGPage(object sender, EventArgs e)
        {
            InitSVGPage();
        }


    }

}
