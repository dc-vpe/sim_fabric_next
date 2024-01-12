//
// Created by krw10 on 11/14/2023.
//

/// \file   Collection.h
///         Creates a hashmap for collection keys.
/// \remark The compiler hash map is setup specifically for the compiler and once the parse is done is
///         no longer used. Collections have to look up key values and return dsl value types
///         at runtime. So the choice is to either use a template hash map or create a custom hash map.
///         Custom map allows this map to be optimized for runtime instead of compile time and avoids
///         having to introduce other complexities because the needs of the runtime collection
///         hash map are different than the needs of the compiler.

#ifndef DSL_CPP_COLLECTION_H
#define DSL_CPP_COLLECTION_H

#ifndef COLLECTION_BUCKETS
#undef COLLECTION_BUCKETS
#endif
#define COLLECTION_BUCKETS 512
#include "../Includes/U8String.h"


/// \desc The Key Data class contains the key and data information for a single element in the hash table.
class KeyData
{
public:

    /// \desc Creates a new empty key data instance.
    KeyData()
    {
        m_key   = new U8String();
        m_keyIndex = -1;
        m_data = nullptr;
    }

    ~KeyData()
    {
        delete m_key;
    }

    /// \desc Gets a reference to the Key value for this key index info.
    U8String *Key() { return m_key; }

    /// \desc Gets the index of the key in the bucketList list.
    [[nodiscard]] int64_t KeyIndex() const { return m_keyIndex; }

    /// \desc Consumer supplied and managed data.
    void *Data()
    {
        if ( m_data == nullptr )
        {
            return nullptr;
        }
        return m_data;
    }

    /// \desc Sets the application controlled data pointer.
    void Data(void *data) { m_data = data; }

    /// \desc Creates this key data instance as a copy of the provided key data.
    /// \param keyData Pointer to the key data to copy.
    explicit KeyData(KeyData *keyData)
    {
        m_key = new U8String(keyData->Key());
        m_keyIndex = keyData->m_keyIndex;
        m_data = keyData->Data();
    }

    /// \desc Creates a new empty key data instance with the provided information.
    KeyData(U8String *key, int64_t keyIndex, void *data)
    {
        m_key = new U8String(key);
        m_keyIndex = keyIndex;
        m_data = data;
    }

private:
    /// \desc U8String key associated with this element.
    U8String *m_key;

    /// \desc Index in the keys list where the actual U8String is stored.
    int64_t  m_keyIndex;

    /// \desc Caller managed data associated with this key value.
    void *m_data;
};

/// \desc Describes the content of a single bucket or element in the bucket list for the hash table.
class CollectionBucket
{
public:
    List<KeyData *> bucketList;

    CollectionBucket()
    {
        bucketList = {};
    }

    ~CollectionBucket()
    {
        for(int ii=0; ii < bucketList.Count(); ++ii)
        {
            delete bucketList[ii];
        }
    }
};

/// \desc Creates an efficient hashmap for quickly looking up keys and dsl values.
class Collection
{
private:
    /// \desc internal array of hashmap buckets, memory managed by the hashmap class.
    CollectionBucket buckets[COLLECTION_BUCKETS];

public:
    List<U8String *>keys;   //List of the unique bucketList in the hash table.
    /// \desc Creates an empty hashmap.
    Collection()
    {
        keys = {};
    }

    /// \desc Frees up the resources used by the hashmap.
    ~Collection()
    {
        Clear();
    }

    /// \desc Sets or updates a key in the hashtable from a class containing the key, index, data.
    /// \param keyData Pointer to the class containing the key, index, and data information.
    bool Set(KeyData *keyData)
    {
        return Set(keyData->Key(), keyData->Data());
    }

