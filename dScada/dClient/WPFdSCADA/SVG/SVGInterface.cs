﻿using System;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Shapes;
using WpfDSCADA.Catalogs;
using System.Collections.Generic;
using System.Windows.Interop;
using System.Windows.Media.Imaging;


namespace WpfDSCADA.SVG
{

   public class SVGInterface
   {
      /*----------------------------------------------------------------------------------------------------------*/
      /* GLOBALS                                                                                                  */
      /*----------------------------------------------------------------------------------------------------------*/
      public SVGParser svgParser = new SVGParser();
      public SVGPage svgPage;
      Canvas MyCanvas;
      //Object used to convert color from string (etc #00 00 00 00)
      BrushConverter colorConverter = new BrushConverter();
      
      /*----------------------------------------------------------------------------------------------------------*/
      /* METHODS                                                                                                  */
      /*----------------------------------------------------------------------------------------------------------*/

      public bool OpenSVGFromFile(String filePath, Canvas imageContainer, bool withFileOpenDlg)
      {
         if (withFileOpenDlg)       //if we wont to use OpenFileDialog
         {
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            dlg.InitialDirectory = Environment.CurrentDirectory + @"\SVG files";
            dlg.FileName = "SVG Files"; // Default file name
            dlg.DefaultExt = ".svg"; // Default file extension
            dlg.Filter = "SVG files (.svg)|*.svg"; // Filter files by extension

            Nullable<bool> result = dlg.ShowDialog();

            if (result == true)
            {
               SVGRoot(dlg.FileName, imageContainer);
               return true;
            }
            return false;
         }
         else
         {
            SVGRoot(filePath, imageContainer);
            return true;
         }

      }

      public void SVGRoot(String SVGFilePath, Canvas imageContainer)
      {
         MyCanvas = imageContainer;
         if (!svgParser.page_list.ContainsKey(SVGFilePath))
         {
            svgParser.ParseSVGFile(SVGFilePath);
         }
         svgParser.page_list.TryGetValue(SVGFilePath, out svgPage);
      }

      public void DrawPage()
      {
         MyCanvas.Children.Clear();
         MyCanvas.Width = svgPage.maxx;
         MyCanvas.Height = svgPage.maxy;
         foreach (SVGShape shape in svgPage.shape_list)
         {
            DrawShape(shape);
         }
      }

      public void DrawShape(SVGShape shape)
      {
         Image shapeImage = new Image();
         TextBlock textBlock = new TextBlock();
         DrawingImage drawingContanier = new DrawingImage();
         GeometryDrawing geometryDrawing = new GeometryDrawing();
         Brush drawingBrush;

         // pocetna inicijalizacija boje prikaza
         if (shape.colorCode != "")
         {
            // dinamicka promena boje
            drawingBrush = colorConverter.ConvertFromString(shape.colorCode) as Brush;
         }
         else
         {
            drawingBrush = Brushes.White;
         }
         geometryDrawing.Pen = new Pen(drawingBrush, 1);

         if (shape.path_list.Count > 0)                        // path figura
         {
            DrawPath(shape, geometryDrawing, shapeImage);
            drawingContanier.Drawing = geometryDrawing;
            shapeImage.Source = drawingContanier;
            MyCanvas.Children.Add(shapeImage);
         }

         if (shape.Rectangle.point != null && shape.Image.point == null)                     // rectangle
         {
            DrawRect(shape, geometryDrawing, shapeImage);
            drawingContanier.Drawing = geometryDrawing;
            shapeImage.Source = drawingContanier;
            MyCanvas.Children.Add(shapeImage);
         }

         if (shape.Ellipse.point != null)                       // elipsa
         {
            DrawEllipse(shape, geometryDrawing, shapeImage);
            drawingContanier.Drawing = geometryDrawing;
            shapeImage.Source = drawingContanier;
            MyCanvas.Children.Add(shapeImage);
         }

         if (shape.Image.point != null)                         //image
         {
             DrawImage(shape, drawingContanier, shapeImage);
             shapeImage.Source = drawingContanier;
             MyCanvas.Children.Add(shapeImage);
         }
         
         if (shape.Text.text != "") // if there is some text with the shape we call DrowText function
         {
            DrawText(shape, textBlock);
            MyCanvas.Children.Add(textBlock);
         }

         // Name the active graphic objects and add exeption handlers on MouseClick
         switch (shape.TypeOfShape)
         {
            case SVGSHAPETYPE.SVG_SYMBOL:
               shapeImage.Name = "ShapeNo" + shape.mID.ToString();
               shapeImage.MouseLeftButtonDown += new System.Windows.Input.MouseButtonEventHandler(_MouseLeftButtonDown);
               shapeImage.MouseRightButtonDown += new System.Windows.Input.MouseButtonEventHandler(_MouseRightButtonDown);
               if (shape.Text.text != "") // if there is some text
               {
                  textBlock.Name = "ShapeNo" + shape.mID.ToString();
                  textBlock.MouseLeftButtonDown += new System.Windows.Input.MouseButtonEventHandler(_MouseLeftButtonDown);
                  textBlock.MouseRightButtonDown += new System.Windows.Input.MouseButtonEventHandler(_MouseRightButtonDown);
               }
               break;
            case SVGSHAPETYPE.SVG_LABEL:
               textBlock.Name = "ShapeNo" + shape.mID.ToString();
               textBlock.MouseRightButtonDown += new System.Windows.Input.MouseButtonEventHandler(_MouseRightButtonDown);
               break;
            case SVGSHAPETYPE.SVG_BUTTON:
               shapeImage.Name = "ShapeNo" + shape.mID.ToString();
               shapeImage.MouseLeftButtonDown += new System.Windows.Input.MouseButtonEventHandler(_MouseLeftButtonDown);
               if (shape.Text.text != "") // if there is some text
               {
                  textBlock.Name = "ShapeNo" + shape.mID.ToString();
                  textBlock.MouseLeftButtonDown += new System.Windows.Input.MouseButtonEventHandler(_MouseLeftButtonDown);
               }
               break;
            case SVGSHAPETYPE.SVG_UNKNOWN:
               break;
         }

         // ispisi dokle si stigao
         //MessageBox.Show(shape.mID.ToString());
         //PutEvent(shape.mID.ToString());

      }

