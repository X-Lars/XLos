#pragma once

int16 StringCompare(const uint8* stringA, const uint8* stringB)
{
    while (*stringA && *stringA == *stringB)
    {
        stringA++;
        stringB++;
    }

    return *stringA - *stringB;
    
}

uint16 StringLength(const uint8* string)
{
    uint16 result;

    while (*string)
    {
        string++;
        result++;
    }
    
    return result;
}

uint8* StringCopy(const uint8* source, uint8* destination)
{
    for (uint8 i = 0; source[i]; i++)
    {
        destination[i] = source[i];
    }
    
    return destination;
}
