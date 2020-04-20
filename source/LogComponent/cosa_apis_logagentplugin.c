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
#include  "safec_lib_common.h"

#define TR069_PROC_NAME "CcspTr069PaSsp"
#define MTA_PROC_NAME "CcspMtaAgentSsp"
#define CM_PROC_NAME "CcspCMAgentSsp"
#define PSM_PROC_NAME "PsmSsp"
#define PAM_PROC_NAME "CcspPandMSsp"
#if !defined(_PLATFORM_RASPBERRYPI_)
#define WIFI_PROC_NAME "wifilog_agent"
#else
#define WIFI_PROC_NAME "CcspWifiSsp"
#endif
#define Harvester_PROC_NAME "harvester"
#define NOTIFY_PROC_NAME "notify_comp"
#define PWRMGR_PROC_NAME "rdkbPowerManager"
#define FSC_PROC_NAME "fscMonitor"
#define MESH_PROC_NAME "meshAgent"
#define ETHAGENT_PROC_NAME "CcspEthAgent"
#define TELCOVOIPAGENT_PROC_NAME "telcovoip_agent"
/*RDKB-7469, CID-33124, defines*/
#define LOGAGENT_MAX_MSG_LENGTH    256
#define LOGAGENT_MAX_BUF_SIZE      241
#define LOGAGENT_MAX_COMMAND_LEN   256
#define LOGAGENT_PROC_NAME_LEN     50
#define LOGAGENT_MAX_READ_SIZE     120

#define NUM_LOGLEVEL_TYPES (sizeof(loglevel_type_table)/sizeof(loglevel_type_table[0]))


/*Structure defined to get the log level type from the given Log Names */

typedef struct loglevel_pair {
  char     *name;
  int      level;
} LOGLEVEL_PAIR;

LOGLEVEL_PAIR loglevel_type_table[] = {
  { "RDK_LOG_ERROR",  RDK_LOG_ERROR },
  { "RDK_LOG_WARN",   RDK_LOG_WARN   },
  { "RDK_LOG_NOTICE", RDK_LOG_NOTICE },
  { "RDK_LOG_INFO",   RDK_LOG_INFO },
  { "RDK_LOG_DEBUG",  RDK_LOG_DEBUG },
  { "RDK_LOG_FATAL",  RDK_LOG_FATAL }
};

int loglevel_type_from_name(char *name, int *type_ptr)
{
  int rc = -1;
  int ind = -1;
  int i = 0;
  if((name == NULL) || (type_ptr == NULL))
     return 0;

  for (i = 0 ; i < NUM_LOGLEVEL_TYPES ; ++i)
  {
      rc = strcmp_s(name, strlen(name), loglevel_type_table[i].name, &ind);
      ERR_CHK(rc);
      if((rc == EOK) && (!ind))
      {
          *type_ptr = loglevel_type_table[i].level;
          return 1;
      }
  }
  return 0;
}