      public void DrawPath(SVGShape shape, GeometryDrawing geometryDrawing, Image shapeImage)
      {
         TransformGroup transformGroup = new TransformGroup();
         StreamGeometry g = new StreamGeometry();

         using (StreamGeometryContext context = g.Open())
         {
            foreach (SVGPath p in shape.path_list)
            {
               if (p.style != null)     // style mora postojati ? BATA!!
               {
                  if (shape.colorCode == "")                  // uzmi boju iz stila
                     geometryDrawing.Pen.Brush = colorConverter.ConvertFromString(p.style.GetStrokeColor()) as Brush;
                  // linija
                  geometryDrawing.Pen.Thickness = p.style.GetStrokeWidth();
                  // ispuna
                  if (p.style.GetFillFlag())
                     g.FillRule = FillRule.Nonzero;
               }
               // uradi definiciju strukture
               switch (p.cmd)
               {
                  case SVGPATHCMD.SVG_PATH_CMD_MOVE_TO:
                     context.BeginFigure(new Point(p.coords_list[0], p.coords_list[1]), p.style.fillFlag, p.style.fillFlag);
                     break;
                  case SVGPATHCMD.SVG_PATH_CMD_LINE_TO:
                     context.LineTo(new Point(p.coords_list[0], p.coords_list[1]), true, false);
                     break;
                  case SVGPATHCMD.SVG_PATH_CMD_ARC_TO:
                     if (p.coords_list[4] != 0)
                        context.ArcTo(new Point(p.coords_list[5], p.coords_list[6]), new Size(p.coords_list[0], p.coords_list[1]), p.coords_list[2], Convert.ToBoolean(p.coords_list[3]), SweepDirection.Clockwise, true, false);
                     else
                        context.ArcTo(new Point(p.coords_list[5], p.coords_list[6]), new Size(p.coords_list[0], p.coords_list[1]), p.coords_list[2], Convert.ToBoolean(p.coords_list[3]), SweepDirection.Counterclockwise, true, false);
                     break;
                  case SVGPATHCMD.SVG_PATH_CMD_CLOSE_PATH:
                     // if the path is closed than we need to set isClosed atribute in starting point from false to true
                     // Note to my self: Ovo je u principu jako lose resenje jer radi samo za slucaj da je prvi path 
                     // zatvoren .. u ostalim slucajevima svi prethodni ce biti obrisani(u datim primerima shema se savrseno uklapa:)
                     g.Clear();
                     if (shape.colorCode == "")                  // uzmi boju iz stila
                        geometryDrawing.Brush = colorConverter.ConvertFromString(p.style.fillColor) as Brush;
                     else
                        geometryDrawing.Brush = colorConverter.ConvertFromString(shape.colorCode) as Brush;
                     break;
                  default:
                     break;
               }
            }//foreach
         }//using
         //Transformation processing
         if (shape.transform.rotate != 0)
         {
            transformGroup.Children.Add(new RotateTransform(shape.transform.rotate));
         }
         if ((shape.transform.tx != 0) || (shape.transform.ty != 0))
         {
            transformGroup.Children.Add(new TranslateTransform(shape.transform.tx, shape.transform.ty));
         }
         if ((shape.transform.sx != 0) || (shape.transform.sy != 0))
         {
            transformGroup.Children.Add(new ScaleTransform(shape.transform.sx, shape.transform.sy));
         }
         if (transformGroup.Children.Count > 0)//ako ima vise transformacija
         {
            g.Transform = transformGroup;
         }
         geometryDrawing.Geometry = g;
         //Placing image on exact coordinates in canvas
         shapeImage.SetValue(Canvas.LeftProperty, g.Bounds.X);
         shapeImage.SetValue(Canvas.TopProperty, g.Bounds.Y);

      }

