#include "hashMap.h"
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Allocates a string for the next word in the file and returns it. This string
 * is null terminated. Returns NULL after reaching the end of the file.
 * @param file
 * @return Allocated string or NULL.
 */
char* nextWord(FILE* file)
{
    int maxLength = 16;
    int length = 0;
    char* word = malloc(sizeof(char) * maxLength);
    while (1)
    {
        char c = fgetc(file);
        if ((c >= '0' && c <= '9') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            c == '\'')
        {
            if (length + 1 >= maxLength)
            {
                maxLength *= 2;
                word = realloc(word, maxLength);
            }
            word[length] = c;
            length++;
        }
        else if (length > 0 || c == EOF)
        {
            break;
        }
    }
    if (length == 0)
    {
        free(word);
        return NULL;
    }
    word[length] = '\0';
    return word;
}

int getMin(int a, int b, int c) //helper function - needed to compute Levenshtein distance with matrix elements
{
	if (a < b && a < c)
	{
		return a;
	}
	else if (b < c)
	{
		return b;
	}
	else
	{
		return c;
	}
}

/**
 * Loads the contents of the file into the hash map.
 * @param file
 * @param map
 */
void loadDictionary(FILE* file, HashMap* map)
{
    // FIXME: implement
    char *word = nextWord(file);

    while(word){
        hashMapPut(map, word, 0);
        free(word);
        word = nextWord(file);
    }

    free(word);
}

/**
 * Checks the spelling of the word provded by the user. If the word is spelled incorrectly,
 * print the 5 closest words as determined by a metric like the Levenshtein distance.
 * Otherwise, indicate that the provded word is spelled correctly. Use dictionary.txt to
 * create the dictionary.
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char** argv)
{
    // FIXME: implement
    HashMap* map = hashMapNew(1000);

    FILE* file = fopen("dictionary.txt", "r");
    clock_t timer = clock();
    loadDictionary(file, map);
    timer = clock() - timer;
    printf("Dictionary loaded in %f seconds\n", (float)timer / (float)CLOCKS_PER_SEC);
    fclose(file);

    char inputBuffer[256];
    int quit = 0;
    while (!quit)
    {
        printf("Enter a word or \"quit\" to quit: ");
        scanf("%s", inputBuffer);

        // Implement the spell checker code here..
       if(strcmp(inputBuffer, "quit") == 0){ //if word quit is entered, change quit value to 1 so that program can terminate
                    quit = 1;
        }

        else
        {
            if(hashMapContainsKey(map, inputBuffer)){ //if word match is found, print that the word is spelled correctly
                printf("The inputted word %s is spelled correctly \n", inputBuffer);
            }

        
            else //if our word is incorrect, we must find the 5 closest matches using the Levenshtein distance
            {
                printf("The inputted word %s is spelled incorrectly \n", inputBuffer);
                
                HashLink* current; // link pointer to iterate through dictionary map
                

                //Levenshtein distance implementation based on code from this page - reference: https://people.cs.pitt.edu/~kirk/cs1501/Pruhs/Spring2006/assignments/editdistance/Levenshtein%20Distance.htm
                int **matrix; // matrix used to compute Levenshtein distance, will be comparing the two words
                int rowCount; // matrix dimension variables
                int columnCount;

                const int closeMatches = 5; // number of close matches to display
                HashLink** matches = malloc(closeMatches * sizeof(HashLink*));
                HashLink* weakestMatch = NULL; //value that will point to the key that matches the least to the input buffer out of the 5
                int matchCount = 0; //total number of matches found

                for (int i = 0; i < map->capacity; i++)
                {
                    current = map->table[i];
                    while (current != NULL) // iterate through each dictionary entry in our map
                    {
                        // Create dynamic 2D matrix with the dimensions (dictionary word length + 1) x (input buffer length + 1)
                        rowCount = strlen(current->key) + 1;
                        columnCount = strlen(inputBuffer) + 1;
                        
                        matrix = malloc(rowCount * sizeof(int*)); 
                        for (int i = 0; i < rowCount; i++)
                        {
                            matrix[i] = malloc(columnCount * sizeof(int));
                        }

                        // Initialize first row and first column values with numerical values
                        for (int i = 0; i < rowCount; i++)
                        {
                            matrix[i][0] = i;
                        }
                        for (int j = 0; j < columnCount; j++)
                        {
                            matrix[0][j] = j;
                        }

                        // Calculate intermediate distances from previous values
                        for (int i = 1; i < rowCount; i++)
                        {
                            for (int j = 1; j < columnCount; j++)
                            {
                                // If no change needed, set to previous distance
                                if (current->key[i - 1] == inputBuffer[j - 1])
                                {
                                    matrix[i][j] = matrix[i - 1][j - 1];
                                }
                                // If change needed, set to minimum of three adjacent previous distances, plus 1
                                else
                                {
                                    matrix[i][j] = getMin(matrix[i - 1][j - 1],
                                                        matrix[i - 1][j],
                                                        matrix[i][j - 1]) + 1;
                                }
                            }
                        }

                        // Value in last matrix cell is final distance
                        current->value = matrix[rowCount - 1][columnCount - 1];

                        // Store as match if we haven't found 5 matches yet
                        if (matchCount < closeMatches) // initial fill of matches
                        {
                            matches[matchCount] = current;
                            matchCount++; //increment number of matches found

                            if (weakestMatch == NULL || 
                                current->value < weakestMatch->value)
                            {
                                weakestMatch = current;
                            }
                        }
                        else if (current->value < weakestMatch->value) //after 5 have been found, all others must be compared before they are considered a match
                        {
                            // Current value is a closer match than the weakest match
                            HashLink* newWeakest = current;
                            for (int i = 0; i < closeMatches; i++)
                            {
                                if (matches[i] == weakestMatch)
                                {
                                    matches[i] = current; // replace weakest
                                }
                                else if (matches[i]->value > newWeakest->value)
                                {
                                    newWeakest = matches[i]; // find new weakest
                                }
                            }
                            weakestMatch = newWeakest; 
                        }
                        
                        // Free matrix in memory
                        for (int i = 0; i < rowCount; i++)
                        {
                            free(matrix[i]);
                        }
                        free(matrix);

                        current = current->next; // go to next dictionary entry
                    }
                }

                // Print closest matches
                printf("Did you mean...?\n");
                for (int i = 0; i < closeMatches; i++)
                {
                    printf("%s \n", matches[i]->key);
                }

                free(matches);
            }
        }
        
        
        
		
    }

    hashMapDelete(map);
    return 0;
}
