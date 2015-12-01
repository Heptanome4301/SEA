#ifndef FREE_PAGES_H
#define FREE_PAGES_H

/*******************************************/
/************* Structs definition **********/
/*******************************************/
typedef struct free_pages_descriptor_s {
  uint32_t length;
  uint8_t* thePageAddress;
  struct free_pages_descriptor_s* next;
} free_pages_descriptor_t;


#endif
