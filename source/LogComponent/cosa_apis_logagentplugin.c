
#include "ansc_platform.h"
#include "cosa_apis_logagentplugin.h"
#include "ccsp_trace.h"

#define TR069_PROC_NAME "CcspTr069PaSsp"
#define MTA_PROC_NAME "CcspMtaAgentSsp"
#define CM_PROC_NAME "CcspCMAgentSsp"
#define PSM_PROC_NAME "PsmSsp"
#define PAM_PROC_NAME "CcspPandMSsp"
#define WIFI_PROC_NAME "wifilog_agent"

/* structure defined for object "PluginSampleObj"  */
typedef  struct
_COSA_PLUGIN_SAMPLE_INFO
{
    ULONG                           loglevel; 
    char                            WifiLogMsg[256];   
   
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
	char Buf[50] = {0};
    char *ptr = Buf;
	char cmd[50] = {0};
	char cmd2[50] = {0};
	sprintf(cmd,"%s %s","pidof ",proc);
    if((f = popen(cmd, "r")) == NULL) {
        printf("popen %s error\n", cmd);
        return -1;
    }

    while(!feof(f))
    {
        *ptr = 0;
        fgets(ptr,120,f);
        if(strlen(ptr) == 0)
        {
            break;
        }
        ptr += strlen(ptr);
    }
    pclose(f);
	sprintf(cmd2,"%s %s","kill -14 ",Buf);
    if((f = popen(cmd2, "r")) == NULL) {
        printf("popen %s error\n", cmd);
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

if( AnscEqualString(ParamName, "loglevel", TRUE))
{
        *puLong =  &i;
   
}
if (AnscEqualString(ParamName, "X_RDKCENTRAL-COM_LogLevel", TRUE))
{
		*puLong  = RDKLogLevel;
        return TRUE;
}
if( AnscEqualString(ParamName, "X_RDKCENTRAL-COM_TR69_LogLevel", TRUE))
{
	printf("Inside LogAgent_GetParamUlongValue TR069LogLevel\n");
    *puLong =  TR69_RDKLogLevel;
   
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

	if( AnscEqualString(ParamName, "loglevel", TRUE))
	{
		printf(" LogAgent_SetParamValues : loglevel \n");
	  
	} 
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
	char LogMsg_arr[300] = {0};
	char *LogMsg = LogMsg_arr;
	char LogLevel[100] = {0};
	int level = 0;
	char WiFiLogeComponent[100] = "com.cisco.spvtg.ccsp.logagent";
	
    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "WifiLogMsg", TRUE))
    {
     
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

    AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName));
    return FALSE;
}


BOOL
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

        AnscCopyString(pValue, str);
        return TRUE;
    }

    AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName));
    return FALSE;
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
    /* AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

