struct re_dfa_t {
  const int *sb_char;
};
struct re_dfa_t *xregcomp (void);
struct re_dfa_t *rpl_regcomp (void);
void rpl_regfree (struct re_dfa_t *);
