//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2014-2019, Lawrence Livermore National Security, LLC.
// 
// Produced at the Lawrence Livermore National Laboratory
// 
// LLNL-CODE-666778
// 
// All rights reserved.
// 
// This file is part of Conduit. 
// 
// For details, see: http://software.llnl.gov/conduit/.
// 
// Please also read conduit/LICENSE
// 
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
// 
// * Redistributions of source code must retain the above copyright notice, 
//   this list of conditions and the disclaimer below.
// 
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the disclaimer (as noted below) in the
//   documentation and/or other materials provided with the distribution.
// 
// * Neither the name of the LLNS/LLNL nor the names of its contributors may
//   be used to endorse or promote products derived from this software without
//   specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL LAWRENCE LIVERMORE NATIONAL SECURITY,
// LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
// DAMAGES  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
// IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGE.
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//-----------------------------------------------------------------------------
///
/// file: conduit_relay_io_handle.cpp
///
//-----------------------------------------------------------------------------

#ifdef CONDUIT_RELAY_IO_MPI_ENABLED
    // FIXME:
    #include "conduit_relay_io_handle.hpp"
#else
    #include "conduit_relay_io_handle.hpp"
#endif

#include "conduit_relay_io.hpp"

#ifdef CONDUIT_RELAY_IO_HDF5_ENABLED
    #include "conduit_relay_io_hdf5.hpp"
#endif


//-----------------------------------------------------------------------------
// standard lib includes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// -- begin conduit:: --
//-----------------------------------------------------------------------------
namespace conduit
{

//-----------------------------------------------------------------------------
// -- begin conduit::relay --
//-----------------------------------------------------------------------------
namespace relay
{

#ifdef CONDUIT_RELAY_IO_MPI_ENABLED
//-----------------------------------------------------------------------------
// -- begin conduit::relay::mpi --
//-----------------------------------------------------------------------------
namespace mpi
{
#endif

//-----------------------------------------------------------------------------
// -- begin conduit::relay::<mpi>::io --
//-----------------------------------------------------------------------------
namespace io
{


//-----------------------------------------------------------------------------
// HandleInterface -- base class for all concrete IO Handle Implementations
//-----------------------------------------------------------------------------
class IOHandle::HandleInterface
{
public:
    
    HandleInterface(const std::string &path,
                    const std::string &protocol,
                    const Node &options);
    virtual ~HandleInterface();

    // main interface methods

    // note: make sure to call HandleInterface::open in derived class 
    //       open() overrides
    virtual void open();
    virtual bool is_open() const = 0;
    virtual void read(Node &node) = 0;
    virtual void read(const std::string &path,
                      Node &node) = 0;
    virtual void write(const Node &node) = 0;
    virtual void write(const Node &node,
               const std::string &path) = 0;
    virtual void list_child_names(std::vector<std::string> &res) const = 0;
    virtual void list_child_names(const std::string &path,
                          std::vector<std::string> &res) const = 0;
    virtual void remove(const std::string &path) = 0;
    virtual bool has_path(const std::string &path) const = 0;
    virtual void close() = 0;

    // factory helper methods used by interface class 
    static HandleInterface *create(const std::string &path);

    static HandleInterface *create(const std::string &path,
                                   const std::string &protocol);

    static HandleInterface *create(const std::string &path,
                                   const std::string &protocol,
                                   const Node &options);
protected:
    // access to common state
    const std::string &path();
    const std::string &protocol();
    const Node        &options();

private:

    std::string m_path;
    std::string m_protocol;
    Node        m_options;
};


//-----------------------------------------------------------------------------
// BasicHandle -- IO Handle implementation for built-in protocols
//-----------------------------------------------------------------------------
class BasicHandle: public IOHandle::HandleInterface
{
public:
    BasicHandle(const std::string &path,
                const std::string &protocol,
                const Node &options);
    virtual ~BasicHandle();

    void open();
    
    bool is_open() const;
    
    // main interface methods
    void read(Node &node);
    void read(const std::string &path,
              Node &node);

    void write(const Node &node);
    void write(const Node &node,
               const std::string &path);

    void remove(const std::string &path);

    void list_child_names(std::vector<std::string> &res) const;
    void list_child_names(const std::string &path,
                          std::vector<std::string> &res) const;

    bool has_path(const std::string &path) const;
    
    void close();
    
private:
    Node m_node;
    bool m_open;

};


//-----------------------------------------------------------------------------
// HDF5Handle -- IO Handle implementation for HDF5
//-----------------------------------------------------------------------------
#ifdef CONDUIT_RELAY_IO_HDF5_ENABLED
//-----------------------------------------------------------------------------
class HDF5Handle: public IOHandle::HandleInterface
{
public:
    HDF5Handle(const std::string &path,
               const std::string &protocol,
               const Node &options);
    virtual ~HDF5Handle();

