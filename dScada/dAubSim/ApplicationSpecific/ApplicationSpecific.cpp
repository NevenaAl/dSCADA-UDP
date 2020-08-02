#include "StdAfx.h"

#include "Catalogs.h"


/*----------------------------------------------------------------------*/
/*              Funkcija za restauraciju zadnje commands                 */
/*----------------------------------------------------------------------*/

void RestoreLastCommand( DIGITAL *digp )
{
   switch( digp->fix.type )
   {
   case DIG_PREKIDAC:
   case DIG_RASTAVLJAC:
   case DIG_UZEMLJIVAC:
      switch( digp->var.state )
      {
      case STS_OTVORENO: 
         digp->var.cmd_requested = CMD_OTVORI;
         break;
      case STS_ZATVORENO: 
         digp->var.cmd_requested = CMD_ZATVORI;
         break;
      case STS_PROMENA:
      case STS_KVAR:
         set( digp->var.status, DIG_ALARM );
         break;
      }
      break;

   case DIG_KONTAKTOR:                             // mada je ovde to nepotrebno
      switch( digp->var.state )
      {
      case STS_ISKLJUCENO: 
         digp->var.cmd_requested = CMD_ISKLJUCI;
         break;
      case STS_UKLJUCENO: 
         digp->var.cmd_requested = CMD_UKLJUCI;
         break;
      }
      break;

   default :
      break;
   }
}

/*----------------------------------------------------------------------*/
/*              Funkcija za proveru spontanih promena                   */
/*----------------------------------------------------------------------*/

void CheckSpontaneousChange( DIGITAL *digp )
{
   reset( digp->var.status, DIG_ALARM );
   switch( digp->fix.type )
   {
   case DIG_PREKIDAC:
   case DIG_RASTAVLJAC:
   case DIG_UZEMLJIVAC:
      switch( digp->var.state )
      {
      case STS_OTVORENO: 
         if( digp->var.cmd_requested != CMD_OTVORI )
            set( digp->var.status, DIG_ALARM );
         break;
      case STS_ZATVORENO: 
         if( digp->var.cmd_requested != CMD_ZATVORI )
            set( digp->var.status, DIG_ALARM );
         break;
      case STS_PROMENA:
      case STS_KVAR:
         set( digp->var.status, DIG_ALARM );
         break;
      }

   case DIG_KONTAKTOR:
      switch( digp->var.state )
      {
      case STS_ISKLJUCENO: 
         if( digp->var.command != CMD_ISKLJUCI )
            set( digp->var.status, DIG_ALARM );
         break;
      case STS_UKLJUCENO: 
         if( digp->var.command != CMD_UKLJUCI )
            set( digp->var.status, DIG_ALARM );
         break;
      }
      break;

      // svako stanje sem NORMALNO je alarm
   case DIG_DIGALARM:
   case DIG_AUTOPRORADA:
   case DIG_AUTONAJAVA:
   case DIG_AUTOMAT:
      if( digp->var.state != STS_NORMALNO )
         set( digp->var.status, DIG_ALARM );
      break;

      // stanje BEZ_NAPONA je alarm
   case DIG_INDNAPONA:
      if( digp->var.state == STS_BEZ_NAPONA )
         set( digp->var.status, DIG_ALARM );
      break;

      // stanje ZABRANJENO je alarm
   case DIG_INDDKONTROLE:
      if( digp->var.state == STS_ZABRANJENO )
         set( digp->var.status, DIG_ALARM );
      break;

   default :
      break;
   }

   // ispisi SVAKI dogadjaj
   if( get_bit(digp->var.status, DIG_ALARM) )
      PutEvent( &digp->fix.mID, "Spontanious Alarm %s", rtdb.DigState[digp->var.state].state );
   else if( !get_bit(digp->var.status, DIG_IN_SCAN1) )
      PutEvent( &digp->fix.mID, "Spontanious Normal %s", rtdb.DigState[digp->var.state].state );
}

