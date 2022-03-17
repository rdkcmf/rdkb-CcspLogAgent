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
#include <syscfg/syscfg.h>

#define TR069_PROC_NAME "CcspTr069PaSsp"
#define MTA_PROC_NAME "CcspMtaAgentSsp"
#define CM_PROC_NAME "CcspCMAgentSsp"
#define PSM_PROC_NAME "PsmSsp"
#define PAM_PROC_NAME "CcspPandMSsp"
#define WIFI_PROC_NAME "CcspWifiSsp"
#define Harvester_PROC_NAME "harvester"
#define NOTIFY_PROC_NAME "notify_comp"
#define PWRMGR_PROC_NAME "rdkbPowerManager"
#define FSC_PROC_NAME "fscMonitor"
#define MESH_PROC_NAME "meshAgent"
#define ETHAGENT_PROC_NAME "CcspEthAgent"
#define TELCOVOIPAGENT_PROC_NAME "telcovoip_agent"
#define DSLAGENT_PROC_NAME "dslagent"
#define XTMAGENT_PROC_NAME "xtmagent"
#define VLANAGENT_PROC_NAME "vlanagent"

#if defined (FEATURE_RDKB_WAN_MANAGER)
#define VLANMANAGER_PROC_NAME "VlanManager"

#if defined (FEATURE_RDKB_XDSL_PPP_MANAGER)
#define XDSLMANAGER_PROC_NAME "xdslmanager"
#define PPPMANAGER_PROC_NAME "pppmanager"
#endif

#if defined (FEATURE_RDKB_TELCOVOICE_MANAGER)
#define TELCOVOICEMANAGER_PROC_NAME "telcovoicemanager"
#endif

#endif

#if defined (FEATURE_RDKB_FWUPGRADE_MANAGER)
#define FWUPGRADEMANAGER_PROC_NAME "fwupgrademanager"
#endif

/*RDKB-7469, CID-33124, defines*/
#define LOGAGENT_MAX_MSG_LENGTH    256
#define LOGAGENT_MAX_BUF_SIZE      241
#define LOGAGENT_MAX_COMMAND_LEN   256
#define LOGAGENT_PROC_NAME_LEN     50
#define LOGAGENT_PROC_NAME_LENGTH  65
#define LOGAGENT_MAX_READ_SIZE     120
#define DEVICE_PROPS_FILE  "/etc/device.properties"

#if defined(_COSA_INTEL_XB3_ARM_)
static int getValueFromDevicePropsFile(char *str, char **value);
#endif

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
  unsigned int i = 0;
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

#if defined(_COSA_INTEL_XB3_ARM_)
static int getValueFromDevicePropsFile(char *str, char **value)
{
    FILE *fp = fopen(DEVICE_PROPS_FILE, "r");
    char buf[ 1024 ] = { 0 };
    char *tempStr = NULL;
	int ret = 0;
    if( NULL != fp )
    {
        while ( fgets( buf, sizeof( buf ), fp ) != NULL )
        {
            if ( strstr( buf, str ) != NULL )
            {
                buf[strcspn( buf, "\r\n" )] = 0; // Strip off any carriage returns
                tempStr = strstr( buf, "=" );
                tempStr++;
                *value = tempStr;
                ret = 0;
                break;
            }
        }
        if( NULL == *value)
        {
            AnscTraceWarning(("%s is not present in device.properties file\n",str));
            ret = -1;
        }
    }
    else
    {
        AnscTraceWarning(("Failed to open file:%s\n", DEVICE_PROPS_FILE));
        return -1;
    }
    if( fp )
    {
        fclose(fp);
    }
   return ret;
}
#endif

#if defined(_COSA_INTEL_XB3_ARM_)
static int SendSignal_wifi()
{
    FILE *f = NULL;
    char Buf[LOGAGENT_MAX_BUF_SIZE] = {0};
    char *ptr = Buf;
    char cmd[LOGAGENT_PROC_NAME_LENGTH] = {0};
    char cmd2[LOGAGENT_MAX_COMMAND_LEN] = {0};
    unsigned int iBufferRead = 0;
    char rpcCmd[128];
    char *atomIp = NULL;
    
    if(getValueFromDevicePropsFile("ATOM_ARPING_IP", &atomIp) == 0)
    {
        AnscTraceWarning(("atom ip present in device.properties file is %s\n",atomIp));
        sprintf(rpcCmd,"rpcclient %s", atomIp);
    }
    snprintf(cmd,sizeof(cmd)-1," %s %s",rpcCmd,"\"pidof CcspWifiSsp\" | grep -v \"RPC\"");	
	    
    if((f = popen(cmd, "r")) == NULL) {
        printf("popen %s error\n", cmd);
        return -1;
    }
	    
    while(!feof(f))
    {
        *ptr = 0;
        fgets(ptr,LOGAGENT_MAX_READ_SIZE,f);
        iBufferRead += strlen(ptr);
	    if((strlen(ptr) == 0) || (iBufferRead >(LOGAGENT_MAX_BUF_SIZE-1)) )
        {
            break;
        }
        ptr += strlen(ptr);
    }
    pclose(f);
	f = NULL;
	const char delim_rpc[3] = "\n";
   	char *token;
   
   	/* get the first token */
   	token = strtok(Buf, delim_rpc);
	if ( token == NULL )
	{
	token ='\0';
	}
    snprintf(cmd2,sizeof(cmd2)-1,"%s %s %s %s",rpcCmd," \"kill -10 ", token,"\""); /*RDKB-7469, CID-33124, limiting Buffer copied to contain in cmd2*/
    
    if((f = popen(cmd2, "r")) == NULL) {
        printf("popen %s error\n", cmd2);
	return -1;
    }
    pclose(f);
    return 0;
}
#endif

