/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2017 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#include "ansc_platform.h"
#include "cosa_apis_logagentplugin.h"
#include "ccsp_trace.h"
#include "ccsp_syslog.h"

#define TR069_PROC_NAME "CcspTr069PaSsp"
#define MTA_PROC_NAME "CcspMtaAgentSsp"
#define CM_PROC_NAME "CcspCMAgentSsp"
#define PSM_PROC_NAME "PsmSsp"
#define PAM_PROC_NAME "CcspPandMSsp"
#define WIFI_PROC_NAME "wifilog_agent"
#define Harvester_PROC_NAME "harvester"
#define NOTIFY_PROC_NAME "notify_comp"
#define PWRMGR_PROC_NAME "rdkbPowerManager"
#define FSC_PROC_NAME "fscMonitor"
#define MESH_PROC_NAME "meshAgent"
#define MDC_PROC_NAME "CcspMdcSsp"
#define ETHAGENT_PROC_NAME "CcspEthAgent"

/*RDKB-7469, CID-33124, defines*/
#define LOGAGENT_MAX_MSG_LENGTH    256
#define LOGAGENT_MAX_BUF_SIZE      241
#define LOGAGENT_MAX_COMMAND_LEN   256
#define LOGAGENT_PROC_NAME_LEN     50
#define LOGAGENT_MAX_READ_SIZE     120
#if defined(_MDC_SUPPORTED_)
  #define MDC_PROC_NAME "CcspMdcSsp"
#endif

/* structure defined for object "PluginSampleObj"  */
typedef  struct
_COSA_PLUGIN_SAMPLE_INFO
{
    ULONG                           loglevel;
    char                            WifiLogMsg[LOGAGENT_MAX_MSG_LENGTH];
    char 			    HarvesterLogMsg[LOGAGENT_MAX_MSG_LENGTH];   //Added for RDKB-4343
#if defined(_MDC_SUPPORTED_)
    char 			    MdcLogMsg[LOGAGENT_MAX_MSG_LENGTH];   //Added for RDKB-4237
#endif

}
COSA_PLUGIN_SAMPLE_INFO,  *PCOSA_PLUGIN_SAMPLE_INFO;

COSA_PLUGIN_SAMPLE_INFO  g_BackPluginInfo ;
void SW_Dealy()
{
	int i = 0;
	int j = 0;
	for(i=0;i<10;i++)
	for(j=0;j<1000;j++);
}
static int SendSignal(char *proc)
{
    FILE *f;
    char Buf[LOGAGENT_MAX_BUF_SIZE] = {0};
    char *ptr = Buf;
    char cmd[LOGAGENT_PROC_NAME_LEN] = {0};
    char cmd2[LOGAGENT_MAX_COMMAND_LEN] = {0};
    unsigned int iBufferRead = 0;
    snprintf(cmd,sizeof(cmd)-1,"%s %s","pidof ",proc);


    if((f = popen(cmd, "r")) == NULL) {
        printf("popen %s error\n", cmd);
        return -1;
    }

    while(!feof(f))
    {
        *ptr = 0;
        fgets(ptr,LOGAGENT_MAX_READ_SIZE,f);
        iBufferRead += strlen(ptr);
        /*
        ** RDKB-7469, CID-33124, break if length read more than buffer size 
        ** This is to avoid memory curruption
        */
        if((strlen(ptr) == 0) || (iBufferRead >(LOGAGENT_MAX_BUF_SIZE-1)) )
        {
            break;
        }
        ptr += strlen(ptr);
    }
    pclose(f);
    snprintf(cmd2,sizeof(cmd2)-1,"%s %s","kill -14 ", Buf); /*RDKB-7469, CID-33124, limiting Buffer copied to contain in cmd2*/
    if((f = popen(cmd2, "r")) == NULL) {
        printf("popen %s error\n", cmd2);
        return -1;
    }
	pclose(f);
    return 0;
}

