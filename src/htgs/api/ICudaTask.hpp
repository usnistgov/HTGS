
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file ICudaTask.hpp
 * @author Timothy Blattner
 * @date Dec 1, 2015
 *
 * @brief ICudaTask.hpp is used to define an NVIDIA Cuda GPU Tasks
 */

#ifdef USE_CUDA
#include <cuda.h>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cuda_runtime_api.h>

#include "ITask.hpp"

#ifndef HTGS_CUDATASK_H
#define HTGS_CUDATASK_H
namespace htgs {

template<class T>
class MemoryData;

/**
 * \class ICudaTask ICudaTask.hpp <htgs/api/ICudaTask.hpp>
 * \brief An ICudaTask is used to attach a task to an NVIDIA Cuda GPU.
 *
 * The task that inherits from this class will automatically be attached to
 * the GPU when launched by the RunTime from within a TaskGraph.
 *
 * An ICudaTask may be bound to one or more GPUs if the task is added into an ExecutionPipeline.
 * The number of CUContexts must match the number of pipeline specified for the ExecutionPipeline.
 *
 * Mechanisms to handle automatic data motion for GPU-to-GPU memories
 * is provided to simplify peer to peer device memory copies.
 * In order to use peer to peer copy, both GPUs must reside on the
 * same IOH (I/O Hub) and be the same GPU model.
 *
 * The automatic data motion function: autoCopy(V destination, std::shared_ptr<MemoryData<V>> data, long numElems)
 *
 * It may be necessary to use the autocopy function if data may reside on two different GPUs.
 * This occurs when there are ghost regions between data domains. If peer to peer copying is allowed
 * between the multiple GPUs, then the autocopy function is not needed. See below for an example of using autocopy.
 *
 * At this time it is necessary for the ICudaTask to copy data from CPU memories to GPU memories.
 *
 * Example implementation:
 * @code
 *
 * #define SIZE 100
 *
 * class SimpleCudaTask : public htgs::ICudaTask<MatrixData, VoidData> {
 * public:
 * SimpleCudaTask(CUcontext *contexts, int *cudaIds, int numGpus) : ICudaTask(contexts, cudaIds, numGpus) { }
 * ~SimpleCudaTask() {}
 * virtual void initializeCudaGPU(CUcontext context, CUstream stream, int cudaId, int numGPUs, int pipelineId,
 *                               int numPipelines)
 * {
 *    // Allocate local GPU memory in initialize will allocate on correct GPU
 *    cudaMalloc(&localMemory, sizeof(double) * SIZE);
 * }
 *
 * virtual void executeGPUTask(std::shared_ptr<MatrixData> data, CUstream stream) {
 *   ...
 *   double * memory;
 *
 *   // Checks if the data received needs to be copied to another GPU
 *   // getCudaMemoryData is defined by the MatrixData class
 *   if (this->autoCopy(localMemory, data->getCudaMemoryData(), SIZE))
 *   {
 *     // Copy was required
 *     memory = localMemory;
 *   }
 *   else
 *   {
 *     // Copy was not required because of peer to peer or same GPU
 *     memory = data->getMemoryData()->get();
 *
 *   }
 *   ...
 * }
 *
 * virtual void shutdownCuda() { cudaFree(localMemory); }
 * virtual void debug() { ... }
 * virtual std::string getName() { return "SimpleCudaTask"; }
 * virtual htgs::ITask<PCIAMData, CCFData> *copy() { return new SimpleCudaTask(...) }
 * virtual bool isTerminated(std::shared_ptr<htgs::BaseConnector> inputConnector) { return inputConnector->isInputTerminated(); }
 *
 * private:
 *   double *localMemory;
 *
 * };
 * @endcode
 *
 * Example usage:
 * @code
 *
 * htgs::TaskGraph<MatrixData, htgs::VoidData> *taskGraph = new htgs::TaskGraph<MatrixData, htgs::VoidData>();
 *
 * SimpleCudaTask *cudaTask = new SimpleCudaTask(...);
 *
 * // Adds cudaTask to process input from taskGraph, input type of cudaTask matches input type of taskGraph
 * taskGraph->addGraphInputConsumer(cudaTask);
 *
 *
 *
 * @endcode
 *
 * @tparam T the input data type for the ICudaTask ITask, T must derive from IData.
 * @tparam U the output data type for the ICudaTask ITask, U must derive from IData.
 */
template<class T, class U>
class ICudaTask: public ITask<T, U> {
  static_assert(std::is_base_of<IData, T>::value, "T must derive from IData");
  static_assert(std::is_base_of<IData, U>::value, "U must derive from IData");

 public:

