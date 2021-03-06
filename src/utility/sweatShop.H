
/******************************************************************************
 *
 *  This file is part of canu, a software program that assembles whole-genome
 *  sequencing reads into contigs.
 *
 *  This software is based on:
 *    'Celera Assembler' (http://wgs-assembler.sourceforge.net)
 *    the 'kmer package' (http://kmer.sourceforge.net)
 *  both originally distributed by Applera Corporation under the GNU General
 *  Public License, version 2.
 *
 *  Canu branched from Celera Assembler at its revision 4587.
 *  Canu branched from the kmer project at its revision 1994.
 *
 *  Modifications by:
 *
 *    Brian P. Walenz beginning on 2018-JUL-20
 *      are a 'United States Government Work', and
 *      are released in the public domain
 *
 *  File 'README.licenses' in the root directory of this distribution contains
 *  full conditions and disclaimers for each license.
 */

#ifndef SWEATSHOP_H
#define SWEATSHOP_H

#include <pthread.h>
#include <semaphore.h>

#include "AS_global.H"

class sweatShopWorker;
class sweatShopState;

class sweatShop {
public:
  sweatShop(void*(*loaderfcn)(void *G),
            void (*workerfcn)(void *G, void *T, void *S),
            void (*writerfcn)(void *G, void *S));
  ~sweatShop();

  void        setNumberOfWorkers(uint32 x) {
    _numberOfWorkers = x;
    _loaderQueueMin  = x * 2;
  };

  void        setThreadData(uint32 t, void *x);

  void        setLoaderBatchSize(uint32 batchSize) { _loaderBatchSize = batchSize; };
  void        setLoaderQueueSize(uint32 queueSize) { _loaderQueueSize = queueSize;  _loaderQueueMax = queueSize; };

  void        setWorkerBatchSize(uint32 batchSize) { _workerBatchSize = batchSize; };

  void        setWriterQueueSize(uint32 queueSize) { _writerQueueSize = queueSize;  _writerQueueMax = queueSize; };

  void        run(void *user=0L, bool beVerbose=false);
private:

  //  Stubs that forward control from the c-based pthread to this class
  friend void  *_sweatshop_loaderThread(void *ss);
  friend void  *_sweatshop_workerThread(void *ss);
  friend void  *_sweatshop_writerThread(void *ss);
  friend void  *_sweatshop_statusThread(void *ss);

  //  The threaded routines
  void   *loader(void);
  void   *worker(sweatShopWorker *workerData);
  void   *writer(void);
  void   *status(void);

  //  Utilities for the loader thread
  //void    loaderAdd(sweatShopState *thisState);
  void    loaderSave(sweatShopState *&tail, sweatShopState *&head, sweatShopState *thisState);
  void    loaderAppend(sweatShopState *&tail, sweatShopState *&head);

  pthread_mutex_t        _stateMutex;

  void                *(*_userLoader)(void *global);
  void                 (*_userWorker)(void *global, void *thread, void *thing);
  void                 (*_userWriter)(void *global, void *thing);

  void                  *_globalUserData;

  sweatShopState        *_writerP;  //  Where output takes stuff from, the tail
  sweatShopState        *_workerP;  //  Where computes happen, the middle
  sweatShopState        *_loaderP;  //  Where input is put, the head

  bool                   _showStatus;

  uint32                 _loaderQueueSize, _loaderQueueMin, _loaderQueueMax;
  uint32                 _loaderBatchSize;
  uint32                 _workerBatchSize;
  uint32                 _writerQueueSize, _writerQueueMax;

  uint32                 _numberOfWorkers;

  sweatShopWorker       *_workerData;

  uint64                 _numberLoaded;
  uint64                 _numberComputed;
  uint64                 _numberOutput;
};

#endif  //  SWEATSHOP_H
