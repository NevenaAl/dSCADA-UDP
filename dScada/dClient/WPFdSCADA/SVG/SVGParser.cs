using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;
using System.Windows;
using System.Windows.Media;
using System.Xml;
using System.IO;

namespace WpfDSCADA.SVG
{
   public enum SVGPARSERSTATE
   {
      NONE,
      //SVG,
      //SVG_STYLE,
      //SVG_STYLE_CDATA,
      //SVG_DEFS,
      //SVG_DEFS_SYMBOL,
      //SVG_DEFS_SYMBOL_G,
      //SVG_G,
      //SVG_G_TEXT,
      SVG_G_SHAPE_TEXT,
      //SVG_G_CUSTOMPROPERTIES,
      //SVG_G_CUSTOMPROPERTIES_CP,
      SVG_G_SHAPE,
      //SVG_G_SHAPE_PATH
   };

   public enum SVGLBLVALUES
   {
       PVID,
       TYPE_SVG_SHAPE,
       SVG_LINK
   };
    

   public class SVGParser
   {
      public Dictionary<String, SVGPage> page_list;
      public Dictionary<SVGLBLVALUES, string> lbl_rows;
      public SVGPARSERSTATE state;
      XmlReader xmlReader;

      public SVGParser()
      {
         page_list = new Dictionary<string, SVGPage>();
         lbl_rows = new Dictionary<SVGLBLVALUES, string>();
         state = SVGPARSERSTATE.NONE;
      }

