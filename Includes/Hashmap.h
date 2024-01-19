//
// Created by krw10 on 6/12/2023.
//
//

#ifndef DSL_HASHMAP_H
#define DSL_HASHMAP_H

#include "U8String.h"
#include "Token.h"

#ifndef BUCKETS
#undef BUCKETS
#endif
#define BUCKETS 512

/// \desc Hashmap bucket, contains the iValues for a single hash map element.
typedef struct HashmapBucketStruct
{
    /// \desc stored key value for this bucket.
    U8String                   *key;
    /// \desc Pointer to the token stored in this bucket, memory owned by the caller to set not the hashmap.
    Token                      *token;
    /// \desc Pointer to the next bucket in the hashmap if more than one key value hashes to the same bucket.
    ///       This value is a nullptr if there is no next bucket. The hash function is chosen to minimize
    ///       buckets with next pointers.
    struct HashmapBucketStruct *next;
} HashmapBucket;

/// \desc Creates an efficient hashmap for quickly looking up tokens.
class Hashmap
{
public:
    /// \desc Creates an empty hashmap.
    Hashmap()
    {
        count   = 0;
        buckets = (HashmapBucket **) calloc(BUCKETS, sizeof(buckets));
        if (buckets == nullptr)
        {
            PrintIssue(2500, "Error, allocating memory for hashmap.\n", true, true);
            return;
        }
    }

    /// \desc Frees up the resources used by the hashmap.
    ~Hashmap()
    {
        Clear();
        free(buckets);
        buckets = nullptr;
    }

    /// \desc Sets a new key in the hashmap with the specified token.
    /// \param  key Pointer to the U8String key.
    /// \param token Pointer to the token to store in the hashmap.
    /// \return True if the key and token are added to the hashmap or false if no memory available.
    bool Set(U8String *key, Token *token)
    {
        size_t        hashed_key = HashFunction(key);
        HashmapBucket *last      = nullptr;
        HashmapBucket *node      = buckets[hashed_key];

        while (node != nullptr)
        {
            if (node->key->IsEqual(key))
            {
                node->token = token;
                return true;
            }
            last = node;
            node = node->next;
        }

        node = (HashmapBucket *) calloc(1, sizeof(HashmapBucket));
        if (node == nullptr)
        {
            PrintIssue(2501, "Allocating memory for hashmap.\n", true, true);
            return false;
        }
        node->key   = new U8String(key);
        node->token = token;
        node->next  = nullptr;
        if (buckets[hashed_key] == nullptr)
        {
            buckets[hashed_key] = node;
        }
        else
        {
            last->next = node;
        }
        ++count;
        return true;
    }


    /// \desc Gets a token stored in the hashmap by its key.
    /// \param key Pointer to the U8String that contains the key.
    /// \return Pointer to the token stored in the hashmap. This is the same pointer that was passed
    /// to the hashmap in set(). If the key is not found NULL is returned.
    Token *Get(U8String *key)
    {
        size_t        hashed_key = HashFunction(key);
        HashmapBucket *entry     = buckets[hashed_key];

        while (entry != nullptr)
        {
            if (entry->key->IsEqual(key))
            {
                return entry->token;
            }
            entry = entry->next;
        }

        return nullptr;
    }


    /// \desc Returns the number of bucketList stored in the hashmap.
    /// \return Total bucketList stored in the hashmap.
    [[maybe_unused]] [[nodiscard]] inline int64_t Count() const
    { return count; }

    /// \desc Checks if the key is stored in the hashmap.
    /// \param key Pointer to the U8String class containing the key to look for.
    /// \return True if the key exists else false.
    bool Exists(U8String *key)
    { return Get(key) != nullptr; }

    /// \desc Removes a key and the value pointer from the hashmap.
    /// \param key Pointer to the U8String that contains the key.
    /// \return True if the key was found and the key and value are successfully removed. If the key
    /// was not found False is returned.
    /// \remark The caller is responsible for freeing the memory used by the token.
    [[maybe_unused]] bool Remove(U8String *key)
    {
        size_t        hashed_key = HashFunction(key);
        HashmapBucket *entry     = buckets[hashed_key];

        if (entry != nullptr)
        {
            if (entry->key->IsEqual(key))
            {
                buckets[hashed_key] = entry->next;
                delete entry->key;
                free(entry);
                return true;
            }

            HashmapBucket *prev = entry;
            entry = entry->next;
            while (entry != nullptr && entry->key->IsEqual(key))
            {
                prev  = entry;
                entry = entry->next;
            }
            if (entry == nullptr)
            {
                return false;
            }
            prev->next = entry->next;
            delete entry->key;
            free(entry);
            return true;
        }

        return false;
    }

    /// \desc Clears all bucketList in the hashmap.
    void Clear()
    {
        for (int64_t ii = 0; ii < BUCKETS; ++ii)
        {
            HashmapBucket *node = buckets[ii];
            while (node != nullptr)
            {
                HashmapBucket *next = node->next;
                delete node->key;
                free(node);
                node = next;
            }
            buckets[ii] = nullptr;
        }
    }

private:
    /// \desc internal array of hashmap buckets, memory managed by the hashmap class.
    HashmapBucket **buckets;

    /// \desc total number of bucketList stored in the hashmap.
    int64_t count;

public:
    /// \desc Jenkins hash function.
    /// \param key The key to search for.</param>
    /// \return The hash value mod against the number of BUCKETS.
    /// \remark The Jenkins hash function us used since it is one of the hash that is quite good at avoiding
    /// key collisions when used with non-deterministic string values.
    static uint32_t HashFunction(U8String *key)
    {
        uint32_t hash = 0;
        for (int64_t ii   = 0; ii < key->Count(); ++ii)
        {
            hash += key->get(ii);
            hash += hash << 10;
            hash ^= hash >> 6;
        }
        hash += hash << 3;
        hash ^= hash >> 11;
        hash += hash << 15;

        return hash % BUCKETS;
    }
};

#endif //DSL_HASHMAP_H
