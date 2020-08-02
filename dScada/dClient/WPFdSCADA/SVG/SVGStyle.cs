using System;

namespace WpfDSCADA.SVG
{
   public struct SVGFont
   {
      public String family;
      public String size;
      public String style;
      public String weight;
      public String underline;
   }

   public struct SVGStroke
   {
      public String color;
      public double width;
   }

   /// <summary>
   /// CLASS SVGStyls
   /// </summary>
   public class SVGStyle
   {
      public String    name;
      public SVGStroke stroke;
      public bool      no_line;
      public SVGFont   font;
      public bool      fillFlag;
      public String    fillColor;

      /// <summary>
      /// Constructor
      /// </summary>
      public SVGStyle()
      {
         name         = "";
         stroke.width = 1.0;
         stroke.color = "#FFFFFFFF";   // default White  BATA!!
         font.family  = "";
         font.size    = "";
         font.style   = "";
         font.weight  = "";
         font.underline = "off";
         fillFlag     = false;
         fillColor    = "#FFFFFFFF";      // default White  BATA!!
      }

      /// <summary>
      /// Get methods
      /// </summary>
      public String GetName()
      {
         return name;
      }

      public String GetFillColor()
      {
         return fillColor;
      }

      public bool GetFillFlag()
      {
         return fillFlag;
      }

      public SVGStroke GetStroke()
      {
         return stroke;
      }

      public String GetStrokeColor()
      {
         return stroke.color;
      }

      public double GetStrokeWidth()
      {
         return stroke.width;
      }

      public SVGFont GetFont()
      {
         return font;
      }

      public String GetFontFamily()
      {
         return font.family;
      }

      public String GetFontSize()
      {
         return font.size;
      }

      public String GetFontStyle()
      {
         return font.style;
      }

      public String GetFontWeight()
      {
         return font.weight;
      }

      /// <summary>
      /// Set Metods
      /// </summary>
      /// 
      public void SetName(String set_name)
      {
         name = set_name;
      }

      public void SetFillColor(String set_fill_color)
      {
         fillColor = set_fill_color;
      }

      public void SetStroke(SVGStroke set_stroke)
      {
         stroke = set_stroke;
      }

      public void SetStrokeWidth(double set_width)
      {
         stroke.width = set_width;
      }

      public void SetStrokeColor(String set_color)
      {
         stroke.color = set_color;
      }

      public void SetFont(SVGFont set_font)
      {
         font = set_font;
      }

      public void SetFontFamily(String set_font_family)
      {
         font.family = set_font_family;
      }

      public void SetFontSize(String set_font_size)
      {
         font.size = set_font_size;
      }

      public void SetFontStyle(String set_font_style)
      {
         font.style = set_font_style;
      }

      public void SetFontWeight(String set_font_weight)
      {
         font.weight = set_font_weight;
      }

   }
}
