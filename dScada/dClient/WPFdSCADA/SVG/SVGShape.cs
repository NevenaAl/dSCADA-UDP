﻿using System;
using System.Collections.Generic;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Diagnostics;

namespace WpfDSCADA.SVG
{

   public enum SVGSHAPETYPE
   {
      SVG_SYMBOL,
      SVG_LABEL,
      SVG_BUTTON,
      SVG_UNKNOWN
   }

   public struct SVGREFRULE
   {
      public String state;
      public Brush color;
      public String audioPath;
      public int refreshTime;
      public Stopwatch stopwatch;
      public Brush blinkColor1;
      public Brush blinkColor2;
   }

   public struct SVGSHAPECUSTOMPROPS
   {
      public String pvid;
      public String svgLink;
      public List<SVGREFRULE> refRules;
   }

   public struct SVGTRANSFORM
   {
      public double tx;       // translate x
      public double ty;       // translate y
      public double rotate;   // angle
      public double sx;       // scale x
      public double sy;       // scale y
   }

   public struct SVGTEXT
   {
      public double x;
      public double y;
      public SVGStyle style;
      public bool underlined;
      public String text;
   }

   public struct SVGRECT
   {
      public double[] point;           // Pointer on a array of doubles for rectangle parametars
      public SVGStyle style;           // Rectangle Style
   }

   public struct SVGELIPS
   {
      public double[] point;	         // Pointer on a array of doubles for ellipse parametars
      public SVGStyle style;	         // Ellipse Style 
   }

   public struct SVGIMAGE
   {
       public double[] point;
       public System.Drawing.Image image;
   }
   
   /// <summary>
   /// Class SVGShape
   /// </summary>
   public class SVGShape
   {
      public SVGSHAPETYPE TypeOfShape;    // SVG specific type of shape {SVG_RECT,SVG_ELLIPSE,SVG_PATH,SVG_TEXT,SVG_UNKNOWN}
      public int mID;                     // Unique integer key of element
      public SVGTEXT Text;	               // Text if needed
      public SVGRECT Rectangle;
      public SVGELIPS Ellipse;
      public List<SVGPath> path_list;     // List of paths if shape consists of them
      public SVGTRANSFORM transform;      // Transform structure that hold's information about transforms {translate,rotate,scale}
      public SVGSHAPECUSTOMPROPS cp;      // Custom Properties
      public String colorCode;
      public bool InvertColor;
      public bool Italic;
      public SVGIMAGE Image;
      /// <summary>
      /// Constructor method
      /// </summary>
      public SVGShape()
      {
         TypeOfShape = SVGSHAPETYPE.SVG_UNKNOWN;
         Text.text = "";
         path_list = new List<SVGPath>();
         Rectangle.point = null;
         Ellipse.point = null;
         Rectangle.style = null;
         Ellipse.style = null;
         Image.point = null;

         transform.tx = 0;
         transform.ty = 0;
         transform.rotate = 0;
         transform.sx = 0;
         transform.sy = 0;

         mID = -1;
         colorCode = "";
         cp.pvid = "";
         cp.svgLink = "";
      }

      public void RectShape(string x, string y, string width, string height, string rx, string ry, SVGStyle style)
      {
         Rectangle.style = style;
         Rectangle.point = new double[6];
         Rectangle.point[0] = Convert.ToDouble(x);
         Rectangle.point[1] = Convert.ToDouble(y);
         Rectangle.point[2] = Convert.ToDouble(width);
         Rectangle.point[3] = Convert.ToDouble(height);
         if (rx!= null)
            Rectangle.point[4] = Convert.ToDouble(rx);
         if (ry != null)
            Rectangle.point[5] = Convert.ToDouble(ry);
      }

      public void EllipseShape(String scx, String scy, String srx, String sry, SVGStyle sstyle)
      {
         Ellipse.style = sstyle;
         Ellipse.point = new double[4];
         Ellipse.point[0] = Convert.ToDouble(scx);
         Ellipse.point[1] = Convert.ToDouble(scy);
         Ellipse.point[2] = Convert.ToDouble(srx);
         Ellipse.point[3] = Convert.ToDouble(sry);
      }

