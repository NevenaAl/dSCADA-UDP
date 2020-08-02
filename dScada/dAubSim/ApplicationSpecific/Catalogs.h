
/*** Beginheader defsts */
#define STS_NEPOZNATO                                 0
#define STS_NEMA_ULAZA                                1
#define STS_ZATVORENO                                 2
#define STS_OTVORENO                                  3
#define STS_PROMENA                                   4
#define STS_KVAR                                      5
#define STS_NORMALNO                                  6
#define STS_ALARM                                     7
#define STS_ISPAD                                     8
#define STS_PRORADA                                   9
#define STS_NAJAVA                                    10
#define STS_ISKLJUCENO                                11
#define STS_UKLJUCENO                                 12
#define STS_POD_NAPONOM                               13
#define STS_BEZ_NAPONA                                14
#define STS_ZABRANJENO                                15
#define STS_DOZVOLJENO                                16

/*** Beginheader defcmd */
#define CMD_NEPOZNATO                                 0
#define CMD_NEMA_IZLAZA                               1
#define CMD_ZABRANJENO                                2
#define CMD_ZATVORI                                   3
#define CMD_OTVORI                                    4
#define CMD_MIRUJ                                     5
#define CMD_ISKLJUCI                                  6
#define CMD_UKLJUCI                                   7

/*** Beginheader defdigdev */
#define DIG_PREKIDAC                                  0
#define DIG_RASTAVLJAC                                1
#define DIG_UZEMLJIVAC                                2
#define DIG_KONTAKTOR                                 3
#define DIG_DIGALARM                                  4
#define DIG_AUTOPRORADA                               5
#define DIG_AUTONAJAVA                                6
#define DIG_AUTOMAT                                   7
#define DIG_INDNAPONA                                 8
#define DIG_INDDKONTROLE                              9

/*** Beginheader defjed */
#define EU_V                                         0
#define EU_KV                                        1
#define EU_A                                         2
#define EU_KA                                        3
#define EU_MW                                        4
#define EU_MVAR                                      5
#define EU_MWH                                       6
#define EU_MVARH                                     7
#define EU_HZ                                        8
#define EU_OC                                        9
#define EU_MA                                        10
#define EU__                                         11

/*** Beginheader defkonv */

/*** Beginheader defobj */
#define OBJ_SABIRNICA                                 0
   #define OELM_SABIRNICA_PK                          0
   #define OELM_SABIRNICA_RS1                         1
   #define OELM_SABIRNICA_RS2                         2
   #define OELM_SABIRNICA_RZ                          3
   #define OELM_SABIRNICA_VPN                         4

#define OBJ_SPOJNICA                                  1
   #define OELM_SPOJNICA_SABIRNICA1                   0
   #define OELM_SPOJNICA_SABIRNICA2                   1
   #define OELM_SPOJNICA_RASTAVLJAC1                  2
   #define OELM_SPOJNICA_RASTAVLJAC2                  3

#define OBJ_SUMA                                      2
   #define OELM_SUMA_SABIRAK1                         0
   #define OELM_SUMA_SABIRAK2                         1

#define OBJ_IZVOD                                     3
  #define OELM_IZVOD_RS                               0
  #define OELM_IZVOD_PK                               1
  #define OELM_IZVOD_RI                               2
  #define OELM_IZVOD_RPS                              3
  #define OELM_IZVOD_RZ                               4
  #define OELM_IZVOD_JS                               5
  #define OELM_IZVOD_US                               6
  #define OELM_IZVOD_ZKS                              7
  #define OELM_IZVOD_ZZS                              8
  #define OELM_IZVOD_ZTP                              9
  #define OELM_IZVOD_VPN                             10

#define OBJ_TRAFO                                     4
  #define OELM_TRAFO_JR                               0
  #define OELM_TRAFO_JS                               1
  #define OELM_TRAFO_JT                               2
  #define OELM_TRAFO_UR                               3
  #define OELM_TRAFO_US                               4
  #define OELM_TRAFO_UT                               5
  #define OELM_TRAFO_FR                               6
  #define OELM_TRAFO_COSF                             7
  #define OELM_TRAFO_PA                               8
  #define OELM_TRAFO_PR                               9
  #define OELM_TRAFO_TEMP                            10
  #define OELM_TRAFO_EA                              11
  #define OELM_TRAFO_ER                              12
  #define OELM_TRAFO_OS                              13
  #define OELM_TRAFO_BN                              14
  #define OELM_TRAFO_BP                              15
  #define OELM_TRAFO_ZTP                             16
