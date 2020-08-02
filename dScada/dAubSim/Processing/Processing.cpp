/**
*  @file  Processing.cpp
*
*  @brief Implementation of functions for processing data received
*         trough acquisition and commands received from clients.
*
*/
#include "StdAfx.h"
#include <math.h>
#include "conversion.h"
#include "processing.h"
#include "ApplicationSpecific\ApplicationSpecific.h"

void pubChg(PVID *pvid, int chgmask = VAR_PART);

// publicate a VAR change to Slave and RepTo stations
void pubChg(PVID *pvid, int chgmask)
{
}



/*----------------------------------------------------------------------*/
/*              Funkcije za obradu analognih ulaza                      */
/*----------------------------------------------------------------------*/
void analog_input_alarm(ANAIN *ai)
{
    // manji od donje granice alarma
    if (ai->var.egu_value < ai->opr.alarm_lim[0])
    {
        if (!(ai->var.status & AI_LO_ALARM))
        {
            put_bit(ai->var.status, AI_LO_ALARM, 1);
            put_bit(ai->var.status, AI_HI_ALARM, 0);
            PutEvent(&ai->fix.mID, "Low Alarm %.2f %s", ai->var.egu_value, rtdb.EGUnit[ai->fix.egu_code].egu);
        }
        return;
    }

    // veci od gornjeg alarma
    if (ai->var.egu_value > ai->opr.alarm_lim[1])
    {
        if (!(ai->var.status & AI_HI_ALARM))
        {
            put_bit(ai->var.status, AI_HI_ALARM, 1);
            put_bit(ai->var.status, AI_LO_ALARM, 0);
            PutEvent(&ai->fix.mID, "High Alarm %.2f %s", ai->var.egu_value, rtdb.EGUnit[ai->fix.egu_code].egu);
        }
        return;
    }

    // izlazak iz alarma se pomera za "dead band" vrednost
    if (ai->var.status & AI_LO_ALARM)
    {
        if (ai->var.egu_value > ai->opr.alarm_lim[0] + ai->opr.deadband)
        {
            put_bit(ai->var.status, AI_LO_ALARM, 0);
            PutEvent(&ai->fix.mID, "Low Stop %.2f %s", ai->var.egu_value, rtdb.EGUnit[ai->fix.egu_code].egu);
        }
    }
    else if (ai->var.status & AI_HI_ALARM)
    {
        if (ai->var.egu_value < ai->opr.alarm_lim[1] - ai->opr.deadband)
        {
            put_bit(ai->var.status, AI_HI_ALARM, 0);
            PutEvent(&ai->fix.mID, "High Stop %.2f %s", ai->var.egu_value, rtdb.EGUnit[ai->fix.egu_code].egu);
        }
    }

    return;
}

void ProcessAnalogInput(ANAIN *ai, int rawValue, float euValue)
{
    WORD ad_count;

    if (rawValue == -1)     // preracunaj euval u odbroje
    {
        ad_count = (WORD)LinearConversion(euValue, ai->fix.raw_range[0], ai->fix.raw_range[1],
            ai->opr.egu_range[0], ai->opr.egu_range[1], 0);
    }
    else
        ad_count = (WORD)rawValue;

    // obrada se forsira u prvom skeniranju ili ako se tako trazi
    if (ai->var.status & (AI_SCAN1 | AI_FORCE))
    {
        if (get_bit(ai->var.status, AI_SCAN1))
            put_bit(ai->var.status, AI_SCAN1, 0);
    }
    else if (ai->var.raw_value == ad_count)
    {
        return;         // Ako nije bilo promene, vrati se 
    }

    // obrada je pocela
    ai->var.raw_value = ad_count;
    ai->var.timestamp = timeS;

    // Ako je neispravno merenje ORL
    if ((ai->var.raw_value < ai->fix.raw_range[0]) || (ai->var.raw_value > ai->fix.raw_range[1]))
    {
        put_bit(ai->var.status, AI_ORL, 1);        // postavi ORL status
        PutEvent(&ai->fix.mID, "ORL Alarm %d", ai->var.raw_value);
    }
    else
    {
        // ako je prethodno merenje bilo neispravno
        if (ai->var.status & AI_ORL)
        {
            put_bit(ai->var.status, AI_ORL, 0);                     // ukloni ORL status
            put_bit(ai->var.status, AI_LO_ALARM | AI_HI_ALARM, 0);    // obezbedi isprabnu proveru granica
            PutEvent(&ai->fix.mID, "ORL Stop %d", ai->var.raw_value);
        }
        // Merenje je ispravno, pretvori u EU format
        ai->var.egu_value = LinearConversion(ai->var.raw_value, ai->fix.raw_range[0], ai->fix.raw_range[1],
            ai->opr.egu_range[0], ai->opr.egu_range[1], 1);
        //provera granica
        analog_input_alarm(ai);
    }
    ai->chg[CHG_VAR] = CHG_FINAL;

    // publikuj promenu
    pubChg(&ai->fix.mID);

    return;
}

