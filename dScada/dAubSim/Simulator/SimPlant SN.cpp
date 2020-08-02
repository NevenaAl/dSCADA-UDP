#include "stdafx.h"
#include "ApplicationSpecific\Catalogs.h"
#include "Simulator.h"

#include "math.h"

#define oELM(obj, elm) &obj->fix.element[OELM_##elm]
#define izvELM(i, elm) &ss[i].izvod->fix.element[OELM_IZVOD_##elm]
#define trfELM(i, elm) &ts[i].trafo->fix.element[OELM_TRAFO_##elm]
#define sabELM(i, elm) &ts[i].sabirnica->fix.element[OELM_SABIRNICA_##elm]

//////////////////////////////////////////////////////////////////////////////////////

#define  TPO   50    // prenosni odnos transformatora 20->0.4 kV

// trafo stanice
#define T1     0
#define T2     1

// izvodi i izvori napajanja
#define S21    0
#define S22    1

// Vrednosti za PowerSouce (+ I21, I22)
#define NONE   0
#define I21    1
#define I22    2
#define OBA    3

// podaci za trafo SN trafo
typedef struct
{
   // ulazno
   float P;             // Aktivna snaga potrosaca u KW
   float cosfi;         // kosinus fi
   float temp;          // temperatura ulja
   // racunato
   float Q;             // Reaktivna snaga potrosaca u KVAr
   float u;             // linijski napon na NN u V
   float j;             // struja na NN u kA
   float incEA;         // inkrement aktivne energije
   float incER;         // inkrement reaktivne energije
   float sinfi;         // sinus fi
   int PowerSource;     // Izvor napajanja trafo stanice
   OBJECT *trafo;
   OBJECT *sabirnica;
} TRAFO_SIM;

typedef struct
{
   // ulazno
   float u;             // linijski napon na NN u kV
   // racunato
   float j;             // struja na NN u A
   OBJECT *izvod;
} SOURCE_SIM;

TRAFO_SIM ts[2];
SOURCE_SIM ss[2];

//////////////////////////////////////////////////////////////////////////////////////


// podesi simulaciju postrojenja
void SetPlantSimulator( void )
{
   // namesti OBJECT reference

   ss[S21].izvod = (OBJECT*) get_pvp( get_PVID("RTU-1", "IZV-I21") );
   ss[S22].izvod = (OBJECT*) get_pvp( get_PVID("RTU-1", "IZV-I22") );
   ts[T1].trafo  = (OBJECT*) get_pvp( get_PVID("RTU-1", "TF-T1") );
   ts[T2].trafo  = (OBJECT*) get_pvp( get_PVID("RTU-1", "TF-T2") );
   ts[T1].sabirnica  = (OBJECT*) get_pvp( get_PVID("RTU-1", "SAB-T1") );
   ts[T2].sabirnica  = (OBJECT*) get_pvp( get_PVID("RTU-1", "SAB-T2") );

   // pocetno uklopno stanje

   // I21: PK, RS, RI = CLOSE
   ssetPVAR( "RTU-1", "PK-I21", 0, STS_ZATVORENO );
   ssetPVAR( "RTU-1", "RS-I21", 0, STS_ZATVORENO );
   ssetPVAR( "RTU-1", "RI-I21", 0, STS_ZATVORENO );

   // T1: PK, RS1 = CLOSE
   ssetPVAR( "RTU-1", "PK-T1",  0, STS_ZATVORENO );
   ssetPVAR( "RTU-1", "RS1-T1", 0, STS_ZATVORENO );

   // I22: RS, PK, RI = CLOSE
   ssetPVAR( "RTU-1", "PK-I22", 0, STS_ZATVORENO );
   ssetPVAR( "RTU-1", "RS-I22", 0, STS_ZATVORENO );
   ssetPVAR( "RTU-1", "RI-I22", 0, STS_ZATVORENO );

   // T2: PK, RS2 = CLOSE
   ssetPVAR( "RTU-1", "PK-T2",  0, STS_ZATVORENO );
   ssetPVAR( "RTU-1", "RS2-T2", 0, STS_ZATVORENO );

   // postavi pocetne vrednosti simulacije
   for( int i=0; i<2; i++ )
   {
      ts[i].P = (float)3.800;              // MW
      ts[i].cosfi = (float)0.8;
      ts[i].temp = 70;
      ts[i].PowerSource = NONE;
      // fazni (linijski) napon na izvodima 20kV
      ss[i].u = 11.5;               // kV
   }

}


//////////////////////////////////////////////////////////////