// aplikativna funkcija provere izdatih komandi pre izvrsenja
int ValidateDigitalCommand( DIGITAL *digp, SCHAR cmd)
{
   int ret = OK;

   if( cmd == CMD_ZABRANJENO )
   {
      ret = NOK;
   }
   else
   {
      switch( digp->fix.type )
      {
      //case DIG_RASTAVLJAC:
      case DIG_UZEMLJIVAC:
         if( (cmd == CMD_ZATVORI) && (digp->fix.objID.type == PVT_OBJ) )          // rastavljac za uzemljenje sabirnice ili izvoda
         {
            OBJECT *sabizv = (OBJECT*) get_pvp( &digp->fix.objID );
            if( (sabizv->var.state == STS_POD_NAPONOM) )                        // da li je pod naponom ?
               ret = NOK;
         }
         break;
      case DIG_PREKIDAC:
      case DIG_KONTAKTOR:
      default:
         break;
      }
   }
   
   if( ret != OK )
      PutEvent( &digp->fix.mID, "DigCmd %s rejected, sts = %d", rtdb.DigCmd[cmd].command, ret );

   return ret;
}

/**************************************************************
*                                                             *
*         Proverava da li je zahtevana komanda izvrsena       *
*                                                             *
***************************************************************/
int CheckCommandExecution( DIGITAL *digp )
{
   int ret = NOK;

   // proveri odgovor uredjaja na komandu (ulaze)
   switch( digp->fix.type )
   {
   case DIG_PREKIDAC:
   case DIG_RASTAVLJAC:
   case DIG_UZEMLJIVAC:
      {
         switch( digp->var.command )
         {
         case CMD_OTVORI: 
            if( digp->var.state == STS_OTVORENO )
               ret = OK;
            break;
         case CMD_ZATVORI:
            if(digp->var.state == STS_ZATVORENO)
               ret = OK;
            break;
         }
         if( (ret==OK) || !digp->var.cmd_timer )  // uspesno izvreseno ili je istekao timeout
         {
            // posalji neutralnu komandu MIRUJ - 00 na digitalne izlaze
            PutCommand( &digp->fix.mID, CMD_MIRUJ, CMD_SKIP_PROC );
         }
      }
      break;

   case DIG_KONTAKTOR:
      switch( digp->var.command )
      {
      case CMD_ISKLJUCI: 
         if( digp->var.state == STS_ISKLJUCENO )
            ret = OK;
         break;
      case CMD_UKLJUCI:
         if(digp->var.state == STS_UKLJUCENO)
            ret = OK;
         break;
      }
      break;

   default:
      ret = OK;
      break;
   }

   return ret;
}

bool ValidateObjectState( OBJECT *obj, SCHAR state )
{
   bool valid = FALSE;
   switch( obj->fix.type )
   {
   case OBJ_IZVOD:                
   case OBJ_SABIRNICA:                
      if( state==STS_POD_NAPONOM || state==STS_BEZ_NAPONA )
         valid = TRUE;
      break;
   case OBJ_SPOJNICA:
      if( state==STS_ISKLJUCENO || state==STS_UKLJUCENO )
         valid = TRUE;
      break;
   default:
      valid = TRUE;
      break;
   }
   return valid;
}

bool ValidateObjectCommand( OBJECT *obj, SCHAR command )
{
   bool valid = FALSE;
   switch( obj->fix.type )
   {
   case OBJ_SPOJNICA:
      if( command==CMD_ISKLJUCI || command==CMD_UKLJUCI || command==CMD_MIRUJ )
         valid = TRUE;
      break;
   case OBJ_IZVOD:                
   case OBJ_SABIRNICA: 
      if( command == CMD_NEMA_IZLAZA )
         valid = TRUE;   
   default:
      break;
   }
   return valid;
}

#define elmID(obj,i) &obj->fix.element[i] 
#define prm(obj,i) &obj->fix.param[i] 