/* structure defined for object "PluginSampleObj"  */
typedef  struct
_COSA_PLUGIN_SAMPLE_INFO
{
    ULONG                           loglevel; 
    char                            WifiLogMsg[LOGAGENT_MAX_MSG_LENGTH];
    char 			    HarvesterLogMsg[LOGAGENT_MAX_MSG_LENGTH];   //Added for RDKB-4343
   
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
	char buf[5] = {0};
        if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_LogLevel", TRUE))
        {
        	syscfg_get( NULL, "X_RDKCENTRAL-COM_LogLevel", buf, sizeof(buf));
        	if( buf[0] != "\0" )
        	{
        		*puLong  = (ULONG )atoi(buf);
                	RDKLogLevel = *puLong;
        	}
        	else
        	{
        		AnscTraceWarning(("Error in syscfg_get for LogLevel\n"));
        	}
        	return TRUE;
        }
        if( AnscEqualString(ParamName, "X_RDKCENTRAL-COM_TR69_LogLevel", TRUE))
        {
        	syscfg_get( NULL, "X_RDKCENTRAL-COM_TR69_LogLevel", buf, sizeof(buf));
        	if( buf[0] != "\0" )
        	{
        		*puLong  = (ULONG )atoi(buf);
                	TR69_RDKLogLevel = *puLong;
        	}
        	else
        	{
        		AnscTraceWarning(("Error in syscfg_get for TR69_LogLevel\n"));
        	}
        	return TRUE;
        }
        if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_PAM_LogLevel", TRUE))
        {
        	syscfg_get( NULL, "X_RDKCENTRAL-COM_PAM_LogLevel", buf, sizeof(buf));
        	if( buf[0] != "\0" )
        	{
        		*puLong  = (ULONG )atoi(buf);
                	PAM_RDKLogLevel = *puLong;
        	}
        	else
        	{
        		AnscTraceWarning(("Error in syscfg_get for PAM_LogLevel\n"));
        	}
        	return TRUE;
        }
        if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_PSM_LogLevel", TRUE))
        {
        	syscfg_get( NULL, "X_RDKCENTRAL-COM_PSM_LogLevel", buf, sizeof(buf));
        	if( buf[0] != "\0" )
        	{
        		*puLong  = (ULONG )atoi(buf);
                	PSM_RDKLogLevel = *puLong;
        	}
        	else
        	{
        		AnscTraceWarning(("Error in syscfg_get for PSM_LogLevel\n"));
        	}
        	return TRUE;
        }
        if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_MTA_LogLevel", TRUE))
        {
        	syscfg_get( NULL, "X_RDKCENTRAL-COM_MTA_LogLevel", buf, sizeof(buf));
        	if( buf[0] != "\0" )
        	{
        		*puLong  = (ULONG )atoi(buf);
                	MTA_RDKLogLevel = *puLong;
        	}
        	else
        	{
        		AnscTraceWarning(("Error in syscfg_get for MTA_LogLevel\n"));
        	}
        	return TRUE;
        }
        if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_CM_LogLevel", TRUE))
        {
        	syscfg_get( NULL, "X_RDKCENTRAL-COM_CM_LogLevel", buf, sizeof(buf));
        	if( buf[0] != "\0" )
        	{
        		*puLong  = (ULONG )atoi(buf);
                	CM_RDKLogLevel = *puLong;
        	}
        	else
        	{
        	AnscTraceWarning(("Error in syscfg_get for CM_LogLevel\n"));
        	}
        	return TRUE;
        }
        if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_WiFi_LogLevel", TRUE))
        {
        	syscfg_get( NULL, "X_RDKCENTRAL-COM_WiFi_LogLevel", buf, sizeof(buf));
        	if( buf[0] != "\0" )
        	{
        		*puLong  = (ULONG )atoi(buf);
                	WiFi_RDKLogLevel = *puLong;
        	}
        	else
        	{
        		AnscTraceWarning(("Error in syscfg_get for WiFi_LogLevel\n"));
        	}
        	return TRUE;
        }
        if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_CR_LogLevel", TRUE))
        {
        	syscfg_get( NULL, "X_RDKCENTRAL-COM_CR_LogLevel", buf, sizeof(buf));
        	if( buf[0] != "\0" )
        	{
        		*puLong  = (ULONG )atoi(buf);
                	CR_RDKLogLevel = *puLong;
        	}
        	else
        	{
        		AnscTraceWarning(("Error in syscfg_get for CR_LogLevel\n"));
        	}
        	return TRUE;
        }
        if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_Harvester_LogLevel", TRUE))
        {
        	syscfg_get( NULL, "X_RDKCENTRAL-COM_Harvester_LogLevel", buf, sizeof(buf));
        	if( buf[0] != "\0" )
        	{
        		*puLong  = (ULONG )atoi(buf);
                	Harvester_RDKLogLevel = *puLong;
        	}
        	else
        	{
        		AnscTraceWarning(("Error in syscfg_get for Harvester_LogLevel\n"));
        	}
        	return TRUE;
        }
        if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_NotifyComp_LogLevel", TRUE))
        {
        	syscfg_get( NULL, "X_RDKCENTRAL-COM_NotifyComp_LogLevel", buf, sizeof(buf));
        	if( buf[0] != "\0" )
        	{
        		*puLong  = (ULONG )atoi(buf);
                	NOTIFY_RDKLogLevel = *puLong;
        	}
        	else
        	{
        		AnscTraceWarning(("Error in syscfg_get for NotifyComp_LogLevel\n"));
        	}
        	return TRUE;
        }
        if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_PowerMgr_LogLevel", TRUE))
        {
        	syscfg_get( NULL, "X_RDKCENTRAL-COM_PowerMgr_LogLevel", buf, sizeof(buf));
        	if( buf[0] != "\0" )
        	{
        		*puLong  = (ULONG )atoi(buf);
                	PWRMGR_RDKLogLevel = *puLong;
        	}
        	else
        	{
        		AnscTraceWarning(("Error in syscfg_get for PowerMgr_LogLevel\n"));
        	}
        	return TRUE;
        }
        if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_EthAgent_LogLevel", TRUE))
        {
        	syscfg_get( NULL, "X_RDKCENTRAL-COM_EthAgent_LogLevel", buf, sizeof(buf));
        	if( buf[0] != "\0" )
        	{
        		*puLong  = (ULONG )atoi(buf);
                	ETHAGENT_RDKLogLevel = *puLong;
        	}
        	else
        	{
        		AnscTraceWarning(("Error in syscfg_get for EthAgent_LogLevel\n"));
        	}
        	return TRUE;
        }
        if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_FSC_LogLevel", TRUE))
        {
        	syscfg_get( NULL, "X_RDKCENTRAL-COM_FSC_LogLevel", buf, sizeof(buf));
        	if( buf[0] != "\0" )
        	{
        		*puLong  = (ULONG )atoi(buf);
                	FSC_RDKLogLevel = *puLong;
        	}
        	else
        	{
        		AnscTraceWarning(("Error in syscfg_get for FSC_LogLevel\n"));
        	}
        	return TRUE;
        }
        if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_Mesh_LogLevel", TRUE))
        {
        	syscfg_get( NULL, "X_RDKCENTRAL-COM_Mesh_LogLevel", buf, sizeof(buf));
        	if( buf[0] != "\0" )
        	{
        		*puLong  = (ULONG )atoi(buf);
                	MESH_RDKLogLevel = *puLong;
        	}
        	else
        	{
        		AnscTraceWarning(("Error in syscfg_get for Mesh_LogLevel\n"));
        	}
        	return TRUE;
        }
        if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_MeshService_LogLevel", TRUE))
        {
        	syscfg_get( NULL, "X_RDKCENTRAL-COM_MeshService_LogLevel", buf, sizeof(buf));
        	if( buf[0] != "\0" )
        	{
        		*puLong  = (ULONG )atoi(buf);
                	MeshService_RDKLogLevel = *puLong;
        	}
        	else
        	{
        		AnscTraceWarning(("Error in syscfg_get for MeshService_LogLevel\n"));
        	}
        	return TRUE;
        }
        if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_TelcoVOIPAgent_LogLevel", TRUE))
        {
            *puLong  = TELCOVOIPAGENT_RDKLogLevel;
            return TRUE;
        }
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
		if ( RDKLogLevel != uValue)
		{
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
				else
				{
					RDKLogLevel = uValue;
				}
			}          
		}
		return TRUE;
	}
	if( AnscEqualString(ParamName, "X_RDKCENTRAL-COM_TR69_LogLevel", TRUE))
	{
		char buf[8];
		if (TR69_RDKLogLevel != uValue)
		{
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
				else
				{
					TR69_RDKLogLevel = uValue;
				}
			}
		}
		SendSignal(TR069_PROC_NAME);
		return TRUE;
	}
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_PAM_LogLevel", TRUE))
        {
		char buf[8];
		if (PAM_RDKLogLevel != uValue)
		{
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
				else
				{
					PAM_RDKLogLevel = uValue;
				}
			}
		}
		SendSignal(PAM_PROC_NAME);
		return TRUE;
	}
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_PSM_LogLevel", TRUE))
	{
		char buf[8];
		if (PSM_RDKLogLevel != uValue)
		{
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
				else
				{
					PSM_RDKLogLevel = uValue;
				}
			}
		}
		SendSignal(PSM_PROC_NAME);
		return TRUE;
	}
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_MTA_LogLevel", TRUE))
	{
		char buf[8];
		if (MTA_RDKLogLevel != uValue)
		{
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
				else
				{
					MTA_RDKLogLevel = uValue;
				}
			}
		}
		SendSignal(MTA_PROC_NAME);
		return TRUE;
        }
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_CM_LogLevel", TRUE))
        {
		char buf[8];
		if (CM_RDKLogLevel != uValue)
		{
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
				else
				{
					CM_RDKLogLevel = uValue;
				}
			}
		}
		SendSignal(CM_PROC_NAME);
		return TRUE;
	}
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_WiFi_LogLevel", TRUE))
	{
		char buf[8];
		if (WiFi_RDKLogLevel != uValue)
		{
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
				else
				{
					WiFi_RDKLogLevel = uValue;
				}
			}
		}
