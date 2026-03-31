/* dnssd-proxy.h
 *
 * Copyright (c) 2018 Apple Computer, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Discovery Proxy globals.
 */

void dp_formerr(comm_t *NONNULL comm);
bool dp_served(dns_name_t *NONNULL name);
void dp_query(comm_t *NONNULL comm, unsigned offset, dns_rr_t *NONNULL question);



