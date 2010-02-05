//////////////////////////////////////////////////////////////////
// (c) Copyright 2010- by Jeongnim Kim
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//   Jeongnim Kim
//   National Center for Supercomputing Applications &
//   Materials Computation Center
//   University of Illinois, Urbana-Champaign
//   Urbana, IL 61801
//   e-mail: jnkim@ncsa.uiuc.edu
//
// Supported by 
//   National Center for Supercomputing Applications, UIUC
//   Materials Computation Center, UIUC
//////////////////////////////////////////////////////////////////
#include <Configuration.h>
#include <io/hdf_archive.h>
namespace qmcplusplus
{
  //const hid_t hdf_archive::is_closed;

  hdf_archive::hdf_archive(Communicate* c, bool use_collective)
    : file_id(is_closed), access_id(H5P_DEFAULT), xfer_plist(H5P_DEFAULT),myComm(c)
  {
    H5E_auto_t func;
    void *client_data;
    H5Eget_auto (&func, &client_data);
    H5Eset_auto (NULL, NULL);
    set_access_plist(use_collective);
  }

  hdf_archive::~hdf_archive() 
  { 
    close();
    if(xfer_plist != H5P_DEFAULT) H5Pclose(xfer_plist);
    if(access_id!= H5P_DEFAULT) H5Pclose(access_id);
    H5E_auto_t func;
    void *client_data;
    H5Eset_auto (func, client_data);
  }

  void hdf_archive::set_access_plist(bool use_collective)
  {
    access_id=H5P_DEFAULT;
#if defined(H5_HAVE_PARALLEL) && defined(ENABLE_PHDF5)
    if(use_collective && myComm && myComm->size()>1)
    {
      MPI_Info info=MPI_INFO_NULL;
      access_id = H5Pcreate(H5P_FILE_ACCESS);
      H5Pset_fapl_mpio(access_id,myComm->getMPI(),info);
      xfer_plist = H5Pcreate(H5P_DATASET_XFER);
      H5Pset_dxpl_mpio(xfer_plist,H5FD_MPIO_COLLECTIVE);
    }
    else
      use_collective=false;
#endif
    Mode.set(IS_PARALLEL,use_collective);
    //true, if this task does not need to participate in I/O
    Mode.set(NOIO,myComm->rank()&&!use_collective);
  }

  bool hdf_archive::create(const std::string& fname, unsigned flags)
  {
    if(Mode[NOIO]) return true;
    close();
    file_id = H5Fcreate(fname.c_str(),flags,H5P_DEFAULT,access_id);
    return file_id != is_closed;
  }

  bool hdf_archive::open(const std::string& fname,unsigned flags)
  {
    if(Mode[NOIO]) return true;
    close();
    file_id = H5Fopen(fname.c_str(),flags,access_id);
    if(file_id==is_closed)
      file_id = H5Fcreate(fname.c_str(),flags,H5P_DEFAULT,access_id);
    return file_id != is_closed;
  }

  bool hdf_archive::is_group(const std::string& aname)
  {
    if(Mode[NOIO]) return true;
    if(file_id==is_closed) return false;
    hid_t p=group_id.empty()? file_id:group_id.top();
    p=(aname[0]=='/')?file_id:p;
    hid_t g=H5Gopen(p,aname.c_str());
    if(g<0) return false;
    H5Gclose(g);
    return true;
  }

  hid_t hdf_archive::push(const std::string& gname, bool createit)
  {
    if(Mode[NOIO]||file_id==is_closed) return is_closed;
    hid_t p=group_id.empty()? file_id:group_id.top();
    hid_t g=H5Gopen(p,gname.c_str());
    if(g<0 && createit) g= H5Gcreate(p,gname.c_str(),0);
    if(g!=is_closed) group_id.push(g);
    return g;
  }

}
/***************************************************************************
 * $RCSfile$   $Author: jnkim $
 * $Revision: 894 $   $Date: 2006-02-03 10:52:38 -0600 (Fri, 03 Feb 2006) $
 * $Id: hdf_file.cpp 894 2006-02-03 16:52:38Z jnkim $ 
 ***************************************************************************/