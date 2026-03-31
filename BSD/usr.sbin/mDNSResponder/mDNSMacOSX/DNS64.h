/*
 * Copyright (c) 2017-2019 Apple Inc. All rights reserved.
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
 */

#ifndef __DNS64_h
#define __DNS64_h

#include "mDNSEmbeddedAPI.h"

#define DNS64IsQueryingARecord(STATE)       (((STATE) == kDNS64State_QueryA) || ((STATE) == kDNS64State_QueryA2))
#define DNS64ShouldAnswerQuestion(Q, RR)    (DNS64IsQueryingARecord((Q)->dns64.state) && ((RR)->rrtype == kDNSType_A))

#ifdef  __cplusplus
extern "C" {
#endif

mDNSexport mDNSBool DNS64StateMachine(mDNS *m, DNSQuestion *inQ, const ResourceRecord *inRR, QC_result inResult);
mDNSexport mStatus  DNS64AnswerCurrentQuestion(mDNS *m, const ResourceRecord *inRR, QC_result inResult);
mDNSexport void     DNS64HandleNewQuestion(mDNS *m, DNSQuestion *inQ);
mDNSexport void     DNS64ResetState(DNSQuestion *inQ);
mDNSexport void     DNS64RestartQuestions(mDNS *m);

#ifdef  __cplusplus
}
#endif

#endif // __DNS64_h
