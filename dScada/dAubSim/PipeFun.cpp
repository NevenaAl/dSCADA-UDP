#pragma once
#include "stdafx.h"

void ProcessCmd();

//#include <WinSock2.h> 
//#define _WINSOCK_DEPCRECATED_NO_WARNINGS
//#include <sys/types.h> /* for type definitions */
//#include <winsock.h> //umjeso sys socketa i arpa
//#include <stdio.h>/* for printf() and fprintf() */
//#include <stdlib.h>/* for atoi() */
//#include <string.h>/* for strlen() */

#define MAX_LEN 1024 /* maximum receive string size */
#define MIN_PORT 1024 /* minimum port allowed */
#define  MAX_PORT 65535 /* maximum port allowed */
#define SERVER_SLEEP_TIME 50
#define IP_ADD_MEMBERSHIP         12 
#define IP_DROP_MEMBERSHIP        13



//static HANDLE AubPipeIN, AubPipeOUT;
//static HANDLE spipe_mutex;
//static bool Client.Connected = FALSE;

#define DEFAULT_CLIENT_PORT 9995
#define CMSG_MAX_SLEN 1500			// Client Send Msg
#define CMSG_MAX_RLEN 250			// Client Recv Msg
#define MAX_CLIENT_NUMBER 10        // Max number of connected clients
#define MC_IP_ADDRESS "238.255.255.255"  // Multicast group ip address
#define MC_PORT 9995					// Multicast group port

typedef struct
{									// Client TCP Pipe
	char sMsg[CMSG_MAX_SLEN];		// bafer za slanje poruke
	char rMsg[CMSG_MAX_RLEN];
	bool Connected;
	bool Loaded;
	int cRead;
} CPIPE;


static SOCKET socklsn;
static CPIPE Client;
int NoConnectedClients = 0;
int NoAllClients = 0;



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

		int flag_on = 1;				 // socket option flag 
		struct sockaddr_in server_addr;  // socket address structure 
		short server_port;				// server port 
		server_port = 8080;				 //server port number

		WSADATA wsaData;
		// Load Winsock 2.0 DLL 
		if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
			fprintf(stderr, "WSAStartup() failed");
			return NOK;
		}

		 // validate the port range 
		if ((server_port < MIN_PORT) || (server_port > MAX_PORT)) {
			fprintf(stderr, "Invalid port number argument %d.\n", server_port);
			fprintf(stderr, "Valid range is between %d and %d.\n", MIN_PORT, MAX_PORT);
			return NOK;
		}

		// create socket  
		if ((socklsn = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
			perror("socket() failed");
			return NOK;
		}

		// construct a server address structure 
		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		server_addr.sin_port = htons((u_short)server_port);

		// bind  address to socket 
		if ((bind(socklsn, (struct sockaddr*) & server_addr, sizeof(server_addr))) < 0) {
			perror("bind() failed");
			return NOK;
		}

		// set socket to non blocking mode
		unsigned long int nonBlockingMode = 1;
		int iResult = ioctlsocket(socklsn, FIONBIO, &nonBlockingMode);

		if (iResult == SOCKET_ERROR)
		{
			printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
			return NOK;
		}

		memset(&Client, 0, sizeof(Client));
		pipe_open = true;
		return OK;
}
void ConnectClient(void) 
{
	Client.Connected = true;
	Client.cRead = 0;
	NoConnectedClients++;
	NoAllClients++;
	InitClient();
	PutLocEvent(NULL, "Station %s: Client %d connected!", StnCfg.StaName, NoAllClients);
}
void DisconnectClient(void)
{
	if (Client.Connected)
	{
		NoConnectedClients--;
		if (NoConnectedClients == 0)
		{
			memset(&Client, 0, sizeof(CPIPE));
			Client.Connected = false;
		}
		
		PutLocEvent( NULL, "Client disconnected!" );
	}
}

void ClosePipe(void)
{
	if (pipe_open)
	{
		closesocket(socklsn);
		
		if (Client.Connected)
		{
			//ovde javiti klijentima da je servr ugasen, da li?
			memset(&Client, 0, sizeof(CPIPE));
		}
	}
	pipe_open = false;
}

int WaitForMessage(void)
{
	char recv_str[MAX_LEN + 1];			// buffer to receive string 
	int recv_len;						// length of string received 
	struct sockaddr_in from_addr;		// packet source 
	int from_len;						// source addr length 

	for (; ;) {

		memset(recv_str, 0, sizeof(recv_str));
		from_len = sizeof(from_addr);
		memset(&from_addr, 0, from_len);

		FD_SET set;
		timeval timeVal;

		FD_ZERO(&set);
		// Add socket we will wait to read from
		FD_SET(socklsn, &set);

		// Set timeouts to zero since we want select to return
		// instantaneously
		timeVal.tv_sec = 0;
		timeVal.tv_usec = 0;

		int iResult = select(0 /* ignored */, &set, NULL, NULL, &timeVal);

		// lets check if there was an error during select
		if (iResult == SOCKET_ERROR)
		{
			fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
			continue;
		}

		// now, lets check if there are any sockets ready
		if (iResult == 0)
		{
			// there are no ready sockets, sleep for a while and check again
			Sleep(SERVER_SLEEP_TIME);
			continue;
		}

		// block waiting to receive a packet 
		if ((recv_len = recvfrom(socklsn, recv_str, MAX_LEN, 0, (struct sockaddr*) & from_addr, &from_len)) <= 0) {
			DisconnectClient();				
			perror("recvfrom() failed");
			break;
		}

		
		if (strcmp(recv_str,"connected\n") ==0)
		{
			ConnectClient();
		}
		else if (strcmp(recv_str, "disconnected\n") == 0)
		{
			DisconnectClient();
		}
		else
		{
			// command from client received
			strcpy(Client.rMsg, recv_str);
			Client.cRead = recv_len;
			Client.rMsg[Client.cRead++] = 0;
			ProcessCmd();
			Client.cRead = 0;
		}
	}
	
	
	return OK;
}


// Primi poruke (commands) koje salje klijent 

// slanje stringa na PipeOUT
// vraca 0/1 da bi se chg mogao direktno azurirati
int SendToPipe(char* msg) {
	if (Client.Connected)
	{
		int  msg_len;
		msg_len = strlen(msg);
		
		struct sockaddr_in mc_addr;		// socket address structure 
		char* mc_addr_str;				// multicast IP address 
		short mc_port;					// multicast port 
		mc_addr_str = MC_IP_ADDRESS;	//multicast ip address
		mc_port = MC_PORT;				//multicast port number
		const char mc_ttl = 6;

		
		if ((setsockopt(socklsn, IPPROTO_IP, IP_MULTICAST_TTL, &mc_ttl, sizeof(mc_ttl))) < 0) {
			perror("setsockopt() failed");
			return NOK;
		}

		memset(&mc_addr, 0, sizeof(mc_addr));
		mc_addr.sin_family = AF_INET;
		mc_addr.sin_addr.s_addr = inet_addr(mc_addr_str);
		mc_addr.sin_port = htons((u_short)mc_port);
		int tolen = sizeof(mc_addr);

		if ((sendto(socklsn, msg, msg_len, 0, (const struct sockaddr*) & mc_addr, tolen)) != msg_len) {
			perror("sendto() sent incorrect number of bytes");
			DisconnectClient();
			return NOK;
		}
		return OK;
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
void ConnectClient(void)
void ClosePipes(void)
int WaitForMessage(void)
int SendToPipe(char *msg)
void PutRtuSts(char *ime, int val)
void InitClient(void)
void RefreshClient(void)
#endif