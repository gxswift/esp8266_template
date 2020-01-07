#include <string.h>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_wifi.h>
#include <nvs_flash.h>
#include <nvs.h>

#include <task.h>
#include <stdlib.h>



#include "lwip/apps/httpd.h"


#include "rtc_memory.h"
#define NUM_CONFIG_CGI_URIS	1
const char *Set_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
static const tCGI ppcURLs[]=
{
	{"/set.cgi",Set_CGI_Handler}
};


void httpd_cgi_init(void)
{
  http_set_cgi_handlers(ppcURLs, NUM_CONFIG_CGI_URIS);
}

static int FindCGIParameter(const char *pcToFind,char *pcParam[],int iNumParams)
{
	int iLoop;
	for(iLoop = 0;iLoop < iNumParams;iLoop ++ )
	{
		if(strcmp(pcToFind,pcParam[iLoop]) == 0)
		{
			return (iLoop);
		}
	}
	return (-1);
}
const char *Set_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
	uint8_t i=0;
	iIndex =FindCGIParameter("ssid",pcParam,iNumParams);
	if(iIndex != -1)
	{
	   for(i = 0;i < iNumParams;i++)
	   {
			if(strcmp(pcParam[i],"ssid")== 0)  //??CGI??
			{
				printf("ssid = %s\t",pcValue[i]);
			  if(strcmp(pcValue[i],"admin")== 0)//?????
			  {

			  }
			}
			else if(strcmp(pcParam[i],"password") == 0)  //??CGI??
			{
				printf("password = %s\r\n",pcValue[i]);
			  if(strcmp(pcValue[i],"admin")== 0)//????
			  {

			  }
			  rtc_mem_write(0,0xEFEFEFEF);
			  esp_restart();//restart
			}
	   }
	}
	return "/alert.html";
}