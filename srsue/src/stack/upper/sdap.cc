/**
 *
 * \section COPYRIGHT
 *
 * Copyright 2013-2021 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "srsue/hdr/stack/upper/sdap.h"

namespace srsue {

sdap::sdap(const char* logname) : logger(srslog::fetch_basic_logger(logname)) {}

bool sdap::init(pdcp_interface_sdap_nr* pdcp_, srsue::gw_interface_pdcp* gw_)
{
  m_pdcp = pdcp_;
  m_gw   = gw_;

  running = true;
  return true;
}

void sdap::stop()
{
  if (running) {
    running = false;
  }
}

void sdap::write_pdu(uint32_t lcid, srsran::unique_byte_buffer_t pdu)
{
  if (!running) {
    return;
  }
  m_gw->write_pdu(lcid, std::move(pdu));
}

void sdap::write_sdu(uint32_t lcid, srsran::unique_byte_buffer_t pdu)
{
  if (!running) {
    return;
  }
  if (lcid < bearers.size()) {
    if (bearers[lcid].add_uplink_header) {
      if (pdu->get_headroom() > 1) {
        pdu->msg -= 1;
        pdu->N_bytes += 1;
        pdu->msg[0] = ((bearers[lcid].is_data ? 1 : 0) << 7) | (bearers[lcid].qfi & 0x3f);
      } else {
        logger.error("Not enough headroom in PDU to add header\n");
      }
    }
  }
  m_pdcp->write_sdu(lcid, std::move(pdu));
}

bool sdap::set_bearer_cfg(uint32_t lcid, const sdap_interface_rrc::bearer_cfg_t& cfg)
{
  if (lcid >= bearers.size()) {
    logger.error("Error setting configuration: invalid lcid=%d\n", lcid);
    return false;
  }
  if (cfg.add_downlink_header) {
    logger.error("Error setting configuration: downlink header not supported\n");
    return false;
  }
  bearers[lcid] = cfg;
  return true;
}

} // namespace srsue
