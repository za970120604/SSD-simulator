#include "SSD_parameter.h"
#include "FTL_structure.h"

void FTL_insertList(FTL* FTLptr , FTL_List* list , DWORD target_block){
    if(list->head_block_number == ULNull && list->tail_block_number == ULNull){
        list->head_block_number = list->tail_block_number = target_block ; 
        FTLptr->Parent_Table[target_block] = FTLptr->Sibling_Table[target_block] = ULNull ; 
    }
    else{
        DWORD oldtail = list->tail_block_number ; 
        list->tail_block_number = target_block ; 
        FTLptr->Parent_Table[target_block] = oldtail ; 
        FTLptr->Sibling_Table[target_block] = ULNull ; 
        // assert(FTLptr->Sibling_Table[oldtail] = ULNull) ; 
        FTLptr->Sibling_Table[oldtail] = target_block ; 
    }
    list->total_blocks_count++ ;
    // assert(FTLptr->Parent_Table[target_block] != target_block && FTLptr->Sibling_Table[target_block] != target_block) ;
}

void FTL_removeList(FTL* FTLptr , FTL_List* list , DWORD target_block){
    // if(list->name == 'U'){ // in 'U'nreferenced list
    //     assert(FTLptr->Blocks[target_block].GC_flag == FreeBlockState) ; 
    // }
    // else if(list->name == 'S'){ // in 'S'pareList
    //     assert(FTLptr->Blocks[target_block].GC_flag == OverProvisionBlockState) ; 
    // }
    // else{ // in 'M'ultilevel LRI List
    //     assert(FTLptr->Blocks[target_block].GC_flag == UsedBlockState) ; 
    //     assert(list->level == FTLptr->Blocks[target_block].valid_page_count) ; 
    //     DWORD cnt = 0 ; 
    //     for(DWORD j = 0 ; j < FTLptr->config.number_of_Page_in_Block ; j++){
    //         if(FTLptr->Blocks[target_block].inside_pages[j].is_valid){
    //             cnt += 1  ; 
    //         }
    //     }
    //     if(cnt != list->level){
    //         printf("list info : %lu , %lu , %lu , %lu\n" , list->head_block_number , list->tail_block_number , list->total_blocks_count , list->level) ; 
    //         printf("wrong info : %lu\n" , cnt) ; 
    //     }
    //     assert(cnt == list->level) ; 
    // }

    if(list->head_block_number == ULNull && list->tail_block_number == ULNull){
        printf("list name : %c, list level : %d\n", list->name, list->level);
        printf("empty list\n") ; 
        for(int i = 0 ; i < Vicitm_Selected_Each_Round ; i++){
            DWORD victimblock = FTLptr->GC_Victim_List[i] ; 
            printf("Block: %d, State: %d, Valid Page Count: %d, Hotness: %d\n", victimblock, FTLptr->Blocks[victimblock].GC_flag, FTLptr->Blocks[victimblock].valid_page_count, FTLptr->Blocks[victimblock].hotness) ; 
        }
        return ; 
    }
    else if(list->head_block_number == list->tail_block_number){
        // assert(list->total_blocks_count == 1) ; 
        list->head_block_number = list->tail_block_number = ULNull ; 
        FTLptr->Parent_Table[target_block] = FTLptr->Sibling_Table[target_block] = ULNull ;
        list->total_blocks_count-- ;  
    }
    else if(target_block == list->head_block_number && list->total_blocks_count != 1 && list->total_blocks_count > 1){
        // assert(FTLptr->Parent_Table[target_block] == ULNull) ; 
        DWORD newhead = FTLptr->Sibling_Table[target_block] ; 
        list->head_block_number = newhead ; 
        FTLptr->Parent_Table[newhead] = ULNull ; 
        FTLptr->Sibling_Table[target_block] = ULNull ; 
        list->total_blocks_count-- ; 
    }
    else if(target_block == list->tail_block_number && list->total_blocks_count != 1 && list->total_blocks_count > 1){
        // assert(FTLptr->Sibling_Table[target_block] == ULNull) ; 
        DWORD newtail = FTLptr->Parent_Table[target_block] ; 
        list->tail_block_number = newtail ; 
        FTLptr->Sibling_Table[newtail] = ULNull ; 
        FTLptr->Parent_Table[target_block] = ULNull ; 
        list->total_blocks_count-- ; 
    }
    else{
        DWORD oldparent = FTLptr->Parent_Table[target_block] ; 
        DWORD oldsibling = FTLptr->Sibling_Table[target_block] ;  
        FTLptr->Sibling_Table[oldparent] = oldsibling ; 
        FTLptr->Parent_Table[oldsibling] = oldparent ; 
        FTLptr->Parent_Table[target_block] = FTLptr->Sibling_Table[target_block] = ULNull ; 
        list->total_blocks_count-- ; 
    }
    // assert(FTLptr->Parent_Table[target_block] == ULNull && FTLptr->Sibling_Table[target_block] == ULNull) ; 
}

