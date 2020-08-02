#include "stdafx.h"
#include "conversion.h"


// funkcije za racunanje slucajnih varijacija
float frand( float value, float range_min, float range_max )
{
   double dd = (range_max - range_min) * 0.01;           // 1% od ukupnog opsega
   double ret = value + ((double)rand() / (RAND_MAX + 1)) * 2 * dd - dd;

   if( ret > range_max )
      ret = range_max;
   else if( ret < range_min )
      ret = range_min;
   return (float) ret;
}

int irand( int value, int range_min, int range_max )
{
   if( value == range_min )
      return value;

   return (int) frand( (float)value, (float)range_min, (float)range_max );
}

/*----------------------------------------------------------------------*/
/*       Funkcije za postavljanje simuliranih vrednosti                 */
/*       set_error flag postavlja gresku koja trazi maualni reset       */
/*----------------------------------------------------------------------*/

//    pristup preko PVID
void ssetPVID( PVID *pvidp, float egu_value, SCHAR state, int set_error )
{
   // uzmi OPR status procesne varijable
   WORD status = get_status( pvidp, OPR_PART );
   // proveri da li je aktivna
   if( !get_bit(status, OPR_ACT) )
      return;
   // ako nije unos greske, a greska se vec desila
   if( !set_error && get_bit(status, OPR_MAN_VAL) )
      return;

   switch( pvidp->type )
   {
   case PVT_AI:
      {
         ANAIN *aip = (ANAIN*) get_pvp( pvidp );
         if( aip->var.egu_value != egu_value )
         {
            aip->var.egu_value = egu_value;
            if( set_error )
               put_bit( aip->opr.status, OPR_MAN_VAL, 1 );
            aip->chg[CHG_VAR] = CHG_SIMUL;
         }
      }
      break;
   case PVT_CNT:
      {
         COUNTER *cp = (COUNTER*) get_pvp( pvidp );
         if( cp->var.egu_value != egu_value )
         {
            cp->var.egu_value = egu_value;
            if( set_error )
               put_bit( cp->opr.status, OPR_MAN_VAL, 1 );
            cp->chg[CHG_VAR] = CHG_SIMUL;
         }
      }
      break;
   case PVT_DIG:
      {
         DIGITAL *digp = (DIGITAL*) get_pvp( pvidp );
         if( !get_bit(digp->var.status, DIG_CMD_REQ) )
         {
            if( digp->fix.no_in && (state != -1) && (state != digp->var.state) )
            {
               digp->var.state = state;
               if( set_error )
                  put_bit( digp->opr.status, OPR_MAN_VAL, 1 );
               digp->chg[CHG_VAR] = CHG_SIMUL;
            }
         }
      }
      break;
   default:
      PutEvent( pvidp, "Simulation request not valid !" );
      break;
   }
}

//    pristup preko imena  RTU i PVAR
void ssetPVAR( char *rtu_name, char* pvar_name, float egu_value, SCHAR state, int set_error )
{
   PVID* pvidp = get_PVID( rtu_name, pvar_name );
   if( pvidp == NULL )
      return;
   else
      ssetPVID( pvidp, egu_value, state, set_error );
}