      public void ParseSVGFile(String SVGSource)
      {
         // inicijalizuj XML reader
         XmlReaderSettings settings = new XmlReaderSettings();
         settings.DtdProcessing = DtdProcessing.Ignore;

         xmlReader = XmlReader.Create(SVGSource, settings);

         SVGPage svgPage = new SVGPage();

         bool notTheEndOfXMLFile = true;
         bool xml_ok = true;

         while (notTheEndOfXMLFile)// Read the line of the xml file
         {
            try
            {
               // read the next node of xml file
               notTheEndOfXMLFile = xmlReader.Read();
            }
            catch (Exception)
            {
               xml_ok = false;
            }

            SVGShape currentShape;

            string[] CPtempStringArray;            //parsing CP custum properties

            switch (xmlReader.NodeType)
            {
               case XmlNodeType.Element:

                  switch (xmlReader.Name) // sad ovde probati isplanirati svaki noodetype ;)
                  {
                     ////////  svg tag  //////////////////////////////////////////////////////////////////////////
                     case "svg":
                        //state = SVGPARSERSTATE.SVG;
                        if (GetAttValue("viewBox") != null)
                        {
                           string[] ValueArray = GetAttValue("viewBox").Split(' ');// if equal
                           svgPage.SetMinX(Convert.ToDouble(ValueArray[0]));
                           svgPage.SetMinY(Convert.ToDouble(ValueArray[1]));
                           svgPage.SetMaxX(Convert.ToDouble(ValueArray[2]));
                           svgPage.SetMaxY(Convert.ToDouble(ValueArray[3]));
                        }
                        else
                        {
                           xml_ok = false;
                        }
                        break;

                     ////////  g tag  //////////////////////////////////////////////////////////////////////////
                     case "g":
                        if (GetAttValue("v:groupContext").Equals("shape"))//if the group represents shape
                        {
                           state = SVGPARSERSTATE.SVG_G_SHAPE;
                           SVGShape newShape = new SVGShape();
                           // pokupi parametre transformacije (translate, rotate, scale)
                           newShape.StrToTransform(GetAttValue("transform"));
                           newShape.mID = Convert.ToInt32(GetAttValue("v:mID"));
                           newShape.cp.refRules = new List<SVGREFRULE>();
                           svgPage.shape_list.Add(newShape);
                        }
                        break;

                     ////////  Custum Properties tag  //////////////////////////////////////////////////////////////////////////                                        
                     case "v:custProps":
                        //no need to parse anything heare
                        break;

                     ////////  Elements of Custum Properties tag  //////////////////////////////////////////////////////////////////////////                
                     case "v:cp":
                        currentShape = svgPage.shape_list.Last();    //current Shape is the one last placed in shape list
                        switch (GetAttValue("v:lbl"))
                        {
                           case "TYPE_SVG_SHAPE":
                              lbl_rows[SVGLBLVALUES.TYPE_SVG_SHAPE] = GetAttValue("v:nameU");
                              if (GetAttValue("v:val") != null)
                              {
                                 CPtempStringArray = GetAttValue("v:val").Split(new Char[] { '(', ')' }, StringSplitOptions.None);
                                 switch (CPtempStringArray[1])
                                 {
                                    case "0":
                                       currentShape.TypeOfShape = SVGSHAPETYPE.SVG_SYMBOL;
                                       break;
                                    case "1":
                                       currentShape.TypeOfShape = SVGSHAPETYPE.SVG_LABEL;
                                       break;
                                    case "2":
                                       currentShape.TypeOfShape = SVGSHAPETYPE.SVG_BUTTON;
                                       break;
                                    default:
                                       currentShape.TypeOfShape = SVGSHAPETYPE.SVG_UNKNOWN;
                                       break;
                                 }
                              }
                              break;

                           case "PVID":
                              lbl_rows[SVGLBLVALUES.PVID] = GetAttValue("v:nameU");
                              //some CP definitions has this tag but doesnt have value ... this if statement protect app from crashing in that case
                              if (GetAttValue("v:val") != null)
                              {
                                 CPtempStringArray = GetAttValue("v:val").Split(new Char[] { '(', ')' }, StringSplitOptions.None); //string that is splited looks like "VTX(value)" so we usuly need second
                                 currentShape.cp.pvid = CPtempStringArray[1];
                              }
                              else
                              {
                                 currentShape.cp.pvid = "";
                              }
                              break;

                           case "SVG_LINK":
                              lbl_rows[SVGLBLVALUES.SVG_LINK] = GetAttValue("v:nameU");
                              if (GetAttValue("v:val") != null)//some CP definitions has this tag but doesnt have value ... this if statment protect app from crashing in that case
                              {
                                 CPtempStringArray = GetAttValue("v:val").Split(new Char[] { '(', ')' }, StringSplitOptions.None); //string that is splited looks like "VTX(value)" so we usuly need second
                                 if (CPtempStringArray != null)
                                 {
                                    currentShape.cp.svgLink = CPtempStringArray[1];
                                 }
                              }
                              break;

                           case "REFRULE":
                              if (GetAttValue("v:val") != null)
                              {
                                 CPtempStringArray = GetAttValue("v:val").Split(new Char[] { '(', ')' }, StringSplitOptions.None); //string that is splited looks like "VTX(value)" so we usuly need second
                                 if (CPtempStringArray != null)
                                 {
                                    string[] CPtemp = CPtempStringArray[1].Split(new Char[] { '=' }, StringSplitOptions.None);
                                    SVGREFRULE rr = new SVGREFRULE();
                                    rr.state = CPtemp[0];
                                    if(CPtemp[1].Equals("COLOR"))
                                    {
                                        rr.color = WpfDSCADA.MainWindow.StringToBrush(CPtempStringArray[2]);
                                    }
                                    else if (CPtemp[1].Equals("AUDIO"))
                                    {
                                        string[] temp = CPtempStringArray[2].Split(',');
                                        rr.audioPath = temp[0];
                                        rr.refreshTime = 4;                         // default vrednost, 4+1=5s
                                        int rtime = Convert.ToInt32(temp[1]);
                                        if (rtime > 5)
                                            rr.refreshTime = rtime - 1;             // smanji za 1s koji se guta
                                        //rr.refreshTime = Convert.ToInt32(temp[1]);
                                        rr.stopwatch = new System.Diagnostics.Stopwatch();
                                    }
                                    else if (CPtemp[1].Equals("BLINK"))
                                    {
                                        string[] temp = CPtempStringArray[2].Split(',');
                                        rr.blinkColor1 = WpfDSCADA.MainWindow.StringToBrush(temp[0]);
                                        rr.blinkColor2 = WpfDSCADA.MainWindow.StringToBrush(temp[1]);
                                        rr.stopwatch = new System.Diagnostics.Stopwatch();
                                    }
                                    
                                     // sacuvaj pravilo uz simbol
                                    currentShape.cp.refRules.Add(rr);
                                 }
                              }
                              break;

                           case "":
                              if (GetAttValue("v:nameU").Equals(lbl_rows[SVGLBLVALUES.TYPE_SVG_SHAPE]))
                              {
                                  if (GetAttValue("v:val") != null)
                                  {
                                      CPtempStringArray = GetAttValue("v:val").Split(new Char[] { '(', ')' }, StringSplitOptions.None);
                                      switch (CPtempStringArray[1])
                                      {
                                          case "0":
                                              currentShape.TypeOfShape = SVGSHAPETYPE.SVG_SYMBOL;
                                              break;
                                          case "1":
                                              currentShape.TypeOfShape = SVGSHAPETYPE.SVG_LABEL;
                                              break;
                                          case "2":
                                              currentShape.TypeOfShape = SVGSHAPETYPE.SVG_BUTTON;
                                              break;
                                          default:
                                              currentShape.TypeOfShape = SVGSHAPETYPE.SVG_UNKNOWN;
                                              break;
                                      }
                                  }
                              }
                              else if (GetAttValue("v:nameU").Equals(lbl_rows[SVGLBLVALUES.PVID]))
                              {
                                  CPtempStringArray = GetAttValue("v:val").Split(new Char[] { '(', ')' }, StringSplitOptions.None); //string that is splited looks like "VTX(value)" so we usuly need second
                                  currentShape.cp.pvid = CPtempStringArray[1];
                              }
                              else if (GetAttValue("v:nameU").Equals(lbl_rows[SVGLBLVALUES.SVG_LINK]))
                              {
                                  CPtempStringArray = GetAttValue("v:val").Split(new Char[] { '(', ')' }, StringSplitOptions.None); //string that is splited looks like "VTX(value)" so we usuly need second
                                  if (CPtempStringArray != null)
                                  {
                                      currentShape.cp.svgLink = CPtempStringArray[1];
                                  }
                              }
                              break;

                           default:
                              break;
                        }//from - switch (GetAttValue(""))
                        break;//case "v:cp"

                     ////////  path tag  //////////////////////////////////////////////////////////////////////////
                     case "path":
                        if (state == SVGPARSERSTATE.SVG_G_SHAPE)
                        {
                           currentShape = svgPage.shape_list.Last();          // tekuci = zadnji upisani shape u listi
                           SVGStyle pathStyle = FindStyle(svgPage, GetAttValue("class"));
                           string path = GetAttValue("d");
                           // resava problem lazno vezanih kontura
                           path = path.Replace( "ZM", "Z M" );
                           String[] path_string = path.Split(' ');
                           // LOGIKA ZA PARSIRANJE STRINGA SA PUTANJOM
                           SVGPath new_path = null;
                           bool is_first = true;
                           double coordinate;
                           for (int i = 0; i < path_string.Count(); i++)
                           {
                              if (path_string[i] != "")
                              {
                                 //is the first character of the string leter comand
                                 if (Char.IsLetter(path_string[i].ToCharArray()[0]))
                                 {
                                    if (is_first)  //ako je prva komanda onda pravim novi SVGPath i pocinjem da ga punim
                                    {
                                       new_path = new SVGPath();
                                       is_first = false;
                                    }
                                    else           //ako nije prva komanda onda prvo sacuvam prethodnu pa ostalo sve isto
                                    {
                                       currentShape.path_list.Add(new_path);
                                       new_path = new SVGPath();
                                    }
                                    //  set svg comand type and style
                                    new_path.SetCmd(new_path.CharToCmd(path_string[i].ToCharArray()[0]));
                                    new_path.SetStyle(pathStyle);
                                    // read coordinates
                                    if (path_string[i].Length > 1)   //ako je napr Z onda nema koordinate pa ovim izbegavam da trazim koordinate za nju
                                    {
                                       string coordinateString = path_string[i];
                                       bool stop = false;
                                       while (!stop)
                                       {
                                          if (Char.IsLetter(coordinateString, 0))
                                          {
                                             coordinateString = coordinateString.Substring(1);
                                          }
                                          else
                                          {
                                             stop = true;
                                          }
                                       }
                                       coordinate = Convert.ToDouble(coordinateString);
                                       new_path.coords_list.Add(coordinate);
                                    }
                                    else // upisujem napr Z bez koordinata
                                    {
                                       currentShape.path_list.Add(new_path);
                                    }

                                 }
                                 else
                                 {
                                    new_path.coords_list.Add(Convert.ToDouble(path_string[i]));
                                 }
                              }
                           }//for
                           currentShape.path_list.Add(new_path);//upisujem poslednju obradjivanu putanju
                        }//if (state == SVGPARSERSTATE.SVG_G_SHAPE)
                        break;//case "path"

                     //////  rect tag  //////////////////////////////////////////////////////////////////////////
                     case "rect":
                        //if (state == SVGPARSERSTATE.SVG_G_SHAPE)
                        //{
                        // if (GetAttValue("v:rectContext") != "textBkgnd")
                        //{

                        currentShape = svgPage.shape_list.Last();//naci zadnji upisani shape u listi                                  
                        SVGStyle rectStyle = FindStyle(svgPage, GetAttValue("class"));
                        string x = GetAttValue("x");
                        string y = GetAttValue("y");
                        string width = GetAttValue("width");
                        string height = GetAttValue("height");
                        string rx = GetAttValue("rx");
                        string ry = GetAttValue("ry");
                        currentShape.RectShape( x, y, width, height, rx, ry, rectStyle);
                        // }
                        //}
                        break;//case "rect"

                     //////////  ellipse tag  //////////////////////////////////////////////////////////////////////////
                     case "ellipse":
                        //if (state == SVGPARSERSTATE.SVG_G_SHAPE)
                        //{
                        currentShape = svgPage.shape_list.Last();//naci zadnji upisani shape u listi
                        SVGStyle ellipseStyle = FindStyle(svgPage, GetAttValue("class"));
                        currentShape.EllipseShape(GetAttValue("cx"), GetAttValue("cy"), GetAttValue("rx"), GetAttValue("ry"), ellipseStyle);
                        //}
                        break;//case "ellipse"

                     ////////  text tag  //////////////////////////////////////////////////////////////////////////
                     case "text":
                        //if (state == SVGPARSERSTATE.SVG_G_SHAPE)
                        //{
                        currentShape = svgPage.shape_list.Last();//naci zadnji upisani shape u listi 
                        SVGStyle textStyle = FindStyle(svgPage, GetAttValue("class"));
                        state = SVGPARSERSTATE.SVG_G_SHAPE_TEXT;
                        currentShape.TextShape(GetAttValue("x"), GetAttValue("y"), textStyle);
                        //}
                        break;//case "text"

                     case "tspan":
                        SVGStyle decorStyle = FindStyle(svgPage, GetAttValue("class"));
                        if ((decorStyle != null) && (decorStyle.font.underline.Equals("on")))
                        {
                           currentShape = svgPage.shape_list.Last();
                           currentShape.Text.underlined = true;
                        }
                        break;

                      ////////  image tag  //////////////////////////////////////////////////////////////////////////
                      case "image":
                          if (GetAttValue("xlink:href") != null)
                          {
                              string data = GetAttValue("xlink:href");
                              string[] dataArray = data.Split(',');
                              string base64String = dataArray[1];
                              currentShape = svgPage.shape_list.Last();
                              currentShape.Image.image = Base64ToImage(base64String);
                              currentShape.ImageShape(GetAttValue("x"), GetAttValue("y"), GetAttValue("width"), GetAttValue("height"));
                          }
                        break;
                     ////////  default tag  //////////////////////////////////////////////////////////////////////////
                     default:
                        break;
                  }
                  break;//case XmlNodeType.Element:

               ////////  style tag  //////////////////////////////////////////////////////////////////////////
               case XmlNodeType.CDATA:
                  //ovde samo dobijam dugacak string kao xmlreader.value sa svim stilovima ... treba parsirati                        
                  string[] styleAtributes;
                  string[] styleSeparator = new string[] { ".st" };
                  string[] styleStrings = xmlReader.Value.Split(styleSeparator, StringSplitOptions.None);

                  foreach (string s in styleStrings)
                  {
                     //kada odsecem .st svi stringovi sa stilovima pocinju sa 1-9 cifrom ... inace sluzi da se izbaci "\r\n\t\t"
                     if (s.StartsWith("1") || s.StartsWith("2") || s.StartsWith("3") || s.StartsWith("4") || s.StartsWith("5") || s.StartsWith("6") || s.StartsWith("7") || s.StartsWith("8") || s.StartsWith("9"))
                     {
                        SVGStyle newStyle = new SVGStyle();
                        styleAtributes = s.Split(new Char[] { ' ', '{', ':', ';', '}' }, StringSplitOptions.RemoveEmptyEntries);
                        newStyle.name = "st" + styleAtributes[0];          //set the style name: for expample st1 or st2 ...
                        for (int i = 0; i < styleAtributes.Count() - 2; i += 2)
                        {
                           switch (styleAtributes[i + 1])                     // name of atribute
                           {
                              case "fill":
                                 switch (styleAtributes[i+2])
                                 {
                                    case "none":                              // nema fill-a
                                       newStyle.fillFlag = false;
                                       break;
                                    case "#000000":                           // Black
                                       newStyle.fillFlag = true;
                                       newStyle.fillColor = "#ffffffff";      // postavi White
                                       break;
                                    default:
                                       newStyle.fillFlag = true;
                                       // reads the RGB value of color (ff ff ff) and put it in ARGB(0x00,0xff,0xff,0xff)
                                       newStyle.fillColor = styleAtributes[i+2].Insert(1, "ff");
                                       break;
                                 }
                                 break;

                              case "stroke":
                                 switch (styleAtributes[i + 2])
                                 {
                                    case "none":                               // nema linije
                                       newStyle.no_line = true;
                                       break;
                                    case "#000000":                            // Black
                                       newStyle.stroke.color = "#ffffffff";    // postavi White
                                       break;
                                    default:
                                       newStyle.stroke.color = styleAtributes[i + 2].Insert(1, "ff");
                                       break;
                                 }
                                 break;

                              case "font-family":
                                 if (styleAtributes[i + 2].CompareTo("none") == 0)
                                    newStyle.font.family = "";
                                 else
                                    newStyle.font.family = styleAtributes[i + 2];
                                 break;

                              case "font-size":
                                 if (styleAtributes[i + 2].CompareTo("none") == 0)
                                    newStyle.font.size = "";
                                 else
                                    newStyle.font.size = styleAtributes[i + 2];
                                 break;

                              case "font-style":
                                 newStyle.font.style = styleAtributes[i + 2];
                                 break;

                              case "font-weight":
                                 newStyle.font.weight = styleAtributes[i + 2];
                                 break;

                              case "stroke-width":
                                 newStyle.stroke.width = Convert.ToDouble(styleAtributes[i + 2]);
                                 break;

                              case "text-decoration":
                                 if (styleAtributes[i + 2].CompareTo("underline") == 0)
                                    newStyle.font.underline = "on";
                                 else
                                    newStyle.font.underline = "off";
                                 break;

                              default:
                                 break;
                           }//switch (styleAtributes[i + 1])//name of atribute
                        }//for (int i = 0; i < styleAtributes.Count() - 2; i += 2)

                        svgPage.style_list.Add(newStyle);

                     }//if (s.StartsWith("1") || s.StartsWith("2") || s.StartsWith("3") || s.StartsWith("4") || s.StartsWith("5") || s.StartsWith("6") || s.StartsWith("7") || s.StartsWith("8") || s.StartsWith("9"))
                  }//foreach (string s in styleStrings)
                  break;

               ////////  TEXT  /////////////////////////////////////////////////////////////////////////////
               case XmlNodeType.Text:
                  if (state == SVGPARSERSTATE.SVG_G_SHAPE_TEXT)  // znaci ako se obradjuje shape i ima text
                  {
                     svgPage.shape_list.Last().Text.text = xmlReader.Value;
                     state = SVGPARSERSTATE.SVG_G_SHAPE;
                  }
                  break;

               ////////  end tag  //////////////////////////////////////////////////////////////////////////
               case XmlNodeType.EndElement:
                  switch (xmlReader.Name)
                  {
                     case "g":
                        state = SVGPARSERSTATE.NONE;
                        break;
                     case "svg":
                        state = SVGPARSERSTATE.NONE;
                        break;
                     default:
                        break;
                  }
                  break;

               default:
                  break;
            }// switch (xmlReader.NodeType)
         }


         if( xml_ok )
            page_list.Add(SVGSource, svgPage);
         else
            MessageBox.Show("Error parsing SVG file!");

         xmlReader.Close();

      }

