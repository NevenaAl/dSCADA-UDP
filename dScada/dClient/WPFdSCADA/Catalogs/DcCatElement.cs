using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Media;

namespace WpfDSCADA.Catalogs
{
	public class DcCatElement
	{
		public string command;
		public Brush color;

		public DcCatElement(String command, Brush color)
		{
			this.command = command;
			this.color = color;
		}
	}
}