  /**
   * Creates an ICudaTask.
   * If this task is added into an ExecutionPipeline, then the number of CUcontexts and cudaIds
   * should match the number of pipelines
   *
   * @param contexts the array of CUcontexts
   * @param cudaIds the array of cudaIds
   * @param numGpus the number of GPUs
   */
  ICudaTask(CUcontext *contexts, int *cudaIds, int numGpus) {
    this->contexts = contexts;
    this->cudaIds = cudaIds;
    this->numGpus = numGpus;
  }

  /**
   * Gets the Cuda Id for this cudaTask.
   * Set only after this task has been bound to a thread during initialization.
   * @return the cudaId associated with this cudaTask
   */
  int getCudaId() {
    return this->cudaId;
  }

  /**
   * Gets the CudaContext associated with this task.
   * Set only after this task has been bound to a thread during initialization.
   * @return the Cuda Context
   */
  CUcontext getTaskCudaContext() {
    return this->context;
  }

  /**
   * Gets the cuda Context for the given cudaId.
   *
   * This can be used to copy between two Cuda GPUs by using cudaMemcpyPeerAsync
   *
   * @param index the index
   * @return the CudaContext at the given cudaId
   */
  CUcontext getGPUIdContext(int index) {
    return this->peerContexts.at(index);
  }

  /**
   * Checks if the requested pipelineId requires GPU-to-GPU copy
   * @param pipelineId the ExecutionPipeline id
   * @return whether the requested pipelineId would require a GPU-to-GPU copy
   * @retval TRUE if copy is required
   * @retval FALSE if copy is not required
   */
  bool requiresCopy(int pipelineId) {
    return std::find(this->nonPeerDevIds.begin(), this->nonPeerDevIds.end(), this->cudaIds[pipelineId])
        != this->nonPeerDevIds.end();
  }

  /**
   * Checks if the requested pipelineId requires GPU-to-GPU copy
   * @param data the memory data to check
   * @return whether the requested MemoryData would require GPU-to-GPU copy
   * @retval TRUE if copy is required
   * @retval FALSE if copy is not required
   * @tparam V a type of MemoryData that is allocated using a CudaMemoryManager (created using taskGraph->addCudaMemoryEdge)
   */
  template<class V>
  bool requiresCopy(std::shared_ptr<MemoryData<V>> data) {
    return this->requiresCopy(data->getPipelineId());
  }

  /**
   * Checks if the requested pipelineId allows peer to peer GPU copy
   * @param pipelineId the pipelineId to check
   * @return Whether the pipeline id has peer to peer GPU copy
   * @retval TRUE if the pipeline id has peer to peer GPU copy
   * @retval FALSE if the pipeline id has peer to peer GPU copy
   */
  bool hasPeerToPeerCopy(int pipelineId) { return !requiresCopy(cudaId); }

  /**
   * Will automatically copy from one GPU to another (if it is required).
   *
   * Will check if the data being copied requires to be copied first, and then execute cudaMemcpyPeerAsync
   * if the data requires to be copied.
   *
   * @param destination cuda memory that can be copied into, must be a pointer
   * @param data the source MemoryData that is allocated using a CudaMemoryManager (created using taskGraph->addCudaMemoryEdge)
   * @param numElems the number of elements to be copied
   * @return Whether the copied occurred or not
   * @retval TRUE if the copy was needed
   * @retval FALSE if the copy was not needed
   * @tparam V a type of MemoryData that is allocated using a CudaMemoryManager (created using taskGraph->addCudaMemoryEdge)
   * AND must be a pointer
   */
  template<class V>
  bool autoCopy(V destination, std::shared_ptr<MemoryData<V>> data, long numElems) {
    static_assert(std::is_pointer<V>::value, "V must be a pointer in order to use autoCopy");

    if (requiresCopy(data)) {
      cudaMemcpyPeerAsync((void *) destination,
                          this->cudaId,
                          (void *) data->get(),
                          this->cudaIds[data->getPipelineId()],
                          sizeof(V) * numElems,
                          this->stream);
      return true;
    }
    else {
      return false;
    }
  }

