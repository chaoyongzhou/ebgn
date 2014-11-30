/******************************************************************************
*
* Copyright (C) Chaoyong Zhou
* Email: bgnvendor@gmail.com 
* QQ: 312230917
*
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif/*__cplusplus*/

extern int main_crfsc(int argc, char **argv);
extern int main_chfs(int argc, char **argv);
extern int main_crfs(int argc, char **argv);
extern int main_cdfs(int argc, char **argv);
extern int main_cbgt(int argc, char **argv);
extern int main_exec(int argc, char **argv);
extern int main_cextsrv(int argc, char **argv);
extern int main_csrv(int argc, char **argv);
extern int main_trans(int argc, char **argv);
extern int main_udp(int argc, char **argv);
extern int main_csession(int argc, char **argv);
extern int main_cscore(int argc, char **argv);
extern int main_ict(int argc, char **argv);
extern int main_default(int argc, char **argv);

int main(int argc, char **argv)
{
    //main_default(argc, argv);
    main_crfsc(argc, argv);
    //main_chfs(argc, argv);
    //main_crfs(argc, argv);
    //main_cdfs(argc, argv);
    //main_cbgt(argc, argv);
    //main_exec(argc, argv);
    //main_cextsrv(argc, argv);
    //main_csrv(argc, argv);
    //main_csession(argc, argv);
    //main_cscore(argc, argv);
    //main_trans(argc, argv);
    //main_udp(argc, argv);
    //main_ict(argc, argv);
    return (0);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

