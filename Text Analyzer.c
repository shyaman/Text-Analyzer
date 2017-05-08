#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <getopt.h>

#define MAXWIDTH 80 //maximum width of the graph


typedef struct word_
{
	char *word;
	int frequency;
	struct word_ *next;
} word;

typedef enum bool_ {FALSE,TRUE} bool;

void bars(float percentage, float maxPercentage, int maxWordLen, bool isScaled);
void bottomLine(int maxWordLen);
void verticalBar();
void rowBar(word *current,int maxWordLen,bool isScaled);
void process(FILE *filePtr,bool isCharProcess);
void wordCharProcess(char const *Word);
void charProcess(char const *Word);
word* createNewWord(char const *insertWord);
void wordRebuild(char *Word);
int maxWordLength(int length);
void drawGraph(int length,bool isScaled);
void sortWordFrequencies();
void usage();


word *wordsList = NULL;
int totalWordsCount = 0, distinctWordCount = 0;

int main(int argc, char **argv) {

	FILE *fop;
	
	int option,length,argument,op,index,scaled,curind;
	bool isScaled,isCharProcess,isWordProcess = FALSE;

	struct option optn[] = {"scaled",no_argument,&scaled,1};
	

	length = 10;	//make default length 10
	isScaled = FALSE;		//make not scaled as default
	isCharProcess = FALSE;	//make word process as default

	if(argc == 1){
		printf("No input files were given\n");
		printf("usage: %s [-l length] [-w | -c] [--scaled] filename1 filename2 ..\n",argv[0]);
		return 0;
	}
	opterr = 0;
	curind = optind;
	while ((op = getopt_long(argc, argv, "l:wc", optn, NULL)) != -1){
	    switch (op) {
	    	case 0:
	    		break;
		    case 'l':
		        length = atoi(optarg);
		        if(length < 0 || isalpha(optarg[0])){
		        	printf("Invalid option for [-l]\n");
   					printf("usage: %s [-l length] [-w | -c] [--scaled] filename1 filename2 ..\n",argv[0]);
   					return;
		        }else if(length == 0){
		        	return 0;
		        }
		        break;
		    case 'w':
		    	isWordProcess = TRUE;
		        break;
		    case 'c':
		    	isCharProcess = TRUE;
		    	break;
		    case '?':
		    	if(optopt == 'l'){
		    		printf("Not enough options for [-%c]\n",optopt);
    				printf("usage: %s [-l length] [-w | -c] [--scaled] filename1 filename2 ..\n",argv[0]);
		    	}else{
		    		printf("Invalid option [%s]\n",argv[curind]);
    				printf("usage: %s [-l length] [-w | -c] [--scaled] filename1 filename2 ..\n",argv[0]);
		    	}
		    	return 0;
		    default:
        		return 0;
	   }
   }
   if(isCharProcess == TRUE && isWordProcess == TRUE){
   		printf("[-c] and [-w] cannot use together\n");
 		printf("usage: %s [-l length] [-w | -c] [--scaled] filename1 filename2 ..\n",argv[0]);
  		return 0;
   }
   if(scaled == 1){
   		isScaled = TRUE;
   }
   if(optind == argc){
   		printf("No input files were given\n");
   		printf("usage: %s [-l length] [-w | -c] [--scaled] filename1 filename2 ..\n",argv[0]);
   		return 0;
   }

   for (index = optind; index < argc; index++){
   		fop = fopen(argv[index],"r");
   		if(fop == NULL){
   			printf("Cannot open one or more given files\n");
   			return 0;
   		}
		process(fop,isCharProcess);
		fclose(fop);
	}
	
	if(totalWordsCount == 0){
		printf("No data to process\n");
		return 0;
	}

	sortWordFrequencies();
	drawGraph(length,isScaled);		

}

	

void bars(float percentage, float maxPercentage, int maxWordLen, bool isScaled){	
	int bar,segments,maxBarWidth,percentageLength;
	
	if(maxPercentage < 10){		//length occupy from MAXWIDTH by maximum percentage
		percentageLength = 5;
	}else if(maxPercentage < 100){
		percentageLength = 6;
	}else{
		percentageLength = 7;
	}

	if(isScaled == FALSE){		//if graph is not scaled maxPercentage = 100.0(total bars width should occupy 100% or else it should occupy maximum percentage) 
		maxPercentage = 100.0;
	}
	maxBarWidth = MAXWIDTH - maxWordLen - percentageLength - 3; 
	segments = (int)((percentage/maxPercentage)*maxBarWidth);

	for (bar = 0; bar < segments; bar++){	
		printf("\u2591");  
	}
}

void bottomLine(int maxWordLen){
	int segment;

	printf("\e[%dC",maxWordLen+2);
	printf("\u2514");
	for(segment = 0; segment < MAXWIDTH-maxWordLen-3; segment++){
		printf("\u2500"); 
	}
	puts(""); //new line at the end of the program
}