/*----------------------------------------------------------------------*/
/*              Funkcije za obradu analognih izlaza                     */
/*----------------------------------------------------------------------*/
void analog_output_alarm(ANAOUT *ao)
{
    //manji od donje granice alarma
    if (ao->var.egu_value < ao->opr.alarm_lim[0])
    {
        if (!(ao->var.status & AO_LO_ALARM))
        {
            put_bit(ao->var.status, AO_LO_ALARM, 1);
            put_bit(ao->var.status, AO_HI_ALARM, 0);
            PutEvent(&ao->fix.mID, "Low Alarm %.2f %s", ao->var.egu_value, rtdb.EGUnit[ao->fix.egu_code].egu);
        }
        return;
    }

    //veci od gornjeg alarma
    if (ao->var.egu_value > ao->opr.alarm_lim[1])
    {
        if (!(ao->var.status & AO_HI_ALARM))
        {
            put_bit(ao->var.status, AO_HI_ALARM, 1);
            put_bit(ao->var.status, AO_LO_ALARM, 0);
            PutEvent(&ao->fix.mID, "High Alarm %.2f %s", ao->var.egu_value, rtdb.EGUnit[ao->fix.egu_code].egu);
        }
        return;
    }

    // izlazak iz alarma pomeren za "deadband" vrednost
    if (ao->var.status & AO_LO_ALARM)
    {
        if (ao->var.egu_value > ao->opr.alarm_lim[0] + ao->opr.deadband)
        {
            put_bit(ao->var.status, AO_LO_ALARM, 0);
            PutEvent(&ao->fix.mID, "High Stop %.2f %s", ao->var.egu_value, rtdb.EGUnit[ao->fix.egu_code].egu);
        }
    }
    else if (ao->var.status & AO_HI_ALARM)
    {
        if (ao->var.egu_value < ao->opr.alarm_lim[1] - ao->opr.deadband)
        {
            put_bit(ao->var.status, AO_HI_ALARM, 0);
            PutEvent(&ao->fix.mID, "Low Stop %.2f %s", ao->var.egu_value, rtdb.EGUnit[ao->fix.egu_code].egu);
        }
    }

    return;
}

float CalculateAnalogCommand(ANAOUT *analogOutput)
{
    float next_value, next_count;

    // prvo odredi za koliko smes da promenis AO
    long dT = timeS - analogOutput->var.timestamp;      // toliko je proteklo od zadnje akvizicije
    float korak = analogOutput->opr.max_rate * dT;

    /*postavljanje analognog izlaza kroz korake*/
    if (fabs(analogOutput->var.egu_value - analogOutput->var.req_value) <= korak)
    {
        // dosli smo do kraja
        next_value = analogOutput->var.req_value;
        put_bit(analogOutput->var.status, AO_CMD_REQ, 0);
    }
    else
    {
        next_value = analogOutput->var.egu_value;
        // odredi sledecu vrednost 
        if (next_value < analogOutput->var.req_value)
            next_value += korak;
        else
            next_value -= korak;
    }
    // proracunaj sledecu (ili zadnju) komandu
    next_count = LinearConversion(next_value, analogOutput->fix.raw_range[0], analogOutput->fix.raw_range[1], analogOutput->opr.egu_range[0], analogOutput->opr.egu_range[1], 0);

    return next_count;
}

void ProcessAnalogOutput(ANAOUT *analogOutput, WORD rawValue, float euValue)
{
    WORD da_odbroj;
    bool chg = false;

    if (rawValue == -1)     // preracunaj euval u odbroje
        da_odbroj = (WORD)LinearConversion(euValue, analogOutput->fix.raw_range[0], analogOutput->fix.raw_range[1], analogOutput->opr.egu_range[0], analogOutput->opr.egu_range[1], 0);
    else
        da_odbroj = rawValue;

    // Ima li promene, ili je SCAN1
    if ((analogOutput->var.raw_value != da_odbroj) || (analogOutput->var.status & AO_SCAN1) || (analogOutput->var.status & AO_FORCE))
    {
        chg = true;
        put_bit(analogOutput->var.status, AO_SCAN1, 0);
        analogOutput->var.raw_value = da_odbroj;
        analogOutput->var.egu_value = LinearConversion(rawValue, analogOutput->fix.raw_range[0], analogOutput->fix.raw_range[1], analogOutput->opr.egu_range[0], analogOutput->opr.egu_range[1], 1);
        analog_output_alarm(analogOutput);
    }

    // vidi da li je komanda izdata nad AO
    if (get_bit(analogOutput->var.status, AO_CMD_REQ))
    {
#ifndef _dSIM
        //int protokol = rtdb.rtut[ao->fix.mID.rtu].fix.protokol;
        int chn = rtdb.rtut[analogOutput->fix.mID.rtu].fix.channel;
        int protokol = rtdb.chanet[chn].fix.protocol;
        RTU_protokoli[protokol]->send_command( &analogOutput->fix.mID, CalculateAnalogCommand(analogOutput) );
#endif
        chg = true;
    }

    if (chg)
    {
        // oznaci promenu
        analogOutput->var.timestamp = timeS;
        analogOutput->chg[CHG_VAR] = CHG_FINAL;
        // publikuj promenu
        pubChg(&analogOutput->fix.mID);
    }

    return;
}


