//
// Created by tjb3 on 11/13/18.
//

#ifndef HTGS_SIMPLETGTASKTESTS_H
#define HTGS_SIMPLETGTASKTESTS_H

void testTGTasks(bool graphIsConsumer, bool graphIsProducer, int numChain, size_t numThreads);
void testGraphsWithinGraphs(int numGraphs, int numChain, size_t numThreads, bool useExecPipeline = false, size_t numPipelines = 1);
void testTGTaskWithExecPipeline(int numPipelines, int numChain, size_t numThreads);



#endif //HTGS_SIMPLETGTASKTESTS_H
