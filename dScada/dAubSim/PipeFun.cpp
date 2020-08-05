#include "stdafx.h"

void ProcessCmd(void);

#include <WinSock2.h> 
#define _WINSOCK_DEPCRECATED_NO_WARNINGS
#include <sys/types.h> /* for type definitions */
#include <winsock.h> //umjeso sys socketa i arpa
#include <stdio.h>/* for printf() and fprintf() */
#include <stdlib.h>/* for atoi() */
#include <string.h>/* for strlen() */

#define MAX_LEN 1024 /* maximum receive string size */
#define MIN_PORT 1024 /* minimum port allowed */
#define  MAX_PORT 65535 /* maximum port allowed */
#define IP_ADD_MEMBERSHIP         12 
#define IP_DROP_MEMBERSHIP        13



//static HANDLE AubPipeIN, AubPipeOUT;
//static HANDLE spipe_mutex;
//static bool Client.Connected = FALSE;

#define DEFAULT_CLIENT_PORT 9995
#define CMSG_MAX_SLEN 1500			// Client Send Msg
#define CMSG_MAX_RLEN 250			// Client Recv Msg
#define MAX_CLIENT_NUMBER 10        // Max number of connected clients

typedef struct
{								// Client TCP Pipe
	SOCKET sockfd;
	struct sockaddr_in address;
	char sMsg[CMSG_MAX_SLEN];		// bafer za slanje poruke
	char rMsg[CMSG_MAX_RLEN];
	bool Connected;
	bool Loaded;
	int cRead;
} CPIPE;

// struct ip_mreq mc_req; /* multicast request structure */
struct ip_mreq {
	struct in_addr imr_multiaddr; /* Group multicast address */
	struct in_addr imr_interface; /* Local interface address */
}mc_req;

static SOCKET socklsn;
//static CPIPE Client[MAX_CLIENT_NUMBER];
static CPIPE Client;
//int NoClients = 0;



// pomocne funkcije za rukovanje podacima u prenosu
char *put_flag(int flag)
{
	if (flag)
		return "on";
	else
		return "off";
}

bool get_flag(char *str)
{
	if (!strcmp(str, "on"))
		return true;
	else
		return false;
}

static bool pipe_open = false;
int OpenPipe(void) {


		//int sock;  /* socket descriptor */
		int flag_on = 1;  /* socket option flag */
		struct sockaddr_in mc_addr;  /* socket address structure */
		
		
		char* mc_addr_str; /* multicast IP address */
		short mc_port; /* multicast port */
		
		WSADATA wsaData;
		/* Load Winsock 2.0 DLL */
		if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
			fprintf(stderr, "WSAStartup() failed");
			return NOK;
		}

		mc_addr_str = "239.255.10.10"; //multicast ip address
		mc_port = 9995;                //multicast port number

									 /* validate the port range */
		if ((mc_port < MIN_PORT) || (mc_port > MAX_PORT)) {
			fprintf(stderr, "Invalid port number argument %d.\n", mc_port);
			fprintf(stderr, "Valid range is between %d and %d.\n", MIN_PORT, MAX_PORT);
			return NOK;
		}

		/* create socket to join multicast group on */
		if ((socklsn = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
			perror("socket() failed");
			return NOK;
		}

		/* set reuse port to on to allow multiple binds per host */
		if ((setsockopt(socklsn, SOL_SOCKET, SO_REUSEADDR, (char*)&flag_on, sizeof(flag_on))) < 0) {
			perror("setsockopt() failed");
			return NOK;
		}

		/* construct a multicast address structure */
		memset(&mc_addr, 0, sizeof(mc_addr));
		mc_addr.sin_family = AF_INET;
		mc_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		mc_addr.sin_port = htons(mc_port);

		/* bind multicast address to socket */
		if ((bind(socklsn, (struct sockaddr*) & mc_addr, sizeof(mc_addr))) < 0) {
			perror("bind() failed");
			return NOK;
		}

		/* construct an IGMP join request structure */
		mc_req.imr_multiaddr.s_addr = inet_addr(mc_addr_str);
		mc_req.imr_interface.s_addr = htonl(INADDR_ANY);

		/* send an ADD MEMBERSHIP message via setsockopt */
		if ((setsockopt(socklsn, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mc_req, sizeof(mc_req))) < 0) {
			perror("setsockopt() failed");
			return NOK;
		}

		memset(&Client, 0, sizeof(Client));
		pipe_open = true;
		return OK;
}