/*----------------------------------------------------------------------*/
/*              Funkcije za obradu countera                             */
/*----------------------------------------------------------------------*/

bool counter_input_alarm(COUNTER *cp, bool chg)
{
    bool chg2 = chg;

    if (chg)
    {
        if (cp->var.status & CNT_STALLED)
        {
            put_bit(cp->var.status, CNT_STALLED, 0);
            PutEvent(&cp->fix.mID, "Counter Active %.2f %s", cp->var.egu_value, rtdb.EGUnit[cp->fix.egu_code].egu);
        }
    }
    else if (cp->opr.watch_period && !get_bit(cp->var.status, CNT_SCAN1) && !(cp->var.status & CNT_STALLED))
    {
        if (timeS - cp->var.timestamp > cp->opr.watch_period)
        {
            chg2 = true;
            put_bit(cp->var.status, CNT_STALLED, 1);
            PutEvent(&cp->fix.mID, "Counter Stalled %.2f %s", cp->var.egu_value, rtdb.EGUnit[cp->fix.egu_code].egu);
        }
    }

    return chg2;
}

void ProcessCounter(COUNTER *counter, long rawValue, float euValue)
{
    long counts;
    bool chg = false;

    if (rawValue == -1)     // preracunaj euval u odbroje
        counts = (long)(euValue / (counter->opr.mul_const / counter->opr.div_const));
    else
        counts = rawValue;

    if ((counter->var.status & (CNT_SCAN1 | CNT_FORCE)) || (counter->var.raw_value != counts))
    {
        // obracunaj novu vrednost
        counter->var.raw_value = counts;
        counter->var.egu_value = counts * (counter->opr.mul_const / counter->opr.div_const);
        // resetuj scan1 flag
        if (get_bit(counter->var.status, CNT_SCAN1))
            put_bit(counter->var.status, CNT_SCAN1, 0);
        chg = true;
    }

    // proveri status
    chg = counter_input_alarm(counter, chg);

    // vremenski oznaci promenu, ako se desila
    if (chg)
    {
        // oznaci promenu
        counter->var.timestamp = timeS;
        counter->chg[CHG_VAR] = CHG_FINAL;
        // publikuj promenu
        pubChg(&counter->fix.mID);
    }

    return;
}

// on_clr_counter_done se poziva nakon prijema potvrde o uspesnom brisanju brojaca
void OnClearCounterDone(COUNTER* cntp)
{
    // zamrzni stanje pre brisanja
    cntp->var.frozen_value = cntp->var.egu_value;
    cntp->var.lastclear = cntp->var.timestamp;
    // namesti novo stanje
    cntp->var.egu_value = 0;
    cntp->var.raw_value = 0;
    put_bit(cntp->var.status, CNT_IN_CLEAR, 0);
    cntp->var.timestamp = timeS;
    // ispisi event
    PutEvent(&cntp->fix.mID, "Counter Cleared");
}

int GetDigitalOutputValue(int rtu, int DOAddress)
{
    BYTE pod = *(rtdb.rtut[rtu].fix.digfut.rawdat_out + DOAddress / 8);
    pod &= 1 << (DOAddress % 8);
    if (pod)
        return 1;
    else
        return 0;
}

// zapisi raw_value DO
void SetDigitalOutputValue(int rtu, int address, int value)
{
    put_bit(rtdb.rtut[rtu].fix.digfut.rawdat_out[address / 8], 1 << (address % 8), value);
}