    /// \desc Sets a new key in the hashmap with the specified token.
    /// \param key Pointer to the U8String key.
    /// \param data optional data to associated with this key valid pair.
    /// \return True if the key and token are added or false if out of memory.
    bool Set(U8String *key, void *data = nullptr)
    {
        size_t hashed_key = HashFunction(key);
        for(int ii=0; ii<buckets[hashed_key].bucketList.Count(); ++ii)
        {
            if ( buckets[hashed_key].bucketList[ii]->Key()->IsEqual(key))
            {
                buckets[hashed_key].bucketList[ii]->Data(data);
                return true;
            }
        }
        //Key does not exist so add it to the end of the buckets key index info list.
        int64_t keyCount = keys.Count();
        if ( !keys.push_back(key) )
        {
            return false;
        }
        if (!buckets[hashed_key].bucketList.
            push_back(new KeyData(keys[keyCount], keyCount, data)))
        {
            return false;
        }
        return true;
    }

    /// \desc Gets a token stored in the hashmap by its key.
    /// \param key Pointer to the U8String that contains the key.
    /// \return Pointer to the KeyData class containing the information about the key, value, pair data, or
    ///         nullptr if the key does not exist om the hashmap.
    KeyData *Get(U8String *key)
    {
        size_t hashed_key = HashFunction(key);

        for(int ii=0; ii<buckets[hashed_key].bucketList.Count(); ++ii)
        {
            if ( buckets[hashed_key].bucketList[ii]->Key()->IsEqual(key))
            {
                return buckets[hashed_key].bucketList[ii];
            }
        }
        return nullptr;
    }

    /// \desc Returns the number of bucketList stored in the hashmap.
    /// \return Total bucketList stored in the hashmap.
    int64_t Count() { return keys.Count();  }

    /// \desc Gets a list of all of the key and data information in the collection.
    List<KeyData *> GetKeyData();

    /// \desc Checks if the key is stored in the hashmap.
    /// \param key Pointer to the U8String class containing the key to look for.
    /// \return True if the key exists else false.
    [[maybe_unused]] bool Exists(U8String *key) { return Get(key) != nullptr; }

    /// \desc Removes the key index value from an element in the bucket list.
    /// \param key Pointer to the U8String that contains the key.
    /// \return True if the key and it's index was removed, else false if the key was not removed.
    [[maybe_unused]] bool Remove(U8String *key)
    {
        size_t hashed_key = HashFunction(key);
        for(int ii=0; ii<buckets[hashed_key].bucketList.Count(); ++ii)
        {
            if ( buckets[hashed_key].bucketList[ii]->Key()->IsEqual(key))
            {
                KeyData *tmp = buckets[hashed_key].bucketList[ii];
                buckets[hashed_key].bucketList.Remove(ii);
                keys.Remove(tmp->KeyIndex());
                delete []tmp;
                break;
            }
        }

        return false;
    }

    /// \desc Sets the number of keys in the bucket list to 0, does not free the memory used by
    /// the key index info class and its list used in each bucket so that it can be reused.
    void Clear()
    {
        keys.Clear();

        for(auto & bucket : buckets)
        {
            bucket.bucketList.Clear();
        }
    }

    /// \desc Copies the provided collection information to this collection.
    /// \param source Collection to copy to this collection.
    /// \return True if successful or false if out of memory.
    bool CopyFrom(Collection *source)
    {
        Clear();

        for(int64_t ii=0; ii < source->Count(); ++ii)
        {
            U8String *s = source->keys[ii];

            if ( !Set(source->Get(s)) )
            {
                return false;
            }
        }

        return true;
    }

private:
    /// \desc Key index information used when getting a list of the key index information in the collection.

    /// \desc Jenkins hash function.
    /// \param key The key to search for.</param>
    /// \return The hash value mod against the number of BUCKETS.
    /// \remark The Jenkins hash function us used since it is one of the hash that is quite good at avoiding
    /// key collisions when used with non-deterministic string values.
    static uint32_t HashFunction(U8String *key)
    {
        uint32_t hash = 0;
        for (int64_t ii = 0; ii < key->Count(); ++ii)
        {
            //Walk backwards as the variable values change the most from the end of the value.
            hash += key->get(ii);
            hash += hash << 10;
            hash ^= hash >> 6;
        }
        hash += hash << 3;
        hash ^= hash >> 11;
        hash += hash << 15;

        return hash % COLLECTION_BUCKETS;
    }
};

#endif //DSL_CPP_COLLECTION_H
