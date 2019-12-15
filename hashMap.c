#include "hashMap.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

int hashFunction1(const char* key)
{
    int r = 0;
    for (int i = 0; key[i] != '\0'; i++)
    {
        r += key[i];
    }
    return r;
}

int hashFunction2(const char* key)
{
    int r = 0;
    for (int i = 0; key[i] != '\0'; i++)
    {
        r += (i + 1) * key[i];
    }
    return r;
}

/**
 * Creates a new hash table link with a copy of the key string.
 * @param key Key string to copy in the link.
 * @param value Value to set in the link.
 * @param next Pointer to set as the link's next.
 * @return Hash table link allocated on the heap.
 */
HashLink* hashLinkNew(const char* key, int value, HashLink* next)
{
    HashLink* link = malloc(sizeof(HashLink));
    link->key = malloc(sizeof(char) * (strlen(key) + 1));
    strcpy(link->key, key);
    link->value = value;
    link->next = next;
    return link;
}

/**
 * Free the allocated memory for a hash table link created with hashLinkNew.
 * @param link
 */
static void hashLinkDelete(HashLink* link)
{
    free(link->key);
    free(link);
}

/**
 * Initializes a hash table map, allocating memory for a link pointer table with
 * the given number of buckets.
 * @param map
 * @param capacity The number of table buckets.
 */
void hashMapInit(HashMap* map, int capacity)
{
    map->capacity = capacity;
    map->size = 0;
    map->table = malloc(sizeof(HashLink*) * capacity);
    for (int i = 0; i < capacity; i++)
    {
        map->table[i] = NULL;
    }
}

/**
 * Removes all links in the map and frees all allocated memory. You can use
 * hashLinkDelete to free the links.
 * @param map
 */
void hashMapCleanUp(HashMap* map)
{
    // FIXME: implement
    assert(map != NULL);

    HashLink *currentLink; //iterator pointers
    HashLink *nextLink;

    for(int i=0; i < map->capacity; i++){
        currentLink = map->table[i];
        while(currentLink != NULL){ //use loop to iterate through table and delete individual links
            nextLink = currentLink->next; //point to next link that will be reassigned after deletion
            hashLinkDelete(currentLink); 
            currentLink = nextLink;
        }
    }
    free(map->table); //free map's memory
    map->size = 0; //set size to empty
}

/**
 * Creates a hash table map, allocating memory for a link pointer table with
 * the given number of buckets.
 * @param capacity The number of buckets.
 * @return The allocated map.
 */
HashMap* hashMapNew(int capacity)
{
    HashMap* map = malloc(sizeof(HashMap));
    hashMapInit(map, capacity);
    return map;
}

/**
 * Removes all links in the map and frees all allocated memory, including the
 * map itself.
 * @param map
 */
void hashMapDelete(HashMap* map)
{
    hashMapCleanUp(map);
    free(map);
}

/**
 * Returns a pointer to the value of the link with the given key  and skip traversing as well. Returns NULL
 * if no link with that key is in the table.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket. Also make sure to search the entire list.
 * 
 * @param map
 * @param key
 * @return Link value or NULL if no matching link.
 */
int* hashMapGet(HashMap* map, const char* key)
{
    // FIXME: implement
    assert(map != NULL);
    assert(key != NULL);

    int index = HASH_FUNCTION(key)%hashMapCapacity(map); //compute index by taking key's hash moddded over map's capacity

    HashLink *current = map->table[index];

    while(current != NULL){ //use loop to iterate through table
        if(strcmp(current->key, key) == 0){ //if parameter key and current link's key match, return value associated with that key
            return &(current->value);
        }
        current = current->next;
    }

    return NULL; //otherwise, return nothing
}

/**
 * Resizes the hash table to have a number of buckets equal to the given 
 * capacity (double of the old capacity). After allocating the new table, 
 * all of the links need to rehashed into it because the capacity has changed.
 * 
 * Remember to free the old table and any old links if you use hashMapPut to
 * rehash them.
 * 
 * @param map
 * @param capacity The new number of buckets.
 */
void resizeTable(HashMap* map, int capacity)
{
    // FIXME: implement
    assert(map != NULL);
    assert(capacity > 0);

    int previousCapacity = hashMapCapacity(map);
    HashMap *temp = hashMapNew(capacity);

    for(int i=0; i<previousCapacity; i++){
        HashLink *current = map->table[i];

        while(current){
            hashMapPut(temp, current->key, current->value);
            current = current->next;
        }
    }

    hashMapCleanUp(map);
    map->table = temp->table;
    map->size = temp->size;
    map->capacity = temp->capacity;

    temp->table = NULL;
    free(temp);
    
}

/**
 * Updates the given key-value pair in the hash table. If a link with the given
 * key already exists, this will just update the value and skip traversing. Otherwise, it will
 * create a new link with the given key and value and add it to the table
 * bucket's linked list. You can use hashLinkNew to create the link.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket.
 * 
 * @param map
 * @param key
 * @param value
 */