      public void DrawRect(SVGShape shape, GeometryDrawing geometryDrawing, Image shapeImage)
      {
         // odredi figuru
         Rect rectS = new Rect();
         rectS.X = shape.Rectangle.point[0];
         rectS.Y = shape.Rectangle.point[1];
         rectS.Width = shape.Rectangle.point[2];
         rectS.Height = shape.Rectangle.point[3];

         // definisi atribute
         if (!shape.Rectangle.style.no_line)
         {
            if (shape.colorCode == "")                  // uzmi boju iz stila
               geometryDrawing.Pen.Brush = colorConverter.ConvertFromString(shape.Rectangle.style.GetStrokeColor()) as Brush;
            // linija
            geometryDrawing.Pen.Thickness = shape.Rectangle.style.GetStrokeWidth();
         }
         else
         {
            geometryDrawing.Pen.Thickness = 0;
         }

         //geometryDrawing.Pen.LineJoin = PenLineJoin.Round;
         //geometryDrawing.Pen.EndLineCap = PenLineCap.Square;

         // ispuna
         if (shape.Rectangle.style.fillFlag)
         {
            if (shape.colorCode == "")                  // uzmi boju iz stila
               geometryDrawing.Brush = (Brush)colorConverter.ConvertFromString(shape.Rectangle.style.fillColor);
            else
               geometryDrawing.Brush = (Brush)colorConverter.ConvertFromString(shape.colorCode);
         }

         //Transformation processing
         TransformGroup transformGroup = new TransformGroup();
         if (shape.transform.rotate != 0)
         {
            transformGroup.Children.Add(new RotateTransform(shape.transform.rotate));
         }
         if ((shape.transform.tx != 0) || (shape.transform.ty != 0))
         {
            transformGroup.Children.Add(new TranslateTransform(shape.transform.tx, shape.transform.ty));
         }
         if ((shape.transform.sx != 0) || (shape.transform.sy != 0))
         {
            transformGroup.Children.Add(new ScaleTransform(shape.transform.sx, shape.transform.sy));
         }

         RectangleGeometry rectSG = new RectangleGeometry(rectS, shape.Rectangle.point[4], shape.Rectangle.point[5]);
         if (transformGroup.Children.Count > 0)       //ako ima transformacija
            rectSG.Transform = transformGroup;

         geometryDrawing.Geometry = rectSG;
         //Placing image on exact coordinates in canvas
         shapeImage.SetValue(Canvas.LeftProperty, rectSG.Bounds.X);
         shapeImage.SetValue(Canvas.TopProperty, rectSG.Bounds.Y);
      }

      public void DrawEllipse(SVGShape shape, GeometryDrawing geometryDrawing, Image shapeImage)
      {
         // odredi atribute
         if (shape.colorCode == "")                  // uzmi boju iz stila
            geometryDrawing.Pen.Brush = colorConverter.ConvertFromString(shape.Ellipse.style.GetStrokeColor()) as Brush;
         // linija
         geometryDrawing.Pen.Thickness = shape.Ellipse.style.GetStrokeWidth();
         // ispuna
         if (shape.Ellipse.style.fillFlag)
         {
            if (shape.colorCode == "")                  // uzmi boju iz stila
               geometryDrawing.Brush = (Brush)colorConverter.ConvertFromString(shape.Ellipse.style.fillColor);
            else
               geometryDrawing.Brush = (Brush)colorConverter.ConvertFromString(shape.colorCode);
         }

         //Transformation processing
         TransformGroup transformGroup = new TransformGroup();
         if (shape.transform.rotate != 0)
         {
            transformGroup.Children.Add(new RotateTransform(shape.transform.rotate));
         }
         if ((shape.transform.tx != 0) || (shape.transform.ty != 0))
         {
            transformGroup.Children.Add(new TranslateTransform(shape.transform.tx, shape.transform.ty));
         }
         if ((shape.transform.sx != 0) || (shape.transform.sy != 0))
         {
            transformGroup.Children.Add(new ScaleTransform(shape.transform.sx, shape.transform.sy));
         }

         // odredi figuru
         Point center = new Point(shape.Ellipse.point[0], shape.Ellipse.point[1]);
         EllipseGeometry ellipseG = new EllipseGeometry( center, shape.Ellipse.point[2], shape.Ellipse.point[3], transformGroup);
         geometryDrawing.Geometry = ellipseG;

         //Placing image on exact coordinates in canvas
         shapeImage.SetValue(Canvas.LeftProperty, ellipseG.Bounds.X);
         shapeImage.SetValue(Canvas.TopProperty, ellipseG.Bounds.Y);
      }