bool ProcessObject( OBJECT *obj )
{
   bool chg = false;

   switch( obj->fix.type )
   {
   case OBJ_SABIRNICA:        // preuzima stanje VPN uredjaja (Vod Pod Naponom)
      {
         DIGITAL *vpn = (DIGITAL*)get_pvp( &obj->fix.element[OELM_SABIRNICA_VPN] );
         if( obj->var.state != vpn->var.state )
         {
            chg = SetObjectValue( obj, 0, vpn->var.state, CMD_NEMA_IZLAZA, false );
         }
      }
      break;

   case OBJ_IZVOD:        // preuzima stanje VPN uredjaja (Vod Pod Naponom)
      {
         DIGITAL *vpn = (DIGITAL*)get_pvp( &obj->fix.element[OELM_IZVOD_VPN] );
         if( obj->var.state != vpn->var.state )
         {
             chg = SetObjectValue(obj, 0, vpn->var.state, CMD_NEMA_IZLAZA, false);
         }
      }
      break;

   case OBJ_SPOJNICA:
      {
         // odredi stanje spojnice
         SCHAR ssp = STS_ISKLJUCENO;            // stanje spojnice
         if( (getDigSts(elmID(obj, OELM_SPOJNICA_RASTAVLJAC1))==STS_ZATVORENO) && (getDigSts(elmID(obj, OELM_SPOJNICA_RASTAVLJAC2))==STS_ZATVORENO) )
         {
            ssp = STS_UKLJUCENO;
         }

         // namesti stanje spojnice
         if( ssp != obj->var.state )
         {
             chg = SetObjectValue(obj, 0, ssp, -1, false);
         }

         // da li je izdata komanda
         if( obj->var.status & OBJ_CMD_REQ )
         {
            put_bit( obj->var.status, OBJ_CMD_REQ, 0 );
            chg = true;

            SCHAR cmd = -1;
            if( obj->var.command==CMD_UKLJUCI && ssp!=STS_UKLJUCENO )
            {
               // proveri da li su obe sabirnice pod naponom
               if( (getDigSts(elmID(obj, OELM_SPOJNICA_SABIRNICA1))==STS_POD_NAPONOM) && (getDigSts(elmID(obj, OELM_SPOJNICA_SABIRNICA2))==STS_POD_NAPONOM) )
               {
                  PutEvent( &obj->fix.mID, "DigCmd %s rejected, obe sabirnice pod naponom", rtdb.DigCmd[obj->var.command].command );
               }
               else
               {
                  cmd = CMD_ZATVORI;
               }
            }
            else if( obj->var.command==CMD_ISKLJUCI && ssp!=STS_ISKLJUCENO )
            {
               cmd = CMD_OTVORI;
            }
            if( cmd != -1 )
            {
               PutCommand( elmID(obj, OELM_SPOJNICA_RASTAVLJAC1), cmd, 0 );
               PutCommand( elmID(obj, OELM_SPOJNICA_RASTAVLJAC2), cmd, 0 );
            }
         }
         // namesti komandu
         if( obj->var.command != CMD_MIRUJ )
         {
             chg = SetObjectValue(obj, 0, -1, CMD_MIRUJ, false);
         }
      }
      break;

   case OBJ_SUMA:
      {
         float suma = getEguVal( elmID(obj, OELM_SUMA_SABIRAK1) ) + getEguVal( elmID(obj, OELM_SUMA_SABIRAK2) );
         if( obj->var.egu_value!= suma )
         {
             chg = SetObjectValue(obj, suma, -1, -1, false);
         }
      }
      break;

   default:
      break;
   }

   return chg;
}

#include <sys\timeb.h>

int initial_control( void )
{
   int ret = NOK;
   for( int rtu=0; rtu < rtdb.no_rtu; rtu++ )
   {
      RTUT* rtup = rtdb.rtut + rtu;
      if( !rtup->var.first_scan && (rtup->var.err == 0) )
      {
         ret = OK;
      }
   }
   return ret;
}

// automatsko brisanje brojaca, svakog minuta
void AutoClearCounters( void )
{
   for( int r=0; r < rtdb.no_rtu; r++ )
   {
      RTUT* rtup = rtdb.rtut + r;
      for( int i=0; i < rtup->fix.cntfut.total; i++ )
      {
         COUNTER *cp = rtup->fix.cntfut.cnt_t + i;
         if( cp->opr.status & OPR_AUTOCLR )
         {
            if( (Now.tm_hour == cp->opr.clrhour) && (Now.tm_min == cp->opr.clrmin) )
            {
               // obrisi brojac jer je vreme
               PutClearCounter( &cp->fix.mID, CMD_PUT_EVENT );
            }
         }
      }
   }
}

void AutoProcedure( void )
{
   static bool first_lap = true;

   if( first_lap )
   {
      int sts = OK;
      sts = initial_control(); 
      if( sts == OK )
         first_lap = false;
   }
   else
   {
      if( !(timeS % 60) )
         AutoClearCounters();
   }

}