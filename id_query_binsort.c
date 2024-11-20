#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "id_query.h"

struct index_record {
  int64_t osm_id;
  const struct record *record;
};

struct indexed_data {
  struct index_record *irs;
  int n;
};

//Function for comparing the osm_id of two records, used with qsort()
int comparator(const void* a, const void* b) {
  struct index_record* p = (struct index_record*) a;
  struct index_record* q = (struct index_record*) b;
  if (p -> osm_id < q -> osm_id) {
    return -1;
  } else if (p -> osm_id > q -> osm_id) {
    return 1;
  } else {
    return 0;
  }
}

//function which given an array of records and that arrays length,
//stores the length of the array and stores pointers to the records of the array indexed by their osm_ids,
//and also sorts the index of records
struct indexed_data* mk_binsort(struct record* rs, int n) {
  //allocate memory for the indexed data struct
  struct indexed_data *data = (struct indexed_data*) malloc(sizeof(struct indexed_data));
  //return NULL if the allocation fails
  if (data == NULL) {
    return NULL;
  }

  //allocate memory for the record index
  struct index_record *irs = malloc(sizeof(struct index_record) * n);
  //return NULL and free the memory allocated to data if allocation fails
  if (irs == NULL) {
    free(data);
    return NULL;
  }

  //iterate through all the records and save their osm_id and address into the index_record array
  for (int i = 0; i < n; i++) {
    irs[i].osm_id = rs[i].osm_id;
    irs[i].record = &rs[i];
  }

  //save the index_record array in the indexed_data struct
  data->irs = irs;
  //save the length of the record array
  data->n = n;

  //sort the index according to osm_ids
  qsort(data->irs, n, sizeof(struct index_record), comparator);
  
  //return a pointer to the sorted indexed_data struct
  return (data);
}

//free the memory allocated to the data structure
void free_binsort(struct indexed_data* data) {
  //free the memory allocated to the indexed records
  free(data->irs);
  //free the memory allocated to the indexed_data struct
  free(data);
}

//function which searches for a record in a naive_data struct which has a given osm_id using binary search
const struct record* lookup_binsort(struct indexed_data *data, int64_t needle) {
  //return NULL if data doesn't exist
  if(data == NULL) {
    return NULL;
  }

  //high: end of the array partition being searched, initialized to the end of the array
  int high = data->n - 1;
  //low: start of the array partion being searched, initialized to the start of the array
  int low = 0;

  //continue looping while the start of the array partition is before the end of the array partition
  while (low <= high) {
    //find the middle of the current array partition
    int mid = low + (high - low) / 2;

    //if the osm_id of the median element of the current array partition is equal to the given osm_id then
    //return that element
    if (data->irs[mid].osm_id == needle) {
      return data->irs[mid].record;
    }

    //if the osm_id of the median element of the current array partition is lower than the given osm_id then
    //set the beginning of the array partition to the median element + 1
    //otherwise set end of the array partition to the median element - 1 
    if (data->irs[mid].osm_id < needle)
      low =  mid + 1;
    else
      high = mid - 1;
  }

  //if an osm_id matching the given osm_id wasn't found return NULL
  return NULL;
}


int main(int argc, char** argv) {
  return id_query_loop(argc, argv,
                    (mk_index_fn)mk_binsort,
                    (free_index_fn)free_binsort,
                    (lookup_fn)lookup_binsort);
}