void ProcessDigitalOutput(DIGITAL *digital)
{
    BYTE bin_cmd = 0;
    for (int i = 0; i < digital->fix.no_out; i++)
    {
        bin_cmd = bin_cmd << 1;
        bin_cmd |= GetDigitalOutputValue(digital->fix.mID.rtu, digital->fix.hw_out[i]);
    }
    SCHAR nova_komanda = digital->fix.commands[bin_cmd];
    if ((digital->var.command != nova_komanda) || (digital->var.status & (DIG_OUT_SCAN1 | DIG_FORCE)))
    {
        put_bit(digital->var.status, DIG_OUT_SCAN1, 0);
        digital->var.command = nova_komanda;
        digital->var.timestamp = timeS;
        digital->chg[CHG_VAR] = CHG_FINAL;
        // publikuj promenu
        pubChg(&digital->fix.mID);
    }
}

// pokupi raw_value DI
int GetRawDigitalInput(int rtu, int diAddress)
{
    BYTE pod = *(rtdb.rtut[rtu].fix.digfut.rawdat_in + diAddress / 8);
    pod &= 1 << (diAddress % 8);
    if (pod)
        return 1;
    else
        return 0;
}

// zapisi raw_value DI
void PutRawDigitalInput(int rtu, int di_adresa, int di_bit)
{
    if (di_bit) di_bit = 1;       // svedi int na bit ( vrednost 0/1 )
    put_bit(rtdb.rtut[rtu].fix.digfut.rawdat_in[di_adresa / 8], 1 << (di_adresa % 8), di_bit);
}

// obrada ocitanih DI
void ProcessDigitalInput(DIGITAL *digp)    // provera stanja ulaza
{
    BYTE di, bin_sts = 0;
    bool chg = false;

    // novo stanje odgovara binarnoj kombinaciji ulaza
    for (int i = 0; i < digp->fix.no_in; i++)
    {
        bin_sts = bin_sts << 1;
        di = GetRawDigitalInput(digp->fix.mID.rtu, digp->fix.hw_in[i]);
        bin_sts |= di;
    }
    SCHAR novo_stanje = digp->fix.states[bin_sts];
    // ako je promena ili se mora obraditi
    if ((digp->var.state != novo_stanje) || (digp->var.status & (DIG_IN_SCAN1 | DIG_IN_SCAN2 | DIG_FORCE)))
    {
        chg = true;
        digp->var.state = novo_stanje;
        digp->var.timestamp = timeS;

        if (get_bit(digp->var.status, DIG_CMD_REQ))
            goto kraj;

        // prvi put
        if (get_bit(digp->var.status, DIG_IN_SCAN1))
        {
            put_bit(digp->var.status, DIG_IN_SCAN1, 0);
            // vidi sta bi mogla biti zadnja komanda
            if (digp->fix.no_out)
                RestoreLastCommand(digp);
        }

        // pri vracanju tacke Manual -> Mereno
        if (get_bit(digp->var.status, DIG_IN_SCAN2))
        {
            put_bit(digp->var.status, DIG_IN_SCAN2, 0);
        }

        // svaki put proveri spontanu promenu
        CheckSpontaneousChange(digp);
    }

kraj:
    if (chg)
    {
        digp->chg[CHG_VAR] = CHG_FINAL;
        // publikuj promenu
        pubChg(&digp->fix.mID);
    }
}

void CommandInspector(void)
{
    int sts = NOK;
    bool chg = false;
    for (int r = 0; r < rtdb.no_rtu; r++)
    {
        RTUT *rtup = rtdb.rtut + r;
        for (int i = 0; i < rtup->fix.digfut.total; i++)
        {
            DIGITAL *digp = rtup->fix.digfut.dig_t + i;
            if (get_bit(digp->var.status, DIG_CMD_REQ))
            {
                // smanji preostalo vreme do timeouta
                if (digp->var.cmd_timer)
                    digp->var.cmd_timer--;

                // ako je izdata komanda realizovana
                if (digp->var.cmd_requested == digp->var.command)
                {
                    // proveri da li je bila uspesna (izazvala promenu ulaza)
                    sts = CheckCommandExecution(digp);
                }

                if (sts == OK)
                {
                    digp->var.cmd_timer = 0;
                    put_bit(digp->var.status, DIG_CMD_REQ, 0);
                    put_bit(digp->var.status, DIG_ALARM, 0);
                    chg = true;
                }
                else if (!digp->var.cmd_timer)  // istekao timeout
                {
                    put_bit(digp->var.status, DIG_CMD_REQ, 0);
                    put_bit(digp->var.status, DIG_ALARM, 1);
                    chg = true;
                }

                if (chg)
                {
                    digp->chg[CHG_VAR] = CHG_FINAL;
                    // publikuj promenu
                    pubChg(&digp->fix.mID);
                }
            }
        }
    }
}