      public void TextShape(String x, String y, SVGStyle sstyle)
      {
         Text.x = Convert.ToDouble(x);
         Text.y = Convert.ToDouble(y);
         //_text.text = text;
         Text.style = sstyle;
      }

      public void ImageShape(string x, string y, string width, string height)
      {
          Image.point = new double[4];
          Image.point[0] = Convert.ToDouble(x);
          Image.point[1] = Convert.ToDouble(y);
          Image.point[2] = Convert.ToDouble(width);
          Image.point[3] = Convert.ToDouble(height);

      }

      #region Get and Set methods
      /// <summary>
      /// Get methods
      /// </summary>
      ///

      public double[] GetRectangle()
      {
         return Rectangle.point;
      }

      public double[] GetEllipse()
      {
         return Ellipse.point;
      }

      public SVGStyle GetRectangleStyle()
      {
         return Rectangle.style;
      }

      public SVGStyle GetEllipseStyle()
      {
         return Ellipse.style;
      }

      public SVGTRANSFORM GetTransformation()
      {
         return transform;
      }

      public SVGTEXT GetText()
      {
         return Text;
      }

      public String GetTextText()
      {
         return Text.text;
      }

      public int GetMID()
      {
         return mID;
      }

      /// <summary>
      /// Set Metods
      /// </summary>
      /// 

      void SetRectangleStyle(SVGStyle set_style)
      {
         Rectangle.style = set_style;
      }

      void SetEllipseStyle(SVGStyle set_style)
      {
         Ellipse.style = set_style;
      }

      void SetTransform(SVGTRANSFORM set_transform)
      {
         transform = set_transform;
      }

      void SetTextStruct(SVGTEXT set_text)
      {
         Text.style = set_text.style;
         Text.x = set_text.x;
         Text.y = set_text.y;
      }

      void SetTextText(SVGTEXT set_text)
      {
         Text.text = set_text.text;
      }

      void SetMID(int id)
      {
         mID = id;
      }

      #endregion

