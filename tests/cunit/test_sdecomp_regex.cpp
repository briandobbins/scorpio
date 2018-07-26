#include <vector>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cassert>
#include "pio_sdecomps_regex.hpp"
extern "C"{
#include <pio_tests.h>
}

#define LOG_RANK0(rank, ...)                     \
            do{                                   \
                if(rank == 0)                     \
                {                                 \
                    fprintf(stderr, __VA_ARGS__); \
                }                                 \
            }while(0);

static const int FAIL = -1;

/* Test creatng the regular expression class */
int test_create_sdecomp_regex(void )
{
    PIO_Util::PIO_save_decomp_regex test_regex("*");

    return PIO_NOERR;
}

/* Test creatng a match all regular expression */
int test_matchall_regex(int wrank)
{
    int ret = PIO_NOERR;
    PIO_Util::PIO_save_decomp_regex test_regex("*");
    std::vector<int> ioids = {-2, -1, 0, 1, 2, 3, 4, 99, 100, 1024, 4096};
    std::vector<std::string> vnames = {
                                          std::string("test_var1"),
                                          std::string("test_var2")
                                      };
    std::vector<std::string> fnames = {
                                          std::string("test_file1"),
                                          std::string("test_file2")
                                      };

    for(std::vector<int>::const_iterator id_iter = ioids.cbegin();
        id_iter != ioids.cend(); ++id_iter){
      for(std::vector<std::string>::const_iterator var_iter = vnames.cbegin();
          var_iter != vnames.cend(); ++var_iter){
        for(std::vector<std::string>::const_iterator file_iter = fnames.cbegin();
          file_iter != fnames.cend(); ++file_iter){
          bool is_match = test_regex.matches(*id_iter, *file_iter, *var_iter);
          if(!is_match){
            LOG_RANK0(wrank, "test_matchall_regex failed for : ioid=%d, fname=%s, vname=%s\n", *id_iter, (*file_iter).c_str(), (*var_iter).c_str());
            ret = PIO_EINTERNAL;
            break;
          }
        }
      }
    }

    return ret;
}

/* Test creatng a regular expression to match ids */
int test_idmatch_regex(int wrank)
{
    int ret = PIO_NOERR;
    const std::string ID_REG_PREFIX("ID=");
    const std::string QUOTE("\"");
    const std::string LEFT_BRACKET("(");
    const std::string RIGHT_BRACKET(")");
    const int MATCH_ID = 99;
    PIO_Util::PIO_save_decomp_regex 
      test_regex(ID_REG_PREFIX +
        QUOTE +
        std::to_string(MATCH_ID) +
        QUOTE);
    std::vector<int> ioids = {-2, -1, 0, 1, 2, 3, 4, MATCH_ID, 100, 1024, 4096};
    std::vector<std::string> vnames = {
                                          std::string("test_var1"),
                                          std::string("test_var2")
                                      };
    std::vector<std::string> fnames = {
                                          std::string("test_file1"),
                                          std::string("test_file2")
                                      };

    for(std::vector<int>::const_iterator id_iter = ioids.cbegin();
        id_iter != ioids.cend(); ++id_iter){
      for(std::vector<std::string>::const_iterator var_iter = vnames.cbegin();
          var_iter != vnames.cend(); ++var_iter){
        for(std::vector<std::string>::const_iterator file_iter = fnames.cbegin();
          file_iter != fnames.cend(); ++file_iter){
          bool is_match = test_regex.matches(*id_iter, *file_iter, *var_iter);
          if( ((!is_match) && (*id_iter == MATCH_ID)) ||
              (is_match && (*id_iter != MATCH_ID)) ){
            LOG_RANK0(wrank, "test_idmatch_regex failed for : ioid=%d, fname=%s, vname=%s\n", *id_iter, (*file_iter).c_str(), (*var_iter).c_str());
            ret = PIO_EINTERNAL;
            break;
          }
        }
      }
    }

    return ret;
}

int test_driver(MPI_Comm comm, int wrank, int wsz, int *num_errors)
{
    int nerrs = 0, ret = PIO_NOERR;
    assert((comm != MPI_COMM_NULL) && (wrank >= 0) && (wsz > 0) && num_errors);
    
    /* Test creating a regular expression */
    try{
      ret = test_create_sdecomp_regex();
    }
    catch(...){
      ret = PIO_EINTERNAL;
    }
    if(ret != PIO_NOERR)
    {
        LOG_RANK0(wrank, "test_create_sdecomp_regex() FAILED, ret = %d\n", ret);
        nerrs++;
    }
    else{
        LOG_RANK0(wrank, "test_create_sdecomp_regex() PASSED\n");
    }

    /* Test creating a simple, match all, regular expression */
    try{
      ret = test_matchall_regex(wrank);
    }
    catch(...){
      ret = PIO_EINTERNAL;
    }
    if(ret != PIO_NOERR)
    {
        LOG_RANK0(wrank, "test_matchall_regex() FAILED, ret = %d\n", ret);
        nerrs++;
    }
    else{
        LOG_RANK0(wrank, "test_matchall_regex() PASSED\n");
    }

    /* Test creating a simple regular expression to match a specific ioid */
    try{
      ret = test_idmatch_regex(wrank);
    }
    catch(...){
      ret = PIO_EINTERNAL;
    }
    if(ret != PIO_NOERR)
    {
        LOG_RANK0(wrank, "test_idmatch_regex() FAILED, ret = %d\n", ret);
        nerrs++;
    }
    else{
        LOG_RANK0(wrank, "test_idmatch_regex() PASSED\n");
    }
    *num_errors += nerrs;
    return nerrs;
}

int main(int argc, char *argv[])
{
    int ret;
    int wrank, wsz;
    int num_errors;
#ifdef TIMING
#ifndef TIMING_INTERNAL
    ret = GPTLinitialize();
    if(ret != 0)
    {
        LOG_RANK0(wrank, "GPTLinitialize() FAILED, ret = %d\n", ret);
        return ret;
    }
#endif /* TIMING_INTERNAL */
#endif /* TIMING */

    ret = MPI_Init(&argc, &argv);
    if(ret != MPI_SUCCESS)
    {
        LOG_RANK0(wrank, "MPI_Init() FAILED, ret = %d\n", ret);
        return ret;
    }

    ret = MPI_Comm_rank(MPI_COMM_WORLD, &wrank);
    if(ret != MPI_SUCCESS)
    {
        LOG_RANK0(wrank, "MPI_Comm_rank() FAILED, ret = %d\n", ret);
        return ret;
    }
    ret = MPI_Comm_size(MPI_COMM_WORLD, &wsz);
    if(ret != MPI_SUCCESS)
    {
        LOG_RANK0(wrank, "MPI_Comm_rank() FAILED, ret = %d\n", ret);
        return ret;
    }

    num_errors = 0;
    ret = test_driver(MPI_COMM_WORLD, wrank, wsz, &num_errors);
    if(ret != 0)
    {
        LOG_RANK0(wrank, "Test driver FAILED\n");
        return FAIL;
    }
    else{
        LOG_RANK0(wrank, "All tests PASSED\n");
    }

    MPI_Finalize();

#ifdef TIMING
#ifndef TIMING_INTERNAL
    ret = GPTLfinalize();
    if(ret != 0)
    {
        LOG_RANK0(wrank, "GPTLinitialize() FAILED, ret = %d\n", ret);
        return ret;
    }
#endif /* TIMING_INTERNAL */
#endif /* TIMING */

    if(num_errors != 0)
    {
        LOG_RANK0(wrank, "Total errors = %d\n", num_errors);
        return FAIL;
    }
    return 0;
}
