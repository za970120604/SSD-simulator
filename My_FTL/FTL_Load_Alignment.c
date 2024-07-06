#include "SSD_parameter.h"
#include "FTL_structure.h"

void FTL_Load_Alignment(FTL* FTLptr , DWORD* offset_sector , DWORD* request_length){
    *offset_sector = *offset_sector / FTLptr->config.number_of_Sector_in_Page ; 
	if((*request_length) % FTLptr->config.number_of_Sector_in_Page != 0){
        *request_length = 1 + *request_length / FTLptr->config.number_of_Sector_in_Page ; 
    }
	else{
        *request_length = *request_length / FTLptr->config.number_of_Sector_in_Page ; 
    }

	*offset_sector	*= (FTLptr->config.number_of_Sector_in_Page) ;
	*request_length	*= (FTLptr->config.number_of_Sector_in_Page) ;
}