BOOL
LogAgent_GetParamUlongValue
    (
         ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    ULONG                           i               = 0;
    //printf("$$$$Inside LogAgent_GetParamUlongValue in cosa_apis_logagentplugin.c\n");
	/*loglevel is a sample parameter. Need to be removed. Changed for RDKB-4800*/
/*
	if( AnscEqualString(ParamName, "loglevel", TRUE))
    {
	 //printf("$$$$Inside LogAgent_GetParamUlongValue loglevel\n");
        *puLong =  &i;
	 //printf("$$$$puLong = %d\n",*puLong);
	return TRUE;
   
    }*/

if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_LogLevel", TRUE))
{
		*puLong  = RDKLogLevel;
        return TRUE;
}
if( AnscEqualString(ParamName, "X_RDKCENTRAL-COM_TR69_LogLevel", TRUE))
{
        *puLong =  TR69_RDKLogLevel;
		return TRUE;
   
}
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_PAM_LogLevel", TRUE))
    {
		*puLong  = PAM_RDKLogLevel;
        return TRUE;
    }
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_PSM_LogLevel", TRUE))
    {
		*puLong  = PSM_RDKLogLevel;
        return TRUE;
    }
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_MTA_LogLevel", TRUE))
    {
		*puLong  = MTA_RDKLogLevel;
        return TRUE;
    }
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_CM_LogLevel", TRUE))
    {
		*puLong  = CM_RDKLogLevel;
        return TRUE;
    }
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_WiFi_LogLevel", TRUE))
    {
		*puLong  = WiFi_RDKLogLevel;
        return TRUE;
    }
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_CR_LogLevel", TRUE))
    {
		*puLong  = CR_RDKLogLevel;
        return TRUE;
    }
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_Harvester_LogLevel", TRUE))
    {
	
		*puLong  = Harvester_RDKLogLevel;
        return TRUE;
    }
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_NotifyComp_LogLevel", TRUE))
    {
	
		*puLong  = NOTIFY_RDKLogLevel;
        return TRUE;
    }

    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_PowerMgr_LogLevel", TRUE))
    {

        *puLong  = PWRMGR_RDKLogLevel;
        return TRUE;
    }
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_FSC_LogLevel", TRUE))
    {

        *puLong  = FSC_RDKLogLevel;
        return TRUE;
    }
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_Mesh_LogLevel", TRUE))
    {

        *puLong  = MESH_RDKLogLevel;
        return TRUE;
    }
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_MeshService_LogLevel", TRUE))
    {
        *puLong  = MeshService_RDKLogLevel;
        return TRUE;
    }


#if defined(_MDC_SUPPORTED_)
       if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_MDC_LogLevel", TRUE))
    {
                *puLong  = MDC_RDKLogLevel;
        return TRUE;
    }
#endif

    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        LogAgent_SetParamValues
            (
                ANSC_HANDLE                 hInsContext,
                char**                      ppParamArray,
                PSLAP_VARIABLE*             ppVarArray,
                ULONG                       ulArraySize,
                PULONG                      pulErrorIndex
            );

    description:

        This function is called to set bulk parameter values; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char**                      ppParamName,
                The parameter name array;

                PSLAP_VARIABLE*             ppVarArray,
                The parameter values array;

                ULONG                       ulArraySize,
                The size of the array;

                PULONG                      pulErrorIndex
                The output parameter index of error;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
LogAgent_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                      uValue
    )
{
    ULONG                           i               = 0;
	/*loglevel is a sample parameter. Need to be removed. Changed for RDKB-4800*/
/*
	if( AnscEqualString(ParamName, "loglevel", TRUE))
	{
		//printf(" LogAgent_SetParamValues : loglevel \n");
		return TRUE;
	  
	}
*/

    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_LogLevel", TRUE))
    {
		char buf[8];
		RDKLogLevel = uValue;
		snprintf(buf,sizeof(buf),"%d",uValue);
		if (syscfg_set(NULL, "X_RDKCENTRAL-COM_LogLevel", buf) != 0) 
		{
			AnscTraceWarning(("syscfg_set failed\n"));
		}
		else 
		{
			if (syscfg_commit() != 0) 
			{
				AnscTraceWarning(("syscfg_commit failed\n"));
			}
		}
		return TRUE;
    }
	if( AnscEqualString(ParamName, "X_RDKCENTRAL-COM_TR69_LogLevel", TRUE))
	{
		char buf[8];
		TR69_RDKLogLevel = uValue;
		snprintf(buf,sizeof(buf),"%d",uValue);
		if (syscfg_set(NULL, "X_RDKCENTRAL-COM_TR69_LogLevel", buf) != 0) 
		{
			AnscTraceWarning(("syscfg_set failed\n"));
		}
		else 
		{
			if (syscfg_commit() != 0) 
			{
				AnscTraceWarning(("syscfg_commit failed\n"));
			}
		}
		SendSignal(TR069_PROC_NAME);
		return TRUE;
	}
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_PAM_LogLevel", TRUE))
    {
		char buf[8];
		PAM_RDKLogLevel = uValue;
	    snprintf(buf,sizeof(buf),"%d",uValue);
		if (syscfg_set(NULL, "X_RDKCENTRAL-COM_PAM_LogLevel", buf) != 0) 
		{
			AnscTraceWarning(("syscfg_set failed\n"));
		}
		else 
		{
			if (syscfg_commit() != 0) 
			{
		    	 AnscTraceWarning(("syscfg_commit failed\n"));
			}
		}
		SendSignal(PAM_PROC_NAME);
		return TRUE;
    }
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_PSM_LogLevel", TRUE))
    {
		char buf[8];
		PSM_RDKLogLevel = uValue;
	    snprintf(buf,sizeof(buf),"%d",uValue);
		if (syscfg_set(NULL, "X_RDKCENTRAL-COM_PSM_LogLevel", buf) != 0) 
		{
			AnscTraceWarning(("syscfg_set failed\n"));
		}
		else 
		{
			if (syscfg_commit() != 0) 
			{
				AnscTraceWarning(("syscfg_commit failed\n"));
			}
		}
		SendSignal(PSM_PROC_NAME);
		return TRUE;
    }
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_MTA_LogLevel", TRUE))
    {
		char buf[8];
		MTA_RDKLogLevel = uValue;
		snprintf(buf,sizeof(buf),"%d",uValue);
		if (syscfg_set(NULL, "X_RDKCENTRAL-COM_MTA_LogLevel", buf) != 0) 
		{
			AnscTraceWarning(("syscfg_set failed\n"));
		}
		else 
		{
			if (syscfg_commit() != 0) 
			{
				AnscTraceWarning(("syscfg_commit failed\n"));
			}
		}
		SendSignal(MTA_PROC_NAME);
		return TRUE;
    }
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_CM_LogLevel", TRUE))
    {
		char buf[8];
		CM_RDKLogLevel = uValue;
		snprintf(buf,sizeof(buf),"%d",uValue);
		if (syscfg_set(NULL, "X_RDKCENTRAL-COM_CM_LogLevel", buf) != 0) 
		{
			AnscTraceWarning(("syscfg_set failed\n"));
		}
		else 
		{
			if (syscfg_commit() != 0) 
			{
				AnscTraceWarning(("syscfg_commit failed\n"));
			}
		}
		SendSignal(CM_PROC_NAME);
		return TRUE;
    }
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_WiFi_LogLevel", TRUE))
    {
		char buf[8];
		WiFi_RDKLogLevel = uValue;
		snprintf(buf,sizeof(buf),"%d",uValue);
		if (syscfg_set(NULL, "X_RDKCENTRAL-COM_WiFi_LogLevel", buf) != 0) 
		{
			AnscTraceWarning(("syscfg_set failed\n"));
		}
		else 
		{
			if (syscfg_commit() != 0) 
			{
				AnscTraceWarning(("syscfg_commit failed\n"));
			}
		}
		return TRUE;
    }
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_CR_LogLevel", TRUE))
    {
		char buf[8];
		CR_RDKLogLevel = uValue;
		snprintf(buf,sizeof(buf),"%d",uValue);
		if (syscfg_set(NULL, "X_RDKCENTRAL-COM_CR_LogLevel", buf) != 0) 
		{
			AnscTraceWarning(("syscfg_set failed\n"));
		}
		else 
		{
			if (syscfg_commit() != 0) 
			{
				AnscTraceWarning(("syscfg_commit failed\n"));
			}
		}
		return TRUE;
    }

/*Added for RDKB-4343*/
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_Harvester_LogLevel", TRUE))
    {
		char buf[8];
		Harvester_RDKLogLevel = uValue;
		printf("Setting Harvester_RDKLogLevel to %d\n",Harvester_RDKLogLevel);
		snprintf(buf,sizeof(buf),"%d",uValue);
		if (syscfg_set(NULL, "X_RDKCENTRAL-COM_Harvester_LogLevel", buf) != 0) 
		{
			AnscTraceWarning(("syscfg_set failed\n"));
			printf("syscfg_set failed\n");
		}
		else 
		{
			if (syscfg_commit() != 0) 
			{
				AnscTraceWarning(("syscfg_commit failed\n"));
				printf("syscfg_commit failed\n");
			}
		}
		return TRUE;
    }
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_NotifyComp_LogLevel", TRUE))
    {
		char buf[8];
		NOTIFY_RDKLogLevel = uValue;
	    snprintf(buf,sizeof(buf),"%d",uValue);
		if (syscfg_set(NULL, "X_RDKCENTRAL-COM_NotifyComp_LogLevel", buf) != 0) 
		{
			AnscTraceWarning(("syscfg_set failed\n"));
		}
		else 
		{
			if (syscfg_commit() != 0) 
			{
		    	 AnscTraceWarning(("syscfg_commit failed\n"));
			}
		}
		SendSignal(NOTIFY_PROC_NAME);
		return TRUE;
    }

    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_PowerMgr_LogLevel", TRUE))
    {
        char buf[8]={ 0 };
        PWRMGR_RDKLogLevel = uValue;
        snprintf(buf,sizeof(buf),"%d",uValue);
        if (syscfg_set(NULL, "X_RDKCENTRAL-COM_PowerMgr_LogLevel", buf) != 0)
        {
            AnscTraceWarning(("syscfg_set failed\n"));
        }
        else
        {
            if (syscfg_commit() != 0)
            {
                 AnscTraceWarning(("syscfg_commit failed\n"));
            }
        }
        SendSignal(PWRMGR_PROC_NAME);
        return TRUE;
    }

    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_FSC_LogLevel", TRUE))
    {
        char buf[8]={ 0 };
        FSC_RDKLogLevel = uValue;
        snprintf(buf,sizeof(buf),"%d",uValue);
        if (syscfg_set(NULL, "X_RDKCENTRAL-COM_FSC_LogLevel", buf) != 0)
        {
            AnscTraceWarning(("syscfg_set failed\n"));
        }
        else
        {
            if (syscfg_commit() != 0)
            {
                 AnscTraceWarning(("syscfg_commit failed\n"));
            }
        }
        SendSignal(FSC_PROC_NAME);
        return TRUE;
    }

    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_EthAgent_LogLevel", TRUE))
    {
                char buf[8];
                ETHAGENT_RDKLogLevel = uValue;
                snprintf(buf,sizeof(buf),"%d",uValue);
                if (syscfg_set(NULL, "X_RDKCENTRAL-COM_EthAgent_LogLevel", buf) != 0)
                {
                        AnscTraceWarning(("syscfg_set failed\n"));
                }
                else
                {
                        if (syscfg_commit() != 0)
                        {
                                AnscTraceWarning(("syscfg_commit failed\n"));
                        }
                }
                SendSignal(ETHAGENT_PROC_NAME);
                return TRUE;
    }

    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_Mesh_LogLevel", TRUE))
    {
        char buf[8]={ 0 };
        MESH_RDKLogLevel = uValue;
        snprintf(buf,sizeof(buf),"%d",uValue);
        if (syscfg_set(NULL, "X_RDKCENTRAL-COM_Mesh_LogLevel", buf) != 0)
        {
            AnscTraceWarning(("syscfg_set failed\n"));
        }
        else
        {
            if (syscfg_commit() != 0)
            {
                 AnscTraceWarning(("syscfg_commit failed\n"));
            }
        }
        SendSignal(MESH_PROC_NAME);
        return TRUE;
    }

    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_MeshService_LogLevel", TRUE))
    {
        char buf[8]={ 0 };
        MeshService_RDKLogLevel = uValue;
        snprintf(buf,sizeof(buf),"%d",uValue);
        if (syscfg_set(NULL, "X_RDKCENTRAL-COM_MeshService_LogLevel", buf) != 0)
        {
            AnscTraceWarning(("syscfg_set failed\n"));
        }
        else
        {
            if (syscfg_commit() != 0)
            {
                 AnscTraceWarning(("syscfg_commit failed\n"));
            }
        }
        return TRUE;
    }

#if defined(_MDC_SUPPORTED_)
/*Added for RDKB-4989*/
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_MDC_LogLevel", TRUE))
    {
                char buf[8];
                MDC_RDKLogLevel = uValue;
                printf("Setting MDC_RDKLogLevel to %d\n",MDC_RDKLogLevel);
                snprintf(buf,sizeof(buf),"%d",uValue);
                if (syscfg_set(NULL, "X_RDKCENTRAL-COM_MDC_LogLevel", buf) != 0)
                {
                        AnscTraceWarning(("syscfg_set failed\n"));
                        printf("syscfg_set failed\n");
                }
                else
                {
                        if (syscfg_commit() != 0)
                        {
                                AnscTraceWarning(("syscfg_commit failed\n"));
                                printf("syscfg_commit failed\n");
                        }
                }
                return TRUE;
    }
#endif

    return FALSE;
}


