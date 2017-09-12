//read file in one at a time
read_file()
{
    while (!feof){
        //fscanf(fp, "%s,%s,%s,%ld", instruction);///not correct format
        fscanf(fp, "%s", instruction);
        while(string[count] != ' '){
            count++;
        }
        
        for (i = 0; i < count; i++){
            instruction[i] = string[i];
            
        }
            checkInstruction(instruction);

    }     
}



