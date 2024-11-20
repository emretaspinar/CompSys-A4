#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "coord_query.h"

//keeps track of which index in the array the next node should be placed at,
//in order to not overwrite another node. 
//Starts at -1 since it is incremented at the start of the mk_tree function, before anything else happens
int nodeStorageIndex = -1;

struct node {
  struct record point;
  struct node* left;
  struct node* right;
  int axis;
};

struct kdtree {
  struct node* nodes;
};

//compare functions based on function found here: https://www.tutorialspoint.com/c_standard_library/c_function_qsort.htm
//Function for comparing the lon of two records, used with qsort()
int compareLon(const void* a, const void* b) {
  const struct record* A = a;
  const struct record* B = b;
  return (int)(A->lon - B->lon);
}

//Function for comparing the lat of two records, used with qsort()
int compareLat(const void* a, const void* b) {
  const struct record* A = a;
  const struct record* B = b;
  return (int)(A->lat - B->lat);
}

//function which constructs a kdtree where k=2 from a list of records and places the kdtree in an array
struct node* mk_tree(struct record* points, struct node* nodes, int depth, int size) {

  //exit condition 
  if (size == 0) {
    return NULL;
  }
  //increment array index
  nodeStorageIndex++;
  //arrayindex of the current node
  int currentNode = nodeStorageIndex;

  //find the axis of the current node
  int axis = depth % 2;

  //sort the points based on the axis, 0 = longitude, 1 = latitude
  if (axis == 0) {
    qsort(points, size, sizeof(struct record), compareLon);
  } else {
    qsort(points, size, sizeof(struct record), compareLat);
  }

  //calculate index of the median element of points 
  int median = size/2;

  //calculate the number of elements to the right of the median element, 
  //based on whether points contains an even or odd number of elements
  int newSizeRight;
  if (size % 2 == 0) {
    newSizeRight = median - 1;
  } else {
    newSizeRight = median;
  }

  //set the current nodes point to the median point of points
  nodes[currentNode].point = points[median];

  //set the current nodes axis to the current axis
  nodes[currentNode].axis = axis;

  //recurse down the left side, using the points before the median 
  nodes[currentNode].left = mk_tree(points, nodes, depth+1, median);

  //recurse down the right side, using the points after the median
  nodes[currentNode].right = mk_tree(&points[median+1], nodes, depth+1, newSizeRight);

  //return a pointer to the current node
  return &nodes[currentNode];
}

//function which initializes the tree construction and allocates memory to the tree
struct kdtree* mk_index(struct record* rs, int n) {
  //allocate memory to the kdtree pointer
  struct kdtree* tree = malloc(sizeof(struct kdtree));
  //return NULL if the allocation failed
  if (tree == NULL) {
    return NULL;
  }

  //allocate memory to the kdtree structure.   
  tree->nodes = malloc(n * sizeof(struct node));
  //free the allocated memory for the kdtree pointer and return NULL if the allocation failed
  if (tree->nodes == NULL) {
    free(tree);
    return NULL;
  }

  //construct the kdtree, no need to save the return value since the nodes are put into an array, 
  //with the root being the first element
  mk_tree(rs, tree->nodes, 0, n);
  //reset the the storage index for where nodes should be placed in an array,
  //necessary if you want to make another tree.
  nodeStorageIndex = -1;
  //return the pointer to the tree
  return tree;
}

//free the memory allocated to the tree
void free_kdtree(struct kdtree* data) {
  //free the memory allocated to the tree itself
  free(data->nodes);
  //free the memory allocated to the kdtree struct
  free(data);
}

//closest function based on code found here: https://bitbucket.org/StableSort/play/src/master/src/com/stablesort/kdtree/KDTree.java 
//function which determines whether n1 or n2 is closer to the point (lon, lat)
struct node* closest(struct node* n1, struct node* n2, int lon, int lat) {
  //n1 doesn't exist n2 is closer
  if (n1 == NULL) {
    return n2;
  }

  //n2 doesn't exist n1 is closer
  if (n2 == NULL) {
    return n1;
  }

  //calculate the distance squared between n1 and the point (lon, lat)
  int n1Dist = (n1->point.lon - lon) * (n1->point.lon - lon) + (n1->point.lat - lat) * (n1->point.lat - lat);
  //calculate the distance squared between n2 and the point (lon, lat)
  int n2Dist = (n2->point.lon - lon) * (n2->point.lon - lon) + (n2->point.lat - lat) * (n2->point.lat - lat);

  //return the node which is the shortes distance from the point (lon, lat)
  if (n1Dist < n2Dist) {
    return n1;
  } else {
    return n2;
  }
}

//lookup algorithm based on pseudocode found at 5 minutes in the following youtube video
//double lon, double lat, is the target point
//https://www.youtube.com/watch?v=Glp7THUpGow&ab_channel=StableSort
struct node* lookup(double lon, double lat, struct node* node) {
  //exit condition
  if (node == NULL) {
    return NULL;
  }

  int distAxis;
  struct node* nextBranch;
  struct node* otherBranch;

  if (node->axis == 0) {
    //if the axis is longitude find the distance between the longitude of node and the target point
    distAxis = lon - node->point.lon;
    //if the axis is longitude and the longitude of the target point is less that the longitude of node
    //search the left branch as the primary branch, otherwise search the right branch as the primary branch
    if (lon < node->point.lon) {
      nextBranch = node->left;
      otherBranch = node->right;
    } else {
      nextBranch = node->right;
      otherBranch = node->left;
    }
  } else {
    //if the axis is latitude find the distance between the latitude of node and the target point
    distAxis = lat - node->point.lat;
    //if the axis is latitude and the latitude of the target point is less that the latitude of node
    //search the left branch as the primary branch, otherwise search the right branch as the primary branch
    if (lat < node->point.lat) {
      nextBranch = node->left;
      otherBranch = node->right;
    } else {
      nextBranch = node->right;
      otherBranch = node->left;
    }
  }

  //set the temporary closest node to the result of searching the primary branch
  struct node* temp = lookup(lon, lat, nextBranch);
  //set the candidate closest to the the closer point of the current node and the temp node
  struct node* closestNode = closest(temp, node, lon, lat);

  //calculate the distance squared between the candidate closest point and the target point  
  int radius = (closestNode->point.lon - lon) * (closestNode->point.lon - lon) + (closestNode->point.lat - lat) * (closestNode->point.lat - lat);

  //if the radius is larger than the current axis distance squared search the secondary branch
  if (radius >= distAxis * distAxis) {
    //set the temporary closest node to the result of searching the secondary branch
    temp = lookup(lon, lat, otherBranch);
    //set the closest to the the closer point of the current node and the temp node
    closestNode = closest(temp, closestNode, lon, lat);
  }

  //return the closest node 
  return closestNode;
}

//function which calls the lookup function
const struct record* lookup_kdtree(struct kdtree* data, double lon, double lat) {
  //return NULL if data doesn't exist
  if (data == NULL) {
    return NULL;
  }

  //return the address to the point of the node returned by the lookup
  return &lookup(lon, lat, &data->nodes[0])->point;
}

int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_index,
                          (free_index_fn)free_kdtree,
                          (lookup_fn)lookup_kdtree);
}