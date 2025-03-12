#ifndef LIBKIWI_H
#define LIBKIWI_H

#include <libkiwi/core/kiwiColor.h>
#include <libkiwi/core/kiwiConsoleOut.h>
#include <libkiwi/core/kiwiController.h>
#include <libkiwi/core/kiwiDvdStream.h>
#include <libkiwi/core/kiwiFileRipper.h>
#include <libkiwi/core/kiwiIBinary.h>
#include <libkiwi/core/kiwiIScene.h>
#include <libkiwi/core/kiwiIStream.h>
#include <libkiwi/core/kiwiJSON.h>
#include <libkiwi/core/kiwiMemStream.h>
#include <libkiwi/core/kiwiMemoryMgr.h>
#include <libkiwi/core/kiwiMessage.h>
#include <libkiwi/core/kiwiNandStream.h>
#include <libkiwi/core/kiwiRuntime.h>
#include <libkiwi/core/kiwiSPR.h>
#include <libkiwi/core/kiwiSceneCreator.h>
#include <libkiwi/core/kiwiSceneHookMgr.h>
#include <libkiwi/core/kiwiThread.h>
#include <libkiwi/crypt/kiwiBase64.h>
#include <libkiwi/crypt/kiwiChecksum.h>
#include <libkiwi/crypt/kiwiSHA1.h>
#include <libkiwi/debug/kiwiAssert.h>
#include <libkiwi/debug/kiwiGeckoDebugger.h>
#include <libkiwi/debug/kiwiIDebugger.h>
#include <libkiwi/debug/kiwiMapFile.h>
#include <libkiwi/debug/kiwiNw4rConsole.h>
#include <libkiwi/debug/kiwiNw4rDirectPrint.h>
#include <libkiwi/debug/kiwiNw4rException.h>
#include <libkiwi/debug/kiwiStackChecker.h>
#include <libkiwi/debug/kiwiTextWriter.h>
#include <libkiwi/fun/kiwiGameCorruptor.h>
#include <libkiwi/math/kiwiAlgorithm.h>
#include <libkiwi/net/kiwiAsyncSocket.h>
#include <libkiwi/net/kiwiEmuRichPresenceClient.h>
#include <libkiwi/net/kiwiHttpRequest.h>
#include <libkiwi/net/kiwiIRichPresenceClient.h>
#include <libkiwi/net/kiwiPacket.h>
#include <libkiwi/net/kiwiReliableClient.h>
#include <libkiwi/net/kiwiReliablePacket.h>
#include <libkiwi/net/kiwiReliableSocket.h>
#include <libkiwi/net/kiwiRichPresenceMgr.h>
#include <libkiwi/net/kiwiSocketBase.h>
#include <libkiwi/net/kiwiSyncSocket.h>
#include <libkiwi/net/kiwiWebSocket.h>
#include <libkiwi/prim/kiwiArray.h>
#include <libkiwi/prim/kiwiBitCast.h>
#include <libkiwi/prim/kiwiHashMap.h>
#include <libkiwi/prim/kiwiLinkList.h>
#include <libkiwi/prim/kiwiOptional.h>
#include <libkiwi/prim/kiwiPair.h>
#include <libkiwi/prim/kiwiSTL.h>
#include <libkiwi/prim/kiwiSmartPtr.h>
#include <libkiwi/prim/kiwiString.h>
#include <libkiwi/prim/kiwiVector.h>
#include <libkiwi/support/kiwiLibGX.h>
#include <libkiwi/support/kiwiLibOS.h>
#include <libkiwi/support/kiwiLibSO.h>
#include <libkiwi/util/kiwiAutoLock.h>
#include <libkiwi/util/kiwiBitUtil.h>
#include <libkiwi/util/kiwiBuildTarget.h>
#include <libkiwi/util/kiwiDynamicSingleton.h>
#include <libkiwi/util/kiwiExtension.h>
#include <libkiwi/util/kiwiGlobalInstance.h>
#include <libkiwi/util/kiwiIosDevice.h>
#include <libkiwi/util/kiwiIosObject.h>
#include <libkiwi/util/kiwiIosVector.h>
#include <libkiwi/util/kiwiNonCopyable.h>
#include <libkiwi/util/kiwiPtrUtil.h>
#include <libkiwi/util/kiwiRandom.h>
#include <libkiwi/util/kiwiStaticSingleton.h>
#include <libkiwi/util/kiwiWatch.h>
#include <libkiwi/util/kiwiWorkBuffer.h>

// Order important from here

#include <libkiwi/k_types.h>
#endif