void DisconnectClient(void)
{
	if (Client.Connected)
	{
		closesocket(Client.sockfd);
		memset(&Client, 0, sizeof(CPIPE));
		//PutLocEvent( NULL, "Client disconnected!" );
	}
}

void ClosePipe(void)
{
	if (pipe_open)
	{
		closesocket(socklsn);
		if ((setsockopt(socklsn, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&mc_req, sizeof(mc_req))) < 0) {
			perror("setsockopt() failed");
			exit(1);
		}
		if (Client.Connected)
		{
			//closesocket(Client.sockfd);
			memset(&Client, 0, sizeof(CPIPE));
		}
	}
	pipe_open = false;
}

int WaitForClient(void)
{
	char recv_str[MAX_LEN + 1]; /* buffer to receive string */
	memset(&Client.address, 0, sizeof(Client.address));
	int recv_len; /* length of string received */
	struct sockaddr_in from_addr;  /* packet source */
	int from_len;  /* source addr length */
	for (; ;) {
		memset(recv_str, 0, sizeof(recv_str));
		from_len = sizeof(from_addr);
		memset(&from_addr, 0, from_len);

		/* block waiting to receive a packet */
		if ((recv_len = recvfrom(socklsn, recv_str, MAX_LEN, 0, (struct sockaddr*) & from_addr, &from_len)) < 0) {
			perror("recvfrom() failed");
			return NOK;
		}

		/* output received string */
		//printf("Received %d bytes from %s: ", recv_len, inet_ntoa(from_addr.sin_addr));
		//printf("%s", recv_str);
		if (!Client.Connected)
		{
			//Client.sockfd = from_addr;
			Client.address =  from_addr;
			Client.Connected = true;
			Client.cRead = 0;
			InitClient();
			PutLocEvent(NULL, "Station %s: Client connected!", StnCfg.StaName);

		}
	}
	
	
	return OK;
}


// Primi poruke (commands) koje salje klijent 
void CheckForCmd(void)
{
	u_long bcnt;
	if (Client.Connected)
	{
		// vidi je li sta i poslao
		int rsts = ioctlsocket(Client.sockfd, FIONREAD, (u_long FAR*)&bcnt);
		// proveri da li je javio neku gresku
		if (rsts == SOCKET_ERROR)
		{
			PutLogMessage("Client ioctlsocket read problem! Error code: %d", WSAGetLastError());
			DisconnectClient();				//  prekid veze
			return;
		}

		while (bcnt)
		{
			// ucitavaj jedan po jedan znak
			char RxChar;
			rsts = recv(Client.sockfd, (char*)&RxChar, 1, 0);
			if (rsts <= 0)
			{
				DisconnectClient();				//  prekid veze
				break;
			}
			else
			{
				// smanji bcnt
				bcnt -= 1;
				// akumuliraj poruku do \n (prihvati poslati string)
				if (RxChar != '\n')
				{
					Client.rMsg[Client.cRead++] = RxChar;
				}
				else
				{
					// terminiraj string i pozovi obradu 
					Client.rMsg[Client.cRead++] = 0;
					ProcessCmd();
					Client.cRead = 0;
					//PutLogMsg( cMsg );
					break;
				}
			}
		}
	}
}

void WaitForCmd(void)
{
	do
	{
		CheckForCmd();
	} while (Client.Connected);
}

// Primi poruke (commands) koje salje klijent 