      /// <summary>
      /// GetAttValue method returns a value for requested atribute
      /// </summary>
      /// <param name="atributName">name of the atribute</param>
      /// <returns></returns>
      public string GetAttValue(string atributName)
      {
         if (xmlReader.HasAttributes)
         {
            for (int i = 0; i < xmlReader.AttributeCount; i++)
            {
               xmlReader.MoveToAttribute(i);
               if (xmlReader.Name.CompareTo(atributName) == 0)
                  return xmlReader.Value;
            }
         }
         return null;
      }
      /// <summary>
      /// FindStyle metod find's object refernce to SVGStyle with disared name in list of style's (wich is instanced in SvgPage object)
      /// </summary>
      /// <param name="current_page">Page that is curently in use</param>
      /// <param name="style_name">Name of style for wich we need object referenc</param>
      /// <returns>Object refernce to SVGStyle object</returns>
      public SVGStyle FindStyle(SVGPage current_page, String style_name)
      {
         foreach (SVGStyle s in current_page.style_list)
         {
            if (s.name.CompareTo(style_name) == 0)
            {
               return s;
            }
         }
         return null;
      }

      public System.Drawing.Image Base64ToImage(string base64String)
        {
            byte[] imageBytes = Convert.FromBase64String(base64String);
            MemoryStream ms = new MemoryStream(imageBytes, 0, imageBytes.Length);

            ms.Write(imageBytes, 0, imageBytes.Length);
            System.Drawing.Image image = System.Drawing.Image.FromStream(ms, true);
            return image;
        }

   }
}






