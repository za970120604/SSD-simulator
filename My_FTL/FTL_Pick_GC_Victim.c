#include "SSD_parameter.h"
#include "FTL_structure.h"

// DWORD Pick_Victim_Block_From_LRI(FTL* FTLptr){
//     DWORD victim_level = FTLptr->config.number_of_Page_in_Block ; 
//     for(DWORD i = 0 ; i <= FTLptr->config.number_of_Page_in_Block ; i++){
//         if(FTLptr->Multilevel_LRI_Lists[i].total_blocks_count != 0){
//             victim_level = i ; 
//             break ; 
//         }
//     }
//     FTLptr->stat.choose_greedy_victim += 1 ;
//     return FTLptr->Multilevel_LRI_Lists[victim_level].head_block_number ; 
// }

void Pick_Victim_Block_From_LRI(FTL* FTLptr){
    int counter = 0 ;  
    for(int i = 0 ; i < Vicitm_Selected_Each_Round ; i++){ // clear the GC Victim List
        FTLptr->GC_Victim_List[i] = -1 ; 
    }
    while(counter < Vicitm_Selected_Each_Round){
        bool flag = false ; 
        // printf("While loop in Pick_Victim_Block_From_LRI\n") ; 
        DWORD greedy_victim = ULNull ; 
        for(DWORD i = 0 ; i <= FTLptr->config.number_of_Page_in_Block && !flag ; i++){ // for every level in LRI
            if(FTLptr->Multilevel_LRI_Lists[i].total_blocks_count != 0){ // ensure that there is blocks in this level of LRI
                DWORD current_block = FTLptr->Multilevel_LRI_Lists[i].head_block_number ; // iterate from the first block in this level 
                for(DWORD j = 0 ; j < FTLptr->Multilevel_LRI_Lists[i].total_blocks_count ; j++){ // iterate at most the number of blocks in this level times
                    assert(FTLptr->Blocks[current_block].GC_flag == UsedBlockState && FTLptr->Blocks[current_block].valid_page_count == i) ; 
                    // if(current_block == 563){
                    //     printf("%d, %d, %d, %d, %d\n", FTLptr->Blocks[current_block].hotness, Check_Picked_Victim(FTLptr, current_block, counter), FTLptr->Blocks[current_block].GC_flag, FTLptr->Blocks[current_block].valid_page_count, FTLptr->Multilevel_LRI_Lists[FTLptr->Blocks[current_block].valid_page_count].total_blocks_count) ;
                    // }
                    if(Check_Picked_Victim(FTLptr, current_block, counter) == false){ // if current block has not been picked as victim block
                        greedy_victim = current_block ; 
                        // printf("Greedy Victim: %d\n", greedy_victim);
                        flag = true ; 
                        break ; 
                    }
                    current_block = FTLptr->Sibling_Table[current_block] ; 
                }
            }
        }

        DWORD proposed_victim = FTLptr->config.number_of_Block_in_Physical_disk_space ; 
        // flag = false ;
        // //DWORD victim_search_space = FTLptr->config.number_of_Page_in_Block ; 
        // DWORD victim_search_space = Number_Of_Max_Valid_Pages_In_A_Victim_Block ;  
        // for(DWORD i = 0 ; i <= victim_search_space && !flag ; i++){ // limit the search space of LRI
        //     if(FTLptr->Multilevel_LRI_Lists[i].total_blocks_count != 0){ // ensure that there is blocks in this level of LRI
        //         DWORD current_block = FTLptr->Multilevel_LRI_Lists[i].head_block_number ; // iterate from the first block in this level
        //         for(DWORD j = 0 ; j < FTLptr->Multilevel_LRI_Lists[i].total_blocks_count ; j++){
        //             assert(FTLptr->Blocks[current_block].GC_flag == UsedBlockState && FTLptr->Blocks[current_block].valid_page_count == i) ; 
        //             if(FTLptr->Blocks[current_block].hotness == 0 && Check_Picked_Victim(FTLptr, current_block, counter) == false){
        //                 proposed_victim = current_block ; 
        //                 flag = true ; 
        //                 break ; 
        //             }
        //             current_block = FTLptr->Sibling_Table[current_block] ; 
        //         }
        //     }
        // }

        if(greedy_victim != ULNull && proposed_victim != FTLptr->config.number_of_Block_in_Physical_disk_space){ // both policies select a valid victim
            FTLptr->stat.choose_proposed_victim += 1 ; 
            FTLptr->GC_Victim_List[counter++] = proposed_victim ; 
        }
        else if(greedy_victim != ULNull && proposed_victim == FTLptr->config.number_of_Block_in_Physical_disk_space){ // only greedy policy select a valid victim
            FTLptr->stat.choose_greedy_victim += 1 ; 
            FTLptr->GC_Victim_List[counter++] = greedy_victim ; 
        }
        else if(greedy_victim == ULNull && proposed_victim == FTLptr->config.number_of_Block_in_Physical_disk_space){ // neither of these policies select a valid victim
            // printf("Counter: %d\n", counter) ; 
            // printf("Spareblocks Count: %d\n", FTLptr->SpareList.total_blocks_count) ; 
            // for(int i = 0 ; i < Vicitm_Selected_Each_Round ;i++){
            //     printf("%d ", FTLptr->GC_Victim_List[i]) ; 
            // }
            // printf("\n") ; 
            break ; 
        }
        else{
            printf("Impossible State!\n") ; 
            printf("greedy_victim = %d; proposed_victim = %d\n", greedy_victim, proposed_victim) ; 
            printf("%d, %d, %d, %d, %d\n", FTLptr->Blocks[proposed_victim].hotness, Check_Picked_Victim(FTLptr, proposed_victim, counter), FTLptr->Blocks[proposed_victim].GC_flag, FTLptr->Blocks[proposed_victim].valid_page_count, FTLptr->Multilevel_LRI_Lists[FTLptr->Blocks[proposed_victim].valid_page_count].total_blocks_count) ; 
            // impossible, since Greedy will traverse through all blocks in LRI and it has more flexible condition than proposed GC policy
        }
    }
}

bool Check_Picked_Victim(FTL* FTLptr, DWORD check_block_number, int counter){
    for(int i = 0 ; i < counter ; i++){
        if(FTLptr->GC_Victim_List[i] == check_block_number){
            return true ; 
        }
    }
    return false; 
}