// slanje stringa na PipeOUT
// vraca 0/1 da bi se chg mogao direktno azurirati
int SendToPipe(char* msg) {
	if (Client.Connected)
	{
		int length, msg_len;
		length = sizeof(Client.address);
		msg_len = strlen(msg);
		if ((sendto(socklsn, msg, msg_len, 0, (const struct sockaddr*)&Client.address, length)) != msg_len) {
			perror("sendto() sent incorrect number of bytes");
			DisconnectClient();
			return NOK;
		}
	}
	return OK;
}


// Oznaci promenu statusa RTU kontrolera
int PutRtuSts(char *ime, int val)
{
	char msg[20];
	sprintf(msg, "rtu;%s;%s\n", put_flag(val), ime);
	return SendToPipe(msg);
}

void putTags(char *string, WORD sts, char *rtu_name, char *pv_name)
{
	sprintf(string, "tags;%s;%s;%s;%s;%s;%s;%s\n", rtu_name, pv_name,
		put_flag(sts & OPR_ACT), put_flag(sts & OPR_MAN_VAL), put_flag(sts & OPR_EVENT_INH),
		put_flag(sts & OPR_CMD_INH), put_flag(sts & OPR_MCMD_INH));
}

// Slanje svih velicina na client preko pipe-a
void InitClient(void)
{
	int i, rtu;
	char string[256];

	SendToPipe("init_start\n");

	// posalji kataloge
	for (i = 0; i < rtdb.no_egu; i++)
	{
		sprintf(string, "cat;eu;%s;%s\n", rtdb.EGUnit[i].egu, ColorNames[rtdb.EGUnit[i].color]);
		SendToPipe(string);
	}

	for (i = 0; i < rtdb.no_state; i++)
	{
		sprintf(string, "cat;ds;%s;%s\n", rtdb.DigState[i].state, ColorNames[rtdb.DigState[i].color]);
		SendToPipe(string);
	}

	for (i = 0; i < rtdb.no_command; i++)
	{
		sprintf(string, "cat;dc;%s;%s\n", rtdb.DigCmd[i].command, ColorNames[rtdb.DigCmd[i].color]);
		SendToPipe(string);
	}

	for (rtu = 0; Client.Connected && rtu < rtdb.no_rtu; rtu++)
	{
		RTUT *rtup = rtdb.rtut + rtu;
		for (i = 0; Client.Connected && !ServerShutdown && i < rtup->fix.ainfut.total; i++)
		{
			ANAIN *ainp = rtup->fix.ainfut.ain_t + i;
			sprintf(string, "init;AnaIn;%s;%s;%-20s;%s;%7.2f;%s;%s\n", rtup->fix.name, ainp->fix.name, ainp->fix.info, get_status_string(&ainp->fix.mID),
				ainp->var.egu_value, rtdb.EGUnit[ainp->fix.egu_code].egu, time2str(ainp->var.timestamp));
			SendToPipe(string);
			// posalji tagove
			putTags(string, ainp->opr.status, rtup->fix.name, ainp->fix.name);
			SendToPipe(string);
		}
		for (i = 0; Client.Connected && !ServerShutdown && i < rtup->fix.aoutfut.total; i++)
		{
			ANAOUT *aoutp = rtup->fix.aoutfut.aout_t + i;
			sprintf(string, "init;AnaOut;%s;%s;%-20s;%s;%7.2f;%s;%s\n", rtup->fix.name, aoutp->fix.name, aoutp->fix.info, get_status_string(&aoutp->fix.mID),
				aoutp->var.egu_value, rtdb.EGUnit[aoutp->fix.egu_code].egu, time2str(aoutp->var.timestamp));
			SendToPipe(string);
			// posalji tagove
			putTags(string, aoutp->opr.status, rtup->fix.name, aoutp->fix.name);
			SendToPipe(string);
		}
		for (i = 0; Client.Connected && !ServerShutdown && i < rtup->fix.cntfut.total; i++)
		{
			COUNTER *cntp = rtup->fix.cntfut.cnt_t + i;
			sprintf(string, "init;Counter;%s;%s;%-20s;%s;%7.2f;%s;%s\n", rtup->fix.name, cntp->fix.name, cntp->fix.info, get_status_string(&cntp->fix.mID),
				cntp->var.egu_value, rtdb.EGUnit[cntp->fix.egu_code].egu, time2str(cntp->var.timestamp));
			SendToPipe(string);
			// posalji tagove
			putTags(string, cntp->opr.status, rtup->fix.name, cntp->fix.name);
			SendToPipe(string);
		}
		for (i = 0; Client.Connected && !ServerShutdown && i < rtup->fix.digfut.total; i++)
		{
			DIGITAL *digp = rtup->fix.digfut.dig_t + i;
			sprintf(string, "init;Digital;%s;%s;%-20s;%s;%s;%s;%s\n", rtup->fix.name, digp->fix.name, digp->fix.info, get_status_string(&digp->fix.mID),
				rtdb.DigState[digp->var.state].state, rtdb.DigCmd[digp->var.command].command, time2str(digp->var.timestamp));
			SendToPipe(string);
			// posalji tagove
			putTags(string, digp->opr.status, rtup->fix.name, digp->fix.name);
			SendToPipe(string);
		}
		for (i = 0; Client.Connected && !ServerShutdown && i < rtup->fix.objfut.total; i++)
		{
			OBJECT *objp = rtup->fix.objfut.obj_t + i;
			if (objp->fix.obj_class == DIG_OBJ)
			{
				sprintf(string, "init;DigObj;%s;%s;%-20s;%s;%s;%s;%s\n", rtup->fix.name, objp->fix.name, objp->fix.info, get_status_string(&objp->fix.mID),
					rtdb.DigState[objp->var.state].state, rtdb.DigCmd[objp->var.command].command, time2str(objp->var.timestamp));
			}
			else
			{
				sprintf(string, "init;AnaObj;%s;%s;%-20s;%s;%7.2f;%s;%s\n", rtup->fix.name, objp->fix.name, objp->fix.info, get_status_string(&objp->fix.mID),
					objp->var.egu_value, rtdb.EGUnit[objp->fix.egu_code].egu, time2str(objp->var.timestamp));
			}
			SendToPipe(string);
			// posalji tagove
			putTags(string, objp->opr.status, rtup->fix.name, objp->fix.name);
			SendToPipe(string);
		}
		// posalji i status RTU komunikacije, sad mozes
		PutRtuSts(rtup->fix.name, rtup->var.val);
	}

	SendToPipe("init_done\n");
}

