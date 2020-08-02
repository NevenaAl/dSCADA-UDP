#include "StdAfx.h"

#include <direct.h>

int ReadConfigFile( void )
{
	FILE *fcfg;
	int ret = OK;
	char buff[128], keyWord[200], arg0[400];

	// otvaranje datoteke
	if( (fcfg=fopen( "dConfig.txt", "rt" ))==NULL )
	{
		return NOK;
	}

    // postavi default vrednosti
    _getcwd( StnCfg.SysFolder, sizeof(StnCfg.SysFolder) );
    strcpy( StnCfg.StaFolder, StnCfg.SysFolder );

	while( !feof(fcfg) && (ret==OK) ) 
	{
		memset(buff, 0, 128);
		if( fgets(buff, 128, fcfg) && (strlen(buff) > 2) )
		{
			// Read keyword
			if( sscanf(buff, "%s", keyWord) != 1 )
			{
				ret = NOK;
				break;
			}

			// read argument
			if(strlen(buff + strlen(keyWord)) > 0)
			{
				if(sscanf(buff + strlen(keyWord), " %[^\n]", arg0) != 1)
				{
					ret = NOK;
					break;
				}
			}
			// da li je komentar
			if( !strncmp(keyWord, "//", 2) )
			{
				continue;
			}

			// proveri sta si ucitao
			if( !strcmp(keyWord, "SysFolder") )
			{
				strcpy( StnCfg.SysFolder, arg0 );
				// iskopiraj ga i na StaFolder kao default
				strcpy( StnCfg.StaFolder, arg0 );
			}
			else if( !strcmp(keyWord, "StaFolder") )
			{
				strcpy( StnCfg.StaFolder, arg0 );
			}
			else if( !strcmp(keyWord, "RtuCfg") && strstr(arg0, ".txt") )
			{
				strcpy( StnCfg.PlcCfg, arg0 );
			}
			else if( !strcmp(keyWord, "AubCfg") && strstr(arg0, ".txt") )
			{
				strcpy( StnCfg.AubCfg, arg0 );
			}
			else if( !strcmp(keyWord, "SvgFile") && strstr(arg0, ".svg") )
			{
				strcpy( StnCfg.SvgGrp, arg0 );
			}
			else if( !strcmp(keyWord, "Station") )
			{
				strcpy( StnCfg.StaName, arg0 );
			}
			else
			{
				ret = NOK;
			}
		}
	}

	fclose( fcfg );

	return ret;
}