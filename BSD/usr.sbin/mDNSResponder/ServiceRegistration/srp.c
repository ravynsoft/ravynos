/* srp.c
 *
 * Service Registration Protocol Client
 *
 * Discover the service registration domain
 * Construct a DNS update
 * Include:
 *  Service Name: PTR Record
 *  Service Instance Name: SRV record
 *  Hostname: A, AAAA, KEY records
 * Sign with key using SIG(0)
 */

int
main(int argc, char *argv)
{
    char *host_name = "thread-demo";
    char *service_type "_printer._tcp";
    char *a_record = "127.0.0.1";
    char *aaaa_Record = "::1";
    int prKeyLen = 0;
    int puKeyLen = 0;
    uint8_t *prKey = NULL;
    uint8_t *puKey = NULL;
    dns_message_t *update;
    dns_name_t *hostname;
    dns_name_t *service_name;
    dns_name_t *service_instance_name;
    dns_wire_t message;

    message.id = srp_random16();
    message.bitfield = 0;
    dns_qr_set(message, dns_qr_query);
    dns_opcode_set(message, dns_opcode_update);
    message.qdcount = message.ancount = message.nscount = message.arcount = 0;

    update = dns_message_create();
    tld = dns_make_fqdn(update, "services.arpa");
    dns_update_initialize(update, tld, "2001:1::3");
    hostname = dns_make_fqdn(update, service_name, "services.arpa.");
    service_name = dns_make_fqdn(message, service_type, "services.arpa.");
    service_instance_name = dns_make_fqdn(message, service_name, service_name);

    // _printer._tcp.services.arpa IN PTR thread-demo._printer._tcp.services.arpa
    // thread-demo._printer._tcp.services.arpa IN SRV 0 0 80 thread-demo.services.arpa
    // thread-demo.services.arpa IN A    127.0.0.1
    //                           IN AAAA ::1
    //                           IN KEY ojwefoijweojfwoeijfoiwejfoiwejfoiejf

    dns_update_add(update, hostname, dns_make_a_rr(a_record));
    dns_update_add(update, hostname, dns_make_a_rr(aaaa_record));
    dns_update_add(update, service_instance_name, dns_make_srv_rr(0, 0, 80, hostname));
    dns_update_add(update, service_name, dns_make_ptr_rr(service_instance_name));
    dns_update_add(update, service_name, dns_make_key_rr(puKey, puKeyLen));
    dns_message_to_wire(update);
    dns_message_sign(update, prKey, prKeyLen, puKey, puKeyLen);
    dns_message_send(update);
    dns_message_await_response(update);

    // Get the service name and type
    // Get the hostname
    // Get the key
    // Discover the registration domain (not for Thread)
    // Discover the SRP server (not for Thread)
    // Generate the update
    // Sign the update
    // Send the update
}

// Local Variables:
// mode: C
// tab-width: 4
// c-file-style: "bsd"
// c-basic-offset: 4
// fill-column: 108
// indent-tabs-mode: nil
// End:
