#include "SSD_parameter.h"
#include "FTL_structure.h"
void FTL_Write_A_Page(FTL* FTLptr , DWORD aligned_offset_sector , FILE* fp){
    DWORD LPA , to_invalidate_block , to_invalidate_page , logblock , logblock_write_entry ; 
    LPA = aligned_offset_sector / FTLptr->config.number_of_Sector_in_Page ; 

    if(FTLptr->current_LogBlock == ULNull){
        logblock = GetSpareBlock_using_GC(FTLptr , fp) ; 
        FTL_removeList(FTLptr , &FTLptr->SpareList , logblock) ;  
        FTLptr->Blocks[logblock].valid_page_count = 0 ;
        for(int i = 0 ; i < FTLptr->config.number_of_Page_in_Block ; i++){
            FTLptr->Blocks[logblock].inside_pages[i].is_valid = false ; 
        }
        FTLptr->current_LogBlock = logblock ; 
        FTLptr->current_LogBlockWrittenPage = 0 ; 
    }    
    logblock = FTLptr->current_LogBlock ; 
    logblock_write_entry = FTLptr->current_LogBlockWrittenPage ; 
    // assert(FTLptr->Parent_Table[logblock] == ULNull && FTLptr->Sibling_Table[logblock] == ULNull) ;
    // assert(FTLptr->Blocks[logblock].GC_flag == OverProvisionBlockState) ; 

    to_invalidate_block = FTLptr->L2P_Table[LPA].corresponding_physical_block_number ; 
    to_invalidate_page = FTLptr->L2P_Table[LPA].corresponding_physical_page_number ; 

    if(FTLptr->Blocks[to_invalidate_block].GC_flag == UsedBlockState){ // the page we update is in block that are in LRI
        FTL_removeList(FTLptr , &(FTLptr->Multilevel_LRI_Lists[FTLptr->Blocks[to_invalidate_block].valid_page_count]) , to_invalidate_block) ;
        FTLptr->Blocks[to_invalidate_block].valid_page_count-- ;
        Assign_Hotness_WhenWrite(FTLptr, to_invalidate_block) ; 
        FTL_insertList(FTLptr , &(FTLptr->Multilevel_LRI_Lists[FTLptr->Blocks[to_invalidate_block].valid_page_count]) , to_invalidate_block) ;
    }
    else if(FTLptr->Blocks[to_invalidate_block].GC_flag == FreeBlockState){ // the page we update is in first use (empty, haven't use anytime since system starts) block
        FTL_removeList(FTLptr , &(FTLptr->unRefList) , to_invalidate_block) ;  
        // #Check how to initialize the valid page count  in blocks in freeblock state 
        // #Q : invalidate and put in Multilevel LRI??? If so , the newst used block will be the least priority to be picked as victim block for GC , since it has many valid pages . 
        FTLptr->Blocks[to_invalidate_block].valid_page_count -- ; 
        FTLptr->Blocks[to_invalidate_block].GC_flag = UsedBlockState ; 
        Assign_Hotness_WhenWrite(FTLptr, to_invalidate_block) ; 
        FTL_insertList(FTLptr , &(FTLptr->Multilevel_LRI_Lists[FTLptr->Blocks[to_invalidate_block].valid_page_count]) , to_invalidate_block) ; 
    }
    else if(FTLptr->Blocks[to_invalidate_block].GC_flag == OverProvisionBlockState){ // the page we update is just in the logblock or in the GC logblock
        if(!((to_invalidate_block == FTLptr->current_GCBlock) || (to_invalidate_block == FTLptr->current_LogBlock))){
            printf("%llu , %llu , %llu\n" , to_invalidate_block , FTLptr->current_GCBlock , FTLptr->current_LogBlock) ; 
            if(FTLptr->Parent_Table[to_invalidate_block] != ULNull){
                fprintf(fp , "Parent : %llu\n" , FTLptr->Blocks[FTLptr->Parent_Table[to_invalidate_block]].GC_flag) ; 
            }
            if(FTLptr->Sibling_Table[to_invalidate_block] != ULNull){
                fprintf(fp , "Sib : %llu\n" , FTLptr->Blocks[FTLptr->Sibling_Table[to_invalidate_block]].GC_flag) ; 
            }
        }
        assert((to_invalidate_block == FTLptr->current_GCBlock) || (to_invalidate_block == FTLptr->current_LogBlock)) ; 
        FTLptr->Blocks[to_invalidate_block].valid_page_count-- ;
        Assign_Hotness_WhenWrite(FTLptr, to_invalidate_block) ; 
    }
    FTLptr->Blocks[to_invalidate_block].inside_pages[to_invalidate_page].is_valid = false ; 

    if(FTLptr->Blocks[to_invalidate_block].valid_page_count == 0 && FTLptr->Blocks[to_invalidate_block].GC_flag == UsedBlockState && to_invalidate_block != FTLptr->current_GCBlock && to_invalidate_block != FTLptr->current_LogBlock){	
        FTL_removeList(FTLptr , &(FTLptr->Multilevel_LRI_Lists[FTLptr->Blocks[to_invalidate_block].valid_page_count]) , to_invalidate_block) ;
        FTLptr->Blocks[to_invalidate_block].GC_flag = OverProvisionBlockState ;  
        FTL_insertList(FTLptr , &(FTLptr->SpareList) , to_invalidate_block) ; 
        FTLptr->Blocks[to_invalidate_block].block_erase_count++ ;
        FTLptr->Blocks[to_invalidate_block].hotness = 0 ;
        FTLptr->stat.BlockErase_count ++ ;	 
	}

    // then , handle logblock
    FTLptr->Blocks[logblock].inside_pages[logblock_write_entry].sector = aligned_offset_sector ; 
	FTLptr->Blocks[logblock].inside_pages[logblock_write_entry].is_valid = true ;  
    FTLptr->Blocks[logblock].valid_page_count ++ ;

    // update L2P table
    FTLptr->L2P_Table[LPA].corresponding_physical_block_number = logblock ; 
    FTLptr->L2P_Table[LPA].corresponding_physical_page_number = logblock_write_entry ; 

    // update statistic
    FTLptr->stat.pageWrite ++ ; 
    FTLptr->stat.userPageWrite ++ ;  

    //update next written entry in logblock 
    FTLptr->current_LogBlockWrittenPage ++ ; // because we pretend to write a page
    logblock_write_entry ++ ; 

    if(FTLptr->current_LogBlockWrittenPage == FTLptr->config.number_of_Page_in_Block){
        if(!(logblock == FTLptr->current_LogBlock && logblock_write_entry == FTLptr->current_LogBlockWrittenPage)){
            printf("%llu , %llu\n" , logblock , FTLptr->current_LogBlock) ; 
        }
        assert(logblock == FTLptr->current_LogBlock && logblock_write_entry == FTLptr->current_LogBlockWrittenPage) ; 
        FTLptr->Blocks[logblock].GC_flag = UsedBlockState; 
        FTL_insertList(FTLptr , &FTLptr->Multilevel_LRI_Lists[FTLptr->Blocks[logblock].valid_page_count] , logblock) ;
        FTLptr->current_LogBlock = ULNull ; 
        FTLptr->current_LogBlockWrittenPage = ULNull ; 
    } 
}