#if defined(_PLATFORM_RASPBERRYPI_)
		SendSignal(WIFI_PROC_NAME);
#endif
		return TRUE;
	}
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_CR_LogLevel", TRUE))
	{
		char buf[8];
		if (CR_RDKLogLevel != uValue)
		{
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
				else
				{
					CR_RDKLogLevel = uValue;
				}
			}
		}
		return TRUE;
	}

/*Added for RDKB-4343*/
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_Harvester_LogLevel", TRUE))
	{
		char buf[8];
		if (Harvester_RDKLogLevel != uValue)
		{
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
				else
				{
					Harvester_RDKLogLevel = uValue;
				}
			} 
		}
		return TRUE;
	}
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_NotifyComp_LogLevel", TRUE))
	{
		char buf[8];
		if (NOTIFY_RDKLogLevel != uValue)
		{
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
				else
				{
					NOTIFY_RDKLogLevel = uValue;
				}
			}
		}
		SendSignal(NOTIFY_PROC_NAME);
		return TRUE;
	}
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_PowerMgr_LogLevel", TRUE))
	{
		char buf[8]={ 0 };
		if (PWRMGR_RDKLogLevel != uValue)
		{
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
				else
				{
					PWRMGR_RDKLogLevel = uValue;
				}
			}
		}
		SendSignal(PWRMGR_PROC_NAME);
		return TRUE;
	}
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_FSC_LogLevel", TRUE))
	{
		char buf[8]={ 0 };
		if (FSC_RDKLogLevel != uValue)
		{
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
				else
				{
					FSC_RDKLogLevel = uValue;
				}
			}
		}
		SendSignal(FSC_PROC_NAME);
		return TRUE;
	}
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_EthAgent_LogLevel", TRUE))
	{
		char buf[8];
		if (ETHAGENT_RDKLogLevel != uValue)
		{
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
				else
				{
					ETHAGENT_RDKLogLevel = uValue;
				}
			}
		}
		SendSignal(ETHAGENT_PROC_NAME);
		return TRUE;
	}
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_Mesh_LogLevel", TRUE))
	{
        	char buf[8]={ 0 };
		if (MESH_RDKLogLevel != uValue)
		{
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
				else
				{
					MESH_RDKLogLevel = uValue;
				}
			}
		}
		SendSignal(MESH_PROC_NAME);
		return TRUE;
	}
	if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_MeshService_LogLevel", TRUE))
	{
		char buf[8]={ 0 };
		if (MeshService_RDKLogLevel != uValue)
		{
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
				else
				{
					MeshService_RDKLogLevel = uValue;
				}
			}
		}
		return TRUE;
	}