// vraca binarni ekvivalent digitalne commands
int GetRawCommand(DIGITAL *digp, SCHAR cmd)
{
    int ret = -1;
    for (int i = 0; i < pow_i(2, digp->fix.no_out); i++)
    {
        // nadji ofset commands u lokalnom katalogu komandi
        if (digp->fix.commands[i] == cmd)
        {
            ret = i;           // binarna komanda koju treba izvrsiti
            break;
        }
    }
    return ret;
}

/*----------------------------------------------------------------------*/
/*    Funkcije za izdavanje komandi. -1 oznacava lokalnu stanicu        */
/*----------------------------------------------------------------------*/
int PutCommand(PVID *pvid, float value, WORD cmdopt, short origin_stn)
{
    RTUT *rtup = rtdb.rtut + pvid->rtu;
    int protokol = rtdb.chanet[rtup->fix.channel].fix.protocol;

    int put_event = get_bit(cmdopt, CMD_PUT_EVENT);
    int skip_proc = get_bit(cmdopt, CMD_SKIP_PROC);
    int man_req = get_bit(cmdopt, CMD_MAN_REQ);

    if (origin_stn == -1)
    {
        origin_stn = StnCfg.MyStaId;
    }

    // uzmi trenutnu prednost opr statusa i proveri da li se komanda sme izvrsiti
    WORD status = get_status(pvid, OPR_PART);
    if (get_bit(status, OPR_CMD_INH))       // proveri zabranu komandi
    {
        PutLocEvent(pvid, "Command not allowed (Cmd Tag)!");
        return -1;
    }
    else if (man_req && get_bit(status, OPR_MCMD_INH)) // proveri zabranu rucnih komandi
    {
        PutLocEvent(pvid, "Command not allowed (ManCmd Tag)!");
        return -1;
    }

    // proveri validnost komande - da li se sme izvrsiti
    bool cmdok = false;
    switch (pvid->type)
    {
    case PVT_AO:               // proveri clamp limite
    {
        ANAOUT *ao = (ANAOUT *)get_pvp(pvid);
        if ((value >= ao->opr.clamp_lim[0]) || (value <= ao->opr.clamp_lim[1]))
            cmdok = true;
    }
    break;

    case PVT_CNT:              // tretiracemo svaku izdatu komandu kao clear counter
        cmdok = true;
        break;

    case PVT_DIG:              // da li ima izlaze i da li je komanda izvrsiva
    {
        SCHAR cmd = (SCHAR)value;
        DIGITAL *digp = (DIGITAL *)get_pvp(pvid);
        // ako digitalni uredjaj ima izlaza
        if (digp->fix.no_out)
        {
            // da li izdata komanda postoji u lokalnom katalogu
            for (int i = 0; i < pow_i(2, digp->fix.no_out); i++)
            {
                if (cmd == digp->fix.commands[i])
                {
                    // da li se sme izdati zbog uslova u postrojenju ?
                    int rqsts = ValidateDigitalCommand(digp, cmd);
                    if (rqsts == OK)
                        cmdok = true;
                    else
                        PutStaEvent(origin_stn, &digp->fix.mID, "Command %s rejected, status = %d", rtdb.DigCmd[cmd].command, rqsts);
                    break;
                }
            }
        }
    }
    break;

    case PVT_OBJ:
    {
        OBJECT *obj = (OBJECT *)get_pvp(pvid);
        if (obj->fix.obj_class == DIG_OBJ)
        {
            SCHAR cmd = (SCHAR)value;
            // da li je komanda validna i sme li izdati zbog uslova u postrojenju ?
            if (ValidateObjectCommand(obj, cmd))
                cmdok = true;
        }
    }
    break;

    case PVT_AI: //ne mogu se izdati komande ovim uredjajima
    default:
        break;
    }

    if (!cmdok)
    {
        PutLocEvent(pvid, "Invalid Cmd Request!");
        return -1;
    }

    // izvrsi sta moras, vreme je
    switch (pvid->type)
    {
    case PVT_AO:
    {
        ANAOUT *ao = (ANAOUT *)get_pvp(pvid);
        ao->var.req_value = value;
        put_bit(ao->var.status, AO_CMD_REQ, 1);
        ao->var.timestamp = timeS;
        ao->chg[CHG_VAR] = CHG_FINAL;
#ifdef _dSIM
        ao->var.egu_value = ao->var.req_value;
        put_bit(ao->var.status, AO_CMD_REQ, 0);
#endif
        // publikuj promenu
        pubChg(&ao->fix.mID);
        wputs(4, "PUT_CMD: %s = %.2f", ao->fix.name, value);
        if (put_event)
            PutStaEvent(origin_stn, &ao->fix.mID, "AnaOutCmd request %.2f %s", value, rtdb.EGUnit[ao->fix.egu_code].egu);
    }
    break;

    case PVT_CNT: //tretiracemo izdatu komandu kao clear counter
    {
        COUNTER *cntp = (COUNTER*)get_pvp(pvid);
        put_bit(cntp->var.status, CNT_IN_CLEAR, 1);
        cntp->var.timestamp = timeS;
        cntp->chg[CHG_VAR] = CHG_FINAL;
#ifndef _dSIM
        RTU_protokoli[protokol]->send_command( &cntp->fix.mID, 0 );
#else
        OnClearCounterDone(cntp);
#endif
        // publikuj promenu
        pubChg(&cntp->fix.mID);
        wputs(4, "CLR_CNT: %s = %d", cntp->fix.name, 0);
        if (put_event)
            PutStaEvent(origin_stn, &cntp->fix.mID, "Clear Counter request");
    }
    break;

    case PVT_DIG:
    {
        SCHAR cmd = (SCHAR)value;
        DIGITAL *digp = (DIGITAL *)get_pvp(pvid);
        if (!skip_proc)
        {
            digp->var.cmd_requested = cmd;
            put_bit(digp->var.status, DIG_CMD_REQ, 1);
            digp->var.cmd_timer = digp->fix.cmd_timout;
            digp->var.timestamp = digp->var.cmd_timestamp = timeS;
            digp->chg[CHG_VAR] = CHG_FINAL;
            // ispisi event
            if (put_event)
                PutStaEvent(origin_stn, &digp->fix.mID, "DigCmd request %s", rtdb.DigCmd[cmd].command);
        }
        wputs(4, "PUT_CMD: %s = %s", digp->fix.name, rtdb.DigCmd[cmd].command);
#ifndef _dSIM
        RTU_protokoli[protokol]->send_command( &digp->fix.mID, cmd );
#else
        digp->var.command = cmd;
#endif
        // publikuj promenu
        pubChg(&digp->fix.mID);
    }
    break;

    case PVT_OBJ:
    {
        OBJECT *obj = (OBJECT *)get_pvp(pvid);
        if (obj->fix.obj_class == ANA_OBJ)
        {
            obj->var.req_value = value;
            obj->var.timestamp = timeS;
            obj->chg[CHG_VAR] = CHG_FINAL;
            // publikuj promenu
            pubChg(&obj->fix.mID);
            wputs(4, "PUT_CMD: %s = %.2f", obj->fix.name, value);
            if (put_event)
                PutStaEvent(origin_stn, &obj->fix.mID, "AnaObjCmd request %.2f %s", value, rtdb.EGUnit[obj->fix.egu_code].egu);
        }
        else
        {
            SCHAR cmd = (SCHAR)value;
            obj->var.command = cmd;
            obj->var.timestamp = timeS;
            put_bit(obj->var.status, OBJ_CMD_REQ, 1);
            obj->chg[CHG_VAR] = CHG_FINAL;
            // publikuj promenu
            pubChg(&obj->fix.mID);
            if (put_event)
                PutStaEvent(origin_stn, &obj->fix.mID, "DigObjCmd request %s", rtdb.DigCmd[cmd].command);
            wputs(4, "PUT_CMD: %s = %s", obj->fix.name, rtdb.DigCmd[cmd].command);
        }
    }
    break;

    default:
        break;
    }

    return 0;
}