void verticalBar(){
	printf("\u2502"); 
}

void rowBar(word *current,int maxWordLen,bool isScaled){
	int barHeight,wordLen;
	float percentage,maxPercentage;

	wordLen = strlen(current->word);
	percentage = (current->frequency*100.0)/totalWordsCount;

	maxPercentage = (wordsList->frequency*100.0)/totalWordsCount;	//taking max percentage

	for(barHeight = 0; barHeight < 4; barHeight++){
		if(barHeight == 1){
			if(maxWordLen == wordLen){
				printf(" %s ",current->word);	//if it's the word with max length
			}else{
				printf(" %s \e[%dC",current->word,maxWordLen - wordLen);	//allocatong front space and print word
			}
		}else{
			printf("\e[%dC",maxWordLen+2);	//allocating front space
		}

		verticalBar();
		if(barHeight < 3){					//print only 3 bars
			bars(percentage,maxPercentage,maxWordLen,isScaled);
		}

		if(barHeight == 1){				//printing the percentage infront of the middle bar
			printf("%.2f%%",percentage);
		}

		puts("");
	}
}

void process(FILE *filePtr,bool isCharProcess){
	char word[255];
	
	while(!feof(filePtr)){		
		fscanf(filePtr,"%s ",word);	//scan and process word by word
		wordRebuild(word);		//rebuild the word
		if(word[0] == '\0'){
			continue;
		}else if(isCharProcess == TRUE){ 
			charProcess(word);	
		}else{
			wordCharProcess(word);
		}
	}
}

void wordCharProcess(char const *Word){
	word *current,*last;
	
	current = wordsList;	
	totalWordsCount++;
	if(wordsList== NULL){		//creating the first entry
		wordsList = createNewWord(Word);
		return;
	}
	while(current != NULL){		//check the list and increase the frequency by 1 if it's already in.
		if(!strcmp(Word,current->word)){
			current->frequency++;
			return;
		}
		last = current;
		current = current->next;
	}
	last->next = createNewWord(Word);	//if it's not on the list new entry is append at the end of the list
}

void charProcess(char const *Word){
	int charNum = 0;
	char character[2];
	character[1] = '\0';
	while(Word[charNum] != '\0'){		//process charcter by character
		strncpy(character,Word + charNum,1);
		wordCharProcess(character);
		charNum++;
	}
}

word* createNewWord(char const *insertWord){
	word *newWord = NULL;
	newWord = (word *)malloc(sizeof(word));
	newWord->word = (char *)malloc(strlen(insertWord) + 1);
	strcpy(newWord->word,insertWord);	//word/character is copied in to new entry
	newWord->frequency = 1;		//frequency of newly created node is set to 1
	newWord->next = NULL;		
	distinctWordCount++;		//distinct word count is increase by 1 when creating a new entry
	return newWord;
}

void wordRebuild(char *Word){	//removing characters otherthan letters and numbers
	int index = 0,temp;
	while(Word[index] != '\0'){
		if(!isalnum(Word[index])){
			temp = index;
			while(Word[temp] != '\0'){
				Word[temp] = Word[temp + 1];
				temp++;
			}
		}else{
			index++;
		}
	}
	index = 0;
	while(Word[index] != '\0'){
		Word[index] = tolower(Word[index]);
		index++;
	}
}

int maxWordLength(int length){	//maximum word length from the words in list
	int maxWLength = 0,tempWLength,tempLength = 0;
	word *current;

	current = wordsList;
	while((current != NULL) && (tempLength < length)){		
		tempWLength = strlen(current->word);
		if(tempWLength > maxWLength){
			maxWLength = tempWLength;
		}
		current = current->next;
		tempLength++;
	}
	
	return maxWLength;
}

void drawGraph(int length,bool isScaled){
	int maxWordLen,distinctWords = 0;
	word *current;
	current = wordsList;
	maxWordLen = maxWordLength(length);

	length = (length > distinctWordCount) ? distinctWordCount : length; //if given length is higher than distinct word count, then the ;length = distinct word count
	
	puts("");
	while((current != NULL)&&(distinctWords < length)){
		rowBar(current,maxWordLen,isScaled);
		current = current->next;
		distinctWords++;
	}
	bottomLine(maxWordLen);
}

void sortWordFrequencies(){	//sorting 
	word *current;
	int tempFrequency,word,wordCount;
	bool swap = TRUE;
	char *tempWord;

	wordCount = distinctWordCount;
	while(swap == TRUE){
		swap = FALSE;
		current = wordsList;
		for (word = 0; word < wordCount-1; word++){
			if(current->frequency < current->next->frequency){
				tempFrequency = current->frequency;
				current->frequency = current->next->frequency;
				current->next->frequency = tempFrequency;

				tempWord = current->word;
				current->word = current->next->word;
				current->next->word = tempWord;

				swap = TRUE;
			}
			current = current->next;	
		}				
		wordCount--;
	}
}
