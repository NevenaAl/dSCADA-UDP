using System.ComponentModel;

namespace WpfDSCADA.Model
{
   public enum cmd_type { CMD_NONE = 0, CMD_DIG, CMD_ANA, CMD_CNT }

   public class ProcVar : INotifyPropertyChanged
   {
      private string devType;
      private string rtu;
      private string name;

      private string description;
      private string rtuStatus;
      private string stateOrVal;
      private string status;
      private string euOrComm;
      private string timeStamp;
      public string tags;
      public bool Active;
      public bool ManValue;
      public bool EventInh;
      public bool CmdInh;
      public bool ManCmdInh;

      public cmd_type CmdType;

      // Declare the event 
      public event PropertyChangedEventHandler PropertyChanged;

      #region Ctors

      public ProcVar( string[] split )
      {
         // split[0] prekacemo jer je to tip poruke ("init")
         this.devType = split[1];
         this.rtu = split[2];
         this.rtuStatus = "on";
         this.name = split[3];
         this.description = split[4];
         this.status = split[5];
         this.stateOrVal = split[6];
         this.euOrComm = split[7];
         this.timeStamp = split[8];
         // namesti OPR flagove
         Active = ManValue = EventInh = CmdInh = ManCmdInh = false;
      }

      #endregion

      #region properties {get; set;}

      public string Name
      {
         get { return name; }
         set
         {
            name = value;
            OnPropertyChanged("Name");
         }
      }

      public string Tags
      {
         get { return tags; }
         set
         {
            tags = value;
            OnPropertyChanged("Tags");
         }
      }

      public string Description
      {
         get { return description; }
         set
         {
            description = value;
            OnPropertyChanged("Description");
         }
      }

      public string Rtu
      {
         get { return rtu; }
         set
         {
            rtu = value;
            OnPropertyChanged("Rtu");
         }
      }

      public string RtuStatus
      {
         get { return rtuStatus; }
         set
         {
            rtuStatus = value;
            OnPropertyChanged("RtuStatus");
         }
      }

      public string DevType
      {
         get { return devType; }
         set
         {
            devType = value;
            OnPropertyChanged("DevType");
         }
      }

      public string StateOrVal
      {
         get { return stateOrVal; }
         set
         {
            stateOrVal = value;
            OnPropertyChanged("StateOrVal");
         }
      }

      public string Status
      {
         get { return status; }
         set
         {
            status = value;
            OnPropertyChanged("Status");
         }
      }

      public string EuOrComm
      {
         get { return euOrComm; }
         set
         {
            euOrComm = value;
            OnPropertyChanged("EuOrComm");
         }
      }

      public string TimeStamp
      {
         get { return timeStamp; }
         set
         {
            timeStamp = value;
            OnPropertyChanged("TimeStamp");
         }
      }

      #endregion

      protected void OnPropertyChanged(string name)
      {
         PropertyChangedEventHandler handler = PropertyChanged;
         if (handler != null)
         {
            handler(this, new PropertyChangedEventArgs(name));
         }
      }

   }
}
