#include "SSD_parameter.h"
#include "FTL_structure.h"

FTL My_FTL ; 
I64	totalReq , writeReq ;

int main(){
    FILE	*traceFile,*ResultFile,*configFile;
	char	lineBuffer[1024];
	char	op[100],time[100],ampm[100];
	char	ResultFileName[500], InitFileName[100], ConfigFileName[100];
	//////////////////////////////////////////////////////// 
	char	*config = "mss_original_GC_ovp=1_cycle=400_th=200.txt";
	char	*config_ = "mss_original_GC_ovp=1_cycle=400_th=200" ; 
	sprintf(ConfigFileName,"%s",config);
	sprintf(InitFileName, "..\\Config File\\%s",ConfigFileName);	
	char	*subdir = Config_Adjusting(InitFileName , config_ , 0 , 0.1 , 100 , true) ;
	//////////////////////////////////////////////////////// 
	DIR *dir;
    struct dirent *entry;
    dir = opendir(subdir);
    while ((entry = readdir(dir)) != NULL) {
		if(strcmp(entry->d_name , ".") == 0 || strcmp(entry->d_name , "..") == 0 ){
			continue ; 
		}
		DWORD	sector_nr,len,i,r,m,offsetSector,lenSector,j,n,k,listPtr;

		m = 0;

		writeReq = 0;					

		// config file setting
		sprintf(ConfigFileName,"%s",entry->d_name);
		sprintf(InitFileName, "..\\Config File\\%s\\%s",config_ , ConfigFileName);			
		printf("Init My FTL flash(%s)...\n",InitFileName);

		configFile = fopen(InitFileName,"rt");
		FTLinit(&My_FTL , configFile);
		printf("Init done.\n");
		sprintf(ResultFileName,"..\\Config File\\%s\\My_FTL_%s",config_ , ConfigFileName);
		ResultFile = fopen(ResultFileName, "w+t");
		//=======Over Provisioning=========================================================================================================
		printf("Overprovisiong = %f %%\n" , (double)100 * (double)(My_FTL.config.number_of_Block_in_Physical_disk_space - My_FTL.config.number_of_Block_in_Logical_disk_space)/My_FTL.config.number_of_Block_in_Physical_disk_space) ; 
		fprintf(ResultFile,"Overprovisiong = %f %%\n" , (double)100 * (double)(My_FTL.config.number_of_Block_in_Physical_disk_space - My_FTL.config.number_of_Block_in_Logical_disk_space)/My_FTL.config.number_of_Block_in_Physical_disk_space);
		fprintf(ResultFile,"================================================================\n") ; 
		//=================================================================================================================================
		
		for(DWORD LUNnum = 0 ; LUNnum <= 6 ; LUNnum++){
			if(LUNnum == 5){
				continue ; 
			}
			char list_path[400] ; 
			sprintf(list_path , "..\\segSB_tracefile\\rawfile952G_simple_LUN%llu.txt" , LUNnum) ;  
			printf(">>> Reading File from path : %s\n" , list_path) ; 
			traceFile = fopen(list_path ,"rt") ;
			fseek(traceFile,0,SEEK_SET);
			FILE * fp;
			for(r=1;r<=1;r++){
				while(!feof(traceFile))
				{ 
					sector_nr = 0;
					len = 0;
					fgets(lineBuffer,1024,traceFile);
					// printf("The line you entered: %s\n", lineBuffer);

					// for IO meter
					// 0 W 3156463 1.0
					sscanf(lineBuffer,"%d %s %d %d",	&i,op,&sector_nr,&len);
					// printf("%s %d %s %d %d\n" , ampm,i,op,sector_nr,len) ;

					if(sector_nr+len >= My_FTL.config.number_of_Sector_in_Logical_disk_space || len == 0)continue;	// out of range
					
					if(op[0] == 'R')totalReq++; // read 

					if(op[0] == 'W'){
						if(writeReq % 100000 == 0)printf("write request = %d\n",writeReq);
						offsetSector = sector_nr;
						lenSector = len;
						FTL_Load_Alignment(&My_FTL ,&offsetSector , &lenSector) ;
						while(1){					
							FTL_Write_A_Page(&My_FTL , offsetSector , fp);
							offsetSector += My_FTL.config.number_of_Sector_in_Page;
							lenSector -= My_FTL.config.number_of_Sector_in_Page;
							if(lenSector <= 0)break;
						}
						if(init_counter < System_hotness_cycle){ // initialization step
							init_counter ++ ; 
						}
						else{ // common step
							user_write_counter -- ; 
							if(user_write_counter < System_hotness_cycle/2){
								Change_Hotness_Overtime(&My_FTL) ; 
								if(user_write_counter == 0){
									user_write_counter = System_hotness_cycle ; 
								}
							}
						}
						totalReq++ ;
						writeReq++ ;
						My_FTL.systemTime++ ; 
						// Write_info(&My_FTL , 400000 , 800000) ; 
					}
					// Check_Blocks(&My_FTL) ; 
				}
				fseek(traceFile,0,SEEK_SET);
			}
			fclose(traceFile);
			printf("LUN-%llu done\n" , LUNnum);
			//=======user page write===========================================================================================================
			printf("user page write = %I64i\n",My_FTL.stat.userPageWrite);
			fprintf(ResultFile,"user page write = %I64i\n",My_FTL.stat.userPageWrite);
			//=================================================================================================================================

			//=======total page write==========================================================================================================
			printf("total page write = %I64i\n",My_FTL.stat.pageWrite);
			fprintf(ResultFile,"total page write = %I64i\n",My_FTL.stat.pageWrite);
			//=================================================================================================================================

			//=======Write Amplification Factor=======================================================================================================================
			printf("WAF = %f\n", (double)My_FTL.stat.pageWrite / My_FTL.stat.userPageWrite);
			fprintf(ResultFile , "WAF = %f\n", (double)My_FTL.stat.pageWrite / My_FTL.stat.userPageWrite);
			//=================================================================================================================================

			//=======Erase Count===============================================================================================================
			printf("totalEC = %I64i\n",My_FTL.stat.BlockErase_count);
			fprintf(ResultFile,"totalEC = %I64i\n",My_FTL.stat.BlockErase_count);
			//=================================================================================================================================

			//=======Max Copy==================================================================================================================
			printf("Max page copy = %f\n",(double)(My_FTL.stat.MaxCopy_count));
			fprintf(ResultFile,"Max page copy = %f\n",(double)(My_FTL.stat.MaxCopy_count));
			//=================================================================================================================================

			//=======Avg Copy==================================================================================================================
			printf("AVG page copy = %f\n",(double)(My_FTL.stat.pageWrite-My_FTL.stat.userPageWrite) / My_FTL.stat.BlockErase_count);
			fprintf(ResultFile,"AVG page copy = %f\n",(double)(My_FTL.stat.pageWrite-My_FTL.stat.userPageWrite)/My_FTL.stat.BlockErase_count);
			//=================================================================================================================================

			//=======Victim Selection==================================================================================================================
			printf("Victim selected from Propsed Policy = %I64i\n", My_FTL.stat.choose_proposed_victim);
			printf("Victim selected from Greedy Policy = %I64i\n", My_FTL.stat.choose_greedy_victim);
			fprintf(ResultFile,"Victim selected from Propsed Policy = %I64i\n", My_FTL.stat.choose_proposed_victim);
			fprintf(ResultFile,"Victim selected from Greedy Policy = %I64i\n", My_FTL.stat.choose_greedy_victim);	
			fprintf(ResultFile,"================================================================\n") ; 
			//=================================================================================================================================
		}
		printf("simulation done\n");
		fclose(ResultFile);
		FTLfree(&My_FTL);
    }
    closedir(dir);
	system("pause");
}
