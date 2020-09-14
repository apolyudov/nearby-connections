// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef CORE_V2_INTERNAL_BWU_HANDLER_H_
#define CORE_V2_INTERNAL_BWU_HANDLER_H_

#include "core_v2/internal/client_proxy.h"
#include "core_v2/internal/endpoint_channel.h"
#include "core_v2/internal/offline_frames.h"
#include "platform_v2/public/count_down_latch.h"
#include "proto/connections_enums.pb.h"

namespace location {
namespace nearby {
namespace connections {

using BwuNegotiationFrame = BandwidthUpgradeNegotiationFrame;

// Defines the set of methods that need to be implemented to handle the
// per-Medium-specific operations needed to upgrade an EndpointChannel.
class BwuHandler {
 public:
  using UpgradePathInfo = parser::UpgradePathInfo;

  virtual ~BwuHandler() = default;

  // Called by the Initiator to setup the upgraded medium for this endpoint (if
  // that hasn't already been done), and returns a serialized UpgradePathInfo
  // that can be sent to the Responder.
  // @BwuHandlerThread
  virtual ByteArray InitializeUpgradedMediumForEndpoint(
      ClientProxy* client, const std::string& service_id,
      const std::string& endpoint_id) = 0;
  // Called to revert any state changed by the Initiator to setup the upgraded
  // medium for an endpoint.
  // @BwuHandlerThread
  virtual void Revert() = 0;

  // Called by the Responder to setup the upgraded medium for this endpoint (if
  // that hasn't already been done) using the UpgradePathInfo sent by the
  // Initiator, and returns a new EndpointChannel for the upgraded medium.
  // @BwuHandlerThread
  virtual std::unique_ptr<EndpointChannel> CreateUpgradedEndpointChannel(
      ClientProxy* client, const std::string& service_id,
      const std::string& endpoint_id,
      const UpgradePathInfo& upgrade_path_info) = 0;
  // Returns the upgrade medium of the BwuHandler.
  // @BwuHandlerThread
  virtual Medium GetUpgradeMedium() const = 0;
  virtual void OnEndpointDisconnect(ClientProxy* client,
                                    const std::string& endpoint_id) = 0;

  class IncomingSocket {
   public:
    virtual ~IncomingSocket() = default;

    virtual std::string ToString() = 0;
    virtual void Close() = 0;
  };

  struct IncomingSocketConnection {
    std::unique_ptr<IncomingSocket> socket;
    std::unique_ptr<EndpointChannel> channel;
  };

  struct BwuNotifications {
    std::function<void(ClientProxy* client,
                       IncomingSocketConnection* connection)>
        incoming_connection_cb;
  };
};

}  // namespace connections
}  // namespace nearby
}  // namespace location

#endif  // CORE_V2_INTERNAL_BWU_HANDLER_H_
