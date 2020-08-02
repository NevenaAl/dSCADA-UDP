using System.ComponentModel;

namespace WpfDSCADA.Model
{
    public class AUBEvent : INotifyPropertyChanged
    {
        private string eventMsg;

        // Declare the event 
        public event PropertyChangedEventHandler PropertyChanged;
        
        #region Ctors

        public AUBEvent()
        {
        }

        public AUBEvent(string eventMsg)
        {            
            this.eventMsg = eventMsg;
        }

        #endregion

        #region properties {get; set;}

        public string EventMsg
        {
            get { return eventMsg; }
            set
            {
                eventMsg = value;
                // Call OnPropertyChanged whenever the property is updated
                OnPropertyChanged("EventMsg");
            }
        }
        #endregion

        // Create the OnPropertyChanged method to raise the event 
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
