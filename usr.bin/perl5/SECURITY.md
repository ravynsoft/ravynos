# Security Policy

Perl's vulnerability handling policies are described fully in
[perlsecpolicy]

## Reporting a Vulnerability

If you believe you have found a security vulnerability in the Perl
interpreter or modules maintained in the core Perl codebase, email
the details to perl-security@perl.org. This address is a closed
membership mailing list monitored by the Perl security team.

You should receive an initial response to your report within 72 hours.
If you do not receive a response in that time, please contact
the [Perl Steering Council](mailto:steering-council@perl.org).

When members of the security team reply to your messages, they will
generally include the perl-security@perl.org address in the "To" or "CC"
fields of the response. This allows all of the security team to follow
the discussion and chime in as needed. Use the "Reply-all" functionality
of your email client when you send subsequent responses so that the
entire security team receives the message.

The security team will evaluate your report and make an initial
determination of whether it is likely to fit the scope of issues the
team handles. General guidelines about how this is determined are
detailed in the ["WHAT ARE SECURITY ISSUES"] section of [perlsecpolicy].

If your report meets the team's criteria, an issue will be opened in the
team's private issue tracker and you will be provided the issue's ID number.
Issue identifiers have the form perl-security#NNN. Include this identifier
with any subsequent messages you send.

The security team will send periodic updates about the status of your
issue and guide you through any further action that is required to complete
the vulnerability remediation process. The stages vulnerabilities typically
go through are explained in the ["HOW WE DEAL WITH SECURITY ISSUES"]
section of [perlsecpolicy].

[perlsecpolicy]: pod/perlsecpolicy.pod
["WHAT ARE SECURITY ISSUES"]: pod/perlsecpolicy.pod#what-are-security-issues
["HOW WE DEAL WITH SECURITY ISSUES"]: pod/perlsecpolicy.pod#how-we-deal-with-security-issues