// ova funkcija je namenjena za pisanje APP procedure
int PutClearCounter(PVID *pvid, WORD cmdopt, short origin_stn)
{
    if (pvid->type == PVT_CNT)
    {
        return PutCommand(pvid, 0, cmdopt, origin_stn);
    }
    PutLocEvent(pvid, "Invalid ClearCounter Request!");
    return -1;
}

void ObjectsProcessor(void)
{
    for (int r = 0; r < rtdb.no_rtu; r++)
    {
        RTUT *rtup = rtdb.rtut + r;
        for (int j = 0; j < rtup->fix.objfut.total; j++)
        {
            OBJECT *objp = rtup->fix.objfut.obj_t + j;
            if (get_bit(objp->opr.status, OPR_MAN_VAL))
                continue;
            if (ProcessObject(objp))
            {
                objp->chg[CHG_VAR] = CHG_FINAL;
                objp->var.timestamp = timeS;
                // publikuj promenu
                pubChg(&objp->fix.mID);
            }
        }
    }
}


// pomocna funkcija za azuriranje trenutnog stanja objekata
// posle nekog proracuna ili provere u okviru object_processing
bool SetObjectValue(OBJECT *obj, float egu_value, SCHAR state, SCHAR command, bool setchg)
{
    bool chg = false;
    if (obj->fix.obj_class == ANA_OBJ)
    {
        obj->var.egu_value = egu_value;
        chg = true;
    }
    else
    {
        if (state >= 0)
        {
            if (ValidateObjectState(obj, state))
            {
                obj->var.state = state;
                chg = true;
            }
        }
        if (command >= 0)
        {
            if (ValidateObjectCommand(obj, command))
            {
                obj->var.command = command;
                chg = true;
            }
        }
    }
    if (chg && setchg)
    {
        // oznaci i pubkikuj promenu
        obj->chg[CHG_VAR] = CHG_FINAL;
        obj->var.timestamp = timeS;
        pubChg(&obj->fix.mID);

    }
    return chg;
}

