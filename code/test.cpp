#include <stdlib.h>
#include <stdio.h>
#include "momo.h"

#if 0
void test_json() {
  const char json_str[] = "{\
    \"car\": 23, \
    \"bus\": [1,2,3], \
    \"str\": \"hello\", \
    \"student\": { \
      \"id\": 12345, \
      \"name\": \"Gerald\", \
    }\
  }";

  arena_t ba = {}; 
  arena_init(&ba, buffer_set(malloc(megabytes(1)), megabytes(1)); 

  make(json_t, json);
  auto* obj =  json_read(json, (u8_t*)json_str, array_count(json_str), &ba);


  // Printing "car"
  {
    auto* val = json_get_value(obj, str_from_lit("car"));
    if(val) {
      auto* element = json_get_element(val);
      s32_t out = 0;
      str_to_s32(element->str, &out);
      printf("%d\n", out);
    }
  }

  // Printing "str"
  {
    auto* val = json_get_value(obj, str_from_lit("str"));
    if(val) {
      auto* element = json_get_element(val);
      s32_t out = 0;
      str_to_s32(element->str, &out);
      for_cnt(i, element->str.size) {
        printf("%c", element->str.e[i]);
      }
      printf("\n");
    }
  }

#if 0
    json_array_t* arr = json_get_array(val);
    if (arr) {
      for(json_array_node_t* itr = arr->head;
          itr != 0; 
          itr = itr->next) 
      {
        json_value_t* val2 = &itr->value; 
        if (json_is_number(val2)) {
          json_element_t* element = json_get_element(val2);
          s32_t out = 0;
          str_to_s32(element->str, &out);
          printf("%d\n", out);
        }
      }
    }
#endif



  //json_object_t* one = json_get_object(json, str8_from_lit("obj"));
  //u32_t* two = json_get_u32(one, str8_from_lit("item3"));

  //printf("hello: %d", *two);

}
#endif

#if 0
static void
test_print_free_blocks(garena_t* ga) {
  auto* itr = ga->free_list;
  printf("Free Blocks: ");
  while(itr != nullptr) {
    printf("[%llu: %llu] ", ptr_to_umi(itr) - ptr_to_umi(ga->memory), itr->size);
    itr = itr->next;
  }
  printf("\n");
}


static void test_print_memory(garena_t* ga, void* addr, const char* name) 
{
  printf("[%s] %llu\n", name, ptr_to_umi(addr) - ptr_to_umi(ga->memory));
}

static void test_assert_16(void* addr) {
  assert(ptr_to_umi(addr) % 16 == 0);
}

int main() {
  printf("Starting test\n");
  u8_t* memory = (u8_t*)malloc(1000);
  make(garena_t, ga);
  garena_init(ga, memory, 1000);
  test_print_free_blocks(ga);

  printf("Allocating m1 @ 100 bytes\n");
  void* m1 = garena_allocate_size(ga,100);
  test_print_free_blocks(ga);
  printf("Allocating m2 @ 100 bytes\n");
  void* m2 = garena_allocate_size(ga,100);
  test_print_free_blocks(ga);
  printf("Allocating m3 @ 100 bytes\n");
  void* m3 = garena_allocate_size(ga,100);
  test_print_free_blocks(ga);
  printf("Allocating m4 @ 590 bytes\n");
  void* m4 = garena_allocate_size(ga,590);
  test_print_free_blocks(ga);

  test_print_memory(ga, m1, "m1");
  test_print_memory(ga, m2, "m2");
  test_print_memory(ga, m3, "m3");
  test_print_memory(ga, m4, "m4");

  test_assert_16(m1);
  test_assert_16(m2);
  test_assert_16(m3);
  test_assert_16(m4);

  test_print_free_blocks(ga);

  printf("Freeing m2\n");
  garena_free(ga, m2);
  test_print_free_blocks(ga);

  printf("Allocating m2 @ 100 bytes\n");
  m2 = garena_allocate_size(ga,100);
  test_print_free_blocks(ga);

  printf("Freeing m2\n");
  garena_free(ga, m2);
  test_print_free_blocks(ga);

  printf("Freeing m1\n");
  garena_free(ga, m1);
  test_print_free_blocks(ga);

  
  printf("Freeing m4\n");
  garena_free(ga, m4);
  test_print_free_blocks(ga);

  printf("Freeing m3\n");
  garena_free(ga, m3);
  test_print_free_blocks(ga);

}
#endif

struct test_arena_node_t {
  str_t os_buffer;

  str_t buffer;
  usz_t pos; 

  test_arena_node_t* prev;
  test_arena_node_t* next;
};

struct test_arena_t {
  test_arena_node_t sentinel;
  test_arena_node_t* current_block;
};

static void test_arena_init(test_arena_t* a) {
  cll_init(&a->sentinel);
  a->current_block = 0;
}

static void* test_arena_push(test_arena_t* a, usz_t size, usz_t min_allocate_size = 0) 
{
  if (size == 0) return nullptr;

  if (!a->current_block || a->current_block->pos + size > a->current_block->buffer.size) {
    usz_t total_size = max_of(size, min_allocate_size) + sizeof(test_arena_node_t);
    str_t blk = os_allocate_memory(total_size);

    if (!blk) return nullptr;

    auto* node = (test_arena_node_t*)blk.e;
    node->os_buffer = blk;
    node->pos = 0;

    node->buffer = str_set(blk.e + sizeof(test_arena_node_t), size);

    cll_append(&a->sentinel, node);
    a->current_block = node;

  }

 
  void* ret = a->current_block->buffer.e + a->current_block->pos;
  a->current_block->pos += size;
  
  return ret;
  
}


int main() {
  //test_json();
  //test_allocate_memory();
  make(test_arena_t, a);
  test_arena_init(a);
  void* one = test_arena_push(a, 256);
  void* two = test_arena_push(a, 256);
  void* three = test_arena_push(a, 256);

  printf("%p\n", one);
  printf("%p\n", two);
  printf("%p\n", three);

 
}