void hashMapPut(HashMap* map, const char* key, int value)
{
    // FIXME: implement
    assert(map != NULL);
    assert(key != NULL);

    float loadFactor;
    int mapCapacity = hashMapCapacity(map);
    int index = HASH_FUNCTION(key)%mapCapacity;

    if(index < 0){ //modify index to be valid if not positive
        index = index + mapCapacity;
    }

    if(hashMapContainsKey(map, key)){ //if key is found, put value in the associated bucket
        int *val = hashMapGet(map, key);
        (*val) += value;
    }
    else { //otherwise, create new link/bucket
        HashLink *newLink = hashLinkNew(key, value, NULL);

        assert(newLink != NULL);

        if(map->table[index] == NULL){
            map->table[index] = newLink;
        }
        else
        {
            HashLink *current = map->table[index];

            while(current->next){
                if(strcmp(current->key, key)){

                }
                current = current->next;
            }
            current->next = newLink;
        }

        map->size++;
        

    }

    loadFactor = hashMapTableLoad(map); //update load factor after addition

    if(loadFactor >= MAX_TABLE_LOAD){ //resize table if load factor becomes too great; reduce chances of collision
        resizeTable(map, 2*mapCapacity);
    }
}

/**
 * Removes and frees the link with the given key from the table. If no such link
 * exists, this does nothing. Remember to search the entire linked list at the
 * bucket. You can use hashLinkDelete to free the link.
 * @param map
 * @param key
 */
void hashMapRemove(HashMap* map, const char* key)
{
    // FIXME: implement
    assert(map != NULL);
    assert(key != NULL);

    int index = HASH_FUNCTION(key)%hashMapCapacity(map);

    HashLink *current = map->table[index];
    HashLink *previous = NULL;

    if(current != NULL){
        if(strcmp(current->key, key) == 0){
            map->table[index] = current->next;
        }
        else
        {
            while(strcmp(current->key, key) != 0){
                previous = current;
                current = current->next;
                previous->next = current->next;
            }
        }

        hashLinkDelete(current);
        map->size--;

    }
}

/**
 * Returns 1 if a link with the given key is in the table and 0 otherwise.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket. Also make sure to search the entire list.
 * 
 * @param map
 * @param key
 * @return 1 if the key is found, 0 otherwise.
 */
int hashMapContainsKey(HashMap* map, const char* key)
{
    // FIXME: implement
    assert(map != NULL);
    assert(key != NULL);

    int mapCapacity = hashMapCapacity(map);
    int index = HASH_FUNCTION(key)%mapCapacity; //hash key into index for table

    if(index < 0){
        index = index + mapCapacity;
    }

    HashLink *current = map->table[index];

    while(current){
        if(strcmp(current->key, key) == 0){ //return true if same key is found in the table
            return 1;
        }
        current = current->next;
    }

    return 0; //return false if loop terminates without matching
}

/**
 * Returns the number of links in the table.
 * @param map
 * @return Number of links in the table.
 */
int hashMapSize(HashMap* map)
{
    // FIXME: implement
    assert(map != NULL);
    return map->size;
}

/**
 * Returns the number of buckets in the table.
 * @param map
 * @return Number of buckets in the table.
 */
int hashMapCapacity(HashMap* map)
{
    // FIXME: implement
    assert(map != NULL);
    return map->capacity;
}

/**
 * Returns the number of table buckets without any links.
 * @param map
 * @return Number of empty buckets.
 */
int hashMapEmptyBuckets(HashMap* map)
{
    // FIXME: implement
    assert(map != NULL);

    int emptyBuckets = 0;
    int capacity = hashMapCapacity(map);

    for(int i = 0; i < capacity; i++){ //loop through entire table
        if(map->table[i] == NULL){ //every time we encounter a null value, this indicates an empty bucket, so increment our count
            emptyBuckets++;
        }
    }
    return emptyBuckets;
}

/**
 * Returns the ratio of (number of links) / (number of buckets) in the table.
 * Remember that the buckets are linked lists, so this ratio tells you nothing
 * about the number of empty buckets. Remember also that the load is a floating
 * point number, so don't do integer division.
 * @param map
 * @return Table load.
 */
float hashMapTableLoad(HashMap* map)
{
    // FIXME: implement
    assert(map != 0);

    float floatSize = (float)hashMapSize(map); //typecast variables to float for float arithmetic
    float floatCap = (float)hashMapCapacity(map);

    float loadFactor = floatSize/floatCap;
    
    return loadFactor;
}

/**
 * Prints all the links in each of the buckets in the table.
 * @param map
 */
void hashMapPrint(HashMap* map)
{
  // FIXME: implement
  for(int i = 0; i < map->capacity; i++){
      HashLink *current = map->table[i];
      if(current != NULL){
          printf("\nBucket %i ->", i); //first print bucket
          while(current != NULL){ //then print all associations within the bucket
              printf(" (%s, %d) ->", current->key, current->value);
              current = current->next; //iterate to next association in bucket
          }
      }
  }

    printf("\n");
   
}