BOOL
LogAgent_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    )
{
	char LogMsg_arr[512] = {0};
	char *LogMsg = LogMsg_arr;
	char LogLevel[512] = {0};
	int level = 0;
	char WiFiLogeComponent[100] = "com.cisco.spvtg.ccsp.logagent";
#if defined(_MDC_SUPPORTED_)
        char MdcLogeComponent[100] = "mdc";
#endif
	char HarvesterLogeComponent[100] = "harvester";

    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "WifiLogMsg", TRUE))
    {
	//printf("$$$$Inside LogAgent_SetParamStringValue for WifiLogMsg \n");
	strcpy (LogLevel, pString);
	strtok_r (LogLevel, ",",&LogMsg);

	if( AnscEqualString(LogLevel, "RDK_LOG_ERROR", TRUE))
	{
	
		level = RDK_LOG_ERROR;
		CcspTraceExec(WiFiLogeComponent,RDK_LOG_ERROR,(LogMsg));
	}
	else if( AnscEqualString(LogLevel, "RDK_LOG_WARN", TRUE))
	{
	
		level = RDK_LOG_WARN;		
		CcspTraceExec(WiFiLogeComponent,RDK_LOG_WARN,(LogMsg));
	}
	else if( AnscEqualString(LogLevel, "RDK_LOG_NOTICE", TRUE))
	{
	
		level = RDK_LOG_NOTICE;	
		CcspTraceExec(WiFiLogeComponent,RDK_LOG_NOTICE,(LogMsg));
	}
	   else if( AnscEqualString(LogLevel, "RDK_LOG_INFO", TRUE))
	{
	
		level = RDK_LOG_INFO;
		CcspTraceExec(WiFiLogeComponent,RDK_LOG_INFO,(LogMsg));
	}
	else if( AnscEqualString(LogLevel, "RDK_LOG_DEBUG", TRUE))
	{
	
		level = RDK_LOG_DEBUG;
		CcspTraceExec(WiFiLogeComponent,RDK_LOG_DEBUG,(LogMsg));
	}
	else if( AnscEqualString(LogLevel, "RDK_LOG_FATAL", TRUE))
	{
	
		level = RDK_LOG_FATAL;
		CcspTraceExec(WiFiLogeComponent,RDK_LOG_FATAL,(LogMsg));
	}
	else
	{	
	
		level = RDK_LOG_INFO;
		CcspTraceExec(WiFiLogeComponent,RDK_LOG_INFO,(LogMsg));
	}
	   
        return TRUE;

    }
