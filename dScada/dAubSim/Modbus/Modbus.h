// Modbus.h : main header file for the Modbus DLL
//

// definicija poruka koje se razmenjuju preko MODBUS kanala
#define READ_COILS      1      // Ocitava DO status
#define READ_INPUTS     2      // Ocitava DI status
#define READ_HOLDREGS   3      // Ocitava Holding registre
#define READ_INREGS     4      // Ocitava AI registre (u odbrojima)
#define FORCE_COIL      5      // Postavlja jedan DO
#define FORCE_COILS    15      // Postavlja niz DO (ne koristimo)
#define SET_HOLDREG     6      // Postavlja jedan Holding registar (ne koristimo)
#define SET_HOLDREGS   16      // Postavlja niz Holding registara

// definicija tipova Modbus registara
enum
{ 
   DO_REG=1, DI_REG, IN_REG, HR_INT,  HR_LONG, HR_FLT
};

typedef struct _modbus_card
{
   // deo koji se konfigurise
   short reg_type;         // tip podatka u registru MODBUS uredjaja
   BYTE  reg_len;          // duzina MODBUS registra u bajtima
   short byte_order;       // kako su BYTE-ovi rasporedjeni
   short dec_point;        // pozicija decimalne tacke
   BYTE  rd_code;          // kod poruke za upit
   BYTE  wr_code;          // kod poruke za zapis
   WORD  adr_reg;          // adresa prvog registra
   short no_reg;           // broj registara u modulu (seriji)
   // deo koji se obracunava
   short pio_type;         // AI, AO, CNT, DI, DO
   short no_pio;           // broj PIO po modulu
   short base_hwa;         // bazna adresa PIO bloka  
} MODBUS_CARD;

typedef struct _modbus_cfg
{
   int no_cards;               // broj konfigurisanih MODBUS kartica
   MODBUS_CARD *mdb_cards;       // direktorijum kartiva
} MODBUS_CFG;



