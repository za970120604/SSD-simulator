#include "SSD_parameter.h"
#include "FTL_structure.h"

bool Pass_GC_threshold(FTL* FTLptr){
    return FTLptr->SpareList.total_blocks_count < 3 ; 
}

DWORD GetSpareBlock_using_GC(FTL* FTLptr , FILE* fp){ 
    // printf("While loop in GetSpareBlock_using_GC\n") ; 
    bool zero_victim_flag = false ;
    while(Pass_GC_threshold(FTLptr) && !zero_victim_flag){ 
        // printf("While loop in Pass_GC_threshold\n") ; 
        // do GC
        // Get Victim block
        DWORD victim_block ;
        Pick_Victim_Block_From_LRI(FTLptr) ; 
        for(int i = 0 ; i < Vicitm_Selected_Each_Round ; i++){
            victim_block = FTLptr->GC_Victim_List[i] ;  
            if(victim_block == -1){
                if(i == 0){ // cannot reclaim any blocks in this round
                    // printf("No available GC target...\n") ; 
                    zero_victim_flag = true ; 
                }
                break ; 
            }
            // assert(FTLptr->Blocks[victim_block].GC_flag == UsedBlockState) ;
            FTL_removeList(FTLptr , &(FTLptr->Multilevel_LRI_Lists[FTLptr->Blocks[victim_block].valid_page_count]) , victim_block) ;  
        
            // copy all valid page data to GC logblock
            DWORD current_check_page = 0 ; 
            DWORD GC_logblock , GC_logblock_write_entry ;
            I64 copy_time = 0 ; 
            // DWORD cnt = 0 ; 
            // for(DWORD j = 0 ; j < FTLptr->config.number_of_Page_in_Block ; j++){
            //     if(FTLptr->Blocks[victim_block].inside_pages[j].is_valid){
            //         cnt += 1 ; 
            //     }
            // }
            // assert(cnt == FTLptr->Blocks[victim_block].valid_page_count) ; 
            while(FTLptr->Blocks[victim_block].valid_page_count != 0){
                copy_time ++ ; 
                while(FTLptr->Blocks[victim_block].inside_pages[current_check_page].is_valid == false && current_check_page < FTLptr->config.number_of_Page_in_Block){
                    current_check_page += 1 ; 
                }
                if(current_check_page == FTLptr->config.number_of_Page_in_Block){
                    assert(1 == 0) ; 
                    break ; 
                }
                
                // get current or a brand-new GC_logblock to write
                if(FTLptr->current_GCBlock == ULNull){
                    GC_logblock = FTLptr->SpareList.head_block_number; 
                    // assert(FTLptr->SpareList.head_block_number != ULNull && FTLptr->SpareList.total_blocks_count > 0 && FTLptr->Blocks[FTLptr->SpareList.head_block_number].GC_flag == OverProvisionBlockState) ; 
                    FTL_removeList(FTLptr , &FTLptr->SpareList , GC_logblock) ; 
                    FTLptr->Blocks[GC_logblock].valid_page_count = 0 ;
                    for(int i = 0 ; i < FTLptr->config.number_of_Page_in_Block ; i++){
                        FTLptr->Blocks[GC_logblock].inside_pages[i].is_valid = false ; 
                    }
                    FTLptr->current_GCBlock = GC_logblock ; 
                    FTLptr->current_GCBlockWrittenPage = 0 ; 
                }
                GC_logblock = FTLptr->current_GCBlock ; 
                GC_logblock_write_entry = FTLptr->current_GCBlockWrittenPage ; 
                // assert(FTLptr->Parent_Table[GC_logblock] == ULNull && FTLptr->Sibling_Table[GC_logblock] == ULNull) ; 
                // assert(FTLptr->Blocks[GC_logblock].GC_flag == OverProvisionBlockState) ; 

                FTLptr->Blocks[victim_block].inside_pages[current_check_page].is_valid = false ; 
                FTLptr->Blocks[victim_block].valid_page_count -= 1 ;

                // emulate write to GC_logblock
                FTLptr->Blocks[GC_logblock].inside_pages[GC_logblock_write_entry].sector = FTLptr->Blocks[victim_block].inside_pages[current_check_page].sector ;
                FTLptr->Blocks[GC_logblock].inside_pages[GC_logblock_write_entry].is_valid = true ;
                FTLptr->Blocks[GC_logblock].valid_page_count += 1 ; 

                // update L2P table 
                FTLptr->L2P_Table[FTLptr->Blocks[victim_block].inside_pages[current_check_page].sector / FTLptr->config.number_of_Sector_in_Page].corresponding_physical_page_number = GC_logblock_write_entry ; 
                FTLptr->L2P_Table[FTLptr->Blocks[victim_block].inside_pages[current_check_page].sector / FTLptr->config.number_of_Sector_in_Page].corresponding_physical_block_number = GC_logblock ;

                // update statistic
                FTLptr->stat.pageWrite ++ ; 
                
                //update next written entry in logblock 
                FTLptr->current_GCBlockWrittenPage += 1 ; 

                if(FTLptr->current_GCBlockWrittenPage == FTLptr->config.number_of_Page_in_Block){
                    // assert(GC_logblock == FTLptr->current_GCBlock) ; 
                    // assert(FTLptr->Parent_Table[GC_logblock] == ULNull && FTLptr->Sibling_Table[GC_logblock] == ULNull && FTLptr->Blocks[GC_logblock].GC_flag == OverProvisionBlockState) ; 
                    FTLptr->Blocks[GC_logblock].GC_flag = UsedBlockState; 
                    FTL_insertList(FTLptr , &FTLptr->Multilevel_LRI_Lists[FTLptr->Blocks[GC_logblock].valid_page_count] , GC_logblock) ;
                    FTLptr->current_GCBlock = ULNull ; 
                    FTLptr->current_GCBlockWrittenPage = ULNull ;
                }
            }

            // recycle the victim block
            FTLptr->Blocks[victim_block].valid_page_count = 0 ;
            FTLptr->Blocks[victim_block].GC_flag = OverProvisionBlockState ; 
            FTLptr->Blocks[victim_block].block_erase_count += 1 ; 
            FTLptr->Blocks[victim_block].hotness = 0 ; 
            FTL_insertList(FTLptr , &(FTLptr->SpareList) , victim_block) ;

            FTLptr->stat.BlockErase_count ++ ;
            if(FTLptr->stat.MaxCopy_count < (I64)copy_time){
                FTLptr->stat.MaxCopy_count = (I64)copy_time ; 
            }
        }
    }
    assert(FTLptr->SpareList.head_block_number != ULNull && FTLptr->SpareList.total_blocks_count > 0 && FTLptr->Blocks[FTLptr->SpareList.head_block_number].GC_flag == OverProvisionBlockState) ; 
    DWORD logblock = FTLptr->SpareList.head_block_number ; 
    return logblock ; 
}