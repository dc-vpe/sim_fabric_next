//
// Created by krw10 on 11/17/2023.
//

#include "../Includes/Collection.h"

List<KeyData *> Collection::GetKeyData()
{
    List<KeyData *> keyData = {};

    for(int ii=0; ii<keys.Count(); ++ii)
    {
        keyData.push_back(Get(keys[ii]));
    }

    return keyData;
}
