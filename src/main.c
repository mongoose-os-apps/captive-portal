#include "mgos.h"

static const char *s_our_ip = "192.168.4.1";

static void ev_handler(struct mg_connection *c, int ev, void *ev_data,
                       void *user_data) {
  struct mg_dns_message *msg = (struct mg_dns_message *) ev_data;
  struct mbuf reply_buf;
  int i;

  if (ev != MG_DNS_MESSAGE) return;

  mbuf_init(&reply_buf, 512);
  struct mg_dns_reply reply = mg_dns_create_reply(&reply_buf, msg);
  for (i = 0; i < msg->num_questions; i++) {
    char rname[256];
    struct mg_dns_resource_record *rr = &msg->questions[i];
    mg_dns_uncompress_name(msg, &rr->name, rname, sizeof(rname) - 1);
    fprintf(stdout, "Q type %d name %s\n", rr->rtype, rname);
    if (rr->rtype == MG_DNS_A_RECORD) {
      uint32_t ip = inet_addr(s_our_ip);
      mg_dns_reply_record(&reply, rr, NULL, rr->rtype, 10, &ip, 4);
    }
  }
  mg_dns_send_reply(c, &reply);
  mbuf_free(&reply_buf);
  (void) user_data;
}

enum mgos_app_init_result mgos_app_init(void) {
  struct mg_connection *c = mg_bind(mgos_get_mgr(), "udp://:53", ev_handler, 0);
  mg_set_protocol_dns(c);
  return MGOS_APP_INIT_SUCCESS;
}