/*----------------------------------------------------------------------*/
/*       Funkcije za realizaciju manual direktiva  od client-a          */
/*----------------------------------------------------------------------*/
void SetManualValue(PVID *pvid, float value)
{
#ifndef _dSIM
    BYTE chg_value = CHG_FINAL;
#else
    BYTE chg_value = CHG_SIMUL;  // postavlja se za pUlaze da bi ih simulator tretirao kao promene
#endif
    switch (pvid->type)
    {
    case PVT_AI:
    {
        ANAIN *aip = (ANAIN*)get_pvp(pvid);
        aip->opr.man_value = aip->var.egu_value = value;
        put_bit(aip->opr.status, OPR_MAN_VAL, 1);
        aip->var.timestamp = timeS;
        aip->chg[CHG_VAR] = aip->chg[CHG_OPR] = chg_value;
    }
    break;
    case PVT_AO:
    {
        ANAOUT *aop = (ANAOUT*)get_pvp(pvid);
        aop->opr.man_value = aop->var.egu_value = value;
        put_bit(aop->opr.status, OPR_MAN_VAL, 1);
        aop->var.timestamp = timeS;
        aop->chg[CHG_VAR] = aop->chg[CHG_OPR] = CHG_FINAL;
    }
    break;
    case PVT_CNT:
    {
        COUNTER *cp = (COUNTER*)get_pvp(pvid);
        cp->opr.man_value = cp->var.egu_value = value;
        put_bit(cp->opr.status, OPR_MAN_VAL, 1);
        cp->var.timestamp = timeS;
        cp->chg[CHG_VAR] = cp->chg[CHG_OPR] = chg_value;
    }
    break;
    case PVT_DIG:
    {
        DIGITAL *digp = (DIGITAL*)get_pvp(pvid);
        digp->opr.oper_in = digp->var.state = (SCHAR)value;
        put_bit(digp->opr.status, OPR_MAN_VAL, 1);
        digp->var.timestamp = timeS;
        digp->chg[CHG_VAR] = digp->chg[CHG_OPR] = chg_value;
    }
    break;
    case PVT_OBJ:
    {
        OBJECT *objp = (OBJECT*)get_pvp(pvid);
        if (objp->fix.obj_class == ANA_OBJ)
        {
            objp->opr.man_value = objp->var.egu_value = value;
        }
        else
        {
            objp->opr.oper_in = objp->var.state = (SCHAR)value;
        }
        put_bit(objp->opr.status, OPR_MAN_VAL, 1);
        objp->var.timestamp = timeS;
        objp->chg[CHG_VAR] = objp->chg[CHG_OPR] = CHG_FINAL;
    }
    break;
    }
}

void ResetManualFlag(PVID *pvid)
{
    void *ppv = get_pvp(pvid);
    switch (pvid->type)
    {
    case PVT_AI:
        put_bit(((ANAIN*)ppv)->opr.status, OPR_MAN_VAL, 0);
        put_bit(((ANAIN*)ppv)->var.status, AI_SCAN1, 1);
        break;
    case PVT_AO:
        put_bit(((ANAOUT*)ppv)->opr.status, OPR_MAN_VAL, 0);
        put_bit(((ANAOUT*)ppv)->var.status, AO_SCAN1, 1);
        break;
    case PVT_CNT:
        put_bit(((COUNTER*)ppv)->opr.status, OPR_MAN_VAL, 0);
        put_bit(((COUNTER*)ppv)->var.status, CNT_SCAN1, 1);
        break;
    case PVT_DIG:
        put_bit(((DIGITAL*)ppv)->opr.status, OPR_MAN_VAL, 0);
        put_bit(((DIGITAL*)ppv)->var.status, DIG_IN_SCAN2, 1);
        break;
    case PVT_OBJ:
        put_bit(((OBJECT*)ppv)->opr.status, OPR_MAN_VAL, 0);
        break;
    }
    put_chg(pvid, CHG_VAR, CHG_FINAL);
    put_chg(pvid, CHG_OPR, CHG_FINAL);
}