#if defined(_MDC_SUPPORTED_)
      /*Added for RDKB-4237*/
   if( AnscEqualString(ParamName, "MdcLogMsg", TRUE))
     {

	strcpy (LogLevel, pString);
	strtok_r (LogLevel, ",",&LogMsg);

	if( AnscEqualString(LogLevel, "RDK_LOG_ERROR", TRUE))
	{

		level = RDK_LOG_ERROR;
		CcspTraceExec(MdcLogeComponent,RDK_LOG_ERROR,(LogMsg));
	}
	else if( AnscEqualString(LogLevel, "RDK_LOG_WARN", TRUE))
	{

		level = RDK_LOG_WARN;
		CcspTraceExec(MdcLogeComponent,RDK_LOG_WARN,(LogMsg));
	}
	else if( AnscEqualString(LogLevel, "RDK_LOG_NOTICE", TRUE))
	{

		level = RDK_LOG_NOTICE;
		CcspTraceExec(MdcLogeComponent,RDK_LOG_NOTICE,(LogMsg));
	}
	   else if( AnscEqualString(LogLevel, "RDK_LOG_INFO", TRUE))
	{

		level = RDK_LOG_INFO;
		CcspTraceExec(MdcLogeComponent,RDK_LOG_INFO,(LogMsg));
	}
	else if( AnscEqualString(LogLevel, "RDK_LOG_DEBUG", TRUE))
	{

		level = RDK_LOG_DEBUG;
		CcspTraceExec(MdcLogeComponent,RDK_LOG_DEBUG,(LogMsg));
	}
	else if( AnscEqualString(LogLevel, "RDK_LOG_FATAL", TRUE))
	{

		level = RDK_LOG_FATAL;
		CcspTraceExec(MdcLogeComponent,RDK_LOG_FATAL,(LogMsg));
	}
	else
	{

		level = RDK_LOG_INFO;
		CcspTraceExec(MdcLogeComponent,RDK_LOG_INFO,(LogMsg));
	}

        return TRUE;

    }
