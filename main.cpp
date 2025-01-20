#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cmath>
// #include <bits/stdc++.h> // Everything in one library for Non-Mac users
using namespace std;

int totHits = 0;
int totMisses = 0;
int offsetBits, indexBits, tagBits; // Number of bits for offset, index and tag

ofstream out("cache_output.txt");

// Extract out bits from start to end
long long int maskInRange(long long int hex, int start, int end)
{
    // hex >> (start) -> erase the bits upto start
    // ((1 << (end-start)) - 1) -> we get end-start set bits(2^(end-start)-1)
    // Taking bitwise and of the two numbers we get the required bits[start, end).
    return ((hex >> (start))) & ((1 << (end - start)) - 1);
}

class block
{
public:
    bool valid; // Each Block has its own valid bit
    int tag;
    bool dirty; // For write-back

    block()
    {
        this->valid = false;
        this->tag = 0;
        this->dirty = false;
    }
    block(long long tag)
    {
        this->valid = true;
        this->tag = tag;
        this->dirty = false;
    }
};

class cache
{
public:
    int cacheSize, blockSize, associativity;
    string replacementPolicy, write;
    int numLines;                    // Number of sets
    vector<vector<block>> dataBlock; // 2D vector to store the cache

    cache(int cacheSize, int blockSize, int associativity, string replacementPolicy, string write)
    {
        this->cacheSize = cacheSize;
        this->blockSize = blockSize;
        this->associativity = associativity;
        this->replacementPolicy = replacementPolicy;
        this->write = write;
        if (associativity != 0)
        {
            this->numLines = cacheSize / (blockSize * associativity); // Number of sets
        }
        else
        {
            this->numLines = 1;                          // Fully Associative
            this->associativity = cacheSize / blockSize; // The whole cache is one set
        }

        this->dataBlock.resize(numLines);
        for (int i = 0; i < numLines; i++)
        {
            this->dataBlock[i].resize(associativity, block()); // Initialize all blocks to invalid
        }
    }

    void access(long long address, int read, long long index, long long tag)
    {
        bool hit = false;
        for (int i = 0; i < associativity; i++)
        {
            if (dataBlock[index][i].valid && dataBlock[index][i].tag == tag)
            {
                hit = true;

                if (read == 1 && this->write == "WB") // Mode: W and WB->Set dirty bit
                    dataBlock[index][i].dirty = true;

                out << "Address: "
                    << "0x" << hex << address << ", Set: "
                    << "0x" << hex << setfill('0') << setw((indexBits + 3) / 4) << index << ", Hit"
                    << ", Tag: "
                    << "0x" << hex << setfill('0') << setw((tagBits + 3) / 4) << tag << endl;

                totHits++;
                if (replacementPolicy == "LRU")
                {
                    // Move the block to the front
                    // block temp = dataBlock[index][i];

                    // dataBlock[index].erase(dataBlock[index].begin() + i);
                    // dataBlock[index].push_back(temp);

                    for (int j = i + 1; j < associativity; j++)
                    {
                        if (dataBlock[index][j].valid)
                            swap(dataBlock[index][j], dataBlock[index][j - 1]); // Getting the hit-block to the end(MRU)
                    }
                }
                break;
            }
        }
        if (!hit)
        {
            out << "Address: "
                << "0x" << hex << address << ", Set: "
                << "0x" << hex << setfill('0') << setw((indexBits + 3) / 4) << index << ", Miss"
                << ", Tag: "
                << "0x" << hex << setfill('0') << setw((tagBits + 3) / 4) << tag << endl;
            totMisses++;
            bool replaced = false;
            if (!(read == 1 && this->write == "WT")) // Mode: W and WT->Don't replace
            {
                for (int i = 0; i < associativity; i++)
                {
                    if (!dataBlock[index][i].valid)
                    {
                        dataBlock[index][i].valid = true;
                        dataBlock[index][i].tag = tag;
                        replaced = true;
                        break;
                    }
                }
                if (!replaced)
                {
                    if (replacementPolicy == "FIFO" || replacementPolicy == "LRU") // Both behave the same way for a miss(As for LRU we move the block to the end)
                    {
                        dataBlock[index].erase(dataBlock[index].begin()); // Remove the first block
                        dataBlock[index].push_back(block(tag));           // Add the new block to the end
                    }
                    else
                    {
                        int randInd = rand() % dataBlock[index].size(); // Randomly select a block to replace
                        dataBlock[index][randInd].tag = tag;            // Replace the block
                    }
                }
            }
        }
    }
};

int main()
{
    ifstream config("cache.config");
    ifstream access("cache.access");

    int cacheSize, blockSize, associativity;

    string replacementPolicy, write;
    config >> cacheSize >> blockSize >> associativity >> replacementPolicy >> write; // Read the config file
    config.close();

    offsetBits = log2(blockSize); // Number of bits for offset

    if (associativity == 0)
        associativity = cacheSize / blockSize; // Fully Associative

    indexBits = log2(cacheSize / (blockSize * associativity));
    tagBits = 32 - offsetBits - indexBits;

    cache cache(cacheSize, blockSize, associativity, replacementPolicy, write);
    string curLine; // Current line in the access file

    if (access.is_open())
    {
        while (getline(access, curLine))
        {
            stringstream myStream(curLine);
            string line;
            myStream >> line;

            int read; // 0 for read, 1 for write
            long long address;
            read = line[0] == 'R' ? 0 : 1; // Read or Write

            myStream >> hex >> address;
            long long index = maskInRange(address, offsetBits, offsetBits + indexBits);
            long long tag = maskInRange(address, offsetBits + indexBits, 32);

            if (indexBits == 0)
                index = 0; // Edge Case: Fully Associative

            cache.access(address, read, index, tag);
        }
    }
    cout << "Total Hits: " << totHits << endl;
    cout << "Total Misses: " << totMisses << endl;
}