void RefreshClient(void)
{
	int i, rtu;
	char string[256];

	// Slanje promena na client preko pipe-a
	for (rtu = 0; Client.Connected && rtu < rtdb.no_rtu; rtu++)
	{
		RTUT *rtup = rtdb.rtut + rtu;
		if (rtup->chg)
		{
			rtup->chg = PutRtuSts(rtup->fix.name, rtup->var.val);
		}
		for (i = 0; Client.Connected && !ServerShutdown && i < rtup->fix.ainfut.total; i++)
		{
			ANAIN *ainp = rtup->fix.ainfut.ain_t + i;
			if (ainp->chg[CHG_VAR] == CHG_FINAL)
			{
				sprintf(string, "refresh;%s;%s;%s;%7.2f;%s\n", rtup->fix.name, ainp->fix.name, get_status_string(&ainp->fix.mID),
					ainp->var.egu_value, time2str(ainp->var.timestamp));
				ainp->chg[CHG_VAR] = SendToPipe(string);
			}
			if (ainp->chg[CHG_OPR] == CHG_FINAL)
			{
				putTags(string, ainp->opr.status, rtup->fix.name, ainp->fix.name);
				ainp->chg[CHG_OPR] = SendToPipe(string);
			}
		}
		for (i = 0; Client.Connected && !ServerShutdown && i < rtup->fix.aoutfut.total; i++)
		{
			ANAOUT *aoutp = rtup->fix.aoutfut.aout_t + i;
			if (aoutp->chg[CHG_VAR] == CHG_FINAL)
			{
				sprintf(string, "refresh;%s;%s;%s;%7.2f;%s\n", rtup->fix.name, aoutp->fix.name, get_status_string(&aoutp->fix.mID),
					aoutp->var.egu_value, time2str(aoutp->var.timestamp));
				aoutp->chg[CHG_VAR] = SendToPipe(string);
			}
			if (aoutp->chg[CHG_OPR] == CHG_FINAL)
			{
				putTags(string, aoutp->opr.status, rtup->fix.name, aoutp->fix.name);
				aoutp->chg[CHG_OPR] = SendToPipe(string);
			}
		}
		for (i = 0; Client.Connected && !ServerShutdown && i < rtup->fix.cntfut.total; i++)
		{
			COUNTER *cntp = rtup->fix.cntfut.cnt_t + i;
			if (cntp->chg[CHG_VAR] == CHG_FINAL)
			{
				sprintf(string, "refresh;%s;%s;%s;%7.2f;%s\n", rtup->fix.name, cntp->fix.name, get_status_string(&cntp->fix.mID),
					cntp->var.egu_value, time2str(cntp->var.timestamp));
				cntp->chg[CHG_VAR] = SendToPipe(string);
			}
			if (cntp->chg[CHG_OPR] == CHG_FINAL)
			{
				putTags(string, cntp->opr.status, rtup->fix.name, cntp->fix.name);
				cntp->chg[CHG_OPR] = SendToPipe(string);
			}
		}
		for (i = 0; Client.Connected && !ServerShutdown && i < rtup->fix.digfut.total; i++)
		{
			DIGITAL *digp = rtup->fix.digfut.dig_t + i;
			if (digp->chg[CHG_VAR] == CHG_FINAL)
			{
				sprintf(string, "refresh;%s;%s;%s;%s;%s;%s\n", rtup->fix.name, digp->fix.name, get_status_string(&digp->fix.mID),
					rtdb.DigState[digp->var.state].state, rtdb.DigCmd[digp->var.command].command, time2str(digp->var.timestamp));
				digp->chg[CHG_VAR] = SendToPipe(string);
			}
			if (digp->chg[CHG_OPR] == CHG_FINAL)
			{
				putTags(string, digp->opr.status, rtup->fix.name, digp->fix.name);
				digp->chg[CHG_OPR] = SendToPipe(string);
			}
		}
		for (i = 0; Client.Connected && !ServerShutdown && i < rtup->fix.objfut.total; i++)
		{
			OBJECT *objp = rtup->fix.objfut.obj_t + i;
			if (objp->chg[CHG_VAR])
			{
				if (objp->fix.obj_class == DIG_OBJ)
				{
					sprintf(string, "refresh;%s;%s;%s;%s;%s;%s\n", rtup->fix.name, objp->fix.name, get_status_string(&objp->fix.mID),
						rtdb.DigState[objp->var.state].state, rtdb.DigCmd[objp->var.command].command, time2str(objp->var.timestamp));
				}
				else
				{
					sprintf(string, "refresh;%s;%s;%s;%7.2f;%s\n", rtup->fix.name, objp->fix.name, get_status_string(&objp->fix.mID),
						objp->var.egu_value, time2str(objp->var.timestamp));
				}
				objp->chg[CHG_VAR] = SendToPipe(string);
			}
			if (objp->chg[CHG_OPR] == CHG_FINAL)
			{
				putTags(string, objp->opr.status, rtup->fix.name, objp->fix.name);
				objp->chg[CHG_OPR] = SendToPipe(string);
			}
		}
	}
}

