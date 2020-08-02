﻿using System.Collections.Generic;
using System.Windows.Media.Imaging;

namespace WpfDSCADA.SVG
{
   public class SVGPage
   {
      public double minx, miny, maxx, maxy;

      public List<SVGShape> shape_list;
      public List<SVGStyle> style_list;


      /// <summary>
      /// Constructor
      /// </summary>
      public SVGPage()
      {
         minx = 0;
         miny = 0;
         maxx = 500;
         maxy = 500;
         shape_list = new List<SVGShape>();
         style_list = new List<SVGStyle>();

      }

      /// <summary>
      /// Set Methods
      /// </summary>        
      public void SetMinX(double set_min_x)
      {
         minx = set_min_x;
      }

      public void SetMinY(double set_min_y)
      {
         miny = set_min_y;
      }

      public void SetMaxX(double set_max_x)
      {
         maxx = set_max_x;
      }

      public void SetMaxY(double set_max_y)
      {
         maxy = set_max_y;
      }

      /// <summary>
      /// Get Methods
      /// </summary>
      public double GetMinX()
      {
         return minx;
      }

      public double GetMinY()
      {
         return miny;
      }

      public double GetMaxX()
      {
         return maxx;
      }

      public double GetMaxY()
      {
         return maxy;
      }

   }
}