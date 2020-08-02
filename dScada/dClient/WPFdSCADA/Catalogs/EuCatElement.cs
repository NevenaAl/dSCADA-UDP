using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Media;

namespace WpfDSCADA.Catalogs
{
	public class EuCatElement
	{
		public string eu;
		public Brush color;

		public EuCatElement(String eu, Brush color)
		{
			this.eu = eu;
			this.color = color;
		}
	}
}