  /**
   * Initializes the CudaTask to be bound to a particular GPU
   * @param pipelineId the pipelineId of the task
   * @param numPipeline the number of pipelines
   * @param ownerTask the owner of the task
   * @param pipelineConnectorList the list of connectors that connect to other duplicate
   * ICudaTask's in an execution pipeline
   *
   * @note This function should only be called by the HTGS API
   */
  void initialize(int pipelineId, int numPipeline, TaskScheduler<T, U> *ownerTask,
                  std::shared_ptr<std::vector<std::shared_ptr<BaseConnector>>> pipelineConnectorList) {
    this->cudaId = this->cudaIds[pipelineId];
    this->context = this->contexts[pipelineId];

    cuCtxSetCurrent(this->context);

    CUstream stream;
    cuStreamCreate(&stream, CU_STREAM_DEFAULT);

    this->stream = stream;

    CUdevice dev;
    cuDeviceGet(&dev, this->cudaId);

    for (int i = 0; i < this->numGpus; i++) {
      CUcontext ctx = this->contexts[i];
      if (ctx != this->context) {
        CUdevice peerDev;
        cuDeviceGet(&peerDev, this->cudaIds[i]);

        int canAccessPeer;
        cuDeviceCanAccessPeer(&canAccessPeer, dev, peerDev);

        if (canAccessPeer == 0) {
          this->nonPeerDevIds.push_back(this->cudaIds[i]);
          this->peerContexts.insert(std::pair<int, CUcontext>(this->cudaIds[i], ctx));
        }
        else {
          cuCtxEnablePeerAccess(ctx, 0);
        }
      }
    }

    this->initializeCudaGPU(this->context, this->stream, this->cudaId, this->numGpus, pipelineId, numPipeline);
  }


  /**
   * Shutsdown the ICudaTask
   *
   * @note This function should only be called by the HTGS API
   */
  void shutdown() {
    this->shutdownCuda();
  }

  /**
   * Executes the ICudaTask on some data
   * @param data the data executed on
   *
   * @note This function should only be called by the HTGS API
   */
  void executeTask(std::shared_ptr<T> data) {
    executeGPUTask(data, this->stream);
  }

  /**
   * Gets the cudaContexts specified during ICudaTask construction
   * @return the cudaContexts
   */
  CUcontext *getContexts() {
    return this->contexts;
  }

  /**
   * Gets the cudaIds specified during ICudaTask construction
   * @return the cudaIds
   */
  int *getCudaIds() {
    return this->cudaIds;
  }

  /**
   * Gets the number of GPUs specified during ICudaTask construction
   * @return the number of GPUs
   */
  int getNumGPUs() {
    return this->numGpus;
  }

  /**
   * Synchronizes the Cuda stream associated with this task.
   *
   * @note Should only be called after initialization
   */
  void syncStream() {
    cudaStreamSynchronize(stream);
  }


  /**
   * Virtual function that is called when the ICudaTask has been initialized.
   * @param context the Cuda GPU context associated with this task
   * @param stream the Cuda stream associated with this task
   * @param cudaId the cuda ID associated with this task
   * @param numGPUs the number GPUs available
   * @param pipelineId the execution pipeline Id
   * @param numPipelines the number of execution pipelines
   */
  virtual void initializeCudaGPU(CUcontext context, CUstream stream, int cudaId, int numGPUs, int pipelineId,
                                 int numPipelines) { }

  /**
   * Pure virtual function that is called when the ICudaTask is executing on data
   * @param data the data to be executed on
   * @param stream the stream to use on the data (it is the same stream passed from initializeCudaGPU)
   */
  virtual void executeGPUTask(std::shared_ptr<T> data, CUstream stream) = 0;

  /**
   * Virtual function that is called when the ICudaTask is shutting down
   */
  virtual void shutdownCuda() {}

  /**
   * Virtual function that gets the name of this ICudaTask
   * @return the name of the ICudaTask
   */
  virtual std::string getName() {
    return "Unnamed GPU ITask";
  }

  /**
   * Pure virtual function that copies this ICudaTask
   * @return the copy of the ICudaTask
   */
  virtual ITask<T, U> *copy() = 0;

  /**
   * Pure virtual function that checks whether this ICudaTask is terminated or not
   * @param inputConnector the input Connector associated with this task
   * @return whether the task can be terminated or not
   * @retval TRUE if the task can be terminated
   * @retval FALSE if the task cannot be terminated
   * @note inputConnector->isInputTerminated() can be used to identify if the input has finished producing data
   */
  virtual bool isTerminated(std::shared_ptr<htgs::BaseConnector> inputConnector) = 0;

  /**
   * Virtual function that can be used to provide debug information.
   */
  virtual void debug() { }

 private:
  CUcontext context; //!< The CUDA GPU context for the ICudaTask (set after initialize)
  CUstream stream; //!< The CUDA stream for the ICudaTask (set after initialize)
  CUcontext *contexts; //!< The array of CUDA contexts (one per GPU)
  int *cudaIds; //!< The array of cuda Ids (one per GPU)

  int numGpus; //!< The number of GPUs
  int cudaId; //!< The CudaID for the ICudaTask (set after initialize)
  std::vector<int> nonPeerDevIds; //!< The list of CudaIds that do not have peer-to-peer access
  std::unordered_map<int, CUcontext> peerContexts; //!< The mapping of CudaId to CUDA Context that has peer-to-peer
};
}
#endif //USE_CUDA
#endif //HTGS_CUDATASK_H

