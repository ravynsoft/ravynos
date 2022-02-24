# wayland-protocols governance

This document governs the maintenance of wayland-protocols and serves to outline
the broader process for standardization of protocol extensions in the Wayland
ecosystem.

## 1. Membership

Membership in wayland-protocols is offered to stakeholders in the Wayland
ecosystem who have an interest in participating in protocol extension
standardization.

### 1.1. Membership requirements

1. Membership is extended to projects, rather than individuals.
2. Members represent general-purpose projects with a stake in multiple Wayland
   protocols (e.g. compositors, GUI toolkits, etc), rather than special-purpose
   applications with a stake in only one or two.
3. Each project must provide one or two named individuals as points-of-contact
   for that project who can be reached to discuss protocol-related matters.
4. During a vote, if two points-of-contact for the same member disagree, the
   member's vote is considered blank.

### 1.2. Becoming a member

1. New members who meet the criteria outlined in 1.1 are established by
   invitation from an existing member. Projects hoping to join should reach out
   to an existing member asking for this invitation.
2. New members shall write to the wayland-devel mailing list stating their
   intention of joining and their sponsor.
3. The sponsor shall respond acknowledging their sponsorship of the membership.
4. A 14 day discussion period for comments from wayland-protocols members will
   be held.
5. At the conclusion of the discussion period, the new membership is established
   unless their application was NACKed by a 1/2 majority of all existing members.

### 1.3. Ceasing membership

1. A member may step down by writing their intention to do so to the
   wayland-devel mailing list.
2. A removal vote may be called for by an existing member by posting to the
   wayland-devel mailing list. This begins a 14 day voting & discussion
   period.
3. At the conclusion of the voting period, the member is removed if the votes
   total 2/3rds of all current members.
4. Removed members are not eligible to apply for membership again for a period
   of 1 year.
5. Following a failed vote, the member who called for the vote cannot
   call for a re-vote or propose any other removal for 90 days.

## 2. Protocols

### 2.1. Protocol namespaces

1. Namespaces are implemented in practice by prefixing each interface name in a
   protocol definition (XML) with the namespace name, and an underscore (e.g.
   "xdg_wm_base").
2. Protocols in a namespace may optionally use the namespace followed by a dash
   in the name (e.g. "xdg-shell").
3. The "xdg" namespace is established for protocols letting clients
   configure its surfaces as "windows", allowing clients to affect how they
   are managed.
4. The "wp" namespace is established for protocols generally useful to Wayland
   implementations (i.e. "plumbing" protocols).
5. The "ext" namespace is established as a general catch-all for protocols that
   fit into no other namespace.

### 2.2. Protocol inclusion requirements

1. All protocols found in the "xdg" and "wp" namespaces at the time of writing
   are grandfathered into their respective namespace without further discussion.
2. Protocols in the "xdg" and "wp" namespace are eligible for inclusion only if
   ACKed by at least 3 members.
3. Protocols in the "xdg" and "wp" namespace are ineligible for inclusion if
   if NACKed by any member.
4. Protocols in the "xdg" and "wp" namespaces must have at least 3 open-source
   implementations (either 1 client + 2 servers, or 2 clients + 1 server) to be
   eligible for inclusion.
5. Protocols in the "ext" namespace are eligible for inclusion only if ACKed by
   at least one other member.
6. Protocols in the "ext" namespace must have at least one open-source client &
   one open-source server implementation to be eligible for inclusion.
7. "Open-source" is defined as distributed with an Open Source Initiative
   approved license.

### 2.3. Introducing new protocols

1. A new protocol may be proposed by submitting a merge request to the
   wayland-protocols Gitlab repository.
2. Protocol proposal posts must include justification for their inclusion in
   their namespace per the requirements outlined in section 2.2.
3. An indefinite discussion period for comments from wayland-protocols members
   will be held, with a minimum duration of 30 days. Protocols which require a
   certain level of implementation status, ACKs from members, and so on, should
   use this time to acquire them.
4. When the proposed protocol meets all requirements for inclusion per section
   2.2, and the minimum discussion period has elapsed, the sponsoring member may
   merge their changes in the wayland-protocol repository.
5. Amendments to existing protocols may be proposed by the same process, with
   no minimum discussion period.
6. Declaring a protocol stable may be proposed by the same process, with the
   regular 30 day minimum discussion period.

## 3. Protocol adoption documentation

### 3.1. Adoption website

1. This section is informational.
2. A website will be made available for interested parties to browse the
   implementation status of protocols included in wayland-protocols.
3. A statement from each member of wayland-protocols will be included on the
   site.
4. Each protocol will be listed along with its approval status from each member.
5. The approval statuses are:
   1. NACK, or "negative acknowledgement", meaning that the member is opposed to
      the protocol in principle.
   2. NOPP, or "no opposition", meaning that the member is not opposed to the
      protocol in principle, but does not provide an implementation.
   3. ACK, or "acknowledged", meaning that the member supports the protocol in
      principle, but does not provide an implementation.
   4. IMPL, or "implemented", meaning that the member supports the protocol and
      provides an implementation.
6. Each member may write a short statement expanding on the rationale for their
   approval status, which will be included on the site.
7. A supplementary list of implementations will also be provided on the site,
   which may include implementations supported by non-members.

### 3.2. Changes to the adoption website

1. This section is informational.
2. A new protocol is added to the website by the sponsoring member at the
   conclusion of the discussion period (section 2.3.3).
3. During the discussion period (section 2.3.3), interested parties may express
   their approval status on the Gitlab merge request. The default approval
   status for members who do not participate in the discussion is "NOPP".
4. Members may change their acknowledgement status or support statement at any
   time after the discussion period (section 2.3.3) has closed by simply merging
   their update in the wayland-protocols repository.

## 4. Amending this document

1. An amendment to this document may be proposed any member by
   submitting a merge request on Gitlab.
2. A 30 day discussion period for comments from wayland-protocols members will
   be held.
3. At the conclusion of the discussion period, an amendment will become
   effective if it's ACKed by at least 2/3rds of all wayland-protocols members,
   and NACKed by none. The sponsoring member may merge their change to the
   wayland-protocols repository at this point.
