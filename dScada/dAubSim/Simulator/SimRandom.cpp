#include "stdafx.h"
#include "conversion.h"

//
// Random simulacija procesnih ULAZA
//

#define AI_RATE   0           // verovatnoca promene
#define AI_CHANGE 8           // iznos promene izrazen u jedinicama db_opsega
void randAI( ANAIN *ainp )
{
   int r = rand() % 100;
   if( r < AI_RATE )
   {
      float promena = ainp->opr.deadband * (r % AI_CHANGE);
      if( r > AI_RATE/2 )
      {
         ainp->var.egu_value+= promena;        // uvecava se
         if( ainp->var.egu_value> ainp->opr.egu_range[1] )
            ainp->var.egu_value= ainp->opr.egu_range[1];
      }
      else
      {
         ainp->var.egu_value-= promena;        // smanjuje se
         if( ainp->var.egu_value< ainp->opr.egu_range[0] )
            ainp->var.egu_value= ainp->opr.egu_range[0];
      }
      ainp->chg[CHG_VAR] = CHG_SIMUL;
   }
}


#define CNT_RATE   0            // verovatnoca promene
#define CNT_CHANGE 8             // iznos promene izrazen u jedinicama db_opsega
void randCNT( COUNTER *cntp )
{
   int r = rand() % 100;
   if( r < CNT_RATE )
   {
      float promena = cntp->opr.deadband * (r % CNT_CHANGE);
      cntp->var.egu_value+= promena;        // uvecava se
      cntp->chg[CHG_VAR] = CHG_SIMUL;
   }
}

#define DI_RATE   0           // verovatnoca promene
void randDigDev( DIGITAL *digp )
{
   int r = rand() % 100;
   if( r < DI_RATE )
   {
      int no_sts = pow_i( 2,digp->fix.no_in );
      SCHAR novo  = digp->fix.states[r % no_sts];
      if( novo != digp->var.state )
      {
         digp->var.state = novo;
         digp->chg[CHG_VAR] = CHG_SIMUL;
      }
   }
}