void ProcessCmd(void)
{
	// primljen je CMD ili MAN zahtev od klijenta
	//lputs( cMsg );
	// zapis komunikacije u LogFile
	//PutLogMsg( cMsg );

	char tip[10], rtu[NAME_LEN], pvar[NAME_LEN];
	sscanf(Client.rMsg, "%s ; %s ; %s", tip, rtu, pvar);

	// nadji PVID, i proveri da li je primljena name referenca ispravna
	PVID *ppvid = get_PVID(rtu, pvar);
	if (ppvid == NULL)
	{
		PutLocEvent(ppvid, "PVAR %s:%s is invalid!", rtu, pvar);
	}
	else
	{
		// koja je direktiva ?
		if (!strcmp(tip, "cmd"))
		{                                // provera i izvrsenje KOMANDE
			char reqval[30];
			float cmdreq;
			bool cmd_valid = true;
			sscanf(Client.rMsg, "%*s ; %*s ; %*s ; %s", reqval);
			// odredi tip komande: digital ili analog
			if ((ppvid->type == PVT_DIG) || ((ppvid->type == PVT_OBJ) && (getObjClass(ppvid) == DIG_OBJ)))
			{
				// proveri da li digitalna komanda postoji u katalogu komandi
				int digcmd = getCmd(reqval);
				cmdreq = (float)digcmd;
				if (digcmd == -1)
				{
					cmd_valid = false;
					PutLocEvent(ppvid, "DigCmd %s is invalid!", reqval);
				}
			}
			else
			{
				// analogna komanda - odradi konverziju zadate vrednosti
				cmdreq = (float)atof(reqval);
			}
			if (cmd_valid)
			{
				// pokreni njeno izvrsenje
				PutCommand(ppvid, cmdreq, CMD_PUT_EVENT | CMD_MAN_REQ, 0);
			}
		}
		else if (!strcmp(tip, "man"))
		{                                      // provera i izvrsenje MANUAL direktive
			char reqval[30], onoff[5];
			sscanf(Client.rMsg, "%*s ; %*s ; %*s ; %s ; %s", reqval, onoff);
			float oprval;
			bool opr_valid = true;
			// odredi tip direktive: digital ili analog
			if ((ppvid->type == PVT_DIG) || ((ppvid->type == PVT_OBJ) && (getObjClass(ppvid) == DIG_OBJ)))
			{
				// proveri da li digitalno stanje postoji u katalogu stanja
				int digsts = getSts(reqval);
				oprval = (float)digsts;
				if (digsts == -1)
				{
					opr_valid = false;
					PutLocEvent(ppvid, "DigSts %s is invalid!", reqval);
				}
			}
			else
			{
				// analogna vrednost - odradi konverziju zadate vrednosti
				oprval = (float)atof(reqval);
			}
			if (opr_valid)
			{
				// pokreni njeno izvrsenje
				PutManualValue(ppvid, oprval, get_flag(onoff));
			}
		}
		else if (!strcmp(tip, "tag"))
		{                                      // provera i izvrsenje TAG direktive
			char act[5], evt[5], cmd[5], mcmd[5];
			sscanf(Client.rMsg, "%*s ; %*s ; %*s ; %s ; %s ; %s ; %s", act, evt, cmd, mcmd);
			// preuzmi zadate tagove
			WORD opr_status = 0;
			put_bit(opr_status, OPR_ACT, get_flag(act));
			put_bit(opr_status, OPR_EVENT_INH, get_flag(evt));
			put_bit(opr_status, OPR_CMD_INH, get_flag(cmd));
			put_bit(opr_status, OPR_MCMD_INH, get_flag(mcmd));
			// odradi direktivu
			PutTags(ppvid, opr_status);
		}
	}
}


#ifndef TRUE
int OpenPipes(void)
void DisconnectClient(void)
void ClosePipes(void)
int WaitForClient(void)
void WaitForCmd(void)
int SendToPipe(char *msg)
void PutRtuSts(char *ime, int val)
void InitClient(void)
void RefreshClient(void)
#endif