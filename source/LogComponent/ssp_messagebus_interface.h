/*********************************************************************************

    description:

        This is the template file of ssp_messagebus_interface.h for XxxxSsp.
        You don't need to do anything here.

    ------------------------------------------------------------------------------

    revision:

        09/08/2011    initial revision.

**********************************************************************************/

#ifndef  _SSP_MESSAGEBUS_INTERFACE_
#define  _SSP_MESSAGEBUS_INTERFACE_

ANSC_STATUS
ssp_Mbi_MessageBusEngage
    (
        char * component_id,
        char * config_file,
        char * path
    );

int
ssp_Mbi_Initialize
    (
        void * user_data
    );

int
ssp_Mbi_Finalize
    (
        void * user_data
    );

int
ssp_Mbi_Buscheck
    (
        void * user_data
    );

int
ssp_Mbi_FreeResources
    (
        int priority,
        void * user_data
    );

#endif
