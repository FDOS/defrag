#ifndef AALOC_H_
#define AALOC_H_

void* amalloc(size_t size);
void* acalloc(size_t num_elements, size_t size);

void autofree();

#endif
