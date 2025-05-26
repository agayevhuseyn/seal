#ifndef SEAL_HASHMAP_H
#define SEAL_HASHMAP_H

#include "seal.h"
#include "sealtypes.h"

#define __hashmap_error(...) do { \
  fprintf(stderr, "error: "); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n"); \
  exit(1); \
} while (0)

struct h_entry {
  unsigned int hash;
  const char* key;
  svalue_t val;
  bool is_tombstone;
};

typedef struct hashmap {
  struct h_entry* entries;
  size_t cap;
  size_t filled;
} hashmap_t;

static inline unsigned int hash_str(const char* key) {
  unsigned int hash = 0;
  while (*key)
    hash = *key++ + (hash << 6) + (hash << 16) - hash;

  return hash;
}

static inline void hashmap_init(hashmap_t* hashmap, size_t size)
{
  hashmap->cap = size;
  hashmap->entries = SEAL_CALLOC(size, sizeof(struct h_entry));
  for (int i = 0; i < size; i++) {
    hashmap->entries[i].key = NULL;
    hashmap->entries[i].is_tombstone = false;
  }
}

static inline void hashmap_init_static(hashmap_t* hashmap, struct h_entry* entries, size_t size)
{
  hashmap->cap = size;
  hashmap->entries = entries;
  for (int i = 0; i < size; i++) {
    hashmap->entries[i].key = NULL;
    hashmap->entries[i].is_tombstone = false;
  }
}

static inline struct h_entry* hashmap_search(hashmap_t* hashmap, const char* key)
{
  unsigned int idx = hash_str(key) % hashmap->cap;
  struct h_entry* tombstone = NULL;

  struct h_entry* e;
  unsigned int start_idx = idx;
  do {
    e = &hashmap->entries[idx];
    if (e->key == NULL) {
      if (e->is_tombstone) {
        if (tombstone == NULL)
          tombstone = e;
      } else {
        return tombstone != NULL ? tombstone : e;
      }
    } else if (strcmp(e->key, key) == 0) {
      return e;
    }
    idx = (idx + 1) % hashmap->cap;
  } while (idx != start_idx);

  return NULL;
}

static inline bool hashmap_insert(hashmap_t* hashmap, const char* key, svalue_t val)
{
  if (hashmap->filled >= hashmap->cap)
    __hashmap_error("hashmap is full");

  struct h_entry* searched = hashmap_search(hashmap, key);
  
  struct h_entry e = {
    hash_str(key),
    key,
    val,
    true
  };

  bool is_new = searched->key == NULL;
  if (is_new)
    hashmap->filled++;

  *searched = e;

  return is_new;
}

static inline bool hashmap_insert_e(hashmap_t* hashmap, struct h_entry* entry, const char* key, svalue_t val)
{
  struct h_entry e = {
    hash_str(key),
    key,
    val,
    true
  };

  bool is_new = entry->key == NULL;
  if (is_new)
    hashmap->filled++;

  *entry = e;

  return is_new;
}

static inline bool hashmap_remove(hashmap_t* hashmap, const char* key)
{
  if (hashmap->filled <= 0)
    __hashmap_error("hashmap is empty");

  struct h_entry* searched = hashmap_search(hashmap, key);

  if (searched->key == NULL)
    return false;

  searched->is_tombstone = true;
  searched->key = NULL;

  hashmap->filled--;

  return true;
}

#endif /* SEAL_HASHMAP_H */
