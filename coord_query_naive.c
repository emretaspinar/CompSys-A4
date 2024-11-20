#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "coord_query.h"

struct naive_data {
  struct record* rs;
  int n;
};

//function which loads an array of records and the arrays lentgh into a naive_data struct
struct naive_data* mk_naive(struct record* rs, int n) {
  //allocate memory for the naive_data struct 
  struct naive_data* data = malloc(sizeof(struct naive_data));
  //if the allocation failed return NULL
  if (data == NULL) {
    return NULL;
  }

  //save the length of the record array
  data->n = n; 

  //save the address to the beginning of the record array
  data->rs = rs;

  //return the pointer to the data structure
  return data;
}

//free the memory allocated to the data structure
void free_naive(struct naive_data* data) {
  free(data);
}

//function which searches through a naive_data struct and returns the point closest to a given point
//the search is carried out using linear search
const struct record* lookup_naive(struct naive_data* data, double lon, double lat) {
  //return NULL if data doesn't exist
  if(data == NULL) {
    return NULL;
  }
  //set the first point in data->rs as the closest point  
  struct record* closest = &data->rs[0];
  //set the shortest distance so far as the distance between the first point in data->rs and the target point
  double minDist = (data->rs[0].lon - lon) * (data->rs[0].lon - lon) + (data->rs[0].lat - lat) * (data->rs[0].lat - lat);
  
  //iterate through all the points in data->rs and if a closer point is found
  //set that point to the closest point and that points distance from the target to the shortest distance 
  for (int i = 0; i < data->n; i++) {
    double lastDist = (data->rs[i].lon - lon) * (data->rs[i].lon - lon) + (data->rs[i].lat - lat) * (data->rs[i].lat - lat);
    if (lastDist < minDist) {
      closest = &data->rs[i];
      minDist = lastDist;
    }
  }

  //return the current closest point
  return closest;
}

int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_naive,
                          (free_index_fn)free_naive,
                          (lookup_fn)lookup_naive);
}