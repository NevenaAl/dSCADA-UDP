using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Media;

namespace WpfDSCADA.Catalogs
{
	public class DsCatElement
	{
		public string state;
		public Brush color;

		public DsCatElement(String state, Brush color)
		{
			this.state = state;
			this.color = color;
		}
	}
}