    void open();

    bool is_open() const;

    // main interface methods
    void read(Node &node);
    void read(const std::string &path,
              Node &node);

    void write(const Node &node);
    void write(const Node &node,
               const std::string &path);

    void remove(const std::string &path);

    void list_child_names(std::vector<std::string> &res) const;
    void list_child_names(const std::string &path,
                          std::vector<std::string> &res) const;

    bool has_path(const std::string &path) const;
    
    void close();
    
private:
    hid_t m_h5_id;
    
};
//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
    

//-----------------------------------------------------------------------------
// HandleInterface Implementation 
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
IOHandle::HandleInterface::HandleInterface(const std::string &path,
                                           const std::string &protocol,
                                           const Node &options)
: m_path(path),
  m_protocol(protocol),
  m_options(options)
{
    // empty
}

//-----------------------------------------------------------------------------
IOHandle::HandleInterface::~HandleInterface()
{
    // empty
}


//-----------------------------------------------------------------------------
void
IOHandle::HandleInterface::open()
{
    // checks for subpaths, which we don't currently support 
    
    std::string file_path;
    std::string subpath;

    // check for ":" split
    conduit::utils::split_file_path(path(),
                                    std::string(":"),
                                    file_path,
                                    subpath);
    if( !subpath.empty() )
    {
        CONDUIT_ERROR("IOHandle does not (yet) support opening paths with "
                      "subpaths specified: \"" << path() << "\"");
    }
}


//-----------------------------------------------------------------------------
IOHandle::HandleInterface *
IOHandle::HandleInterface::create(const std::string &path)
{
    std::string protocol;
    Node options;
    return create(path,protocol,options);
}

//-----------------------------------------------------------------------------
IOHandle::HandleInterface *
IOHandle::HandleInterface::create(const std::string &path,
                                  const std::string &protocol)
{
    Node options;
    return create(path,protocol,options);
}

//-----------------------------------------------------------------------------
IOHandle::HandleInterface *
IOHandle::HandleInterface::create(const std::string &path,
                                  const std::string &protocol_,
                                  const Node &options)
{
    HandleInterface *res = NULL;
    std::string protocol = protocol_;
    
    // allow empty protocol to be used for auto detect
    if(protocol.empty())
    {
        conduit::relay::io::identify_protocol(path,protocol);
    }

    if(protocol == "conduit_bin" ||
       protocol == "json" ||
       protocol == "conduit_json" ||
       protocol == "conduit_base64_json" ||
       protocol == "yaml" )
    {
        res = new BasicHandle(path, protocol, options);
    }
    else if( protocol == "hdf5" )
    {
    #ifdef CONDUIT_RELAY_IO_HDF5_ENABLED
        res = new HDF5Handle(path, protocol, options);
    #else
        CONDUIT_ERROR("conduit_relay lacks HDF5 support: " << 
                      "Cannot create Relay I/O Handle for HDF5" << path);
    #endif
    }
    else
    {
        CONDUIT_ERROR("Relay I/O Handle does not support the protocol: " 
                      << protocol);
    }
    return res;
}

//-----------------------------------------------------------------------------
const std::string &
IOHandle::HandleInterface::path()
{
    return m_path;
}

//-----------------------------------------------------------------------------
const std::string &
IOHandle::HandleInterface::protocol()
{
    return m_protocol;
}

//-----------------------------------------------------------------------------
const Node &
IOHandle::HandleInterface::options()
{
    return m_options;
}


//-----------------------------------------------------------------------------
// BasicHandle Implementation 
//-----------------------------------------------------------------------------
BasicHandle::BasicHandle(const std::string &path,
                         const std::string &protocol,
                         const Node &options)
: HandleInterface(path,protocol,options),
  m_node(),
  m_open(false)
{
    // empty
}
//-----------------------------------------------------------------------------
BasicHandle::~BasicHandle()
{
    close();
}

//-----------------------------------------------------------------------------
void 
BasicHandle::open()
{
    close();
    // call base class method, which does final sanity checks
    HandleInterface::open();

    // read from file if it already exists, other wise
    // we start out with a blank slate
    if( utils::is_file( path() ) )
    {
        // read from file 
        io::load(path(),
                 protocol(),
                 options(),
                 m_node);
    }
    else
    {
        // make sure we can actually write to this location
        // we don't want to fail on close if the path 
        // is bogus
        relay::io::save(m_node, path());
    }

    m_open = true;
}

//-----------------------------------------------------------------------------
bool
BasicHandle::is_open() const
{
    return m_open;
}

//-----------------------------------------------------------------------------
void 
BasicHandle::read(Node &node)
{
    node.update(m_node);
}

//-----------------------------------------------------------------------------
void 
BasicHandle::read(const std::string &path,
                  Node &node)
{
    if(m_node.has_path(path))
    {
        node.update(m_node[path]);
    }
}

//-----------------------------------------------------------------------------
void 
BasicHandle::write(const Node &node)
{
    m_node.update(node);
}


//-----------------------------------------------------------------------------
void 
BasicHandle::write(const Node &node,
                   const std::string &path)
{
    m_node[path].update(node);
}

//-----------------------------------------------------------------------------
void
BasicHandle::list_child_names(std::vector<std::string> &res) const
{
    res = m_node.child_names();
}

//-----------------------------------------------------------------------------
void
BasicHandle::list_child_names(const std::string &path,
                              std::vector<std::string> &res) const
{
    res.clear();
    if(m_node.has_path(path))
        res = m_node[path].child_names();
}

//-----------------------------------------------------------------------------
void 
BasicHandle::remove(const std::string &path)
{
    m_node.remove(path);
}

//-----------------------------------------------------------------------------
bool 
BasicHandle::has_path(const std::string &path) const
{
    return m_node.has_path(path);
}

//-----------------------------------------------------------------------------
void 
BasicHandle::close()
{
    if(m_open)
    {
        // here is where it actually gets realized on disk
        io::save(m_node,
                 path(),
                 protocol(),
                 options());
        m_node.reset();
        m_open = false;
    }
}


//-----------------------------------------------------------------------------
// HDF5Handle Implementation 
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#ifdef CONDUIT_RELAY_IO_HDF5_ENABLED
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
HDF5Handle::HDF5Handle(const std::string &path,
                       const std::string &protocol,
                       const Node &options)
: HandleInterface(path,protocol,options),
  m_h5_id(-1)
{
    // empty
}
//-----------------------------------------------------------------------------
HDF5Handle::~HDF5Handle()
{
    close();
}

//-----------------------------------------------------------------------------
void 
HDF5Handle::open()
{
    close();

    // call base class method, which does final sanity checks
    HandleInterface::open();
    
    if( !utils::is_file( path() ) )
    {
        m_h5_id = hdf5_create_file( path() );
    }
    else
    {
        m_h5_id = hdf5_open_file_for_read_write( path() );
    }
}


//-----------------------------------------------------------------------------
bool
HDF5Handle::is_open() const
{
    return m_h5_id != -1;
}

//-----------------------------------------------------------------------------
void 
HDF5Handle::read(Node &node)
{
    hdf5_read(m_h5_id,node);
}

//-----------------------------------------------------------------------------
void 
HDF5Handle::read(const std::string &path,
                 Node &node)
{
    hdf5_read(m_h5_id,path,node);
}

//-----------------------------------------------------------------------------
void 
HDF5Handle::write(const Node &node)
{
    // Options Push / Pop (only needed for write, since hdf5 only supports
    // write options
    Node prev_options;
    if(options().has_child("hdf5"))
    {
        hdf5_options(prev_options);
        hdf5_set_options(options()["hdf5"]);
    }

    hdf5_write(node,m_h5_id);

    if(!prev_options.dtype().is_empty())
    {
        hdf5_set_options(prev_options);
    }
}


//-----------------------------------------------------------------------------
void 
HDF5Handle::write(const Node &node,
                  const std::string &path)
{
    // Options Push / Pop (only needed for write, since hdf5 only supports
    // write options
    Node prev_options;
    if(options().has_child("hdf5"))
    {
        hdf5_options(prev_options);
        hdf5_set_options(options()["hdf5"]);
    }

    hdf5_write(node,m_h5_id,path);
    
    if(!prev_options.dtype().is_empty())
    {
        hdf5_set_options(prev_options);
    }
}

//-----------------------------------------------------------------------------
void
HDF5Handle::list_child_names(std::vector<std::string> &res) const
{
    hdf5_group_list_child_names(m_h5_id, "/", res);
}

//-----------------------------------------------------------------------------
void
HDF5Handle::list_child_names(const std::string &path,
                             std::vector<std::string> &res) const
{
    hdf5_group_list_child_names(m_h5_id, path, res);
}

//-----------------------------------------------------------------------------
void 
HDF5Handle::remove(const std::string &path)
{
    hdf5_remove_path(m_h5_id,path);
}

//-----------------------------------------------------------------------------
bool 
HDF5Handle::has_path(const std::string &path) const
{
    return hdf5_has_path(m_h5_id,path);
}


//-----------------------------------------------------------------------------
void 
HDF5Handle::close()
{
    if(m_h5_id >= 0)
    {
        hdf5_close_file(m_h5_id);
    }
    m_h5_id = -1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------




//-----------------------------------------------------------------------------
// IOHandle Implementation 
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
IOHandle::IOHandle()
: m_handle(NULL)
{

}

//-----------------------------------------------------------------------------
IOHandle::~IOHandle()
{
    close();
}

//-----------------------------------------------------------------------------
void
IOHandle::open(const std::string &path)
{
    close();
    m_handle = HandleInterface::create(path);
    if(m_handle != NULL)
    {
        m_handle->open();
    }
}

//-----------------------------------------------------------------------------
void
IOHandle::open(const std::string &path,
                    const std::string &protocol)
{
    close();
    m_handle = HandleInterface::create(path, protocol);
    if(m_handle != NULL)
    {
        m_handle->open();
    }
}

//-----------------------------------------------------------------------------
void
IOHandle::open(const std::string &path,
               const std::string &protocol,
               const Node &options)
{
    close();
    m_handle = HandleInterface::create(path, protocol, options);
    if(m_handle != NULL)
    {
        m_handle->open();
    }
}

//-----------------------------------------------------------------------------
bool
IOHandle::is_open() const
{
    bool res = false;

    if(m_handle != NULL)
    {
        res = m_handle->is_open();
    }

    return res;
}

//-----------------------------------------------------------------------------
void
IOHandle::read(Node &node)
{    
    if(m_handle != NULL)
    {
        m_handle->read(node);
    }
    else
    {
        CONDUIT_ERROR("Invalid or closed handle.");
    }
}

//-----------------------------------------------------------------------------
void
IOHandle::read(const std::string &path,
               Node &node)
{
    if(m_handle != NULL)
    {
        m_handle->read(path, node);
    }
    else
    {
        CONDUIT_ERROR("Invalid or closed handle.");
    }
}

//-----------------------------------------------------------------------------
void
IOHandle::write(const Node &node)
{
    if(m_handle != NULL)
    {
        m_handle->write(node);
    }
    else
    {
        CONDUIT_ERROR("Invalid or closed handle.");
    }
}

//-----------------------------------------------------------------------------
void
IOHandle::write(const Node &node,
                const std::string &path)
{
    if(m_handle != NULL)
    {
        m_handle->write(node, path);
    }
    else
    {
        CONDUIT_ERROR("Invalid or closed handle.");
    }
    
}

//-----------------------------------------------------------------------------
void
IOHandle::remove(const std::string &path)
{
    if(m_handle != NULL)
    {
        m_handle->remove(path);
    }
    else
    {
        CONDUIT_ERROR("Invalid or closed handle.");
    }
}

//-----------------------------------------------------------------------------
void
IOHandle::list_child_names(std::vector<std::string> &names) const
{
    names.clear();
    if(m_handle != NULL)
    {
        return m_handle->list_child_names(names);
    }
    else
    {
        CONDUIT_ERROR("Invalid or closed handle.");
    }
}


//-----------------------------------------------------------------------------
void
IOHandle::list_child_names(const std::string &path,
                           std::vector<std::string> &names) const
{
    names.clear();
    if(m_handle != NULL)
    {
        return m_handle->list_child_names(path, names);
    }
    else
    {
        CONDUIT_ERROR("Invalid or closed handle.");
    }
}

//-----------------------------------------------------------------------------
bool
IOHandle::has_path(const std::string &path) const
{
    if(m_handle != NULL)
    {
        return m_handle->has_path(path);
    }
    else
    {
        CONDUIT_ERROR("Invalid or closed handle.");
    }
    
    return false;
}

//-----------------------------------------------------------------------------
void
IOHandle::close()
{
    if(m_handle != NULL)
    {
        m_handle->close();
        delete m_handle;
        m_handle = NULL;
    }
    // else, ignore ... 
}


}
//-----------------------------------------------------------------------------
// -- end conduit::relay::<mpi>::io --
//-----------------------------------------------------------------------------

#ifdef CONDUIT_RELAY_IO_MPI_ENABLED
}
//-----------------------------------------------------------------------------
// -- begin conduit::relay::mpi --
//-----------------------------------------------------------------------------
#endif

}
//-----------------------------------------------------------------------------
// -- end conduit::relay --
//-----------------------------------------------------------------------------

}
//-----------------------------------------------------------------------------
// -- end conduit:: --
//-----------------------------------------------------------------------------
