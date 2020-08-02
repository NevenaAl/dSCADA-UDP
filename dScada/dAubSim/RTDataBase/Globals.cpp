/***********************************************************************/
/*       Globals je modul u kom se nalazi RealTime baza podataka       */
/***********************************************************************/

#include "stdafx.h"

SYSTEM   rtdb;

#ifndef _dSIM
extern RTU_PROCEDURE MODBUS_protokol;

RTU_PROCEDURE *RTU_protokoli[PROTOCOL_TYPE_NO] = 
{ 
   NULL,    //&TANDEM_protokol,       //TANDEM_DATA
   NULL,    //&REPBUS_protokol,      //REPBUS
   &MODBUS_protokol,       //USERTYPE1_DATA
   NULL,                   //USERTYPE2_DATA
   NULL,                   //USERTYPE3_DATA
   NULL,                   //USERTYPE4_DATA
   NULL,                   //USERTYPE5_DATA
   NULL,                   //USERTYPE6_DATA
   NULL,                   //USERTYPE7_DATA
   NULL,                   //USERTYPE8_DATA
};
#endif


char* ColorNames[] =
{
"AliceBlue",           "AntiqueWhite",    "Aqua",             "Aquamarine",       "Azure",          "Beige",       
"Bisque",              "Black",           "BlanchedAlmond",   "Blue",             "BlueViolet",     "Brown",
"BurlyWood",           "CadetBlue",       "Chartreuse",       "Chocolate",        "Coral",          "CornflowerBlue",
"Cornsilk",            "Crimson",         "Cyan",             "DarkBlue",         "DarkCyan",       "DarkGoldenrod",
"DarkGray",            "DarkGreen",       "DarkKhaki",        "DarkMagena",       "DarkOliveGreen", "DarkOrange",  
"DarkOrchid",          "DarkRed",         "DarkSalmon",       "DarkSeaGreen",     "DarkSlateBlue",  "DarkSlateGray",
"DarkTurquoise",       "DarkViolet",      "DeepPink",         "DeepSkyBlue",      "DimGray",        "DodgerBlue",
"Firebrick",           "FloralWhite",     "ForestGreen",      "Fuchsia",          "Gainsboro",      "GhostWhite",
"Gold",                "Goldenrod",       "Gray",             "Green",            "GreenYellow",    "Honeydew",
"HotPink",             "IndianRed",       "Indigo",           "Ivory",            "Khaki",          "Lavender",
"LavenderBlush",       "LawnGreen",       "LemonChiffon",     "LightBlue",        "LightCoral",     "LightCyan",
"LightGoldenrodYellow","LightGray",       "LightGreen",       "LightPink",        "LightSalmon",    "LightSeaGreen",
"LightSkyBlue",        "LightSlateGray",  "LightSteelBlue",   "LightYellow",      "Lime",           "LimeGreen",
"Linen",               "Magenta",         "Maroon",           "MediumAquamarine", "MediumBlue",     "MediumOrchid",
"MediumPurple",        "MediumSeaGreen",  "MediumSlateBlue",  "MediumSpringGreen","MediumTurquoise","MediumVioletRed",
"MidnightBlue",        "MintCream",       "MistyRose",        "Moccasin",         "NavajoWhite",    "Navy",
"OldLace",             "Olive",           "OliveDrab",        "Orange",           "OrangeRed",      "Orchid",
"PaleGoldenrod",       "PaleGreen",       "PaleTurquoise",    "PaleVioletRed",    "PapayaWhip",     "PeachPuff",
"Peru",                "Pink",            "Plum",             "PowderBlue",       "Purple",         "Red",
"RosyBrown",           "RoyalBlue",       "SaddleBrown",      "Salmon",           "SandyBrown",     "SeaGreen",
"Seashell",            "Sienna",          "Silver",           "SkyBlue",          "SlateBlue",      "SlateGray",
"Snow",                "SpringGreen",     "SteelBlue",        "Tan",              "Teal",           "Thistle",
"Tomato",              "Turquoise",       "Violet",           "Wheat",          "White",          "WhiteSmoke",
"Yellow",              "YellowGreen",     NULL
};