static int SendSignal(char *proc)
{
    FILE *f = NULL;
    char Buf[LOGAGENT_MAX_BUF_SIZE] = {0};
    char *ptr = Buf;
    char cmd[LOGAGENT_PROC_NAME_LEN] = {0};
    char cmd2[LOGAGENT_MAX_COMMAND_LEN] = {0};
    unsigned int iBufferRead = 0;
    errno_t rc = -1;
    
    rc =  sprintf_s(cmd,sizeof(cmd),"%s %s","pidof ",proc);
    if(rc < EOK)
    {
        ERR_CHK(rc);
        return -1;
    }

    if((f = popen(cmd, "r")) == NULL) {
        printf("popen %s error\n", cmd);
        return -1;
    }

    while(!feof(f))
    {
        *ptr = 0;
        if (fgets(ptr,LOGAGENT_MAX_READ_SIZE,f) == NULL)
        {
            break;
        }
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
    /*RDKB-7469, CID-33124, limiting Buffer copied to contain in cmd2*/
    f = NULL;
    rc = sprintf_s(cmd2,sizeof(cmd2),"kill -10 %s", Buf);
    if(rc < EOK)
    {
        ERR_CHK(rc);
        return -1;
    }

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
    UNREFERENCED_PARAMETER(hInsContext);
    errno_t rc       = -1;
    int     ind      = -1;
    //printf("$$$$Inside LogAgent_GetParamUlongValue in cosa_apis_logagentplugin.c\n");
        /*loglevel is a sample parameter. Need to be removed. Changed for RDKB-4800*/
/*
        if (strcmp(ParamName, "loglevel") == 0)
    {
         //printf("$$$$Inside LogAgent_GetParamUlongValue loglevel\n");
        *puLong =  &i;
         //printf("$$$$puLong = %d\n",*puLong);
        return TRUE;
   
    }*/
	char buf[5] = {0};
        rc = strcmp_s("X_RDKCENTRAL-COM_LogLevel",strlen("X_RDKCENTRAL-COM_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
        {
        	syscfg_get( NULL, "X_RDKCENTRAL-COM_LogLevel", buf, sizeof(buf));
        	if( buf[0] != '\0' )
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
        rc = strcmp_s("X_RDKCENTRAL-COM_TR69_LogLevel",strlen("X_RDKCENTRAL-COM_TR69_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
        {
        	syscfg_get( NULL, "X_RDKCENTRAL-COM_TR69_LogLevel", buf, sizeof(buf));
        	if( buf[0] != '\0' )
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
        rc = strcmp_s("X_RDKCENTRAL-COM_PAM_LogLevel",strlen("X_RDKCENTRAL-COM_PAM_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
        {
        	syscfg_get( NULL, "X_RDKCENTRAL-COM_PAM_LogLevel", buf, sizeof(buf));
        	if( buf[0] != '\0' )
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
        rc = strcmp_s("X_RDKCENTRAL-COM_PSM_LogLevel",strlen("X_RDKCENTRAL-COM_PSM_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
        {
        	syscfg_get( NULL, "X_RDKCENTRAL-COM_PSM_LogLevel", buf, sizeof(buf));
        	if( buf[0] != '\0' )
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
        rc = strcmp_s("X_RDKCENTRAL-COM_MTA_LogLevel",strlen("X_RDKCENTRAL-COM_MTA_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
        {
        	syscfg_get( NULL, "X_RDKCENTRAL-COM_MTA_LogLevel", buf, sizeof(buf));
        	if( buf[0] != '\0' )
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
        rc = strcmp_s("X_RDKCENTRAL-COM_CM_LogLevel",strlen("X_RDKCENTRAL-COM_CM_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
        {
        	syscfg_get( NULL, "X_RDKCENTRAL-COM_CM_LogLevel", buf, sizeof(buf));
        	if( buf[0] != '\0' )
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
        rc = strcmp_s("X_RDKCENTRAL-COM_WiFi_LogLevel",strlen("X_RDKCENTRAL-COM_WiFi_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
        {
        	syscfg_get( NULL, "X_RDKCENTRAL-COM_WiFi_LogLevel", buf, sizeof(buf));
        	if( buf[0] != '\0' )
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
        rc = strcmp_s("X_RDKCENTRAL-COM_CR_LogLevel",strlen("X_RDKCENTRAL-COM_CR_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
        {
        	syscfg_get( NULL, "X_RDKCENTRAL-COM_CR_LogLevel", buf, sizeof(buf));
        	if( buf[0] != '\0' )
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
        rc = strcmp_s("X_RDKCENTRAL-COM_Harvester_LogLevel",strlen("X_RDKCENTRAL-COM_Harvester_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
        {
        	syscfg_get( NULL, "X_RDKCENTRAL-COM_Harvester_LogLevel", buf, sizeof(buf));
        	if( buf[0] != '\0' )
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
        rc = strcmp_s("X_RDKCENTRAL-COM_NotifyComp_LogLevel",strlen("X_RDKCENTRAL-COM_NotifyComp_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
        {
        	syscfg_get( NULL, "X_RDKCENTRAL-COM_NotifyComp_LogLevel", buf, sizeof(buf));
        	if( buf[0] != '\0' )
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
        rc = strcmp_s("X_RDKCENTRAL-COM_PowerMgr_LogLevel",strlen("X_RDKCENTRAL-COM_PowerMgr_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
        {
        	syscfg_get( NULL, "X_RDKCENTRAL-COM_PowerMgr_LogLevel", buf, sizeof(buf));
        	if( buf[0] != '\0' )
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
        rc = strcmp_s("X_RDKCENTRAL-COM_EthAgent_LogLevel",strlen("X_RDKCENTRAL-COM_EthAgent_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
        {
        	syscfg_get( NULL, "X_RDKCENTRAL-COM_EthAgent_LogLevel", buf, sizeof(buf));
        	if( buf[0] != '\0' )
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
        rc = strcmp_s("X_RDKCENTRAL-COM_FSC_LogLevel",strlen("X_RDKCENTRAL-COM_FSC_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
        {
        	syscfg_get( NULL, "X_RDKCENTRAL-COM_FSC_LogLevel", buf, sizeof(buf));
        	if( buf[0] != '\0' )
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
        rc = strcmp_s("X_RDKCENTRAL-COM_Mesh_LogLevel",strlen("X_RDKCENTRAL-COM_Mesh_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
        {
        	syscfg_get( NULL, "X_RDKCENTRAL-COM_Mesh_LogLevel", buf, sizeof(buf));
        	if( buf[0] != '\0' )
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
        rc = strcmp_s("X_RDKCENTRAL-COM_MeshService_LogLevel",strlen("X_RDKCENTRAL-COM_MeshService_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
        {
        	syscfg_get( NULL, "X_RDKCENTRAL-COM_MeshService_LogLevel", buf, sizeof(buf));
        	if( buf[0] != '\0' )
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
        rc = strcmp_s("X_RDKCENTRAL-COM_TelcoVOIPAgent_LogLevel",strlen("X_RDKCENTRAL-COM_TelcoVOIPAgent_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
        {
            *puLong  = TELCOVOIPAGENT_RDKLogLevel;
            return TRUE;
        }
#if defined (FEATURE_RDKB_WAN_MANAGER)
        if (strcmp(ParamName, "X_RDKCENTRAL-COM_VLANManager_LogLevel") == 0)
        {
            *puLong  = VLANMANAGER_RDKLogLevel;
            return TRUE;
        }

#if defined (FEATURE_RDKB_XDSL_PPP_MANAGER)
        if (strcmp(ParamName, "X_RDKCENTRAL-COM_XDSLManager_LogLevel") == 0)
        {
            *puLong  = XDSLManager_RDKLogLevel;
            return TRUE;
        }
        if (strcmp(ParamName, "X_RDKCENTRAL-COM_PppManager_LogLevel") == 0)
        {
            *puLong  = PPPMANAGER_RDKLogLevel;
            return TRUE;
        }
#endif
#if defined (FEATURE_RDKB_TELCOVOICE_MANAGER)
        if (strcmp(ParamName, "X_RDKCENTRAL-COM_TelcoVOICEManager_LogLevel") == 0)
        {
            *puLong  = TELCOVOICEMANAGER_RDKLogLevel;
            return TRUE;
        }
#endif
#endif
#if defined (FEATURE_FWUPGRADE_MANAGER)
        if (strcmp(ParamName, "X_RDKCENTRAL-COM_FwUpgradeManager_LogLevel") == 0)
        {
            *puLong  = FWUPGRADEMGR_RDKLogLevel;
            return TRUE;
        }
#endif
        if (strcmp(ParamName, "X_RDKCENTRAL-COM_DSLAgent_LogLevel") == 0)
        {
            *puLong  = DSLAGENT_RDKLogLevel;
            return TRUE;
        }
        if (strcmp(ParamName, "X_RDKCENTRAL-COM_XTMAgent_LogLevel") == 0)
        {
            *puLong  = XTMAGENT_RDKLogLevel;
            return TRUE;
        }	
        if (strcmp(ParamName, "X_RDKCENTRAL-COM_VLANAgent_LogLevel") == 0)
        {
            *puLong  = VLANAGENT_RDKLogLevel;
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
    UNREFERENCED_PARAMETER(hInsContext);
    errno_t rc       = -1;
    int     ind      = -1;
	/*loglevel is a sample parameter. Need to be removed. Changed for RDKB-4800*/
/*
	if (strcmp(ParamName, "loglevel") == 0)
	{
		//printf(" LogAgent_SetParamValues : loglevel \n");
		return TRUE;
	  
	}
*/

        rc = strcmp_s("X_RDKCENTRAL-COM_LogLevel",strlen("X_RDKCENTRAL-COM_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
	{
		if ( RDKLogLevel != uValue)
		{
			if (syscfg_set_u_commit(NULL, "X_RDKCENTRAL-COM_LogLevel", uValue) != 0)
			{
				AnscTraceWarning(("syscfg_set failed\n"));
			}
			else 
			{
				RDKLogLevel = uValue;
			}          
		}
		return TRUE;
	}
        rc = strcmp_s("X_RDKCENTRAL-COM_TR69_LogLevel",strlen("X_RDKCENTRAL-COM_TR69_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
	{
		if (TR69_RDKLogLevel != uValue)
		{
			if (syscfg_set_u_commit(NULL, "X_RDKCENTRAL-COM_TR69_LogLevel", uValue) != 0)
			{
				AnscTraceWarning(("syscfg_set failed\n"));
			}
			else 
			{
				TR69_RDKLogLevel = uValue;
			}
		}
		SendSignal(TR069_PROC_NAME);
		return TRUE;
	}
        rc = strcmp_s("X_RDKCENTRAL-COM_PAM_LogLevel",strlen("X_RDKCENTRAL-COM_PAM_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
        {
		if (PAM_RDKLogLevel != uValue)
		{
			if (syscfg_set_u_commit(NULL, "X_RDKCENTRAL-COM_PAM_LogLevel", uValue) != 0)
			{
				AnscTraceWarning(("syscfg_set failed\n"));
			}
			else 
			{
				PAM_RDKLogLevel = uValue;
			}
		}
		SendSignal(PAM_PROC_NAME);
		return TRUE;
	}
        rc = strcmp_s("X_RDKCENTRAL-COM_PSM_LogLevel",strlen("X_RDKCENTRAL-COM_PSM_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
	{
		if (PSM_RDKLogLevel != uValue)
		{
			if (syscfg_set_u_commit(NULL, "X_RDKCENTRAL-COM_PSM_LogLevel", uValue) != 0)
			{
				AnscTraceWarning(("syscfg_set failed\n"));
			}
			else 
			{
				PSM_RDKLogLevel = uValue;
			}
		}
		SendSignal(PSM_PROC_NAME);
		return TRUE;
	}
        rc = strcmp_s("X_RDKCENTRAL-COM_MTA_LogLevel",strlen("X_RDKCENTRAL-COM_MTA_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
	{
		if (MTA_RDKLogLevel != uValue)
		{
			if (syscfg_set_u_commit(NULL, "X_RDKCENTRAL-COM_MTA_LogLevel", uValue) != 0)
			{
				AnscTraceWarning(("syscfg_set failed\n"));
			}
			else 
			{
				MTA_RDKLogLevel = uValue;
			}
		}
		SendSignal(MTA_PROC_NAME);
		return TRUE;
        }
        rc = strcmp_s("X_RDKCENTRAL-COM_CM_LogLevel",strlen("X_RDKCENTRAL-COM_CM_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
        {
		if (CM_RDKLogLevel != uValue)
		{
			if (syscfg_set_u_commit(NULL, "X_RDKCENTRAL-COM_CM_LogLevel", uValue) != 0)
			{
				AnscTraceWarning(("syscfg_set failed\n"));
			}
			else 
			{
				CM_RDKLogLevel = uValue;
			}
		}
		SendSignal(CM_PROC_NAME);
		return TRUE;
	}
        rc = strcmp_s("X_RDKCENTRAL-COM_WiFi_LogLevel",strlen("X_RDKCENTRAL-COM_WiFi_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
	{
		if (WiFi_RDKLogLevel != uValue)
		{
			if (syscfg_set_u_commit(NULL, "X_RDKCENTRAL-COM_WiFi_LogLevel", uValue) != 0)
			{
				AnscTraceWarning(("syscfg_set failed\n"));
			}
			else 
			{
				WiFi_RDKLogLevel = uValue;
			}
		}
		
		#if defined(_COSA_INTEL_XB3_ARM_)
		SendSignal_wifi();
		#else 
		SendSignal(WIFI_PROC_NAME);
#endif
		return TRUE;
	}
        rc = strcmp_s("X_RDKCENTRAL-COM_CR_LogLevel",strlen("X_RDKCENTRAL-COM_CR_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
	{
		if (CR_RDKLogLevel != uValue)
		{
			if (syscfg_set_u_commit(NULL, "X_RDKCENTRAL-COM_CR_LogLevel", uValue) != 0)
			{
				AnscTraceWarning(("syscfg_set failed\n"));
			}
			else 
			{
				CR_RDKLogLevel = uValue;
			}
		}
		return TRUE;
	}

/*Added for RDKB-4343*/
        rc = strcmp_s("X_RDKCENTRAL-COM_Harvester_LogLevel",strlen("X_RDKCENTRAL-COM_Harvester_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
	{
		if (Harvester_RDKLogLevel != uValue)
		{
			printf("Setting Harvester_RDKLogLevel to %d\n",Harvester_RDKLogLevel);
			if (syscfg_set_u_commit(NULL, "X_RDKCENTRAL-COM_Harvester_LogLevel", uValue) != 0)
			{
				AnscTraceWarning(("syscfg_set failed\n"));
				printf("syscfg_set failed\n");
			}
			else 
			{
				Harvester_RDKLogLevel = uValue;
			} 
		}
		return TRUE;
	}
        rc = strcmp_s("X_RDKCENTRAL-COM_NotifyComp_LogLevel",strlen("X_RDKCENTRAL-COM_NotifyComp_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
	{
		if (NOTIFY_RDKLogLevel != uValue)
		{
			if (syscfg_set_u_commit(NULL, "X_RDKCENTRAL-COM_NotifyComp_LogLevel", uValue) != 0)
			{
				AnscTraceWarning(("syscfg_set failed\n"));
			}
			else 
			{
				NOTIFY_RDKLogLevel = uValue;
			}
		}
		SendSignal(NOTIFY_PROC_NAME);
		return TRUE;
	}
        rc = strcmp_s("X_RDKCENTRAL-COM_PowerMgr_LogLevel",strlen("X_RDKCENTRAL-COM_PowerMgr_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
	{
		if (PWRMGR_RDKLogLevel != uValue)
		{
			if (syscfg_set_u_commit(NULL, "X_RDKCENTRAL-COM_PowerMgr_LogLevel", uValue) != 0)
			{
				AnscTraceWarning(("syscfg_set failed\n"));
			}
			else
			{
				PWRMGR_RDKLogLevel = uValue;
			}
		}
		SendSignal(PWRMGR_PROC_NAME);
		return TRUE;
	}
        rc = strcmp_s("X_RDKCENTRAL-COM_FSC_LogLevel",strlen("X_RDKCENTRAL-COM_FSC_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
	{
		if (FSC_RDKLogLevel != uValue)
		{
			if (syscfg_set_u_commit(NULL, "X_RDKCENTRAL-COM_FSC_LogLevel", uValue) != 0)
			{
				AnscTraceWarning(("syscfg_set failed\n"));
			}
			else
			{
				FSC_RDKLogLevel = uValue;
			}
		}
		SendSignal(FSC_PROC_NAME);
		return TRUE;
	}
        rc = strcmp_s("X_RDKCENTRAL-COM_EthAgent_LogLevel",strlen("X_RDKCENTRAL-COM_EthAgent_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
	{
		if (ETHAGENT_RDKLogLevel != uValue)
		{
			if (syscfg_set_u_commit(NULL, "X_RDKCENTRAL-COM_EthAgent_LogLevel", uValue) != 0)
			{
				AnscTraceWarning(("syscfg_set failed\n"));
			}
			else
			{
				ETHAGENT_RDKLogLevel = uValue;
			}
		}
		SendSignal(ETHAGENT_PROC_NAME);
		return TRUE;
	}
        rc = strcmp_s("X_RDKCENTRAL-COM_Mesh_LogLevel",strlen("X_RDKCENTRAL-COM_Mesh_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
	{
		if (MESH_RDKLogLevel != uValue)
		{
			if (syscfg_set_u_commit(NULL, "X_RDKCENTRAL-COM_Mesh_LogLevel", uValue) != 0)
			{
				AnscTraceWarning(("syscfg_set failed\n"));
			}
			else
			{
				MESH_RDKLogLevel = uValue;
			}
		}
		SendSignal(MESH_PROC_NAME);
		return TRUE;
	}
        rc = strcmp_s("X_RDKCENTRAL-COM_MeshService_LogLevel",strlen("X_RDKCENTRAL-COM_MeshService_LogLevel"),ParamName,&ind);
        ERR_CHK(rc);
        if ((!ind) && (rc == EOK))
	{
		if (MeshService_RDKLogLevel != uValue)
		{
			if (syscfg_set_u_commit(NULL, "X_RDKCENTRAL-COM_MeshService_LogLevel", uValue) != 0)
			{
				AnscTraceWarning(("syscfg_set failed\n"));
			}
			else
			{
				MeshService_RDKLogLevel = uValue;
			}
		}
		return TRUE;
	}

#if defined(_HUB4_PRODUCT_REQ_)
    rc = strcmp_s("X_RDKCENTRAL-COM_TelcoVOIPAgent_LogLevel",strlen("X_RDKCENTRAL-COM_TelcoVOIPAgent_LogLevel"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
        TELCOVOIPAGENT_RDKLogLevel = uValue;
        if (syscfg_set_u_commit(NULL, "X_RDKCENTRAL-COM_TelcoVOIPAgent_LogLevel", uValue) != 0)
        {
            AnscTraceWarning(("syscfg_set failed\n"));
        }
        SendSignal(TELCOVOIPAGENT_PROC_NAME);
        return TRUE;
    }
#endif /* _HUB4_PRODUCT_REQ_ */

#if defined(FEATURE_RDKB_WAN_MANAGER)
    if (strcmp(ParamName, "X_RDKCENTRAL-COM_VLANManager_LogLevel") == 0)
    {
        VLANMANAGER_RDKLogLevel = uValue;
        if (syscfg_set_u_commit(NULL, "X_RDKCENTRAL-COM_VLANManager_LogLevel", uValue) != 0)
        {
            AnscTraceWarning(("syscfg_set failed\n"));
        }
        SendSignal(VLANMANAGER_PROC_NAME);
        return TRUE;
    }

#if defined (FEATURE_RDKB_XDSL_PPP_MANAGER)
    if (strcmp(ParamName, "X_RDKCENTRAL-COM_XDSLManager_LogLevel") == 0)
    {
        XDSLManager_RDKLogLevel = uValue;
        if (syscfg_set_u_commit(NULL, "X_RDKCENTRAL-COM_XDSLManager_LogLevel", uValue) != 0)
        {
            AnscTraceWarning(("syscfg_set failed\n"));
        }
        SendSignal(XDSLMANAGER_PROC_NAME);
        return TRUE;
    }

   if (strcmp(ParamName, "X_RDKCENTRAL-COM_PppManager_LogLevel") == 0)
    {
        PPPMANAGER_RDKLogLevel = uValue;
        if (syscfg_set_u_commit(NULL, "X_RDKCENTRAL-COM_PppManager_LogLevel", uValue) != 0)
        {
            AnscTraceWarning(("syscfg_set failed\n"));
        }
        SendSignal(PPPMANAGER_PROC_NAME);
        return TRUE;
    }    
#endif

#if defined(FEATURE_RDKB_TELCOVOICE_MANAGER)
   if (strcmp(ParamName, "X_RDKCENTRAL-COM_TelcoVOICEManager_LogLevel") == 0)
    {
        TELCOVOICEMANAGER_RDKLogLevel = uValue;
        if (syscfg_set_u_commit(NULL, "X_RDKCENTRAL-COM_TelcoVOICEManager_LogLevel", uValue) != 0)
        {
            AnscTraceWarning(("syscfg_set failed\n"));
        }
        SendSignal(TELCOVOICEMANAGER_PROC_NAME);
        return TRUE;
    }
#endif
#endif

#if defined (FEATURE_RDKB_FWUPGRADE_MANAGER)
   if (strcmp(ParamName, "X_RDKCENTRAL-COM_FwUpgradeManager_LogLevel") == 0)
    {
        FWUPGRADEMGR_RDKLogLevel = uValue;
        if (syscfg_set_u_commit(NULL, "X_RDKCENTRAL-COM_FwUpgradeManager_LogLevel", uValue) != 0)
        {
            AnscTraceWarning(("syscfg_set failed\n"));
        }
        SendSignal(FWUPGRADEMANAGER_PROC_NAME);
        return TRUE;
    }
#endif
    if (strcmp(ParamName, "X_RDKCENTRAL-COM_DSLAgent_LogLevel") == 0)
    {
        DSLAGENT_RDKLogLevel = uValue;
        if (syscfg_set_u_commit(NULL, "X_RDKCENTRAL-COM_DSLAgent_LogLevel", uValue) != 0)
        {
            AnscTraceWarning(("syscfg_set failed\n"));
        }
        SendSignal(DSLAGENT_PROC_NAME);
        return TRUE;
    }
    if (strcmp(ParamName, "X_RDKCENTRAL-COM_XTMAgent_LogLevel") == 0)
    {
        XTMAGENT_RDKLogLevel = uValue;
        if (syscfg_set_u_commit(NULL, "X_RDKCENTRAL-COM_XTMAgent_LogLevel", uValue) != 0)
        {
            AnscTraceWarning(("syscfg_set failed\n"));
        }
        SendSignal(XTMAGENT_PROC_NAME);
        return TRUE;
    }
    if (strcmp(ParamName, "X_RDKCENTRAL-COM_VLANAgent_LogLevel") == 0)
    {
        VLANAGENT_RDKLogLevel = uValue;
        if (syscfg_set_u_commit(NULL, "X_RDKCENTRAL-COM_VLANAgent_LogLevel", uValue) != 0)
        {
            AnscTraceWarning(("syscfg_set failed\n"));
        }
        SendSignal(VLANAGENT_PROC_NAME);
        return TRUE;
    }

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
    UNREFERENCED_PARAMETER(hInsContext);
    char LogMsg_arr[512] = {0};
    char *LogMsg = LogMsg_arr;
    char LogLevel[512] = {0};
    int level = 0;
    char WiFiLogeComponent[100] = "com.cisco.spvtg.ccsp.logagent";
    char HarvesterLogeComponent[100] = "harvester";
    errno_t rc = -1;
    int ind = -1;
    size_t len = 0;
        
    #if !defined(__STDC_LIB_EXT1__)
       UNREFERENCED_PARAMETER(len);
    #endif

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
        strtok_s (LogLevel, &len, ",",&LogMsg);
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
          strtok_s(LogLevel, &len , ",", &LogMsg);
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
         strtok_s (LogLevel, &len, ",", &LogMsg);
         if(errno != EOK)
         {
            ERR_CHK(errno);
            return FALSE;
         }

        syslog_eventlog("Wifi", LOG_NOTICE, "%s", LogMsg);
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
         strtok_s(LogLevel, &len,",",&LogMsg);
         if(errno != EOK)
         {
              ERR_CHK(errno);
              return FALSE;
         }

        syslog_eventlog("Harvester", LOG_NOTICE, "%s", LogMsg);
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
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(pUlSize);
    errno_t rc       = -1;
    int     ind      = -1;
   
    /* check the parameter name and set the corresponding value */
    rc = strcmp_s("WifiLogMsg",strlen("WifiLogMsg"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK)) 
    {
        //printf("$$$$ Inside LogAgent_GetParamStringValue for WiFiLogMsg\n");
        rc  = strcpy_s(pValue, *pUlSize, "WiFiLogMsg");
        if(rc != EOK)
        {
             ERR_CHK(rc);
             return -1;
        }
        return 0;
    }
/*Added for rdkb-4343*/
    rc = strcmp_s("HarvesterLogMsg",strlen("HarvesterLogMsg"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK)) 
    {
        //printf("$$$$ Inside LogAgent_GetParamStringValue for HarvesterLogMsg\n");
        rc  = strcpy_s(pValue, *pUlSize, "HarvesterLogMsg");
        if(rc != EOK)
        {
              ERR_CHK(rc);
              return -1;
        }

        return 0;
    }

/*rdkb-4776*/
    rc = strcmp_s("WifiEventLogMsg",strlen("WifiEventLogMsg"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
	//printf("$$$$Inside WifiEventLogMsg Returning true\n");
        rc  = strcpy_s(pValue, *pUlSize, "WifiEventLogMsg");
        if(rc != EOK)
        {
              ERR_CHK(rc);
              return -1;
        }

        return 0;
    }
    rc = strcmp_s("HarvesterEventLogMsg",strlen("HarvesterEventLogMsg"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
	//printf("$$$$Inside HarvesterEventLogMsg Returning true\n");
        rc  = strcpy_s(pValue, *pUlSize, "HarvesterEventLogMsg");
        if(rc != EOK)
        {
              ERR_CHK(rc);
              return -1;
        }

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
    UNREFERENCED_PARAMETER(hInsContext);
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
    UNREFERENCED_PARAMETER(hInsContext);
    /* check the parameter name and return the corresponding value */
    char buf[5] = {0};
    errno_t rc       = -1;
    int     ind      = -1;
    rc = strcmp_s("X_RDKCENTRAL-COM_LoggerEnable",strlen("X_RDKCENTRAL-COM_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
    	syscfg_get( NULL, "X_RDKCENTRAL-COM_LoggerEnable", buf, sizeof(buf));
    	if( buf[0] != '\0' )
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
    rc = strcmp_s("X_RDKCENTRAL-COM_TR69_LoggerEnable",strlen("X_RDKCENTRAL-COM_TR69_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
    	syscfg_get( NULL, "X_RDKCENTRAL-COM_TR69_LoggerEnable", buf, sizeof(buf));
    	if( buf[0] != '\0' )
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
    rc = strcmp_s("X_RDKCENTRAL-COM_PAM_LoggerEnable",strlen("X_RDKCENTRAL-COM_PAM_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
	syscfg_get( NULL, "X_RDKCENTRAL-COM_PAM_LoggerEnable", buf, sizeof(buf));
    	if( buf[0] != '\0' )
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
    rc = strcmp_s("X_RDKCENTRAL-COM_PSM_LoggerEnable",strlen("X_RDKCENTRAL-COM_PSM_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
	syscfg_get( NULL, "X_RDKCENTRAL-COM_PSM_LoggerEnable", buf, sizeof(buf));
    	if( buf[0] != '\0' )
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
    rc = strcmp_s("X_RDKCENTRAL-COM_MTA_LoggerEnable",strlen("X_RDKCENTRAL-COM_MTA_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
	syscfg_get( NULL, "X_RDKCENTRAL-COM_MTA_LoggerEnable", buf, sizeof(buf));
    	if( buf[0] != '\0' )
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
    rc = strcmp_s("X_RDKCENTRAL-COM_CM_LoggerEnable",strlen("X_RDKCENTRAL-COM_CM_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))

    {
	syscfg_get( NULL, "X_RDKCENTRAL-COM_CM_LoggerEnable", buf, sizeof(buf));
    	if( buf[0] != '\0' )
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
    rc = strcmp_s("X_RDKCENTRAL-COM_WiFi_LoggerEnable",strlen("X_RDKCENTRAL-COM_WiFi_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
	syscfg_get( NULL, "X_RDKCENTRAL-COM_WiFi_LoggerEnable", buf, sizeof(buf));
    	if( buf[0] != '\0' )
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
    rc = strcmp_s("X_RDKCENTRAL-COM_CR_LoggerEnable",strlen("X_RDKCENTRAL-COM_CR_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
	syscfg_get( NULL, "X_RDKCENTRAL-COM_CR_LoggerEnable", buf, sizeof(buf));
    	if( buf[0] != '\0' )
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
    rc = strcmp_s("X_RDKCENTRAL-COM_Harvester_LoggerEnable",strlen("X_RDKCENTRAL-COM_Harvester_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
	syscfg_get( NULL, "X_RDKCENTRAL-COM_Harvester_LoggerEnable", buf, sizeof(buf));
    	if( buf[0] != '\0' )
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
    rc = strcmp_s("X_RDKCENTRAL-COM_NotifyComp_LoggerEnable",strlen("X_RDKCENTRAL-COM_NotifyComp_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
        syscfg_get( NULL, "X_RDKCENTRAL-COM_NotifyComp_LoggerEnable", buf, sizeof(buf));
        if( buf[0] != '\0' )
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
    rc = strcmp_s("X_RDKCENTRAL-COM_PowerMgr_LoggerEnable",strlen("X_RDKCENTRAL-COM_PowerMgr_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
        syscfg_get( NULL, "X_RDKCENTRAL-COM_PowerMgr_LoggerEnable", buf, sizeof(buf));
        if( buf[0] != '\0' )
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
    rc = strcmp_s("X_RDKCENTRAL-COM_EthAgent_LoggerEnable",strlen("X_RDKCENTRAL-COM_EthAgent_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
        syscfg_get( NULL, "X_RDKCENTRAL-COM_EthAgent_LoggerEnable", buf, sizeof(buf));
        if( buf[0] != '\0' )
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
    rc = strcmp_s("X_RDKCENTRAL-COM_FSC_LoggerEnable",strlen("X_RDKCENTRAL-COM_FSC_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
        syscfg_get( NULL, "X_RDKCENTRAL-COM_FSC_LoggerEnable", buf, sizeof(buf));
        if( buf[0] != '\0' )
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
    rc = strcmp_s("X_RDKCENTRAL-COM_Mesh_LoggerEnable",strlen("X_RDKCENTRAL-COM_Mesh_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
        syscfg_get( NULL, "X_RDKCENTRAL-COM_Mesh_LoggerEnable", buf, sizeof(buf));
        if( buf[0] != '\0' )
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
    rc = strcmp_s("X_RDKCENTRAL-COM_MeshService_LoggerEnable",strlen("X_RDKCENTRAL-COM_MeshService_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
        syscfg_get( NULL, "X_RDKCENTRAL-COM_MeshService_LoggerEnable", buf, sizeof(buf));
        if( buf[0] != '\0' )
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

    rc = strcmp_s("X_RDKCENTRAL-COM_TelcoVOIPAgent_LoggerEnable",strlen("X_RDKCENTRAL-COM_TelcoVOIPAgent_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
        *pBool  = TELCOVOIPAGENT_RDKLogEnable;
        return TRUE;
    }
#if defined (FEATURE_RDKB_WAN_MANAGER)
    if (strcmp(ParamName, "X_RDKCENTRAL-COM_VLANManager_LoggerEnable") == 0)
    {
        *pBool  = VLANMANAGER_RDKLogEnable;
        return TRUE;
    }

#if defined (FEATURE_RDKB_XDSL_PPP_MANAGER)
    if (strcmp(ParamName, "X_RDKCENTRAL-COM_XDSLManager_LoggerEnable") == 0)
    {
        *pBool  = XDSLManager_RDKLogEnable;
        return TRUE;
    }
    if (strcmp(ParamName, "X_RDKCENTRAL-COM_PppManager_LoggerEnable") == 0)
    {
        *pBool  = PPPMANAGER_RDKLogEnable;
        return TRUE;
    }
#endif

#if defined (FEATURE_RDKB_TELCOVOICE_MANAGER)
    if (strcmp(ParamName, "X_RDKCENTRAL-COM_TelcoVOICEManager_LoggerEnable") == 0)
    {
        *pBool  = TELCOVOICEMANAGER_RDKLogEnable;
        return TRUE;
    }
#endif
#endif

#if defined (FEATURE_FWUPGRADE_MANAGER)
    if (strcmp(ParamName, "X_RDKCENTRAL-COM_FwUpgradeManager_LoggerEnable") == 0)
    {
        *pBool  = FWUPGRADEMGR_RDKLogEnable;
        return TRUE;
    }
#endif
    if (strcmp(ParamName, "X_RDKCENTRAL-COM_DSLAgent_LoggerEnable") == 0)
    {
        *pBool  = DSLAGENT_RDKLogEnable;
        return TRUE;
    }
    if (strcmp(ParamName, "X_RDKCENTRAL-COM_XTMAgent_LoggerEnable") == 0)
    {
        *pBool  = XTMAGENT_RDKLogEnable;
        return TRUE;
    }    
    if (strcmp(ParamName, "X_RDKCENTRAL-COM_VLANAgent_LoggerEnable") == 0)
    {
        *pBool  = VLANAGENT_RDKLogEnable;
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
    UNREFERENCED_PARAMETER(hInsContext);
    /* check the parameter name and set the corresponding value */
    errno_t rc       = -1;
    int     ind      = -1;
    rc = strcmp_s("X_RDKCENTRAL-COM_LoggerEnable",strlen("X_RDKCENTRAL-COM_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
		RDKLogEnable = bValue;

        if (syscfg_set_commit(NULL, "X_RDKCENTRAL-COM_LoggerEnable", bValue ? "1" : "0") != 0)
		{
             AnscTraceWarning(("syscfg_set failed\n"));
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
		SW_Dealy();
		#if defined(_COSA_INTEL_XB3_ARM_)
		SendSignal_wifi();
		SW_Dealy();
		#else 
		SendSignal(WIFI_PROC_NAME);
		SW_Dealy();
		#endif													
		return TRUE;
    }
    rc = strcmp_s("X_RDKCENTRAL-COM_TR69_LoggerEnable",strlen("X_RDKCENTRAL-COM_TR69_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
		TR69_RDKLogEnable = bValue;

		if (syscfg_set_commit(NULL, "X_RDKCENTRAL-COM_TR69_LoggerEnable", bValue ? "1" : "0") != 0)
		{
			AnscTraceWarning(("syscfg_set failed\n"));
		}
		SendSignal(TR069_PROC_NAME);
		return TRUE;
    }
    rc = strcmp_s("X_RDKCENTRAL-COM_PAM_LoggerEnable",strlen("X_RDKCENTRAL-COM_PAM_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
		PAM_RDKLogEnable = bValue;
        if (syscfg_set_commit(NULL, "X_RDKCENTRAL-COM_PAM_LoggerEnable", bValue ? "1" : "0") != 0)
		{
             AnscTraceWarning(("syscfg_set failed\n"));
		}
		SendSignal(PAM_PROC_NAME);
		return TRUE;
    }
    rc = strcmp_s("X_RDKCENTRAL-COM_PSM_LoggerEnable",strlen("X_RDKCENTRAL-COM_PSM_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
	{
		PSM_RDKLogEnable = bValue;
		if (syscfg_set_commit(NULL, "X_RDKCENTRAL-COM_PSM_LoggerEnable", bValue ? "1" : "0") != 0)
		{
			 AnscTraceWarning(("syscfg_set failed\n"));
		}
		SendSignal(PSM_PROC_NAME);
		return TRUE;
	}
    rc = strcmp_s("X_RDKCENTRAL-COM_MTA_LoggerEnable",strlen("X_RDKCENTRAL-COM_MTA_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
		MTA_RDKLogEnable = bValue;
		if (syscfg_set_commit(NULL, "X_RDKCENTRAL-COM_MTA_LoggerEnable", bValue ? "1" : "0") != 0)
		{
			 AnscTraceWarning(("syscfg_set failed\n"));
		}
		SendSignal(MTA_PROC_NAME);
		return TRUE;
    }
    rc = strcmp_s("X_RDKCENTRAL-COM_CM_LoggerEnable",strlen("X_RDKCENTRAL-COM_CM_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
		CM_RDKLogEnable = bValue;
		if (syscfg_set_commit(NULL, "X_RDKCENTRAL-COM_CM_LoggerEnable", bValue ? "1" : "0") != 0)
		{
			AnscTraceWarning(("syscfg_set failed\n"));
		}
		SendSignal(CM_PROC_NAME);
		return TRUE;
    }
    rc = strcmp_s("X_RDKCENTRAL-COM_WiFi_LoggerEnable",strlen("X_RDKCENTRAL-COM_WiFi_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
		WiFi_RDKLogEnable = bValue;
		if (syscfg_set_commit(NULL, "X_RDKCENTRAL-COM_WiFi_LoggerEnable", bValue ? "1" : "0") != 0)
		{
			AnscTraceWarning(("syscfg_set failed\n"));
		}
		#if defined(_COSA_INTEL_XB3_ARM_)
		SendSignal_wifi();
		SW_Dealy();
		#else 
		SendSignal(WIFI_PROC_NAME);
		SW_Dealy();
		#endif
		return TRUE;
    }
    rc = strcmp_s("X_RDKCENTRAL-COM_CR_LoggerEnable",strlen("X_RDKCENTRAL-COM_CR_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
		CR_RDKLogEnable = bValue;
		if (syscfg_set_commit(NULL, "X_RDKCENTRAL-COM_CR_LoggerEnable", bValue ? "1" : "0") != 0)
		{
		 	AnscTraceWarning(("syscfg_set failed\n"));
		}
		return TRUE;
    }
    rc = strcmp_s("X_RDKCENTRAL-COM_Harvester_LoggerEnable",strlen("X_RDKCENTRAL-COM_Harvester_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
		Harvester_RDKLogEnable = bValue;
		printf("$$$$ Harvester_RDKLogEnable = %d\n",Harvester_RDKLogEnable);
		if (syscfg_set_commit(NULL, "X_RDKCENTRAL-COM_Harvester_LoggerEnable", bValue ? "1" : "0") != 0)
		{
		 	AnscTraceWarning(("syscfg_set failed\n"));
			printf("syscfg_set failed\n");
		}
		return TRUE;
    }
    rc = strcmp_s("X_RDKCENTRAL-COM_NotifyComp_LoggerEnable",strlen("X_RDKCENTRAL-COM_NotifyComp_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
		NOTIFY_RDKLogEnable = bValue;
        if (syscfg_set_commit(NULL, "X_RDKCENTRAL-COM_NotifyComp_LoggerEnable", bValue ? "1" : "0") != 0)
		{
             AnscTraceWarning(("syscfg_set failed\n"));
		}
		SendSignal(NOTIFY_PROC_NAME);
		return TRUE;
    }
    rc = strcmp_s("X_RDKCENTRAL-COM_PowerMgr_LoggerEnable",strlen("X_RDKCENTRAL-COM_PowerMgr_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))

    {
        PWRMGR_RDKLogEnable = bValue;
        if (syscfg_set_commit(NULL, "X_RDKCENTRAL-COM_PowerMgr_LoggerEnable", bValue ? "1" : "0") != 0)
        {
             AnscTraceWarning(("syscfg_set failed\n"));
        }
        SendSignal(PWRMGR_PROC_NAME);
        return TRUE;
    }
    rc = strcmp_s("X_RDKCENTRAL-COM_FSC_LoggerEnable",strlen("X_RDKCENTRAL-COM_FSC_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
        FSC_RDKLogEnable = bValue;
        if (syscfg_set_commit(NULL, "X_RDKCENTRAL-COM_FSC_LoggerEnable", bValue ? "1" : "0") != 0)
        {
             AnscTraceWarning(("syscfg_set failed\n"));
        }
        SendSignal(FSC_PROC_NAME);
        return TRUE;
    }
    rc = strcmp_s("X_RDKCENTRAL-COM_Mesh_LoggerEnable",strlen("X_RDKCENTRAL-COM_Mesh_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
        MESH_RDKLogEnable = bValue;
        if (syscfg_set_commit(NULL, "X_RDKCENTRAL-COM_Mesh_LoggerEnable", bValue ? "1" : "0") != 0)
        {
             AnscTraceWarning(("syscfg_set failed\n"));
        }
        SendSignal(MESH_PROC_NAME);
        return TRUE;
    }
    rc = strcmp_s("X_RDKCENTRAL-COM_MeshService_LoggerEnable",strlen("X_RDKCENTRAL-COM_MeshService_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
        MeshService_RDKLogEnable = bValue;
        if (syscfg_set_commit(NULL, "X_RDKCENTRAL-COM_MeshService_LoggerEnable", bValue ? "1" : "0") != 0)
        {
             AnscTraceWarning(("syscfg_set failed\n"));
        }
        return TRUE;
    }

    rc = strcmp_s("X_RDKCENTRAL-COM_EthAgent_LoggerEnable",strlen("X_RDKCENTRAL-COM_EthAgent_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
        ETHAGENT_RDKLogEnable = bValue;
        if (syscfg_set_commit(NULL, "X_RDKCENTRAL-COM_EthAgent_LoggerEnable", bValue ? "1" : "0") != 0)
        {
             AnscTraceWarning(("syscfg_set failed\n"));
        }
        SendSignal(ETHAGENT_PROC_NAME);
        return TRUE;
    }

#if defined(_HUB4_PRODUCT_REQ_)
    rc = strcmp_s("X_RDKCENTRAL-COM_TelcoVOIPAgent_LoggerEnable",strlen("X_RDKCENTRAL-COM_TelcoVOIPAgent_LoggerEnable"),ParamName,&ind);
    ERR_CHK(rc);
    if ((!ind) && (rc == EOK))
    {
        TELCOVOIPAGENT_RDKLogEnable = bValue;
        if (syscfg_set_commit(NULL, "X_RDKCENTRAL-COM_TelcoVOIPAgent_LoggerEnable", bValue ? "1" : "0") != 0)
        {
            AnscTraceWarning(("syscfg_set failed\n"));
        }
        SendSignal(TELCOVOIPAGENT_PROC_NAME);
        return TRUE;
    }
#endif /* _HUB4_PRODUCT_REQ_ */

#if defined (FEATURE_RDKB_WAN_MANAGER)
    if (strcmp(ParamName, "X_RDKCENTRAL-COM_VLANManager_LoggerEnable") == 0)
    {
        VLANMANAGER_RDKLogEnable = bValue;
        if (syscfg_set_commit(NULL, "X_RDKCENTRAL-COM_VLANManager_LoggerEnable", bValue ? "1" : "0") != 0)
        {
            AnscTraceWarning(("syscfg_set failed\n"));
        }
        SendSignal(VLANMANAGER_PROC_NAME);
        return TRUE;
    }

#if defined (FEATURE_RDKB_XDSL_PPP_MANAGER)
    if (strcmp(ParamName, "X_RDKCENTRAL-COM_XDSLManager_LoggerEnable") == 0)
    {
        XDSLManager_RDKLogEnable = bValue;
        if (syscfg_set_commit(NULL, "X_RDKCENTRAL-COM_XDSLManager_LoggerEnable", bValue ? "1" : "0") != 0)
        {
            AnscTraceWarning(("syscfg_set failed\n"));
        }
        SendSignal(XDSLMANAGER_PROC_NAME);
        return TRUE;
    }
    if (strcmp(ParamName, "X_RDKCENTRAL-COM_PppManager_LoggerEnable") == 0)
    {
        PPPMANAGER_RDKLogEnable = bValue;
        if (syscfg_set_commit(NULL, "X_RDKCENTRAL-COM_PppManager_LoggerEnable", bValue ? "1" : "0") != 0)
        {
            AnscTraceWarning(("syscfg_set failed\n"));
        }
        SendSignal(PPPMANAGER_PROC_NAME);
        return TRUE;
    }
#endif

#if defined (FEATURE_RDKB_TELCOVOICE_MANAGER)
    if (strcmp(ParamName, "X_RDKCENTRAL-COM_TelcoVOICEManager_LoggerEnable") == 0)
    {
        TELCOVOICEMANAGER_RDKLogEnable = bValue;
        if (syscfg_set_commit(NULL, "X_RDKCENTRAL-COM_TelcoVOICEManager_LoggerEnable", bValue ? "1" : "0") != 0)
        {
            AnscTraceWarning(("syscfg_set failed\n"));
        }
        SendSignal(TELCOVOICEMANAGER_PROC_NAME);
        return TRUE;
    }
#endif
#endif

#if defined (FEATURE_RDKB_FWUPGRADE_MANAGER)
    if (strcmp(ParamName, "X_RDKCENTRAL-COM_FwUpgradeManager_LoggerEnable") == 0)
    {
        FWUPGRADEMGR_RDKLogEnable = bValue;
        if (syscfg_set_commit(NULL, "X_RDKCENTRAL-COM_FwUpgradeManager_LoggerEnable", bValue ? "1" : "0") != 0)
        {
            AnscTraceWarning(("syscfg_set failed\n"));
        }
        SendSignal(FWUPGRADEMANAGER_PROC_NAME);
        return TRUE;
    }
#endif
    if (strcmp(ParamName, "X_RDKCENTRAL-COM_DSLAgent_LoggerEnable") == 0)
    {
        DSLAGENT_RDKLogEnable = bValue;
        if (syscfg_set_commit(NULL, "X_RDKCENTRAL-COM_DSLAgent_LoggerEnable", bValue ? "1" : "0") != 0)
        {
            AnscTraceWarning(("syscfg_set failed\n"));
        }
        SendSignal(DSLAGENT_PROC_NAME);
        return TRUE;
    }
    if (strcmp(ParamName, "X_RDKCENTRAL-COM_XTMAgent_LoggerEnable") == 0)
    {
        XTMAGENT_RDKLogEnable = bValue;
        if (syscfg_set_commit(NULL, "X_RDKCENTRAL-COM_XTMAgent_LoggerEnable", bValue ? "1" : "0") != 0)
        {
            AnscTraceWarning(("syscfg_set failed\n"));
        }
        SendSignal(XTMAGENT_PROC_NAME);
        return TRUE;
    }    
    if (strcmp(ParamName, "X_RDKCENTRAL-COM_VLANAgent_LoggerEnable") == 0)
    {
        VLANAGENT_RDKLogEnable = bValue;
        if (syscfg_set_commit(NULL, "X_RDKCENTRAL-COM_VLANAgent_LoggerEnable", bValue ? "1" : "0") != 0)
        {
            AnscTraceWarning(("syscfg_set failed\n"));
        }
        SendSignal(VLANAGENT_PROC_NAME);
        return TRUE;
    }
    /* AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

