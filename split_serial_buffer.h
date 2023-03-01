//---------------------------------------------------------------------------

#pragma once
#include "javelin/queue.h"
#include "split_tx_rx.h"
#include <stdint.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT

class SplitSerialBuffer {
public:
  static void Add(const uint8_t *data, size_t length) {
    instance.Add(data, length);
  }

  static void RegisterTxHandler() { SplitTxRx::RegisterTxHandler(&instance); }

  static void RegisterRxHandler() {
    SplitTxRx::RegisterRxHandler(SplitHandlerId::SERIAL, &instance);
  }

private:
  struct EntryData {
    size_t length;
    uint8_t data[0];
  };

  struct SplitSerialBufferData : public Queue<EntryData>,
                                 public SplitTxHandler,
                                 public SplitRxHandler {
    void Add(const uint8_t *data, size_t length);

    virtual void UpdateBuffer(TxBuffer &buffer);
    virtual void OnDataReceived(const void *data, size_t length);

    static QueueEntry<EntryData> *CreateEntry(const uint8_t *data,
                                              size_t length);
  };

  static SplitSerialBufferData instance;
};

#else

class SplitSerialBuffer {
public:
  static void Add(const uint8_t *data) {}

  static void RegisterTxHandler() {}
  static void RegisterRxHandler() {}
};

#endif // JAVELIN_SPLIT

//---------------------------------------------------------------------------