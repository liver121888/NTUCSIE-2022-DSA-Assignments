#define _CRT_SECURE_NO_DEPRECATE
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
#include"price.h"

int FileFlag = 1;

typedef struct query
{
    int stockFlag;
    int sweetK;
    int currentCount; // counter of sequential draws for this query toward to the target sweetK
    unsigned long long answer;
} Query;

typedef struct segmentNode
{
    unsigned long long value;
    int seqID;   //sequence index;  unsigned int should be OK to save memory
} SegmentNode;

typedef struct stockNode
{
    int stockID; // unsigned int should be OK to save memory
    unsigned char extra; // =1: extra, 0 not extra; basic
    SegmentNode** segmentHeapArray;
} StockNode;

// Instead of using an object array whose orders are fixed, use pointer array for easier heapify operation
// During the operation, only swap the pointers is required
StockNode** stockHeapArray;

int numOfStocks, numQuery, increasePeriod, totalNumber, arraySize;
unsigned long long kSweet, extra;
unsigned long long* stockIDs, * activeIDs;
Query* queryArray;


char answer[80];
FILE* filePtr;


// Construct the first heap array of StockNode according to their first min values.
// It will be constructed as a min-heap, where the root is the smallest and all children are larger than their parents.
// Once completed the root is the smallest node; yet the tree is not sorted (we don't need a sorted heap)!
void StockHeapArrayFirstMinHipified()
{
    StockNode* temp;
    // Start from last parent backward to do min heapify operation
    for (int parent = totalNumber / 2; parent >= 0; parent--)
    {
        int parentID = parent;
        int childID = parentID * 2 + 1;

        while (childID < totalNumber) // child is traversed one by one
        {
            // Select the child with the smaller value
            if (childID + 1 < totalNumber && stockHeapArray[childID + 1]->segmentHeapArray[0]->value < stockHeapArray[childID]->segmentHeapArray[0]->value)
                childID++; // second child is smaller than first child
            if (stockHeapArray[parentID]->segmentHeapArray[0]->value <= stockHeapArray[childID]->segmentHeapArray[0]->value)
                break; // Done! Since the parent is smaller or equal to the smaller child
            else
            {
                // Let the smaller child turn parent, and downgrade the parent to the child
                // Swap child and parent
                temp = stockHeapArray[parentID];
                stockHeapArray[parentID] = stockHeapArray[childID];
                stockHeapArray[childID] = temp;
                // Since parent and child are swapped, we need to traverse down further
                parentID = childID;
                childID = parentID * 2 + 1;
            }
        }
    }
}


void SegmentHeapArrayFirstMinHeapified(SegmentNode** segmentHeap)
{
    SegmentNode* temp;
    // Start from last parent backward to do min heapify operation
    for (int parent = increasePeriod / 2; parent >= 0; parent--)
    {
        int parentID = parent;
        int childID = parentID * 2 + 1;

        while (childID < increasePeriod) // child is traversed one by one
        {
            // Select the child with the smaller value
            if (childID + 1 < increasePeriod && segmentHeap[childID + 1]->value < segmentHeap[childID]->value)
                childID++; // second child is smaller than first child
            if (segmentHeap[parentID]->value <= segmentHeap[childID]->value)
                break; // Done! Since the parent is smaller or equal to the smaller child
            else
            {
                // Let the smaller child turn parent, and downgrade the parent to the child
                // Swap child and parent
                temp = segmentHeap[parentID];
                segmentHeap[parentID] = segmentHeap[childID];
                segmentHeap[childID] = temp;
                // Since parent and child are swapped, we need to traverse down further
                parentID = childID;
                childID = parentID * 2 + 1;
            }
        }
    }
}