void ZastitaTrafoa( int tsi )          // Zastita SN trafoa
{
   bool prorada = false;
   // Zastita bimetalom (termicko preopterecenje)
   // Prag struje ~ 1.2 Inominalno
   float jt = getEguVal( trfELM(tsi, JT) );
   if( jt > 9 )
   {
      ssetPVID( trfELM(tsi, ZTP), 0, STS_PRORADA, SET_ERROR );
      //ssetPVID( trfELM(tsi, ZTP), 0, STS_PRORADA );
      //prorada = true;
   }
   else
   {
      ssetPVID( trfELM(tsi, ZTP), 0, STS_NORMALNO );
   }

   // Buholz zastita - kad poraste temperatura ulja u trafou
   float temp_ulja = getEguVal( trfELM(tsi, TEMP) );
   if( temp_ulja > 120 )
   {
      ssetPVID( trfELM(tsi, BP), 0, STS_PRORADA );     // Buholz Alarm prorada
      ssetPVID( trfELM(tsi, BN), 0, STS_NAJAVA );      // Buholz Alarm najava
      prorada = true;
   }
   else if( temp_ulja > 100 )
   {
      ssetPVID( trfELM(tsi, BP), 0, STS_NORMALNO );    // Buholz Alarm prorada
      ssetPVID( trfELM(tsi, BN), 0, STS_NAJAVA );      // Buholz Alarm najava
   }
   else
   {
      ssetPVID( trfELM(tsi, BP), 0, STS_NORMALNO );    // Buholz Alarm prorada
      ssetPVID( trfELM(tsi, BN), 0, STS_NORMALNO );    // Buholz Alarm najava
   }
   
   // ako treba, otvori PK_T2
   if( prorada || (getDigSts(trfELM(tsi, ZTP))==STS_PRORADA) )
   {
      ssetPVID( sabELM(tsi, PK), 0, STS_OTVORENO ); 
   }
}

void ZastitaIzvoda( int ii  )
{
   bool prorada = false;
   // prvo odrade kontrole kratkog i zemljospoja (zks, zzs )
   if( (getDigSts(izvELM(ii, ZKS)) == STS_PRORADA) || (getDigSts(izvELM(ii, ZZS)) == STS_PRORADA) )
   {
      prorada = true;
   }
   else
   {
      // ako one nisu odradile, proveri preopterecenje voda (ztp)
      if( getEguVal(izvELM(ii, JS)) > 450 )
      {
         ssetPVID( izvELM(ii, ZTP), 0, STS_PRORADA, SET_ERROR );
         //prorada = true;
      }
      else
      {
         ssetPVID( izvELM(ii, ZTP), 0, STS_NORMALNO );
      }
   }
   if( prorada || (getDigSts(izvELM(ii, ZTP))==STS_PRORADA) )
   {
      // iskljuci PK_I22
      ssetPVID( izvELM(ii, PK), 0, STS_OTVORENO );
   }
}


void StanjeIzvoda( int ii )
{
   bool vpn = false;
   if( getDigSts(izvELM(ii, RZ)) == STS_OTVORENO )
   {
      if(  (getDigSts(izvELM(ii, RPS)) == STS_ZATVORENO) ||
          ((getDigSts(izvELM(ii, RS)) == STS_ZATVORENO) && (getDigSts(izvELM(ii, PK)) == STS_ZATVORENO) && (getDigSts(izvELM(ii, RI)) == STS_ZATVORENO)) )
      {
         vpn = true;
      }
   }
   // Postavi napon na izvodu I21
   if( vpn )
   {
      ssetPVID( izvELM(ii, VPN), 0, STS_POD_NAPONOM );
      ssetPVID( izvELM(ii, US), ss[ii].u, -1 );
   }
   else
   {
      ssetPVID( izvELM(ii, VPN), 0, STS_BEZ_NAPONA );
      ssetPVID( izvELM(ii, US), 0, -1 );
   }
}

