using System.Collections.Generic;

namespace WpfDSCADA.SVG
{
   public enum SVGPATHCMD
   {
      SVG_PATH_CMD_MOVE_TO,
      SVG_PATH_CMD_REL_MOVE_TO,
      SVG_PATH_CMD_LINE_TO,
      SVG_PATH_CMD_REL_LINE_TO,
      SVG_PATH_CMD_HORIZONTAL_LINE_TO,
      SVG_PATH_CMD_REL_HORIZONTAL_LINE_TO,
      SVG_PATH_CMD_VERTICAL_LINE_TO,
      SVG_PATH_CMD_REL_VERTICAL_LINE_TO,
      SVG_PATH_CMD_CURVE_TO,
      SVG_PATH_CMD_REL_CURVE_TO,
      SVG_PATH_CMD_SMOOTH_CURVE_TO,
      SVG_PATH_CMD_REL_SMOOTH_CURVE_TO,
      SVG_PATH_CMD_QUADRATIC_CURVE_TO,
      SVG_PATH_CMD_REL_QUADRATIC_CURVE_TO,
      SVG_PATH_CMD_SMOOTH_QUADRATIC_CURVE_TO,
      SVG_PATH_CMD_REL_SMOOTH_QUADRATIC_CURVE_TO,
      SVG_PATH_CMD_ARC_TO,
      SVG_PATH_CMD_REL_ARC_TO,
      SVG_PATH_CMD_CLOSE_PATH,
      SVG_PATH_CMD_UNKNOWN
   }

   /// <summary>
   /// Class SVGPath
   /// </summary>
   /// 
   public class SVGPath
   {
      public SVGPATHCMD cmd;
      public List<double> coords_list;
      public SVGStyle style;

      /// <summary>
      /// Constructor with no parametars
      /// </summary>
      public SVGPath()
      {
         style = null;
         cmd = SVGPATHCMD.SVG_PATH_CMD_UNKNOWN;
         coords_list = new List<double>();
      }

      /// <summary>
      /// CharToComand 
      /// </summary>
      /// <param name="c"></param>
      /// <returns></returns>
      /// 
      public SVGPATHCMD CharToCmd(char c)
      {
         SVGPATHCMD cmd;

         switch (c)
         {
            case 'M': cmd = SVGPATHCMD.SVG_PATH_CMD_MOVE_TO; break;
            case 'm': cmd = SVGPATHCMD.SVG_PATH_CMD_REL_MOVE_TO; break;
            case 'L': cmd = SVGPATHCMD.SVG_PATH_CMD_LINE_TO; break;
            case 'l': cmd = SVGPATHCMD.SVG_PATH_CMD_REL_LINE_TO; break;
            case 'H': cmd = SVGPATHCMD.SVG_PATH_CMD_HORIZONTAL_LINE_TO; break;
            case 'h': cmd = SVGPATHCMD.SVG_PATH_CMD_REL_HORIZONTAL_LINE_TO; break;
            case 'V': cmd = SVGPATHCMD.SVG_PATH_CMD_VERTICAL_LINE_TO; break;
            case 'v': cmd = SVGPATHCMD.SVG_PATH_CMD_REL_VERTICAL_LINE_TO; break;
            case 'C': cmd = SVGPATHCMD.SVG_PATH_CMD_CURVE_TO; break;
            case 'c': cmd = SVGPATHCMD.SVG_PATH_CMD_REL_CURVE_TO; break;
            case 'S': cmd = SVGPATHCMD.SVG_PATH_CMD_SMOOTH_CURVE_TO; break;
            case 's': cmd = SVGPATHCMD.SVG_PATH_CMD_REL_SMOOTH_CURVE_TO; break;
            case 'Q': cmd = SVGPATHCMD.SVG_PATH_CMD_QUADRATIC_CURVE_TO; break;
            case 'q': cmd = SVGPATHCMD.SVG_PATH_CMD_REL_QUADRATIC_CURVE_TO; break;
            case 'T': cmd = SVGPATHCMD.SVG_PATH_CMD_SMOOTH_QUADRATIC_CURVE_TO; break;
            case 't': cmd = SVGPATHCMD.SVG_PATH_CMD_REL_SMOOTH_QUADRATIC_CURVE_TO; break;
            case 'A': cmd = SVGPATHCMD.SVG_PATH_CMD_ARC_TO; break;
            case 'a': cmd = SVGPATHCMD.SVG_PATH_CMD_REL_ARC_TO; break;
            case 'Z': cmd = SVGPATHCMD.SVG_PATH_CMD_CLOSE_PATH; break;
            case 'z': cmd = SVGPATHCMD.SVG_PATH_CMD_CLOSE_PATH; break;
            default: cmd = SVGPATHCMD.SVG_PATH_CMD_UNKNOWN; break;
         }

         return cmd;
      }


      /// <summary>
      /// Get methods
      /// </summary>
      ///
      public SVGPATHCMD GetCmd()
      {
         return cmd;
      }

      //public double[] GetCoords()
      //{
      //    return coords;
      //}

      public SVGStyle GetStyle()
      {
         return style;
      }

      /// <summary>
      /// Set methods
      /// </summary>
      /// 
      public void SetCmd(SVGPATHCMD set_cmd)
      {
         cmd = set_cmd;
      }

      public void SetStyle(SVGStyle set_style)
      {
         style = set_style;
      }

   }
}