      public void StrToTransform(String stransform)
      {
         char[] scoord = new char[32];
         String scoordString = "";
         String s_temp = String.Empty;
         int i;

         SVGTRANSFORM transform_tmp;

         int string_index;

         if (!stransform.Equals(String.Empty))                       // ako ulazni string nije prazan
         {
            // ZA TRANSLATE trasformaciju
            string_index = stransform.IndexOf("translate");          //vraca index u stringu gde nadje "translate"
            if (string_index >= 0)                                   //moze jos biti -1 ako IndexOf funkcija nije nasla trazeni string ili 0 ako je string u kom se trazi prazan
            {
               //string_index = stransform.IndexOf('(');             //posto je sting tipa translate(198,198) trazimo pocetak zagrade 
               string_index += "translate".Length;
               string_index++;                                       //prealzimo na cifru posle otvorene zagrade
               i = 0;                                                //inicijalizujemo brojac za string coord
               while (stransform.ToCharArray()[string_index] != ',') //dalje idemo do zareza sto je prva cifra
               {
                  scoord[i] = stransform.ToCharArray()[string_index];//slazemo prvu cifru u coord promenljivu char[]
                  i++;                                               //povecavamo brojac za scoord
                  string_index++;                                    //povecavamo brojac za ulzni string stransform
               }
               scoord[i] = '\0';                                     //oznacavamo kraj stringa u kom se sad nalazi cifra

               scoordString = "";
               foreach (Char c in scoord)
               {
                  scoordString = scoordString + c;
                  if (c == '\0')
                  {
                     break;
                  }
               }
               transform_tmp.tx = Convert.ToDouble(scoordString);

               ///////////ponavljamo proceduru za drugu cifru////////////
               string_index++;                                 //preskacemo zarez
               i = 0;
               while (stransform.ToCharArray()[string_index] != ')')//dalje idemo do zatvorene zagrede sto je druga cifra
               {
                  scoord[i] = stransform.ToCharArray()[string_index];//slazemo drugu cifru u coord promenljivu char[]
                  i++;                                        //povecavamo brojac za scoord
                  string_index++;                             //povecavamo brojac za ulzni string stransform
               }
               scoord[i] = '\0';                               //oznacavamo kraj stringa u kom se sad nalazi cifra

               scoordString = "";
               foreach (Char c in scoord)
               {
                  scoordString = scoordString + c;
                  if (c == '\0')
                  {
                     break;
                  }
               }
               transform_tmp.ty = Convert.ToDouble(scoordString);
            }
            else
            {
               transform_tmp.tx = 0;
               transform_tmp.ty = 0;
            }


            //ZA ROTATE
            string_index = stransform.IndexOf("rotate");    //vraca index u stringu gde nadje "translate"
            if (string_index >= 0)                           //moze jos biti -1 ako IndexOf funkcija nije nasla trazeni string ili 0 ako je string u kom se trazi prazan
            {
               //string_index = stransform.IndexOf('(');     //posto je sting tipa translate(198,198) trazimo pocetak zagrade 
               string_index += "rotate".Length;
               string_index++;                             //prealzimo na cifru posle otvorene zagrade
               i = 0;                                      //inicijalizujemo brojac za string coord
               while (stransform.ToCharArray()[string_index] != ')')//dalje idemo do zatvorene zagrade sto je prva i jedina cifra za rotaiju
               {
                  scoord[i] = stransform.ToCharArray()[string_index];//slazemo prvu cifru u coord promenljivu char[]
                  i++;                                    //povecavamo brojac za scoord
                  string_index++;                         //povecavamo brojac za ulzni string stransform
               }
               scoord[i] = '\0';                           //oznacavamo kraj stringa u kom se sad nalazi cifra

               //scoordString2 = scoord.ToString();
               scoordString = "";
               foreach (Char c in scoord)
               {
                  scoordString = scoordString + c;
                  if (c == '\0')
                  {
                     break;
                  }
               }
               transform_tmp.rotate = Convert.ToDouble(scoordString);
            }
            else
            {
               transform_tmp.rotate = 0;
            }

            //ZA SCALE
            string_index = stransform.IndexOf("scale");//vraca index u stringu gde nadje "translate"
            if (string_index >= 0) //moze jos biti -1 ako IndexOf funkcija nije nasla trazeni string ili 0 ako je string u kom se trazi prazan
            {
               string_index = stransform.IndexOf('(');//posto je sting tipa translate(198,198) trazimo pocetak zagrade 
               string_index += "scale".Length;
               string_index++;//prealzimo na cifru posle otvorene zagrade
               i = 0;//inicijalizujemo brojac za string coord
               while (stransform.ToCharArray()[string_index] != ',')//dalje idemo do zareza sto je prva cifra
               {
                  scoord[i] = stransform.ToCharArray()[string_index];//slazemo prvu cifru u coord promenljivu char[]
                  i++;//povecavamo brojac za scoord
                  string_index++;//povecavamo brojac za ulzni string stransform
               }
               scoord[i] = '\0'; //oznacavamo kraj stringa u kom se sad nalazi cifra
               string sc = scoord.ToString();
               transform_tmp.sx = Convert.ToDouble(sc);
               //transform_tmp.sx = Convert.ToDouble(scoord);

               ///////////ponavljamo proceduru za drugu cifru////////////
               string_index++;//preskacemo zarez
               i = 0;
               while (stransform.ToCharArray()[string_index] != ')')//dalje idemo do zatvorene zagrede sto je druga cifra
               {
                  scoord[i] = stransform.ToCharArray()[string_index];//slazemo drugu cifru u coord promenljivu char[]
                  i++;//povecavamo brojac za scoord
                  string_index++;//povecavamo brojac za ulzni string stransform
               }
               scoord[i] = '\0'; //oznacavamo kraj stringa u kom se sad nalazi cifra
               transform_tmp.sy = Convert.ToDouble(scoord);
            }
            else
            {
               transform_tmp.sx = 0;
               transform_tmp.sy = 0;
            }

            SetTransform(transform_tmp);
         }//od if (!stransform.Equals(String.Empty))// ako ulazni string nije prazan
      }
   }
}