void RunPlantSimulator( int Tsim )
{
   // simuliraj promene ulaznog napona i potrosnje
   for( int i=0; i<2; i++ )
   {
      ss[i].u = frand( ss[i].u, 10, 13 );     
      ts[i].P = frand( ts[i].P, 3, 5 );
      ts[i].cosfi = frand( ts[i].cosfi, 0.5, 1 );
      ts[i].temp = frand( ts[i].temp, 0, 150 );
      // obracunaj sinusf i Q za obe trafo stanice
      ts[i].sinfi = sqrt(1-pow(ts[i].cosfi, 2) );
      ts[i].Q = ts[i].P * (ts[i].sinfi / ts[i].cosfi);
   }

   for( int i=0; i<2; i++ )
   {
      // proveri ima li uslova za proradu automatske zastite
      ZastitaIzvoda( i );
      ZastitaTrafoa( i );
      // Da li su izvodi I21/I22 pod naponom ?
      StanjeIzvoda( i );
   }

   // sracunaj gde ima napona i kolike su struje

   // Da li su T1 i T2 pod naponom, i odakle se napajaju ?

   bool ps121  = (getDigSts(izvELM(S21, VPN)) == STS_POD_NAPONOM) &&            // T1 sa I21
                 (getDigSts(izvELM(S21, RZ)) == STS_OTVORENO) && (getDigSts(sabELM(T1, RS1)) == STS_ZATVORENO);

   //bool ps121  = (getDigSts(&ss[S21].izvod->fix.element[OELM_IZVOD_VPN]) == STS_POD_NAPONOM) &&            // T1 sa I21
   //              (getDigSts(&ss[S21].izvod->fix.element[OELM_IZVOD_RZ]) == STS_OTVORENO) &&
   //              (getDigSts(&ts[T1].trafo->fix.element[OELM_TRAFO_RS1]) == STS_ZATVORENO);

   bool ps222  = (getDigSts(izvELM(S22, VPN)) == STS_POD_NAPONOM) &&            // T2 sa I22
                 (getDigSts(izvELM(S22, RZ)) == STS_OTVORENO) && (getDigSts(sabELM(T2, RS2)) == STS_ZATVORENO);

   bool spojka = (ps121 || ps222) &&                                                                     // aktivna spojka
                 (getDigSts(sabELM(T1, RS2)) == STS_ZATVORENO) && (getDigSts(sabELM(T2, RS1)) == STS_ZATVORENO);

   ts[T1].PowerSource = ts[T2].PowerSource = NONE;
   if( spojka )
   {
      if( ps121 && ps222 )
      {
         ts[T1].PowerSource = ts[T2].PowerSource = OBA;
      }
      else if( ps121 )
      {
         ts[T1].PowerSource = I21;
         ts[T2].PowerSource = I21;
      }
      else if( ps222 )
      {
         ts[T1].PowerSource = I22;
         ts[T2].PowerSource = I22;
      }
   }
   else
   {
      if( ps121 )
         ts[T1].PowerSource = I21;

      if( ps222 )
         ts[T2].PowerSource = I22;
   }

   // Postavi signale za T1 i T2
   for( int i=0; i<2; i++ )
   {
      ts[i].u = ts[i].j = 0;
      ts[i].incEA = ts[i].incER = 0;
      if( ts[i].PowerSource )
      {
         ssetPVID( sabELM(i, VPN), 0, STS_POD_NAPONOM );
         // ima napajanja, da li je i TS prikljucena
         if( (getDigSts(sabELM(i, PK)) == STS_ZATVORENO) && (getDigSts(trfELM(i, OS)) == STS_NORMALNO) )
         {
            // TS je u pogonu - odredi napone i struje na sekundaru (strani 0.4 kV)
            switch( ts[i].PowerSource )
            {
            case I21:
               ts[i].u = ss[S21].u * (1000/TPO);   break;                     // napon I21 u voltima
            case I22:
               ts[i].u = ss[S22].u * (1000/TPO);   break;                     // napon I22 u voltima
            case OBA:
               ts[i].u = ((ss[S21].u + ss[S22].u)/2) * (1000/TPO);   break;   // usrednji napon na oba izvoda
            }
            ts[i].j = (ts[i].P*1000) / (3*ts[i].u*ts[i].cosfi);      // *1000 zbog konverzije egu
            ts[i].incEA = (ts[i].P*Tsim)/3600;
            ts[i].incER = (ts[i].Q*Tsim)/3600;
         }
      }
      else
      {
         ssetPVID( sabELM(i, VPN), 0, STS_BEZ_NAPONA );
      }

      // postavi napone
      ssetPVID( trfELM(i, UR), ts[i].u, -1 );                 // V
      ssetPVID( trfELM(i, US), ts[i].u, -1 );
      ssetPVID( trfELM(i, UT), ts[i].u, -1 );

      // postavi struje
      ssetPVID( trfELM(i, JR), (float)(ts[i].j*0.95), -1 );   // kA
      ssetPVID( trfELM(i, JS), ts[i].j, -1 );
      ssetPVID( trfELM(i, JT), (float)(ts[i].j*1.05), -1 );

      // postavi snagu 
      if( ts[i].j > 0 )
      {
         ssetPVID( trfELM(i, PA), ts[i].P, -1 );
         ssetPVID( trfELM(i, PR), ts[i].Q, -1 );
      }
      else
      {
         ssetPVID( trfELM(i, PA), 0, -1 );
         ssetPVID( trfELM(i, PR), 0, -1 );
      }

      // Akumulisana snaga = energija
      if( ts[i].incEA > 0 )
      {
         float fval = getEguVal(trfELM(i, EA)) + ts[i].incEA;
         ssetPVID( trfELM(i, EA), fval, -1 );
         fval = getEguVal(trfELM(i, ER)) + ts[i].incER;
         ssetPVID( trfELM(i, ER), fval, -1 );
      }

      // prenesi temperaturu ulja, cos(fi) i frekvenciju 
      ssetPVID( trfELM(i, TEMP), ts[i].temp, -1 );
      if( ts[i].u > 0 )
      {
         ssetPVID( trfELM(i, COSF), ts[i].cosfi, -1 );
         ssetPVID( trfELM(i, FR), frand(50, 48, 52), -1 );
      }
      else
      {
         ssetPVID( trfELM(i, COSF), 0, -1 );
         ssetPVID( trfELM(i, FR), 0, -1 );
      }

   }

   // namesti struju u granama I21 i I22

   // prvo sracunaj struje iz oba izvoda na nivou 0.4kV
   ss[S21].j = ss[S22].j = 0;
   switch( ts[T1].PowerSource )
   {
   case I21:   ss[S21].j += ts[T1].j;     break;
   case I22:   ss[S22].j += ts[T1].j;     break;
   case OBA:   ss[S21].j += ts[T1].j/2;   ss[S22].j += ts[T1].j/2;   break;
   }
   switch( ts[T2].PowerSource )
   {
   case I21:   ss[S21].j += ts[T2].j;     break;
   case I22:   ss[S22].j += ts[T2].j;     break;
   case OBA:   ss[S21].j += ts[T2].j/2;   ss[S22].j += ts[T2].j/2;   break;
   }

   // potom svedi struje na nivo 20kV i prevedi u Ampere
   ss[S21].j = (ss[S21].j * 1050) / TPO;        // dodao 5% zbog gubitaka
   ssetPVID( izvELM(S21, JS), ss[S21].j, -1 );
   ss[S22].j = (ss[S22].j * 1060) / TPO;        // dodao 6% zbog gubitaka
   ssetPVID( izvELM(S22, JS), ss[S22].j, -1 );

}