/*changes end here*/
#endif /* _MDC_SUPPORTED */

/*Added for RDKB-4343*/
   if( AnscEqualString(ParamName, "HarvesterLogMsg", TRUE))
     {
	//printf("$$$$Inside LogAgent_SetParamStringValue for HarvesterLogMsg \n");
     	strcpy (LogLevel, pString);
	strtok_r (LogLevel, ",",&LogMsg);

	if( AnscEqualString(LogLevel, "RDK_LOG_ERROR", TRUE))
	{
	
		level = RDK_LOG_ERROR;
		CcspTraceExec(HarvesterLogeComponent,RDK_LOG_ERROR,(LogMsg));
	}
	else if( AnscEqualString(LogLevel, "RDK_LOG_WARN", TRUE))
	{
	
		level = RDK_LOG_WARN;		
		CcspTraceExec(HarvesterLogeComponent,RDK_LOG_WARN,(LogMsg));
	}
	else if( AnscEqualString(LogLevel, "RDK_LOG_NOTICE", TRUE))
	{
	
		level = RDK_LOG_NOTICE;	
		CcspTraceExec(HarvesterLogeComponent,RDK_LOG_NOTICE,(LogMsg));
	}
	   else if( AnscEqualString(LogLevel, "RDK_LOG_INFO", TRUE))
	{
	
		level = RDK_LOG_INFO;
		CcspTraceExec(HarvesterLogeComponent,RDK_LOG_INFO,(LogMsg));
	}
	else if( AnscEqualString(LogLevel, "RDK_LOG_DEBUG", TRUE))
	{
	
		level = RDK_LOG_DEBUG;
		CcspTraceExec(HarvesterLogeComponent,RDK_LOG_DEBUG,(LogMsg));
	}
	else if( AnscEqualString(LogLevel, "RDK_LOG_FATAL", TRUE))
	{
	
		level = RDK_LOG_FATAL;
		CcspTraceExec(HarvesterLogeComponent,RDK_LOG_FATAL,(LogMsg));
	}
	else
	{	
	
		level = RDK_LOG_INFO;
		CcspTraceExec(HarvesterLogeComponent,RDK_LOG_INFO,(LogMsg));
	}
	   
        return TRUE;
		
    }
/*changes end here*/
    if( AnscEqualString(ParamName, "WifiEventLogMsg", TRUE))
    {
	//printf("$$$$Inside WifiEventLogMsg\n");
	strcpy (LogLevel, pString);
	strtok_r (LogLevel, ",",&LogMsg);
        syslog_eventlog("Wifi", LOG_NOTICE, LogMsg);
        return TRUE;
    }

    if( AnscEqualString(ParamName, "HarvesterEventLogMsg", TRUE))
    {
	//printf("$$$$Inside HarvesterEventLogMsg\n");
	strcpy (LogLevel, pString);
	strtok_r (LogLevel, ",",&LogMsg);
        syslog_eventlog("Harvester", LOG_NOTICE, LogMsg);
        return TRUE;
    }
#if defined(_MDC_SUPPORTED_)
/*Added for RDKB-4989 */
    if( AnscEqualString(ParamName, "MdcEventLogMsg", TRUE))
    {

	strcpy (LogLevel, pString);
	strtok_r (LogLevel, ",",&LogMsg);
        syslog_eventlog("mdc", LOG_NOTICE, LogMsg);
        return TRUE;
    }
#endif

    AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName));
    return FALSE;
}