      [System.Runtime.InteropServices.DllImport("gdi32.dll")]
      public static extern bool DeleteObject(IntPtr hObject);
      
      public void DrawImage(SVGShape shape, DrawingImage drawingContanier, Image shapeImage)
      {
          TransformGroup transformGroup = new TransformGroup();
          if (shape.transform.rotate != 0)
          {
              transformGroup.Children.Add(new RotateTransform(shape.transform.rotate));
          }
          if ((shape.transform.tx != 0) || (shape.transform.ty != 0))
          {
              transformGroup.Children.Add(new TranslateTransform(shape.transform.tx, shape.transform.ty));
          }
          if ((shape.transform.sx != 0) || (shape.transform.sy != 0))
          {
              transformGroup.Children.Add(new ScaleTransform(shape.transform.sx, shape.transform.sy));
          }

          ImageDrawing imageDrawing = new ImageDrawing();
          imageDrawing.Rect = new Rect(shape.Image.point[0], shape.Image.point[1], shape.Image.point[2], shape.Image.point[3]);


          using (System.Drawing.Bitmap bitmap = new System.Drawing.Bitmap(shape.Image.image))
          {
              IntPtr hBitmap = bitmap.GetHbitmap();
              try
              {
                  var bitmapSource = Imaging.CreateBitmapSourceFromHBitmap(hBitmap,
                                                                           IntPtr.Zero,
                                                                           Int32Rect.Empty,
                                                                           BitmapSizeOptions.FromEmptyOptions());
                  imageDrawing.ImageSource = bitmapSource;
              }
              finally
              {
                  DeleteObject(hBitmap);
              }
          }

          shapeImage.RenderTransform = transformGroup;
          shapeImage.SetValue(Canvas.LeftProperty, imageDrawing.Rect.X);
          shapeImage.SetValue(Canvas.TopProperty, imageDrawing.Rect.Y);

          drawingContanier.Drawing = imageDrawing;

      }

