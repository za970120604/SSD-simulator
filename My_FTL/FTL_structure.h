#include "stdio.h" // for "FILE" type
#include "stdbool.h" // for "bool" type

// Lists
typedef struct {
	char name ;  
	DWORD level ; 	
    DWORD head_block_number ; 
    DWORD tail_block_number ; 
    DWORD total_blocks_count ; 
} FTL_List ; 

// Page
typedef struct {
	DWORD sector ; 
    bool is_valid ; 
    // int page_time_stamp ; // recording the time when this page is written on new data
} FTL_Page ; 

// Block
typedef struct {
    FTL_Page* inside_pages ; // use Page* , because we can malloc a portion of memory for pages in a block and assign the function's return pointer to this variable
    DWORD valid_page_count ; 
    DWORD GC_flag ; // use this flag to see the "state" of this block (just created/free ? used ? ... )
    DWORD block_erase_count ; 
    // int block_time_stamp ; // recording the time when this block is written on new data
    // int last_invalidated_time_stamp ; // recording last time when inside this block there is page been invalidated
	DWORD hotness ; // proposed GC policy
} FTL_Block ; 

// Table
typedef struct {
    DWORD corresponding_physical_page_number ; 
    DWORD corresponding_physical_block_number ; 
} FTL_Table_Element ; // Goal : page-level mapping , given a "logical" page_number as index , we should be able to find the corresponding "physical page" index in the corresponding "physical block"

// Config
typedef struct {
	DWORD	number_of_Byte_in_Sector ; // smallest granularity          

	DWORD	number_of_Byte_in_Page ;   // middle granularity
	DWORD	number_of_Sector_in_Page ;

	DWORD	number_of_Byte_in_Block ;  // largest granularity
    DWORD	number_of_Sector_in_Block ;
	DWORD	number_of_Page_in_Block ;
	
	I64	Physical_disk_space_size_in_Byte ;
    DWORD	number_of_Page_in_Physical_disk_space ;
	DWORD	number_of_Block_in_Physical_disk_space ;
	
	I64	Logical_disk_space_size_in_Byte ;
	DWORD	number_of_Sector_in_Logical_disk_space ;
	DWORD	number_of_Page_in_Logical_disk_space ;
	DWORD	number_of_Block_in_Logical_disk_space ;

} FTL_config ;

typedef struct {
	I64	pageWrite ;
	I64	pageRead ;

	I64	userPageWrite ;
	I64	userPageRead ;

	I64	BlockErase_count ;
	I64	MaxCopy_count ;

	I64 choose_proposed_victim ; 
	I64 choose_greedy_victim ; 
} FTL_stat ;

// FTL
typedef struct {
	// Structures
	FTL_Block* Blocks ;	
    FTL_List unRefList ;			
	FTL_List SpareList ;				
	FTL_List* Multilevel_LRI_Lists ;					
	
	// Tables
	FTL_Table_Element* L2P_Table ;				
	DWORD* Parent_Table ;		
	DWORD* Sibling_Table ;
	DWORD* GC_Victim_List ;
	
	// Log Block and GC Log Block information
	DWORD current_LogBlock ; 
	DWORD current_LogBlockWrittenPage ; 
	DWORD current_GCBlock ; 
	DWORD current_GCBlockWrittenPage ; 

	// Others
	FTL_config config ;				
	FTL_stat stat ; 
    DWORD systemTime ; 
} FTL ;

// Function Declaration
void FTLinit(FTL* FTLptr , FILE* config_file) ;
void FTLfree(FTL* FTLptr) ; 
void FTL_insertList(FTL* FTLptr , FTL_List* list , DWORD target_block) ; 
void FTL_removeList(FTL* FTLptr , FTL_List* list , DWORD target_block) ; 
void FTL_Load_Alignment(FTL* FTLptr , DWORD* offset_sector , DWORD* request_length) ;
void FTL_Write_A_Page(FTL* FTLptr , DWORD aligned_offset_sector , FILE* fp) ; 
void Pick_Victim_Block_From_LRI(FTL* FTLptr) ; 
DWORD GetSpareBlock_using_GC(FTL* FTLptr , FILE* fp) ; 
bool Pass_GC_threshold(FTL* FTLptr) ;  
void Check_Blocks(FTL* FTLptr) ; 
void Write_info(FTL* FTLptr , DWORD min_Record_operations_count , DWORD max_Record_operations_count) ;
char* Config_Adjusting(char* original_fullpath , char* original_filename , double min_percentage , double max_percenteage , int changement , bool only_ori) ;
int deleteAllFilesInDirectory(const char *directoryPath) ;  
void Change_Hotness_Overtime(FTL* FTLptr) ;
void Assign_Hotness_WhenWrite(FTL* FTLptr, DWORD to_invalidate_block) ; 
bool Check_Picked_Victim(FTL* FTLptr, DWORD check_block_number, int counter) ;