ULONG
LogAgent_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    char str[24];
   
    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "WifiLogMsg", TRUE))
    {
        //printf("$$$$ Inside LogAgent_GetParamStringValue for WiFiLogMsg\n");
		AnscCopyString(str, "WiFiLogMsg");
        AnscCopyString(pValue, str);
        return 0;
    }
#if defined(_MDC_SUPPORTED_)
     /*Added for rdkb-4237*/
    if( AnscEqualString(ParamName, "MdcLogMsg", TRUE))
    {

	AnscCopyString(str, "MdcLogMsg");
        AnscCopyString(pValue, str);
        return 0;
    }
#endif
/*Added for rdkb-4343*/
    if( AnscEqualString(ParamName, "HarvesterLogMsg", TRUE))
    {
        //printf("$$$$ Inside LogAgent_GetParamStringValue for HarvesterLogMsg\n");
		AnscCopyString(str, "HarvesterLogMsg");
        AnscCopyString(pValue, str);
        return 0;
    }

/*rdkb-4776*/
    if( AnscEqualString(ParamName, "WifiEventLogMsg", TRUE))
    {
	//printf("$$$$Inside WifiEventLogMsg Returning true\n");
        AnscCopyString(str, "WifiEventLogMsg");
        AnscCopyString(pValue, str);
        return 0;
    }

    if( AnscEqualString(ParamName, "HarvesterEventLogMsg", TRUE))
    {
	//printf("$$$$Inside HarvesterEventLogMsg Returning true\n");
        AnscCopyString(str, "HarvesterEventLogMsg");
        AnscCopyString(pValue, str);
        return 0;
    }
#if defined(_MDC_SUPPORTED_)
 /*Added for RDKB-4989 */
    if( AnscEqualString(ParamName, "MdcEventLogMsg", TRUE))
    {

        AnscCopyString(str, "MdcEventLogMsg");
        AnscCopyString(pValue, str);
        return 0;
    }

/*changes end here*/
#endif
    AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName));
    return -1;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        LogAgent_Commit
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to finally commit all the update.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
LogAgent_Commit
    (
        ANSC_HANDLE                 hInsContext
    )
{
   
    return 0;
}
BOOL
LogAgent_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    /* check the parameter name and return the corresponding value */
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_LoggerEnable", TRUE))
    {
		*pBool = RDKLogEnable;
        return TRUE;
    }
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_TR69_LoggerEnable", TRUE))
    {
		*pBool  = TR69_RDKLogEnable;
        return TRUE;
    }
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_PAM_LoggerEnable", TRUE))
    {
		*pBool  = PAM_RDKLogEnable;
        return TRUE;
    }
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_PSM_LoggerEnable", TRUE))
    {
		*pBool  = PSM_RDKLogEnable;
        return TRUE;
    }
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_MTA_LoggerEnable", TRUE))
    {
		*pBool  = MTA_RDKLogEnable;
        return TRUE;
    }
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_CM_LoggerEnable", TRUE))
    {
		*pBool  = CM_RDKLogEnable;
        return TRUE;
    }
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_WiFi_LoggerEnable", TRUE))
    {
		*pBool  = WiFi_RDKLogEnable;
        return TRUE;
    }
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_CR_LoggerEnable", TRUE))
    {
		*pBool  = CR_RDKLogEnable;
        return TRUE;
    }
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_Harvester_LoggerEnable", TRUE))
    {
	
		*pBool  = Harvester_RDKLogEnable;
        return TRUE;
    }
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_NotifyComp_LoggerEnable", TRUE))
    {
		*pBool  = NOTIFY_RDKLogEnable;
        return TRUE;
    }

    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_PowerMgr_LoggerEnable", TRUE))
    {
        *pBool  = PWRMGR_RDKLogEnable;
        return TRUE;
    }

    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_FSC_LoggerEnable", TRUE))
    {
        *pBool  = FSC_RDKLogEnable;
        return TRUE;
    }

    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_Mesh_LoggerEnable", TRUE))
    {
        *pBool  = MESH_RDKLogEnable;
        return TRUE;
    }

    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_MeshService_LoggerEnable", TRUE))
    {
        *pBool  = MeshService_RDKLogEnable;
        return TRUE;
    }

#if defined(_MDC_SUPPORTED_)
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_MDC_LoggerEnable", TRUE))
    {
		*pBool  = MDC_RDKLogEnable;
        return TRUE;
    }