void Check_Blocks(FTL* FTLptr){
    DWORD i ; 
    for(i = 0 ; i < FTLptr->config.number_of_Block_in_Physical_disk_space ; i++){
        DWORD correct_flag = FTLptr->Blocks[i].GC_flag ; 
        DWORD parent = FTLptr->Parent_Table[i] ; 
        DWORD sibling = FTLptr->Sibling_Table[i] ; 
        assert((parent == ULNull || FTLptr->Blocks[parent].GC_flag == correct_flag) && (sibling == ULNull || FTLptr->Blocks[sibling].GC_flag == correct_flag)) ; 
        if(correct_flag == UsedBlockState){
            DWORD search = FTLptr->Blocks[i].valid_page_count ;
            bool flag = false ; 
            DWORD cur = FTLptr->Multilevel_LRI_Lists[search].head_block_number ; 
            while(cur != ULNull){
                if(cur == i && FTLptr->Parent_Table[cur] == parent && FTLptr->Sibling_Table[cur] == sibling){
                    flag = true ; 
                    break ; 
                }
                else{
                    cur = FTLptr->Sibling_Table[cur] ; 
                }
            }
            assert(flag == true) ; 
        }
    }
    assert(FTLptr->current_GCBlock == ULNull || FTLptr->Blocks[FTLptr->current_GCBlock].GC_flag == OverProvisionBlockState) ; 
    assert(FTLptr->current_LogBlock == ULNull || FTLptr->Blocks[FTLptr->current_LogBlock].GC_flag == OverProvisionBlockState) ;
    assert(FTLptr->current_GCBlock == ULNull || (FTLptr->Parent_Table[FTLptr->current_GCBlock] == ULNull && FTLptr->Sibling_Table[FTLptr->current_GCBlock] == ULNull)) ; 
    assert(FTLptr->current_LogBlock == ULNull || (FTLptr->Parent_Table[FTLptr->current_LogBlock] == ULNull && FTLptr->Sibling_Table[FTLptr->current_LogBlock] == ULNull)) ; 
}