int checkManReq(PVID *pvid, float value)
{
    int ret = NOK;
    switch (pvid->type)
    {
    case PVT_AI:
    {
        ANAIN *aip = (ANAIN*)get_pvp(pvid);
        if ((value >= aip->opr.egu_range[0]) && (value <= aip->opr.egu_range[1]))
            ret = OK;
    }
    break;
    case PVT_AO:
    {
        ANAOUT *aop = (ANAOUT*)get_pvp(pvid);
        if ((value >= aop->opr.egu_range[0]) && (value <= aop->opr.egu_range[1]))
            ret = OK;
    }
    break;
    case PVT_CNT:
        ret = OK;
        break;
    case PVT_DIG:
    {
        SCHAR sts = (SCHAR)value;
        DIGITAL *digp = (DIGITAL*)get_pvp(pvid);
        // ako nema ulaza, nema ni manual stanja
        if (digp->fix.no_in)
        {
            // da li izdato stanje postoji u lokalnom katalogu
            for (int i = 0; i < pow_i(2, digp->fix.no_in); i++)
            {
                if (sts == digp->fix.states[i])
                {
                    ret = OK;
                    break;
                }
            }
        }
    }
    break;
    case PVT_OBJ:
    {
        OBJECT *objp = (OBJECT*)get_pvp(pvid);
        if (objp->fix.obj_class == ANA_OBJ)
        {
            ret = OK;
        }
        else
        {
            SCHAR sts = (SCHAR)value;
            if (ValidateObjectState(objp, sts))
                ret = OK;
        }
    }
    break;
    }
    return ret;
}


int PutManualValue(PVID *pvid, float value, bool set_on, short origin_stn)
{
    if (set_on && (checkManReq(pvid, value) != OK))
    {
        // zadata opr vrednost nije korektna
        PutLocEvent(pvid, "Manual Rejected!");
        return -1;
    }

    RTUT *rtup = rtdb.rtut + pvid->rtu;
    if (set_on)
    {                 // set manual ON
        SetManualValue(pvid, value);
        PutStaEvent(origin_stn, pvid, "Manual ON!");
    }
    else
    {                 // set manual OFF
        ResetManualFlag(pvid);
        PutStaEvent(origin_stn, pvid, "Manual OFF!");
    }

    // publikuj promenu
    pubChg(pvid, VAR_PART | OPR_PART);

    return 0;
}


// only a single flag can be set at a time
int put_tag(PVID *pvid, const WORD flag, bool val)
{
    int ret = 0;
    // uzmi trenutnu prednost opr statusa
    WORD status = get_status(pvid, OPR_PART);
    // izvadi trenutno stanje
    bool curr_tag = get_bit(status, flag);
    if (curr_tag != val)
    {
        switch (flag)
        {
        case OPR_ACT:
            put_pvid_status(pvid, OPR_PART, flag, val);
            PutEvent(pvid, "Active = %d", val);
            ret = 1;                                        // vrati 1 da se postavi i VAR_CHG
            break;
        case OPR_EVENT_INH:
            put_pvid_status(pvid, OPR_PART, flag, val);
            PutEvent(pvid, "Event Inhibit = %d", val);
            break;
        case OPR_CMD_INH:
            put_pvid_status(pvid, OPR_PART, flag, val);
            PutEvent(pvid, "Cmd Inhibit = %d", val);
            break;
        case OPR_MCMD_INH:
            put_pvid_status(pvid, OPR_PART, flag, val);
            PutEvent(pvid, "ManCmd Inhibit = %d", val);
            break;
        default:
            PutEvent(pvid, "Illegal OPR_TAG value : %d", flag);
            break;
        }
    }
    return ret;
}


int PutTags(PVID *pvid, WORD tags, short origin_stn)
{
    // izvrsi u suprotnom 
    int varchg = put_tag(pvid, OPR_ACT, get_bit(tags, OPR_ACT));
    put_tag(pvid, OPR_EVENT_INH, get_bit(tags, OPR_EVENT_INH));
    put_tag(pvid, OPR_CMD_INH, get_bit(tags, OPR_CMD_INH));
    put_tag(pvid, OPR_MCMD_INH, get_bit(tags, OPR_MCMD_INH));

    put_chg(pvid, CHG_OPR, CHG_FINAL);
    pubChg(pvid, OPR_PART);                    // publikuj OPR promenu

    if (varchg)
    {
        put_chg(pvid, CHG_VAR, CHG_FINAL);      // mora zbog osvezavanja dClienta
        pubChg(pvid, VAR_PART);                 // publikuj VAR promenu
    }

    return 0;
}