#if defined(_HUB4_PRODUCT_REQ_)
   if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_TelcoVOIPAgent_LogLevel", TRUE))
    {
        char buf[8]={ 0 };
        TELCOVOIPAGENT_RDKLogLevel = uValue;
        snprintf(buf,sizeof(buf),"%d",uValue);
        if (syscfg_set(NULL, "X_RDKCENTRAL-COM_TelcoVOIPAgent_LogLevel", buf) != 0)
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
        SendSignal(TELCOVOIPAGENT_PROC_NAME);
        return TRUE;
    }
#endif /* _HUB4_PRODUCT_REQ_ */

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
	char HarvesterLogeComponent[100] = "harvester";
        errno_t rc = -1;
        int ind = -1;
        char *tok;
        size_t len = 0;

     if((pString == NULL) || (strlen(pString) >= sizeof(LogLevel)))
           return FALSE;
	
    /* check the parameter name and set the corresponding value */

    rc = strcmp_s("WifiLogMsg",strlen("WifiLogMsg"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
	//printf("$$$$Inside LogAgent_SetParamStringValue for WifiLogMsg \n");
	rc =  strcpy_s(LogLevel,sizeof(LogLevel),pString);
        if(rc != EOK)
        {
            ERR_CHK(rc);
            return FALSE;
        }

        len = strlen(LogLevel);
        tok = strtok_s (LogLevel, &len, ",",&LogMsg);
        if(errno != EOK)
        {
           ERR_CHK(errno);
           return FALSE;
        }

        if (!loglevel_type_from_name(LogLevel, &level))
        {
            printf("unrecognized type name");
            level = RDK_LOG_INFO;
        }
        else
        {
	    printf("\ntype name found - %d\n",level);
        }

        CcspTraceExec(WiFiLogeComponent, level, (LogMsg));

        return TRUE;
		
    }

/*Added for RDKB-4343*/
    rc = strcmp_s("HarvesterLogMsg",strlen("HarvesterLogMsg"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
	 //printf("$$$$Inside LogAgent_SetParamStringValue for HarvesterLogMsg \n");
          rc =  strcpy_s(LogLevel,sizeof(LogLevel),pString);
          if(rc != EOK)
          {
             ERR_CHK(rc);
             return FALSE;
          }

          len = strlen(LogLevel);
          tok  =  strtok_s(LogLevel, &len , ",", &LogMsg);
          if(errno != EOK)
          {
             ERR_CHK(errno);
             return FALSE;
          }

          if (!loglevel_type_from_name(LogLevel, &level))
          {
              printf("unrecognized type name");
              level = RDK_LOG_INFO;
          }
          else
          {
              printf("\ntype name found - %d\n",level);
          }

          CcspTraceExec(HarvesterLogeComponent, level, (LogMsg));

          return TRUE;
   }
/*changes end here*/
    rc = strcmp_s("WifiEventLogMsg",strlen("WifiEventLogMsg"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
	//printf("$$$$Inside WifiEventLogMsg\n");
        rc =  strcpy_s(LogLevel,sizeof(LogLevel),pString);
        if(rc != EOK)
         {
             ERR_CHK(rc);
             return FALSE;
         }

         len = strlen(LogLevel);
	 tok = strtok_s (LogLevel, &len, ",", &LogMsg);
         if(errno != EOK)
         {
            ERR_CHK(errno);
            return FALSE;
         }

        syslog_eventlog("Wifi", LOG_NOTICE, LogMsg);
        return TRUE;
    }

    rc = strcmp_s("HarvesterEventLogMsg",strlen("HarvesterEventLogMsg"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
	//printf("$$$$Inside HarvesterEventLogMsg\n");
        rc =  strcpy_s(LogLevel,sizeof(LogLevel),pString);
        if(rc != EOK)
         {
            ERR_CHK(rc);
            return FALSE;
         }

         len = strlen(LogLevel);
	 tok = strtok_s(LogLevel, &len,",",&LogMsg);
         if(errno != EOK)
         {
              ERR_CHK(errno);
              return FALSE;
         }

        syslog_eventlog("Harvester", LOG_NOTICE, LogMsg);
        return TRUE;
    }
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
/*changes end here*/
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
    char buf[5] = {0};
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_LoggerEnable", TRUE))
    {
    	syscfg_get( NULL, "X_RDKCENTRAL-COM_LoggerEnable", buf, sizeof(buf));
    	if( buf[0] != "\0" )
    	{
    		*pBool = (BOOL)atoi(buf);
    		RDKLogEnable = *pBool;
    	}
    	else
        {
        	AnscTraceWarning(("Error in syscfg_get for LoggerEnable\n"));
        }
        return TRUE;
    }
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_TR69_LoggerEnable", TRUE))
    {
    	syscfg_get( NULL, "X_RDKCENTRAL-COM_TR69_LoggerEnable", buf, sizeof(buf));
    	if( buf[0] != "\0" )
    	{
    		*pBool = (BOOL)atoi(buf);
    		TR69_RDKLogEnable = *pBool;
    	}
    	else
        {
            	AnscTraceWarning(("Error in syscfg_get for TR69_LoggerEnable\n"));
        }
        return TRUE;
    }
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_PAM_LoggerEnable", TRUE))
    {
	syscfg_get( NULL, "X_RDKCENTRAL-COM_PAM_LoggerEnable", buf, sizeof(buf));
    	if( buf[0] != "\0" )
    	{
    		*pBool = (BOOL)atoi(buf);
    		PAM_RDKLogEnable = *pBool;
    	}
    	else
        {
            	AnscTraceWarning(("Error in syscfg_get for PAM_LoggerEnable\n"));;
        }
        return TRUE;
    }
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_PSM_LoggerEnable", TRUE))
    {
	syscfg_get( NULL, "X_RDKCENTRAL-COM_PSM_LoggerEnable", buf, sizeof(buf));
    	if( buf[0] != "\0" )
    	{
    		*pBool = (BOOL)atoi(buf);
    		PSM_RDKLogEnable = *pBool;
    	}
    	else
        {
            	AnscTraceWarning(("Error in syscfg_get for PSM_LoggerEnable\n"));
        }
        return TRUE;
    }
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_MTA_LoggerEnable", TRUE))
    {
	syscfg_get( NULL, "X_RDKCENTRAL-COM_MTA_LoggerEnable", buf, sizeof(buf));
    	if( buf[0] != "\0" )
    	{
    		*pBool = (BOOL)atoi(buf);
    		MTA_RDKLogEnable = *pBool;
    	}
    	else
        {
            	AnscTraceWarning(("Error in syscfg_get for MTA_LoggerEnable\n"));
        }
        return TRUE;
    }
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_CM_LoggerEnable", TRUE))
    {
	syscfg_get( NULL, "X_RDKCENTRAL-COM_CM_LoggerEnable", buf, sizeof(buf));
    	if( buf[0] != "\0" )
    	{
    		*pBool = (BOOL)atoi(buf);
    		CM_RDKLogEnable = *pBool;
    	}
        else
        {
            	AnscTraceWarning(("Error in syscfg_get for CM_LoggerEnable\n"));
        }
        return TRUE;
    }
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_WiFi_LoggerEnable", TRUE))
    {
	syscfg_get( NULL, "X_RDKCENTRAL-COM_WiFi_LoggerEnable", buf, sizeof(buf));
    	if( buf[0] != "\0" )
    	{
    		*pBool =(BOOL)atoi(buf);
    		WiFi_RDKLogEnable = *pBool;
    	}
    	else
        {
            	AnscTraceWarning(("Error in syscfg_get for WiFi_LoggerEnable\n"));
        }
        return TRUE;
    }
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_CR_LoggerEnable", TRUE))
    {
	syscfg_get( NULL, "X_RDKCENTRAL-COM_CR_LoggerEnable", buf, sizeof(buf));
    	if( buf[0] != "\0" )
    	{
    		*pBool = (BOOL)atoi(buf);
    		CR_RDKLogEnable = *pBool;
    	}
    	else
        {
           	 AnscTraceWarning(("Error in syscfg_get for CR_LoggerEnable\n"));
        }
        return TRUE;
    }
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_Harvester_LoggerEnable", TRUE))
    {
	syscfg_get( NULL, "X_RDKCENTRAL-COM_Harvester_LoggerEnable", buf, sizeof(buf));
    	if( buf[0] != "\0" )
    	{
    		*pBool = (BOOL)atoi(buf);
    		Harvester_RDKLogEnable = *pBool;
    	}
    	else
        {
            	AnscTraceWarning(("Error in syscfg_get for Harvester_LoggerEnable\n"));
        }
        return TRUE;
    }
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_NotifyComp_LoggerEnable", TRUE))
    {
        syscfg_get( NULL, "X_RDKCENTRAL-COM_NotifyComp_LoggerEnable", buf, sizeof(buf));
        if( buf[0] != "\0" )
        {
            	*pBool = (BOOL)atoi(buf);
            	NOTIFY_RDKLogEnable = *pBool;
        }
        else
        {
            	AnscTraceWarning(("Error in syscfg_get for NotifyComp_LoggerEnable\n"));
        }
        return TRUE;
    }
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_PowerMgr_LoggerEnable", TRUE))
    {
        syscfg_get( NULL, "X_RDKCENTRAL-COM_PowerMgr_LoggerEnable", buf, sizeof(buf));
        if( buf[0] != "\0" )
        {
            	*pBool = (BOOL)atoi(buf);
            	PWRMGR_RDKLogEnable = *pBool;
        }
        else
        {
            	AnscTraceWarning(("Error in syscfg_get for PowerMgr_LoggerEnable\n"));
        }
        return TRUE;
    }
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_EthAgent_LoggerEnable", TRUE))
    {
        syscfg_get( NULL, "X_RDKCENTRAL-COM_EthAgent_LoggerEnable", buf, sizeof(buf));
        if( buf[0] != "\0" )
        {
            	*pBool = (BOOL)atoi(buf);
            	ETHAGENT_RDKLogEnable = *pBool;
        }
        else
        {
            	AnscTraceWarning(("Error in syscfg_get for EthAgent_LoggerEnable\n"));
        }
        return TRUE;
    }
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_FSC_LoggerEnable", TRUE))
    {
        syscfg_get( NULL, "X_RDKCENTRAL-COM_FSC_LoggerEnable", buf, sizeof(buf));
        if( buf[0] != "\0" )
        {
            	*pBool = (BOOL)atoi(buf);
            	FSC_RDKLogEnable = *pBool;
        }
        else
        {
            	AnscTraceWarning(("Error in syscfg_get for FSC_LoggerEnable\n"));
        }
        return TRUE;
    }
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_Mesh_LoggerEnable", TRUE))
    {
        syscfg_get( NULL, "X_RDKCENTRAL-COM_Mesh_LoggerEnable", buf, sizeof(buf));
        if( buf[0] != "\0" )
        {
            	*pBool = (BOOL)atoi(buf);
            	MESH_RDKLogEnable = *pBool;
	}
	else
        {
            	AnscTraceWarning(("Error in syscfg_get for Mesh_LoggerEnable\n"));
        }
        return TRUE;
    }
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_MeshService_LoggerEnable", TRUE))
    {
        syscfg_get( NULL, "X_RDKCENTRAL-COM_MeshService_LoggerEnable", buf, sizeof(buf));
        if( buf[0] != "\0" )
        {
            	*pBool = (BOOL)atoi(buf);
            	MeshService_RDKLogEnable = *pBool;
        }
        else
        {
            	AnscTraceWarning(("Error in syscfg_get for MeshService_LoggerEnable\n"));
        }
        return TRUE;
    }

    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_TelcoVOIPAgent_LoggerEnable", TRUE))
    {
        *pBool  = TELCOVOIPAGENT_RDKLogEnable;
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
#if defined(_PLATFORM_RASPBERRYPI_)
                SW_Dealy();
                SendSignal(WIFI_PROC_NAME);
		SW_Dealy();
#endif																				
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
#if defined(_PLATFORM_RASPBERRYPI_)
		SendSignal(WIFI_PROC_NAME);
#endif
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

#if defined(_HUB4_PRODUCT_REQ_)
    if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_TelcoVOIPAgent_LoggerEnable", TRUE))
    {
        char buf[8];
        TELCOVOIPAGENT_RDKLogEnable = bValue;
        snprintf(buf,sizeof(buf),"%d",bValue);
        if (syscfg_set(NULL, "X_RDKCENTRAL-COM_TelcoVOIPAgent_LoggerEnable", buf) != 0)
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
        SendSignal(TELCOVOIPAGENT_PROC_NAME);
        return TRUE;
    }
#endif /* _HUB4_PRODUCT_REQ_ */

    /* AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