#endif

    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_EthAgent_LoggerEnable", TRUE))
    {
        *pBool  = ETHAGENT_RDKLogEnable;
        return TRUE;
    }
    /* AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}
BOOL
LogAgent_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    )
{
    /* check the parameter name and set the corresponding value */
		if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_LoggerEnable", TRUE))
    {
		char buf[8];
		RDKLogEnable = bValue;
	    snprintf(buf,sizeof(buf),"%d",bValue);
        if (syscfg_set(NULL, "X_RDKCENTRAL-COM_LoggerEnable", buf) != 0) 
		{
             AnscTraceWarning(("syscfg_set failed\n"));
		}
		else 
		{
		     if (syscfg_commit() != 0) 
			 {
		         AnscTraceWarning(("syscfg_commit failed\n"));
		     }
		}
		SendSignal(TR069_PROC_NAME);
		SW_Dealy();
		SendSignal(MTA_PROC_NAME);
		SW_Dealy();
		SendSignal(CM_PROC_NAME);
		SW_Dealy();
		SendSignal(PSM_PROC_NAME);
		SW_Dealy();
		SendSignal(PAM_PROC_NAME);
		SW_Dealy();
		SendSignal(NOTIFY_PROC_NAME);
		SW_Dealy();
        SendSignal(PWRMGR_PROC_NAME);
		SW_Dealy();
        SendSignal(ETHAGENT_PROC_NAME);
		return TRUE;
    }
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_TR69_LoggerEnable", TRUE))
    {
		char buf[8];
		TR69_RDKLogEnable = bValue;
	    snprintf(buf,sizeof(buf),"%d",bValue);
        if (syscfg_set(NULL, "X_RDKCENTRAL-COM_TR69_LoggerEnable", buf) != 0) 
		{
             AnscTraceWarning(("syscfg_set failed\n"));
		}
		else 
		{
			 if (syscfg_commit() != 0) 
			 {
				 AnscTraceWarning(("syscfg_commit failed\n"));
			 }
		}
		SendSignal(TR069_PROC_NAME);
		return TRUE;
    }
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_PAM_LoggerEnable", TRUE))
    {
		char buf[8];
		PAM_RDKLogEnable = bValue;
	    snprintf(buf,sizeof(buf),"%d",bValue);
        if (syscfg_set(NULL, "X_RDKCENTRAL-COM_PAM_LoggerEnable", buf) != 0) 
		{
             AnscTraceWarning(("syscfg_set failed\n"));
		}
		else 
		{
			 if (syscfg_commit() != 0) 
			 {
				 AnscTraceWarning(("syscfg_commit failed\n"));
			 }
		}
		SendSignal(PAM_PROC_NAME);
		return TRUE;
    }
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_PSM_LoggerEnable", TRUE))
	{
		char buf[8];
		PSM_RDKLogEnable = bValue;
		snprintf(buf,sizeof(buf),"%d",bValue);
		if (syscfg_set(NULL, "X_RDKCENTRAL-COM_PSM_LoggerEnable", buf) != 0) 
		{
			 AnscTraceWarning(("syscfg_set failed\n"));
		}
		else 
		{
			if (syscfg_commit() != 0) 
			{
			 	AnscTraceWarning(("syscfg_commit failed\n"));
			}
		}
		SendSignal(PSM_PROC_NAME);
		return TRUE;
	}
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_MTA_LoggerEnable", TRUE))
    {
		char buf[8];
		MTA_RDKLogEnable = bValue;
		snprintf(buf,sizeof(buf),"%d",bValue);
		if (syscfg_set(NULL, "X_RDKCENTRAL-COM_MTA_LoggerEnable", buf) != 0) 
		{
			 AnscTraceWarning(("syscfg_set failed\n"));
		}
		else 
		{
			if (syscfg_commit() != 0) 
			{
				 AnscTraceWarning(("syscfg_commit failed\n"));
			}
		}
		SendSignal(MTA_PROC_NAME);
		return TRUE;
    }
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_CM_LoggerEnable", TRUE))
    {
		char buf[8];
		CM_RDKLogEnable = bValue;
	    snprintf(buf,sizeof(buf),"%d",bValue);
		if (syscfg_set(NULL, "X_RDKCENTRAL-COM_CM_LoggerEnable", buf) != 0) 
		{
			AnscTraceWarning(("syscfg_set failed\n"));
		}
		else 
		{
			if (syscfg_commit() != 0) 
			{
				AnscTraceWarning(("syscfg_commit failed\n"));
			}
		}
		SendSignal(CM_PROC_NAME);
		return TRUE;
    }
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_WiFi_LoggerEnable", TRUE))
    {
		char buf[8];
		WiFi_RDKLogEnable = bValue;
		snprintf(buf,sizeof(buf),"%d",bValue);
		if (syscfg_set(NULL, "X_RDKCENTRAL-COM_WiFi_LoggerEnable", buf) != 0) 
		{
			AnscTraceWarning(("syscfg_set failed\n"));
		}
		else 
		{
			if (syscfg_commit() != 0) 
			{
				AnscTraceWarning(("syscfg_commit failed\n"));
			}
		}

		return TRUE;
    }
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_CR_LoggerEnable", TRUE))
    {
		char buf[8];
		CR_RDKLogEnable = bValue;
		snprintf(buf,sizeof(buf),"%d",bValue);
		if (syscfg_set(NULL, "X_RDKCENTRAL-COM_CR_LoggerEnable", buf) != 0) 
		{
		 	AnscTraceWarning(("syscfg_set failed\n"));
		}
		else 
		{
			if (syscfg_commit() != 0) 
			{
			 	AnscTraceWarning(("syscfg_commit failed\n"));
			}
		}
		return TRUE;
    }
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_Harvester_LoggerEnable", TRUE))
    {
		char buf[8];
		Harvester_RDKLogEnable = bValue;
		printf("$$$$ Harvester_RDKLogEnable = %d\n",Harvester_RDKLogEnable);
		snprintf(buf,sizeof(buf),"%d",bValue);
		if (syscfg_set(NULL, "X_RDKCENTRAL-COM_Harvester_LoggerEnable", buf) != 0) 
		{
		 	AnscTraceWarning(("syscfg_set failed\n"));
			printf("syscfg_set failed\n");
		}
		else 
		{
			if (syscfg_commit() != 0) 
			{
			 	AnscTraceWarning(("syscfg_commit failed\n"));
				printf("syscfg_commit failed\n");
			}
		}
		return TRUE;
    }
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_NotifyComp_LoggerEnable", TRUE))
    {
		char buf[8];
		NOTIFY_RDKLogEnable = bValue;
	    snprintf(buf,sizeof(buf),"%d",bValue);
        if (syscfg_set(NULL, "X_RDKCENTRAL-COM_NotifyComp_LoggerEnable", buf) != 0) 
		{
             AnscTraceWarning(("syscfg_set failed\n"));
		}
		else 
		{
			 if (syscfg_commit() != 0) 
			 {
				 AnscTraceWarning(("syscfg_commit failed\n"));
			 }
		}
		SendSignal(NOTIFY_PROC_NAME);
		return TRUE;
    }

    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_PowerMgr_LoggerEnable", TRUE))
    {
        char buf[8];
        PWRMGR_RDKLogEnable = bValue;
        snprintf(buf,sizeof(buf),"%d",bValue);
        if (syscfg_set(NULL, "X_RDKCENTRAL-COM_PowerMgr_LoggerEnable", buf) != 0)
        {
             AnscTraceWarning(("syscfg_set failed\n"));
        }
        else
        {
             if (syscfg_commit() != 0)
             {
                 AnscTraceWarning(("syscfg_commit failed\n"));
             }
        }
        SendSignal(PWRMGR_PROC_NAME);
        return TRUE;
    }

    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_FSC_LoggerEnable", TRUE))
    {
        char buf[8];
        FSC_RDKLogEnable = bValue;
        snprintf(buf,sizeof(buf),"%d",bValue);
        if (syscfg_set(NULL, "X_RDKCENTRAL-COM_FSC_LoggerEnable", buf) != 0)
        {
             AnscTraceWarning(("syscfg_set failed\n"));
        }
        else
        {
             if (syscfg_commit() != 0)
             {
                 AnscTraceWarning(("syscfg_commit failed\n"));
             }
        }
        SendSignal(FSC_PROC_NAME);
        return TRUE;
    }
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_Mesh_LoggerEnable", TRUE))
    {
        char buf[8];
        MESH_RDKLogEnable = bValue;
        snprintf(buf,sizeof(buf),"%d",bValue);
        if (syscfg_set(NULL, "X_RDKCENTRAL-COM_Mesh_LoggerEnable", buf) != 0)
        {
             AnscTraceWarning(("syscfg_set failed\n"));
        }
        else
        {
             if (syscfg_commit() != 0)
             {
                 AnscTraceWarning(("syscfg_commit failed\n"));
             }
        }
        SendSignal(MESH_PROC_NAME);
        return TRUE;
    }
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_MeshService_LoggerEnable", TRUE))
    {
        char buf[8];
        MeshService_RDKLogEnable = bValue;
        snprintf(buf,sizeof(buf),"%d",bValue);
        if (syscfg_set(NULL, "X_RDKCENTRAL-COM_MeshService_LoggerEnable", buf) != 0)
        {
             AnscTraceWarning(("syscfg_set failed\n"));
        }
        else
        {
             if (syscfg_commit() != 0)
             {
                 AnscTraceWarning(("syscfg_commit failed\n"));
             }
        }
        return TRUE;
    }