void Write_info(FTL* FTLptr , DWORD min_Record_operations_count , DWORD max_Record_operations_count){
    if(max_Record_operations_count < FTLptr->systemTime || min_Record_operations_count > FTLptr->systemTime){
        return ; 
    }
    FILE* file = fopen("My_FTL_trace.txt" , "a") ; 
    fprintf(file , "============ Time %lu Info ============\n" , FTLptr->systemTime) ; 
    // Write unRefList : 
    DWORD unref_head = FTLptr->unRefList.head_block_number ; 
    if(unref_head == ULNull){
        fprintf(file , "No free logcial blocks!\n") ; 
    }
    else{
        fprintf(file , "In unRefList : \n") ; 
        DWORD cur = unref_head ; 
        while(cur != ULNull){
            fprintf(file , "%lu -> " , cur) ; 
            cur = FTLptr->Sibling_Table[cur] ; 
        }
        fprintf(file , "\n") ; 
    }
    // Write SpareList ;
    DWORD spare_head = FTLptr->SpareList.head_block_number ; 
    if(spare_head == ULNull){
        fprintf(file , "No free physical blocks!\n") ; 
    }
    else{
        fprintf(file , "In SpareList : \n") ;
        DWORD cur = spare_head ; 
        while(cur != ULNull){
            fprintf(file , "%lu -> " , cur) ; 
            cur = FTLptr->Sibling_Table[cur] ; 
        }
        fprintf(file , "\n") ; 
    }
    // Write Mutilevel_LRI_List ; 
    for(DWORD i = 0 ; i <= FTLptr->config.number_of_Page_in_Block ; i++){
        DWORD level_head = FTLptr->Multilevel_LRI_Lists[i].head_block_number ; 
        if(level_head != ULNull){
            fprintf(file , "In Level %lu : \n" , i) ;
            DWORD cur = level_head ; 
            while(cur != ULNull){
                fprintf(file , "%lu -> " , cur) ; 
                cur = FTLptr->Sibling_Table[cur] ; 
            }
            fprintf(file , "\n") ;
        } 
    }
    // Record logblock and its entry
    fprintf(file , "Log block and its entry : %lu , %lu\n" , FTLptr->current_LogBlock , FTLptr->current_LogBlockWrittenPage) ; 
    // Record GC_logblock and its entry
    fprintf(file , "GC logblock and its entry : %lu , %lu\n" , FTLptr->current_GCBlock , FTLptr->current_GCBlockWrittenPage) ; 
    fprintf(file , "============ Time %lu End ============\n" , FTLptr->systemTime) ; 
    fclose(file) ; 
}

void Change_Hotness_Overtime(FTL* FTLptr){
    // DWORD i ; 
    // for(i = 0 ; i < FTLptr->config.number_of_Block_in_Physical_disk_space ; i++){
    //     if((FTLptr->Blocks[i].GC_flag == UsedBlockState || i == FTLptr->current_GCBlock || i == FTLptr->current_LogBlock)&& FTLptr->Blocks[i].hotness > 0){
    //         FTLptr->Blocks[i].hotness -= 1 ; 
    //     }
    // }
    for(DWORD i = 0 ; i <= FTLptr->config.number_of_Page_in_Block ; i++){ 
        if(FTLptr->Multilevel_LRI_Lists[i].total_blocks_count != 0){ 
            DWORD current_block = FTLptr->Multilevel_LRI_Lists[i].head_block_number ; 
            for(DWORD j = 0 ; j < FTLptr->Multilevel_LRI_Lists[i].total_blocks_count ; j++){ 
                if(FTLptr->Blocks[current_block].hotness > 0){
                    FTLptr->Blocks[current_block].hotness -= 1 ;
                }
                current_block = FTLptr->Sibling_Table[current_block] ; 
            }
        }
    }
    if(FTLptr->current_GCBlock != ULNull && FTLptr->Blocks[FTLptr->current_GCBlock].hotness > 0){
        FTLptr->Blocks[FTLptr->current_GCBlock].hotness -= 1 ; 
    }
    if(FTLptr->current_LogBlock != ULNull && FTLptr->Blocks[FTLptr->current_LogBlock].hotness > 0){
        FTLptr->Blocks[FTLptr->current_LogBlock].hotness -= 1 ; 
    } 
}

void Assign_Hotness_WhenWrite(FTL* FTLptr, DWORD to_invalidate_block){
    if(init_counter < (System_hotness_cycle/2)){
        FTLptr->Blocks[to_invalidate_block].hotness = 1 ; 
        return ; 
    }
    else if(init_counter < System_hotness_cycle){
        FTLptr->Blocks[to_invalidate_block].hotness = 2 ; 
        return ; 
    }

    if(FTLptr->Blocks[to_invalidate_block].hotness == 0){ // 0 --> 1, hotness rise because it is the first time we out-of-place update this block
        FTLptr->Blocks[to_invalidate_block].hotness = 1 ; 
    }
    else if(FTLptr->Blocks[to_invalidate_block].hotness == 1){ // 1 --> 2, hotness rise since we reference it again(0-->1 and then 1-->2)
        FTLptr->Blocks[to_invalidate_block].hotness = 2 ; 
    }
    else if(FTLptr->Blocks[to_invalidate_block].hotness == 2){ // 2 --> 2, hotness remain in the "most hot" level
        // pass
    }
}