int r;
void main()
{

    FILE* filePtr = 0;
    if (FileFlag)
    {
        filePtr = fopen("D:\\Senior_Spring\\DSA\\NTUCSIE-2022-DSA-Assignments\\HW2\\HW2\\hw2_testdata\\P5\\4.in", "r");
        r = fscanf(filePtr, "%d %d %d", &numOfStocks, &numQuery, &increasePeriod);
    }
    else
    {
        r = scanf("%d %d %d", &numOfStocks, &numQuery, &increasePeriod);
    }
    // Allocate memory of the basic stocks
    StockNode* basticStocks = malloc(numOfStocks * sizeof(StockNode));

    for (int i = 0; i < numOfStocks; i++)
    {
        // Get stock id for each stock node object
        if (FileFlag)
        {
            r = fscanf(filePtr, "%d", &basticStocks[i].stockID); // or %I64u

        }
        else
        {
            r = scanf("%d", &basticStocks[i].stockID); // or %
        }
        basticStocks[i].extra = 0;
        // Allocate segment pointer heap array for each basic stock node
        basticStocks[i].segmentHeapArray = malloc(increasePeriod * sizeof(SegmentNode*));
    }

    // Preview all queries of this problem and store their parameters
    queryArray = malloc(sizeof(Query) * numQuery);

    int numberOfExtraStrocks = 0;
    // We need to store all of the extra stock IDs in this problem
    int* extraIDArray = malloc(numQuery * sizeof(int));

    // Pre-read all queries and store their information at query array
    for (int j = 0; j < numQuery; j++)
    {
        if (FileFlag)
        {
            r = fscanf(filePtr, "%d %d", &queryArray[j].stockFlag, &queryArray[j].sweetK);
        }
        else
        {
            r = scanf("%d %d", &queryArray[j].stockFlag, &queryArray[j].sweetK);
        }
        queryArray[j].currentCount = 0;
        // If an extra stock is introduced, check whether it has appeared before
        if (queryArray[j].stockFlag != 0)
        {
            int ok = 1;
            // Check duplication of this extra stock
            for (int z = 0; z < numberOfExtraStrocks; z++)
            {
                if (queryArray[j].stockFlag == extraIDArray[z])
                {
                    ok = 0;
                    break;
                }
            }
            if (ok)
            {
                // a new extra stock; store its id
                extraIDArray[numberOfExtraStrocks++] = queryArray[j].stockFlag;
            }
        }
    }

    // Now we can setup all involved stock nodes
    totalNumber = numOfStocks + numberOfExtraStrocks;
    stockHeapArray = malloc(totalNumber * sizeof(StockNode*));
    // Copy the addresses of the allocated basic stock nodes
    for (int i = 0; i < numOfStocks; i++)
        stockHeapArray[i] = &basticStocks[i];
    for (int i = numOfStocks; i < totalNumber; i++)
    {
        // For extra stock we need to allocate memory first
        stockHeapArray[i] = malloc(sizeof(StockNode));
        stockHeapArray[i]->stockID = extraIDArray[i - numOfStocks];
        stockHeapArray[i]->extra = 1;
        // Allocate segment pointer heap array for each extra stock node
        stockHeapArray[i]->segmentHeapArray = malloc(increasePeriod * sizeof(SegmentNode*));
    }

    // Now build up the inner segment heap arrays of all of the involved stock nodes
    for (int i = 0; i < totalNumber; i++)
    {
        for (int p = 0; p < increasePeriod; p++)
        {
            // Allocate memory for each segment of a stock
            stockHeapArray[i]->segmentHeapArray[p] = malloc(sizeof(SegmentNode));
            stockHeapArray[i]->segmentHeapArray[p]->seqID = p + 1; // 1 + p * increasePeriod;
            stockHeapArray[i]->segmentHeapArray[p]->value = price(stockHeapArray[i]->stockID, stockHeapArray[i]->segmentHeapArray[p]->seqID);
        }
        // build up the min-heap of the inner segment nodes for this stock node
        SegmentHeapArrayFirstMinHeapified(stockHeapArray[i]->segmentHeapArray);
    }

    // Then build up the min-heap of the outer stock nodes
    StockHeapArrayFirstMinHipified();

    // Now start draw the current minimal segment of the current minimal stock, until all queries are solved
    int numberSolved = 0;

    SegmentNode* segmentTemp;
    StockNode* stockTemp;
    //unsigned long long minValue;
    //unsigned long long minStockID;

    int nodeReduced;
    int drawCount = 0;
    while (numberSolved != numQuery)
    {

    RootChanged:

        nodeReduced = 0;
        // loop through all queries to update their counters
        for (int q = 0; q < numQuery; q++)
        {
            if (queryArray[q].currentCount == queryArray[q].sweetK) continue; // query is solved

            if (stockHeapArray[0]->extra == 0)
            {
                // Basic stock, all counter forward by one
                queryArray[q].currentCount++;
            }
            else
            {
                // Extra stock, only the query have this extra stock need to forward its counter
                if (stockHeapArray[0]->stockID == queryArray[q].stockFlag)
                    queryArray[q].currentCount++;
            }

            if (queryArray[q].currentCount == queryArray[q].sweetK)
            {
                queryArray[q].answer = stockHeapArray[0]->segmentHeapArray[0]->value;;
                numberSolved++;//One query is solved

                // If this query is an extra stock; we need to remove this stock node. 
                // Otherwise, useless draws will add up solving time
                if (stockHeapArray[0]->extra != 0)
                {
                    nodeReduced = 1;
                }
            }
        }

        if (nodeReduced == 1)
        {
            // Replace the root node with the last node
            stockHeapArray[0] = stockHeapArray[totalNumber - 1];
            // Reduce the number of stock nodes
            totalNumber--;
            // Re min-heapify the stock node heap array
            StockHeapArrayFirstMinHipified();
            goto RootChanged;
        }

        // The smallest Stock node upgrades its heap to the next value
        stockHeapArray[0]->segmentHeapArray[0]->seqID += increasePeriod; // day id jump to next increased day
        unsigned long long v = price(stockHeapArray[0]->stockID, stockHeapArray[0]->segmentHeapArray[0]->seqID); // get upgraded value
        stockHeapArray[0]->segmentHeapArray[0]->value = v; // update value

        // Top-down min heapify the nested min-heap of the stock node
        int parentID = 0; // The root is upgraded
        int childID = 1;
        while (childID < increasePeriod) // child is traversed one by one
        {
            // Select the child with the smaller value
            if (childID + 1 < increasePeriod && stockHeapArray[0]->segmentHeapArray[childID + 1]->value < stockHeapArray[0]->segmentHeapArray[childID]->value)
                childID++; // second child is smaller than first child
            if (stockHeapArray[0]->segmentHeapArray[parentID]->value <= stockHeapArray[0]->segmentHeapArray[childID]->value)
            {
                break; // Done! Since the parent is smaller or equal to the smaller child
            }
            else
            {
                // Let the smaller child turn parent, and downgrade the parent to the child
                // Swap child and parent
                segmentTemp = stockHeapArray[0]->segmentHeapArray[parentID];
                stockHeapArray[0]->segmentHeapArray[parentID] = stockHeapArray[0]->segmentHeapArray[childID];
                stockHeapArray[0]->segmentHeapArray[childID] = segmentTemp;
                // Since parent and child are swapped, we need to traverse down further
                parentID = childID;
                childID = parentID * 2 + 1;
            }
        }

        // The root stock has upgraded its heap, now hepify the outer stock node heap
        parentID = 0; // The root stock has upgraded
        childID = 1;
        // Top-down min heapify the outer stock heap
        while (childID < totalNumber) // child is traversed one by one
        {
            // Select the child with the smaller value
            if (childID + 1 < totalNumber && stockHeapArray[childID + 1]->segmentHeapArray[0]->value < stockHeapArray[childID]->segmentHeapArray[0]->value)
                childID++; // second child is smaller than first child
            if (stockHeapArray[parentID]->segmentHeapArray[0]->value <= stockHeapArray[childID]->segmentHeapArray[0]->value)
            {
                break; // Done! Since the parent is smaller or equal to the smaller child
            }
            else
            {
                // Let the smaller child turn parent, and downgrade the parent to the child
                // Swap child and parent
                stockTemp = stockHeapArray[parentID];
                stockHeapArray[parentID] = stockHeapArray[childID];
                stockHeapArray[childID] = stockTemp;
                // Since parent and child are swapped, we need to traverse down further
                parentID = childID;
                childID = parentID * 2 + 1;
            }
        }
        drawCount++;
    }

    // Print out answers for all queries
    for (int i = 0; i < numQuery; i++)
    {
        printf("%llu\n", queryArray[i].answer);
    }

    if (FileFlag)
    {
        fclose(filePtr);
    }
    return;
}