#include "SSD_parameter.h"
#include "FTL_structure.h"

void FTLinit(FTL* FTLptr , FILE* config_file) {
    FTL_config* C = &(FTLptr->config) ;

    // read config file into C ...
	//	FILE	*fp;
	unsigned char	buf[101],junk[101];
	
	assert(config_file!=NULL);

	// initialize the configurations

	// blockEndurance
	fgets(buf,100, config_file);

	// PsizeByte
	fgets(buf,100, config_file);
	sscanf(buf,"%s %I64i",junk,&(C->Physical_disk_space_size_in_Byte));
	assert(strcmp(junk,"PsizeByte")==0);

	// LsizeByte
	fgets(buf,100, config_file);
	sscanf(buf,"%s %I64i",junk,&(C->Logical_disk_space_size_in_Byte));
	assert(strcmp(junk,"LsizeByte")==0);

	// blockSizeByte
	fgets(buf,100, config_file);
	sscanf(buf,"%s %lu",junk,&(C->number_of_Byte_in_Block));
	assert(strcmp(junk,"blockSizeByte")==0);

	// pageSizeByte
	fgets(buf,100, config_file);
	sscanf(buf,"%s %lu",junk,&(C->number_of_Byte_in_Page));
	assert(strcmp(junk,"pageSizeByte")==0);

	// sectorSizeByte
	fgets(buf,100, config_file);
	sscanf(buf,"%s %lu",junk,&(C->number_of_Byte_in_Sector));
	assert(strcmp(junk,"sectorSizeByte")==0);

	// maxLogBlocks
	fgets(buf,100, config_file);

	// foldingLookahead
	fgets(buf,100, config_file);

	fclose(config_file);
	
    // assert that the input config is reasonable
	printf("Physical_disk_space_size_in_Byte , %I64i \n" , C->Physical_disk_space_size_in_Byte) ; 
	printf("Logical_disk_space_size_in_Byte , %I64i \n" , C->Logical_disk_space_size_in_Byte) ;
	printf("number_of_Byte_in_Block , %lu \n" , C->number_of_Byte_in_Block) ;
	printf("number_of_Byte_in_Page , %lu \n" , C->number_of_Byte_in_Page) ;
	printf("number_of_Byte_in_Sector , %lu \n" , C->number_of_Byte_in_Sector) ;

    assert(C->Logical_disk_space_size_in_Byte % C->number_of_Byte_in_Block == 0) ; 
	assert(C->Physical_disk_space_size_in_Byte % C->number_of_Byte_in_Block == 0) ; 
	assert(C->number_of_Byte_in_Block % C->number_of_Byte_in_Page == 0) ;
	assert(C->number_of_Byte_in_Page % C->number_of_Byte_in_Sector == 0) ;
	assert(C->Logical_disk_space_size_in_Byte < C->Physical_disk_space_size_in_Byte) ; //  for over-provisioning

	C->number_of_Sector_in_Page	= C->number_of_Byte_in_Page / C->number_of_Byte_in_Sector ; 
	C->number_of_Page_in_Block = C->number_of_Byte_in_Block / C->number_of_Byte_in_Page ; 
	C->number_of_Sector_in_Block = C->number_of_Byte_in_Block / C->number_of_Byte_in_Sector ;
    
    C->number_of_Page_in_Physical_disk_space =  (DWORD)(C->Physical_disk_space_size_in_Byte / (I64)C->number_of_Byte_in_Page) ; 
	C->number_of_Block_in_Physical_disk_space = (DWORD)(C->Physical_disk_space_size_in_Byte / (I64)C->number_of_Byte_in_Block) ; 
	 
	C->number_of_Sector_in_Logical_disk_space = (DWORD)(C->Logical_disk_space_size_in_Byte / (I64)C->number_of_Byte_in_Sector) ; 
	C->number_of_Page_in_Logical_disk_space = (DWORD)(C->Logical_disk_space_size_in_Byte / (I64)C->number_of_Byte_in_Page) ; 
	C->number_of_Block_in_Logical_disk_space = (DWORD)(C->Logical_disk_space_size_in_Byte / (I64)C->number_of_Byte_in_Block) ; 

    // FTL Space Allocation
    FTLptr->Blocks = (FTL_Block* )calloc(C->number_of_Block_in_Physical_disk_space , sizeof(FTL_Block)) ; 
	FTLptr->Blocks[0].inside_pages = (FTL_Page* )calloc(C->number_of_Page_in_Physical_disk_space , sizeof(FTL_Page)) ; 

    DWORD i , j ; 
	for(i = 0 ; i < C->number_of_Block_in_Physical_disk_space ; i++){
	    FTLptr->Blocks[i].inside_pages = FTLptr->Blocks[0].inside_pages + i * C->number_of_Page_in_Block ;
        // #Q
		// FTLptr->Blocks[i].valid_page_count = 0 ;
		// for(j = 0 ; j < C->number_of_Page_in_Block ; j++){
		// 	FTLptr->Blocks[i].inside_pages[j].corresponding_sector_number = -1 ;
		// 	FTLptr->Blocks[i].inside_pages[j].is_valid = false ;
		// }
	}

    // List Allocation
	FTLptr->unRefList.name = 'U' ; 
	FTLptr->unRefList.level = -1 ; 
    FTLptr->unRefList.head_block_number = FTLptr->unRefList.tail_block_number = ULNull ; 
	FTLptr->unRefList.total_blocks_count = 0;

	FTLptr->SpareList.name = 'S' ; 
	FTLptr->SpareList.level = -1 ;
	FTLptr->SpareList.head_block_number	= FTLptr->SpareList.tail_block_number = ULNull ; 
	FTLptr->SpareList.total_blocks_count = 0;	

	FTLptr->Multilevel_LRI_Lists = (FTL_List* )calloc(C->number_of_Page_in_Block + 1 , sizeof(FTL_List)) ; 

    // Table Allocation
	FTLptr->L2P_Table = (FTL_Table_Element* )calloc(C->number_of_Page_in_Logical_disk_space , sizeof(FTL_Table_Element)) ;				
	FTLptr->Parent_Table = (DWORD* )calloc(C->number_of_Block_in_Physical_disk_space , sizeof(DWORD)) ;		
	FTLptr->Sibling_Table = (DWORD* )calloc(C->number_of_Block_in_Physical_disk_space , sizeof(DWORD)) ;	
	FTLptr->GC_Victim_List = (DWORD* )calloc(Vicitm_Selected_Each_Round , sizeof(DWORD)) ;

	// Lists Filling and Initialization
	for(i = 0 ; i <= FTLptr->config.number_of_Page_in_Block ; i++){
		FTLptr->Multilevel_LRI_Lists[i].name = 'M' ; 
		FTLptr->Multilevel_LRI_Lists[i].level = i ;
		FTLptr->Multilevel_LRI_Lists[i].total_blocks_count = 0 ;
		FTLptr->Multilevel_LRI_Lists[i].head_block_number = FTLptr->Multilevel_LRI_Lists[i].tail_block_number = ULNull ;
	}

    // Table Filling and Initialization
    for(i = 0 ; i < C->number_of_Block_in_Logical_disk_space ; i++){


        for(j = 0 ; j < C->number_of_Page_in_Block ; j++){
			FTLptr->Blocks[i].inside_pages[j].sector = i * C->number_of_Sector_in_Block + j * C->number_of_Sector_in_Page ; 
			FTLptr->Blocks[i].inside_pages[j].is_valid = true ; 	 
			FTLptr->L2P_Table[i * C->number_of_Page_in_Block + j].corresponding_physical_block_number = i ; 
			FTLptr->L2P_Table[i * C->number_of_Page_in_Block + j].corresponding_physical_page_number = j ; 
		}

		// #Q
		FTLptr->Parent_Table[i] = FTLptr->Sibling_Table[i] = ULNull ; 
		FTL_insertList(FTLptr , &FTLptr->unRefList , i) ; 

		FTLptr->Blocks[i].valid_page_count = FTLptr->config.number_of_Page_in_Block ;
        FTLptr->Blocks[i].GC_flag = FreeBlockState ; 
        FTLptr->Blocks[i].block_erase_count = 0 ; 
		FTLptr->Blocks[i].hotness = 0 ;
    }

	for(; i < C->number_of_Block_in_Physical_disk_space ; i++){

		for(j = 0 ; j < C->number_of_Page_in_Block ; j++){
			FTLptr->Blocks[i].inside_pages[j].sector = -1 ; // initiailization
			FTLptr->Blocks[i].inside_pages[j].is_valid = false ; // initialization
		}

		// FTL_insertList(FTLptr , &FTLptr->SpareList , i) ; 

		// #Q
		if(i==C->number_of_Block_in_Logical_disk_space)
		{
			FTLptr->Sibling_Table[i] = ULNull; // LsizeBlock+1 -> LsizeBlock -> nullptr
			FTLptr->Parent_Table[i] = i+1;
			FTLptr->SpareList.tail_block_number = i;
		}
		else if(i==C->number_of_Block_in_Physical_disk_space-1)
		{
			FTLptr->Sibling_Table[i] = i-1; // nullptr -> PsizeBlock-1 -> PsizeBlock-2
			FTLptr->Parent_Table[i] = ULNull;
			FTLptr->SpareList.head_block_number = i;
		}
		else
		{
			FTLptr->Sibling_Table[i] = i-1; // (#block bigger)nullptr -> PsizeBlock-1 -> PsizeBlock-2 -> .... -> i+1 -> i -> i-1 -> .... -> LsizeBlock+1 -> LsizeBlock -> nullptr(#block smaller)
			FTLptr->Parent_Table[i] = i+1;
		}
		FTLptr->SpareList.total_blocks_count += 1 ;

		FTLptr->Blocks[i].valid_page_count = 0 ;
		FTLptr->Blocks[i].GC_flag = OverProvisionBlockState ; 
		FTLptr->Blocks[i].block_erase_count = 0 ;
		FTLptr->Blocks[i].hotness = 0 ;
	}
	printf("LsizeBlock:%d\tPsizeBlock:%d\tSpareBlockCount:%d\n",C->number_of_Block_in_Logical_disk_space,C->number_of_Block_in_Physical_disk_space,FTLptr->SpareList.total_blocks_count);

	FTLptr->current_LogBlock = ULNull ; 
	FTLptr->current_LogBlockWrittenPage = ULNull ; 
	FTLptr->current_GCBlock = ULNull ; 
	FTLptr->current_GCBlockWrittenPage = ULNull ; 

	FTLptr->stat.pageWrite = 0 ; 
	FTLptr->stat.pageRead = 0 ;
	FTLptr->stat.userPageWrite = 0 ;
	FTLptr->stat.userPageRead = 0 ;
	FTLptr->stat.BlockErase_count = 0;
	FTLptr->stat.MaxCopy_count = 0 ; 
	FTLptr->systemTime = SystemStartTime ; 
	FTLptr->stat.choose_proposed_victim = 0 ; 
	FTLptr->stat.choose_greedy_victim = 0 ; 
}
void FTLfree(FTL* FTLptr) {
	free(FTLptr->Blocks[0].inside_pages);
	free(FTLptr->Blocks);
	free(FTLptr->Multilevel_LRI_Lists);	
	free(FTLptr->L2P_Table) ; 			
	free(FTLptr->Parent_Table) ; 	
	free(FTLptr->Sibling_Table) ;
}