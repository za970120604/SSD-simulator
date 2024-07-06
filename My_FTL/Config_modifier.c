#include "SSD_parameter.h"
#include "FTL_structure.h"
int deleteAllFilesInDirectory(const char *directoryPath) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(directoryPath);
    if (dir == NULL) {
        perror("Error opening directory");
        return 0;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            // Construct the full file path
            char filePath[PATH_MAX];
            snprintf(filePath, sizeof(filePath), "%s/%s", directoryPath, entry->d_name);

            // Use remove to delete the file
            if (remove(filePath) != 0) {
                perror("Error deleting file");
                closedir(dir);
                return 0;
            }
        }
    }

    closedir(dir);
    return 1; // All files deleted successfully
}
char* Config_Adjusting(char* original_fullpath , char* original_filename , double min_percentage , double max_percentage , int changement , bool only_ori){
    FILE* config_file = fopen(original_fullpath , "r+") ; 
    assert(config_file != NULL) ; 
    unsigned char buf[101],junk[101];
    char lineBuffer[1024] ; 
    I64 P_Byte ; 
    I64 L_Byte ; 
    DWORD Block_Byte ; 
    while(!feof(config_file)){ 
	    fgets(lineBuffer , sizeof(lineBuffer) , config_file);
        if(strstr(lineBuffer , "PsizeByte") != NULL) { 
            sscanf(lineBuffer,"%s %I64i",junk,&P_Byte);
	        assert(strcmp(junk,"PsizeByte")==0);
        }
        if(strstr(lineBuffer , "LsizeByte") != NULL){
            sscanf(lineBuffer,"%s %I64i",junk,&L_Byte);
	        assert(strcmp(junk,"LsizeByte")==0); 
        }
        if(strstr(lineBuffer , "blockSizeByte") != NULL){
           	sscanf(lineBuffer,"%s %lu",junk,&Block_Byte);
	        assert(strcmp(junk,"blockSizeByte")==0);
        }
    }
    fclose(config_file) ; 
    // printf("%I64i\n" , P_Byte) ; 
    DWORD Block_in_P = (DWORD)(P_Byte / (I64)Block_Byte) ; 
    DWORD Block_in_L = (DWORD)(L_Byte / (I64)Block_Byte) ; 
    double Overprovisioing = (double)100 * (double)(Block_in_P - Block_in_L) / Block_in_P ;
    char* new_directory = (char*)malloc(2000 * sizeof(char)); ; 
    sprintf(new_directory , "..\\%s\\%s\\" , "Config File" , original_filename) ; 
    _mkdir(new_directory); 
    deleteAllFilesInDirectory(new_directory) ; 
    // Copy the original file , at least do one time with the original config file
    double ori_percentage = (double)100 * (double)(Block_in_P - Block_in_L) / Block_in_P ; 
    char postfix[30] ; 
    sprintf(postfix , "_%.4f.txt" , ori_percentage) ; 
    char full_subpath[1000] ; 
    char full_filename[500] ; 
    strcpy(full_filename , original_filename) ; 
    strcat(full_filename , postfix) ; 
    strcpy(full_subpath , new_directory);
    strcat(full_subpath , full_filename) ; ; 
    FILE* subfile = fopen(full_subpath , "w+") ; 
    FILE* parentfile = fopen(original_fullpath , "r+") ; 
    char buff[500] ; 
    while(!feof(parentfile)){ 
        fgets(buff , sizeof(buff) , parentfile);
        fprintf(subfile , buff) ; 
    }
    fclose(subfile) ; 
    fclose(parentfile) ;
    printf("total %d for original\n" , 1) ; 
    if(only_ori){
        return new_directory ;
    }
    I64 cur_P_Byte = P_Byte; 
    int cnt = 0 ; 
    while(1){
        cur_P_Byte -= (I64)Block_Byte * (I64)changement ; 
        DWORD cur_Block_in_P = (DWORD)(cur_P_Byte / (I64)Block_Byte) ; 
        double cur_percentage = (double)100 * (double)(cur_Block_in_P - Block_in_L) / Block_in_P ; 
        if(cur_P_Byte < L_Byte || cur_percentage < min_percentage || (cur_Block_in_P - Block_in_L) <= 10){
            break ; 
        }
        cnt += 1 ; 
        char postfix[30] ; 
        sprintf(postfix , "_%.4f.txt" , cur_percentage) ; 
        char full_subpath[1000] ; 
        char full_filename[500] ; 
        strcpy(full_filename , original_filename) ; 
        strcat(full_filename , postfix) ; 
        strcpy(full_subpath , new_directory);
        strcat(full_subpath , full_filename) ; ; 
        FILE* subfile = fopen(full_subpath , "w+") ; 
        FILE* parentfile = fopen(original_fullpath , "r+") ; 
        char buff[500] ; 
        while(!feof(parentfile)){ 
            fgets(buff , sizeof(buff) , parentfile);
            if(strstr(buff , "PsizeByte") != NULL) { 
                fprintf(subfile , "PsizeByte\t%I64i ;\n" , cur_P_Byte) ;  
            }
            else{
                fprintf(subfile , buff) ; 
            }
        }
        fclose(subfile) ; 
        fclose(parentfile) ; 
    }
    printf("total %d for smaller\n" , cnt) ; 
    cnt = 0 ; 
    cur_P_Byte = P_Byte; 
    while(1){
        cur_P_Byte += (I64)Block_Byte * (I64)changement  ; 
        DWORD cur_Block_in_P = (DWORD)(cur_P_Byte / (I64)Block_Byte) ; 
        double cur_percentage = (double)100 * (double)(cur_Block_in_P - Block_in_L) / Block_in_P ; 
        // printf("%.2f\n" , cur_percentage) ; 
        if(cur_P_Byte < L_Byte || cur_percentage > max_percentage){
            break ; 
        }
        cnt += 1 ; 
        char postfix[30] ; 
        sprintf(postfix , "_%.2f.txt" , cur_percentage) ; 
        char full_subpath[1000] ; 
        char full_filename[500] ; 
        strcpy(full_filename , original_filename) ; 
        strcat(full_filename , postfix) ; 
        strcpy(full_subpath , new_directory);
        strcat(full_subpath , full_filename) ; ; 
        FILE* subfile = fopen(full_subpath , "w+") ; 
        FILE* parentfile = fopen(original_fullpath , "r+") ; 
        char buff[500] ; 
        while(!feof(parentfile)){ 
            fgets(buff , sizeof(buff) , parentfile);
            if(strstr(buff , "PsizeByte") != NULL) { 
                fprintf(subfile , "PsizeByte\t%I64i ;\n" , cur_P_Byte) ;  
            }
            else{
                fprintf(subfile , buff) ; 
            }
        }
        fclose(subfile) ; 
        fclose(parentfile) ; 
    }
    printf("total %d for larger\n" , cnt) ; 
    return new_directory ; 
}

// int main(){
//     char* retrieve = Config_Adjusting("..\\Config File\\8k1m40g4G.txt" , "8k1m40g4G" , 1 , 30 , 500) ; 
//     printf("%s\n" , retrieve) ; 
//     return 0 ; 
// }