///////////////////////////////////////////////////////////////////////////

// postavlja stanje i komandu digitalnog uredjaja
// iskljucivo na osnovu tipa uredjaja - validno stanje 
void SetPlantDigDev( DIGITAL *digp )
{
   switch( digp->fix.type )
   {
   case DIG_PREKIDAC:
   case DIG_RASTAVLJAC:
   case DIG_UZEMLJIVAC:
      digp->var.state = STS_OTVORENO;
      digp->var.command = CMD_MIRUJ;
      break;
   case DIG_KONTAKTOR:
      digp->var.state = STS_ISKLJUCENO;
      digp->var.command = CMD_ISKLJUCI;
      break;
   case DIG_DIGALARM:
   case DIG_AUTOPRORADA:
   case DIG_AUTONAJAVA:
   case DIG_AUTOMAT:
      digp->var.state = STS_NORMALNO;
      break;
   case DIG_INDNAPONA:
      digp->var.state = STS_BEZ_NAPONA;
      break;
   case DIG_INDDKONTROLE:
      digp->var.state = STS_DOZVOLJENO;
      break;
 default:
    break;
   }
}

// postavlja legalan odgovor na komandu
void MakeCmdResponse( DIGITAL *digp )
{
   bool done = false;
   
   // proveri da li je dozvoljena daljinska komanda
   char *tname =  strrchr( digp->fix.name, '-' );
   if( tname != NULL )
   {
      if( !strcmp(tname, "-T1") )
      {
         if( getDigSts(get_PVID("RTU-1", "DK-T1")) == STS_ZABRANJENO )
            return;
      }
      else if( !strcmp(tname, "-T2") )
      {
         if( getDigSts(get_PVID("RTU-1", "DK-T2")) == STS_ZABRANJENO )
            return;
      }
   }

   switch( digp->fix.type )
   {
   case DIG_PREKIDAC:
   case DIG_RASTAVLJAC:
   case DIG_UZEMLJIVAC:
      switch( digp->var.cmd_requested )
      {
      case CMD_OTVORI: 
         digp->var.state = STS_OTVORENO; done = true;
         break;
      case CMD_ZATVORI:
         digp->var.state = STS_ZATVORENO; done = true;
         break;
      }
      break;

   case DIG_KONTAKTOR:
      switch( digp->var.cmd_requested )
      {
      case CMD_ISKLJUCI: 
         digp->var.state = STS_ISKLJUCENO; done = true;
         break;
      case CMD_UKLJUCI:
         digp->var.state = STS_UKLJUCENO; done = true;
         break;
      }
      break;

   default:
      break;
   }

   if( done )
   {
      digp->chg[CHG_VAR] = CHG_SIMUL;
   }
}