// CDP   void fn{t2, opc1, opc2}(crd, crn, crm)
DEF_HELPER_2(beco_cdp, void, env, i32)

// MCR   void fn{t2, opc1, opc2}(crn, crm), rt
DEF_HELPER_3(beco_mcr, void, env, i32, i32)

// MRC   rt  fn{t2, opc1, opc2}(crn, crm), rt
DEF_HELPER_2(beco_mrc, i32, env, i32)

// MCRR   void  fn{t2, opc1}(crm), rt, rt2
DEF_HELPER_4(beco_mcrr, void, env, i32, i32, i32)

// MRRC   {rt, rt2}  fn{t2, opc1}(crm)
DEF_HELPER_2(beco_mrrc, i64, env, i32)

