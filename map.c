
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#define DEFAULT_CAPACITY 1023

typedef void (*CleanupValueFn)( void* addr);

struct map{
  char**buckets;
  size_t nelems , elemsz , nbuckets ;
  CleanupValueFn cleanup ;
};

typedef struct map CMap ;


#define PTR_TO_NEXT(i)    (char*)((unsigned long*)i)[0]
#define GET_KEY(i)        i+sizeof(char*)
#define GET_VAL(i)        i+sizeof(char*)+strlen(GET_KEY(i))+1

/* 
 * This function adapted from Eric Roberts' _The Art and Science of C_
 */
static int hash(const char *s, int nbuckets)
{
   const unsigned long MULTIPLIER = 2630849305L; // magic number
   unsigned long hashcode = 0;
   for (int i = 0; s[i] != '\0'; i++)
      hashcode = hashcode * MULTIPLIER + s[i];
   return hashcode % nbuckets;
}

/*
  Takes as input a ssize_t representing the size of the values that will be placed into the map
  another size_t which represents the predicted capacity of the map, and a function pointer to
  a custom deallocation function if necessary (ie the values in the map are strings).
  Returns a CMap with the given parameters.
 */
CMap *cmap_create(size_t valuesz, size_t capacity_hint, CleanupValueFn fn)
{
  if(valuesz == 0) assert("0 value size") ;
  CMap * map = calloc(sizeof(CMap),1) ;
  if(map == NULL) assert("Allocation failure") ;
  map->elemsz=  valuesz ;

  if(capacity_hint ==0) capacity_hint = DEFAULT_CAPACITY ;
  map->nbuckets = capacity_hint ;

  map->buckets = calloc(sizeof(char*) , map->nbuckets) ;
  if(map->buckets==NULL) assert("Allocation failure") ;
  map->nelems = 0 ;

  map->cleanup = fn;

  return map ;
}

/*
  Takes as input a CMap (cm) and handles the deallocation of all dynamically allocated memory calling 
  he user defined cleanup function if necessary.
 */
void cmap_dispose(CMap *cm)
{
  for(int x = 0 ; x< cm->nbuckets ; x++){

    char * cur = cm->buckets[x] ;
    while(1){
      if(cur == NULL) break ;

      char * next = PTR_TO_NEXT(cur) ;
      char * todealloc = GET_VAL(cur) ;
      if(cm->cleanup !=NULL) cm->cleanup(todealloc) ;
      free(cur) ;

      cur= next ;
    }
  }

  free(cm->buckets) ;
  free(cm) ;
}

/*
  Takes as input a CMap (cm) and returns the number of key value pairs in the map.
 */
int cmap_count(const CMap *cm){
  return cm->nelems ;
}

/*
  Takes as  input a CMap (cm), a string (key) and the address of some value to associate with the key.
  NOTE that we always put the new value at the begining of the bucket as in some applications the most recently input
  values may be the most used values.
 */
void cmap_put(CMap *cm, const char *key, const void *addr){
  int hashval= hash(key , cm->nbuckets) ;
  char *curfirst = cm->buckets[hashval] ;

  while(curfirst!=NULL){ // loop through the bucket to find if we ust need to replace the value.
    if(strcmp(GET_KEY(curfirst),key)==0){

      if(cm->cleanup !=NULL) cm->cleanup(GET_VAL(curfirst)) ; // cleanup the old value if cleanup is non null

      memcpy(GET_VAL(curfirst) , addr , cm->elemsz) ; // copy in the new value.
      return ;
    }
    curfirst=PTR_TO_NEXT(curfirst) ;
  }

  curfirst =cm->buckets[hashval] ; // pointer to the first element of this bucket.

  char *newFirst = calloc(sizeof(char) , sizeof(char*)+strlen(key)+cm->elemsz+1) ;
  if(newFirst==NULL) assert("Allocation failure") ;

  ((unsigned long *)newFirst)[0] = (unsigned long)curfirst ; // relink the linked list

  strcpy(GET_KEY(newFirst),key) ; // copy in the key
  memcpy(GET_VAL(newFirst) , addr , cm->elemsz) ; // copy in the value

  cm->buckets[hashval] = newFirst ;

  cm->nelems++ ;
}

/*
  Takes as input a CMap (cm) and a string (key) and returns the value associated with the key in the map
  rturns NULL if the key is not in the map.
 */
void *cmap_get(const CMap *cm, const char *key){
  int hashval= hash(key , cm->nbuckets) ;
  char * cur = cm->buckets[hashval] ;

  while(cur !=NULL){
    if(strcmp(GET_KEY(cur) , key)==0) return (void*)(GET_VAL(cur)) ;

    cur = PTR_TO_NEXT(cur) ;
  }
  return NULL ;
}

/*
  Takes as  input a CMap (cm) and a string (key) and removes the key key from the map along with its associated value
  relinks the linked list and frees all dynamically allocated memory.
 */
void cmap_remove(CMap *cm, const char *key){
  int hashval= hash(key , cm->nbuckets) ;

  char * cur = cm->buckets[hashval] ; // we only want to search the bucket that the string would be in. 
  char *previous = NULL ; // to relink the list.

  while(cur !=NULL){
    if(strcmp(GET_KEY(cur) , key) == 0){
      char * next = PTR_TO_NEXT(cur) ;
      if(previous !=NULL) ((unsigned long *)previous)[0] = (unsigned long)next ;
      
      if(cm->cleanup !=NULL) cm->cleanup(GET_VAL(cur)) ; // call custom cleanup function if it exists.

      // handle the freeing taking into account the edge case that the element being removed is the first one. 
      if(cur != cm->buckets[hashval]) free(cur) ;
      else{
	free(cur) ;
	cm->buckets[hashval] = NULL ;
      } 
      cm->nelems -- ;
      return ;
    }

    previous= cur ;
    cur = PTR_TO_NEXT(cur) ; // follow the linked list.
  } 
}

/*
  Takes as input a CMap (cm) and returns the first key in the map.
 */
const char *cmap_first(const CMap *cm)
{ 
  if(cm->nelems==0) return NULL ; // edge case to avoid searching all buckets in the case of empty map.
  for(int x = 0 ; x< cm->nbuckets ; x++){
    if(cm->buckets[x] != NULL){
      return GET_KEY(cm->buckets[x]) ;
    }
  }

  return NULL ;
}

/*
  Takes as input a CMap (cm) and a string (prevkey) and returns the next key in the map or a NULL if prevkey is the last key in the map.
 */
const char *cmap_next(const CMap *cm, const char *prevkey)
{
  int hashval = hash(prevkey , cm->nbuckets) ;
  char * next =(char*)((unsigned long *)(prevkey-sizeof(char*)))[0] ; // get the pointer to the next cell from prevkey

  if(next != NULL) return GET_KEY(next) ; // if non-null return it. 

  for(int x = hashval+1 ; x< cm->nbuckets ; x++){
    if(cm->buckets[x] != NULL) return GET_KEY(cm->buckets[x]) ; // search the rest of the buckets for the next key.
  }

  return NULL ;
}