      public void DrawText(SVGShape shape, TextBlock textBlock)
      {
         //Font adjustment           
         //Note to my self: Ovde ima mesta za dalja fina podesavnja
         if (shape.Text.style.font.family != "")
         {
            textBlock.FontFamily = new FontFamily(shape.Text.style.font.family);

            if (shape.Text.style.font.style.Equals("italic"))
               textBlock.FontStyle = FontStyles.Italic;

            if (shape.Text.style.font.weight.Equals("bold"))
               textBlock.FontWeight = FontWeights.Bold;

            if (shape.Text.underlined)
            {
               // Create an underline text decoration. Default is underline.
               TextDecoration myUnderline = new TextDecoration();
               // Set the underline decoration to a TextDecorationCollection and add it to the text block.
               TextDecorationCollection myCollection = new TextDecorationCollection();
               myCollection.Add(myUnderline);
               textBlock.TextDecorations = myCollection;
            }

         }
         if (shape.Text.style.font.size != "")                //set the font size only if there is any value defined
         {
            if (shape.Text.style.font.size.Contains("em"))
            {
               //size string is given in format(0.499em).
               // EM means "emphemeral unit" which is relative to the current font size.
               // For example, if the current font size was set to 16px, then the bottom padding would be set to 160px.
               textBlock.FontSize = textBlock.FontSize * Convert.ToDouble(shape.Text.style.font.size.Replace("em", ""));
            }
            else
            {
               //if size value is given without in normal format without em                
               textBlock.FontSize = Convert.ToDouble(shape.Text.style.font.size);
            }
         }

         // default boja prikaza je iz style-a
         textBlock.Foreground = (Brush)colorConverter.ConvertFromString(shape.Text.style.fillColor);

         // promeni je ako postoji dinamicko farbanje
         if (shape.colorCode != "")
         {
            if (!shape.InvertColor)
            {
               textBlock.Foreground = (Brush)colorConverter.ConvertFromString(shape.colorCode.ToString());
            }
            else
            {
               textBlock.Foreground = (Brush)colorConverter.ConvertFromString(Brushes.Black.ToString());
               textBlock.Background = (Brush)colorConverter.ConvertFromString(shape.colorCode.ToString());
            }
            if (shape.Italic)
            {
               textBlock.FontStyle = FontStyles.Italic;
            }
         }

         //Transformation processing
         double X = shape.Text.x;
         double Y = shape.Text.y - textBlock.FontSize;
         // Pomeri pocetak za 1/2 default margine = 4/2=2;
         double angle = shape.transform.rotate * Math.PI / 180;
         X = X + 2 * Math.Sin(angle);
         Y = Y + 2 * Math.Cos(angle);
         
         TransformGroup transformGroup = new TransformGroup();
         if (shape.transform.rotate != 0)
         {
            RotateTransform rotateTransform = new RotateTransform(shape.transform.rotate);
            Point newPoint = rotateTransform.Transform(new Point(X, Y));
            X = newPoint.X;
            Y = newPoint.Y;
            transformGroup.Children.Add(rotateTransform);
         }
         if ((shape.transform.tx != 0) || (shape.transform.ty != 0))
         {
            transformGroup.Children.Add(new TranslateTransform(shape.transform.tx, shape.transform.ty));
         }
         if ((shape.transform.sx != 0) || (shape.transform.sy != 0))
         {
            transformGroup.Children.Add(new ScaleTransform(shape.transform.sx, shape.transform.sy));
         }
         if (transformGroup.Children.Count > 0)    //if there is at least one transformation
         {
            textBlock.RenderTransform = transformGroup;
         }
         textBlock.Text = shape.Text.text;

         textBlock.SetValue(Canvas.LeftProperty, X);
         textBlock.SetValue(Canvas.TopProperty, Y);
      }

      /*----------------------------------------------------------------------------------------------------------*/
      /* Event Function
      /*----------------------------------------------------------------------------------------------------------*/


      void _MouseRightButtonDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
      {
         int mIDofShape = -1;
         // Name of sender -> mID of selected shape
         if (sender is Image)
         {
            mIDofShape = Convert.ToInt32(((Image)sender).Name.Split('o')[1]);
         }
         else if (sender is TextBlock)
         {
            mIDofShape = Convert.ToInt32(((TextBlock)sender).Name.Split('o')[1]);
         }
         OnSvgShapeRightClicked(EventArgs.Empty, mIDofShape);
      }

      void _MouseLeftButtonDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
      {
         int mIDofShape = -1;
         // Name of sender -> mID of selected shape
         if (sender is Image)
         {
            mIDofShape = Convert.ToInt32(((Image)sender).Name.Split('o')[1]);
         }
         else if (sender is TextBlock)
         {
            mIDofShape = Convert.ToInt32(((TextBlock)sender).Name.Split('o')[1]);
         }

         // vidi sta je u pitanju
         foreach (SVGShape shape in svgPage.shape_list)
         {
            if (shape.mID == mIDofShape)        //the selected one
            {
               switch( shape.TypeOfShape )
               {
                  case SVGSHAPETYPE.SVG_SYMBOL:
                     if (shape.cp.pvid != "")
                        OnSvgShapeClicked(EventArgs.Empty, shape.mID);
                     break;
                  case SVGSHAPETYPE.SVG_BUTTON:
                     if( (shape.cp.svgLink != "") )
                        OnSvgShapeClicked(EventArgs.Empty, shape.mID);
                     break;
               }
               break;
            }
         }
      }

      public delegate void SvgShapeClickedEventHandler(object sender, EventArgs e, int mID);

      public event SvgShapeClickedEventHandler SvgShapeClicked;

      protected virtual void OnSvgShapeClicked(EventArgs e, int mID)
      {
         if (SvgShapeClicked != null)
            SvgShapeClicked(this, e, mID);
      }

      public delegate void SvgShapeRightClickedEventHandler(object sender, EventArgs e, int mID);

      public event SvgShapeRightClickedEventHandler SvgShapeRightClicked;

      protected virtual void OnSvgShapeRightClicked(EventArgs e, int mID)
      {
         if (SvgShapeRightClicked != null)
            SvgShapeRightClicked(this, e, mID);
      }
   }
}