#if defined(_MDC_SUPPORTED_)
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_MDC_LoggerEnable", TRUE))
    {
		char buf[8];
		MDC_RDKLogEnable = bValue;
		printf("$$$$ MDC_RDKLogEnable = %d\n",MDC_RDKLogEnable);
		snprintf(buf,sizeof(buf),"%d",bValue);
		if (syscfg_set(NULL, "X_RDKCENTRAL-COM_MDC_LoggerEnable", buf) != 0)
		{
			AnscTraceWarning(("syscfg_set failed\n"));
			printf("syscfg_set failed\n");
		}
		else
		{
			if (syscfg_commit() != 0)
			{
				AnscTraceWarning(("syscfg_commit failed\n"));
				printf("syscfg_commit failed\n");
			}
		}
		return TRUE;
    }
#endif

    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_EthAgent_LoggerEnable", TRUE))
    {
        char buf[8];
        ETHAGENT_RDKLogEnable = bValue;
        snprintf(buf,sizeof(buf),"%d",bValue);
        if (syscfg_set(NULL, "X_RDKCENTRAL-COM_EthAgent_LoggerEnable", buf) != 0)
        {
             AnscTraceWarning(("syscfg_set failed\n"));
        }
        else
        {
             if (syscfg_commit() != 0)
             {
                 AnscTraceWarning(("syscfg_commit failed\n"));
             }
        }
        SendSignal(ETHAGENT_PROC_NAME);
        return TRUE;
    }

    /* AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

